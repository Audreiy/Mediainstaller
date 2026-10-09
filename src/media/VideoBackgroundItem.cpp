#include "VideoBackgroundItem.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <propvarutil.h>

#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QDebug>

VideoBackgroundItem::VideoBackgroundItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    setOpaquePainting(true);
    setRenderTarget(QQuickPaintedItem::FramebufferObject);
    m_timer.setTimerType(Qt::CoarseTimer);
    connect(&m_timer, &QTimer::timeout, this, &VideoBackgroundItem::onTimerTick);
}

VideoBackgroundItem::~VideoBackgroundItem() {
    closeMedia();
    if (m_mfInitialized) {
        MFShutdown();
        m_mfInitialized = false;
    }
}

void VideoBackgroundItem::setSource(const QString &source) {
    if (m_source == source) return;
    m_source = source;
    emit sourceChanged();

    openMedia();
}

void VideoBackgroundItem::setPlaying(bool play) {
    if (m_playing == play) return;
    m_playing = play;
    emit playingChanged();

    if (m_playing && m_pReader) {
        m_timer.start(m_frameIntervalMs);
    } else {
        m_timer.stop();
    }
}

void VideoBackgroundItem::setTargetFps(int fps) {
    int clamped = qBound(10, fps, 60);
    if (m_targetFps == clamped) return;
    m_targetFps = clamped;
    emit targetFpsChanged();

    m_frameIntervalMs = qMax(1, qRound(1000.0 / m_targetFps));
    if (m_playing && m_pReader) {
        m_timer.start(m_frameIntervalMs);
    }
}

void VideoBackgroundItem::openMedia() {
    closeMedia();

    if (m_source.isEmpty()) return;

    if (!m_mfInitialized) {
        HRESULT hrMf = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
        if (FAILED(hrMf)) {
            qWarning() << "[VideoBackgroundItem] MFStartup failed:" << hrMf;
            return;
        }
        m_mfInitialized = true;
    }

    QString resolvedPath = m_source;
    if (resolvedPath.startsWith("file:///")) {
        resolvedPath = QUrl(resolvedPath).toLocalFile();
    }

    QFileInfo fi(resolvedPath);
    if (!fi.exists()) {
        // Try relative to application directory
        QString appPath = QDir(QCoreApplication::applicationDirPath()).filePath(m_source);
        if (QFileInfo::exists(appPath)) {
            resolvedPath = appPath;
        } else {
            // Try relative to application directory/assets/
            QString assetPath = QDir(QCoreApplication::applicationDirPath()).filePath("assets/" + fi.fileName());
            if (QFileInfo::exists(assetPath)) {
                resolvedPath = assetPath;
            } else {
                // Try parent assets (e.g. build/../assets/)
                QString parentAsset = QDir(QCoreApplication::applicationDirPath()).filePath("../assets/" + fi.fileName());
                if (QFileInfo::exists(parentAsset)) {
                    resolvedPath = parentAsset;
                }
            }
        }
    }

    if (!QFileInfo::exists(resolvedPath)) {
        qWarning() << "[VideoBackgroundItem] Video file not found:" << resolvedPath;
        return;
    }

    IMFAttributes *pAttributes = nullptr;
    MFCreateAttributes(&pAttributes, 2);
    if (pAttributes) {
        pAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
        pAttributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
    }

    std::wstring wpath = QDir::toNativeSeparators(resolvedPath).toStdWString();
    HRESULT hr = MFCreateSourceReaderFromURL(wpath.c_str(), pAttributes, &m_pReader);
    if (pAttributes) {
        pAttributes->Release();
    }

    if (FAILED(hr) || !m_pReader) {
        qWarning() << "[VideoBackgroundItem] Failed to create IMFSourceReader, hr:" << hr;
        return;
    }

    // Select first video stream
    m_pReader->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_ALL_STREAMS), FALSE);
    m_pReader->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), TRUE);

    // Request 32-bit RGB
    IMFMediaType *pMediaType = nullptr;
    if (SUCCEEDED(MFCreateMediaType(&pMediaType))) {
        pMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
        pMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
        m_pReader->SetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), nullptr, pMediaType);
        pMediaType->Release();
    }

    // Determine dimensions and frame rate
    IMFMediaType *pCurrentType = nullptr;
    if (SUCCEEDED(m_pReader->GetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), &pCurrentType))) {
        UINT32 w = 0, h = 0;
        MFGetAttributeSize(pCurrentType, MF_MT_FRAME_SIZE, &w, &h);
        m_videoWidth = static_cast<int>(w);
        m_videoHeight = static_cast<int>(h);

        UINT32 num = 0, den = 0;
        if (SUCCEEDED(MFGetAttributeRatio(pCurrentType, MF_MT_FRAME_RATE, &num, &den)) && den > 0 && num > 0) {
            double nativeFps = static_cast<double>(num) / den;
            qDebug() << "[VideoBackgroundItem] Detected stream FPS:" << nativeFps;
            if (m_targetFps <= 0) {
                m_targetFps = qBound(10, qRound(nativeFps), 60);
                m_frameIntervalMs = qMax(1, qRound(1000.0 / m_targetFps));
            }
        }
        pCurrentType->Release();
    }

    if (m_targetFps <= 0) {
        m_targetFps = 45;
    }
    m_targetFps = qBound(10, m_targetFps, 60);
    m_frameIntervalMs = qMax(1, qRound(1000.0 / m_targetFps));

    readNextFrame();

    if (m_playing) {
        m_timer.start(m_frameIntervalMs);
    }
}

void VideoBackgroundItem::closeMedia() {
    m_timer.stop();
    if (m_pReader) {
        m_pReader->Release();
        m_pReader = nullptr;
    }
    m_currentFrame = QImage();
    m_videoWidth = 0;
    m_videoHeight = 0;
}

void VideoBackgroundItem::readNextFrame() {
    if (!m_pReader) return;

    DWORD streamIndex = 0;
    DWORD flags = 0;
    LONGLONG timestamp = 0;
    IMFSample *pSample = nullptr;

    HRESULT hr = m_pReader->ReadSample(
        static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM),
        0,
        &streamIndex,
        &flags,
        &timestamp,
        &pSample
    );

    if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
        // Rewind to start for seamless loop
        PROPVARIANT var;
        PropVariantInit(&var);
        var.vt = VT_I8;
        var.hVal.QuadPart = 0;
        m_pReader->SetCurrentPosition(GUID_NULL, var);
        PropVariantClear(&var);
        if (pSample) pSample->Release();
        return;
    }

    if (SUCCEEDED(hr) && pSample) {
        IMFMediaBuffer *pBuffer = nullptr;
        if (SUCCEEDED(pSample->ConvertToContiguousBuffer(&pBuffer))) {
            BYTE *pData = nullptr;
            DWORD maxLength = 0, currentLength = 0;
            if (SUCCEEDED(pBuffer->Lock(&pData, &maxLength, &currentLength))) {
                if (m_videoWidth > 0 && m_videoHeight > 0) {
                    if (m_currentFrame.width() != m_videoWidth || m_currentFrame.height() != m_videoHeight || m_currentFrame.format() != QImage::Format_RGB32) {
                        m_currentFrame = QImage(m_videoWidth, m_videoHeight, QImage::Format_RGB32);
                    }
                    uchar *destBits = m_currentFrame.bits();
                    qsizetype bytesPerLine = m_currentFrame.bytesPerLine();
                    int srcStride = m_videoWidth * 4;
                    if (bytesPerLine == srcStride) {
                        memcpy(destBits, pData, static_cast<size_t>(m_videoHeight) * srcStride);
                    } else {
                        for (int y = 0; y < m_videoHeight; ++y) {
                            memcpy(destBits + y * bytesPerLine, pData + y * srcStride, srcStride);
                        }
                    }
                }
                pBuffer->Unlock();
            }
            pBuffer->Release();
        }
        pSample->Release();
        update();
    }
}

void VideoBackgroundItem::onTimerTick() {
    if (m_playing && isVisible() && opacity() > 0.01) {
        readNextFrame();
    }
}

void VideoBackgroundItem::paint(QPainter *painter) {
    if (m_currentFrame.isNull()) {
        painter->fillRect(boundingRect(), QColor("#0a0d13"));
        return;
    }

    qreal itemW = width();
    qreal itemH = height();
    if (itemW <= 0 || itemH <= 0) return;

    qreal imgW = m_currentFrame.width();
    qreal imgH = m_currentFrame.height();
    if (imgW <= 0 || imgH <= 0) return;

    qreal scale = qMax(itemW / imgW, itemH / imgH);
    qreal drawW = imgW * scale;
    qreal drawH = imgH * scale;
    qreal drawX = (itemW - drawW) / 2.0;
    qreal drawY = (itemH - drawH) / 2.0;

    painter->drawImage(QRectF(drawX, drawY, drawW, drawH), m_currentFrame);
}
