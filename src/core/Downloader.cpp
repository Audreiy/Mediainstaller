#include "Downloader.h"
#include <QNetworkRequest>
#include <QFileInfo>
#include <QDir>

Downloader::Downloader(const QString &id, const QString &url, const QString &targetPath, QObject *parent)
    : QObject(parent)
    , m_id(id)
    , m_url(url)
    , m_targetPath(targetPath)
    , m_outputFile(targetPath)
{
    m_throttleTimer = new QTimer(this);
    m_throttleTimer->setInterval(50);
    connect(m_throttleTimer, &QTimer::timeout, this, &Downloader::onThrottleTimeout);
}

Downloader::~Downloader() {
    cleanup();
}

void Downloader::setSpeedLimitMBps(int mbps) {
    m_speedLimitMBps = mbps;
    if (m_speedLimitMBps > 0 && m_isRunning) {
        if (m_reply) {
            m_reply->setReadBufferSize(128 * 1024);
        }
        m_rateTimer.restart();
        if (!m_throttleTimer->isActive()) {
            m_throttleTimer->start();
        }
    } else {
        if (m_reply) {
            m_reply->setReadBufferSize(0);
        }
        if (m_throttleTimer) {
            m_throttleTimer->stop();
        }
        if (m_reply && m_outputFile.isOpen()) {
            QByteArray chunk = m_reply->readAll();
            if (!chunk.isEmpty()) {
                m_outputFile.write(chunk);
            }
        }
    }
}

void Downloader::start() {
    if (m_isRunning) return;

    // Ensure target folder exists
    QFileInfo fi(m_targetPath);
    QDir().mkpath(fi.absolutePath());

    qint64 existingBytes = 0;
    if (QFile::exists(m_targetPath)) {
        existingBytes = QFileInfo(m_targetPath).size();
    }

    if (existingBytes > 0 && m_isPaused) {
        if (!m_outputFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
            emit downloadFailed(m_id, QString("Cannot open file: ") + m_outputFile.errorString());
            return;
        }
        m_downloadedBytes = existingBytes;
    } else {
        if (!m_outputFile.open(QIODevice::WriteOnly)) {
            emit downloadFailed(m_id, QString("Cannot create file: ") + m_outputFile.errorString());
            return;
        }
        m_downloadedBytes = 0;
    }

    QNetworkRequest request;
    QUrl reqUrl = QUrl::fromUserInput(m_url);
    if (!reqUrl.isValid()) reqUrl = QUrl(m_url);
    request.setUrl(reqUrl);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, true);
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36");
    if (!m_referer.isEmpty()) {
        request.setRawHeader("Referer", m_referer.toUtf8());
    }

    if (m_downloadedBytes > 0) {
        request.setRawHeader("Range", QString("bytes=%1-").arg(m_downloadedBytes).toUtf8());
    }

    m_isPaused = false;
    m_isRunning = true;
    m_speedTimer.start();
    m_totalTimer.start();
    m_lastSampleBytes = m_downloadedBytes;
    emit runningChanged();

    m_reply = m_netManager.get(request);
    m_reply->ignoreSslErrors();

    if (m_speedLimitMBps > 0) {
        m_reply->setReadBufferSize(128 * 1024);
        m_tokenBucket = qint64(m_speedLimitMBps) * 1024 * 1024 / 10;
        m_rateTimer.restart();
        m_throttleTimer->start();
    } else {
        m_throttleTimer->stop();
    }

    connect(m_reply, &QNetworkReply::readyRead, this, &Downloader::onReadyRead);
    connect(m_reply, &QNetworkReply::downloadProgress, this, &Downloader::onDownloadProgress);
    connect(m_reply, &QNetworkReply::finished, this, &Downloader::onFinished);
    connect(m_reply, &QNetworkReply::errorOccurred, this, &Downloader::onErrorOccurred);
    connect(m_reply, &QNetworkReply::metaDataChanged, this, [this]() {
        if (!m_reply) return;
        int statusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (m_downloadedBytes > 0 && statusCode == 200) {
            if (m_outputFile.isOpen()) {
                m_outputFile.seek(0);
                m_outputFile.resize(0);
            }
            m_downloadedBytes = 0;
            m_lastSampleBytes = 0;
        }
    });
}

void Downloader::pause() {
    if (!m_isRunning) return;
    m_isPaused = true;
    m_isRunning = false;
    if (m_throttleTimer) {
        m_throttleTimer->stop();
    }
    emit runningChanged();

    if (m_reply) {
        m_reply->abort();
    }
    if (m_outputFile.isOpen()) {
        m_outputFile.flush();
        m_outputFile.close();
    }
}

void Downloader::resume() {
    if (m_isRunning) return;
    start();
}

void Downloader::cancel() {
    m_isPaused = false;
    m_isRunning = false;
    if (m_throttleTimer) {
        m_throttleTimer->stop();
    }
    emit runningChanged();

    if (m_reply) {
        m_reply->abort();
    }
    cleanup();

    // Remove partially downloaded file
    m_outputFile.remove();
    emit downloadCancelled(m_id);
}

void Downloader::onReadyRead() {
    if (!m_reply || !m_outputFile.isOpen()) return;

    if (m_speedLimitMBps <= 0) {
        QByteArray chunk = m_reply->readAll();
        if (!chunk.isEmpty()) {
            m_outputFile.write(chunk);
        }
    } else {
        processThrottledRead();
    }
}

void Downloader::onThrottleTimeout() {
    if (!m_reply || !m_outputFile.isOpen()) return;

    if (m_speedLimitMBps > 0) {
        qint64 elapsedMs = m_rateTimer.restart();
        if (elapsedMs > 500) elapsedMs = 500;
        qint64 allowedBytesPerSec = qint64(m_speedLimitMBps) * 1024 * 1024;
        m_tokenBucket += (allowedBytesPerSec * elapsedMs) / 1000;

        qint64 maxBurst = allowedBytesPerSec / 2;
        if (m_tokenBucket > maxBurst) {
            m_tokenBucket = maxBurst;
        }
        processThrottledRead();
    }
}

void Downloader::processThrottledRead() {
    if (!m_reply || !m_outputFile.isOpen()) return;

    while (m_reply->bytesAvailable() > 0 && m_tokenBucket > 0) {
        qint64 bytesToRead = qMin(m_reply->bytesAvailable(), m_tokenBucket);
        bytesToRead = qMin(bytesToRead, qint64(64 * 1024));
        QByteArray chunk = m_reply->read(bytesToRead);
        if (chunk.isEmpty()) break;
        m_outputFile.write(chunk);
        m_tokenBucket -= chunk.size();
    }
}

void Downloader::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    if (m_isPaused) return;

    qint64 currentDownloaded = m_downloadedBytes + bytesReceived;
    qint64 actualTotal = (bytesTotal > 0) ? (bytesTotal + m_downloadedBytes) : m_totalBytes;
    if (actualTotal > 0) {
        m_totalBytes = actualTotal;
    }

    double percent = 0.0;
    if (actualTotal > 0) {
        percent = (double(currentDownloaded) / double(actualTotal)) * 100.0;
        if (percent > 100.0) percent = 100.0;
    }
    m_progress = percent;

    qint64 elapsedMs = m_speedTimer.elapsed();
    if (elapsedMs >= 500) {
        qint64 deltaBytes = currentDownloaded - m_lastSampleBytes;
        double speedBps = (double(deltaBytes) / double(elapsedMs)) * 1000.0;
        m_speedString = formatSpeed(speedBps);

        QString etaStr = "--:--";
        if (speedBps > 1024 && actualTotal > currentDownloaded) {
            double remainingSec = double(actualTotal - currentDownloaded) / speedBps;
            etaStr = formatEta(remainingSec);
        }

        m_lastSampleBytes = currentDownloaded;
        m_speedTimer.restart();

        emit progressUpdated(percent, m_speedString, etaStr, currentDownloaded, actualTotal);
    }
}

void Downloader::onFinished() {
    if (!m_reply) return;

    if (m_throttleTimer) {
        m_throttleTimer->stop();
    }

    if (m_isPaused) {
        cleanup();
        return;
    }

    // Flush any pending buffered data in reply
    if (m_outputFile.isOpen()) {
        QByteArray chunk = m_reply->readAll();
        if (!chunk.isEmpty()) {
            m_outputFile.write(chunk);
        }
    }

    int statusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    bool isOk = (m_reply->error() == QNetworkReply::NoError) && (statusCode == 200 || statusCode == 206 || statusCode == 0);
    qint64 fileSize = m_outputFile.isOpen() ? m_outputFile.size() : QFileInfo(m_targetPath).size();

    bool isInvalidMedia = false;
    QString invalidReason;
    if (fileSize < 8192) {
        isInvalidMedia = true;
        invalidReason = QString("File too small (%1 bytes). Expected media stream.").arg(fileSize);
    } else if (m_outputFile.isOpen()) {
        m_outputFile.seek(0);
        QByteArray header = m_outputFile.read(256).toLower();
        if (header.contains("<html") || header.contains("<!doctype") || header.contains("#extm3u")) {
            isInvalidMedia = true;
            invalidReason = "Server returned HTML error or text playlist instead of raw media file.";
        }
    }

    if (isOk && fileSize > 0 && !isInvalidMedia) {
        m_outputFile.flush();
        m_outputFile.close();
        m_isRunning = false;
        emit runningChanged();
        emit progressUpdated(100.0, "Done", "00:00", fileSize, fileSize);
        emit downloadCompleted(m_id, m_targetPath);
    } else {
        if (m_outputFile.isOpen()) {
            m_outputFile.close();
        }
        m_outputFile.remove();
        m_isRunning = false;
        emit runningChanged();
        QString err = isInvalidMedia ? invalidReason : ((fileSize == 0 && isOk) ? "Download failed: Server sent 0 bytes (access restricted or expired link)." : QString("HTTP error (Status %1)").arg(statusCode));
        QString details = QString("HTTP Status: %1\nURL: %2\nOutput File: %3\nFile Size: %4 bytes\nReason: %5").arg(statusCode).arg(m_url, m_targetPath).arg(fileSize).arg(err);
        emit downloadFailed(m_id, err, details);
    }
    cleanup();
}

void Downloader::onErrorOccurred(QNetworkReply::NetworkError code) {
    Q_UNUSED(code);
    if (!m_reply) return;

    if (m_throttleTimer) {
        m_throttleTimer->stop();
    }

    if (m_isPaused || m_reply->error() == QNetworkReply::OperationCanceledError) {
        return;
    }

    QString err = m_reply->errorString();
    int httpCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString details = QString("HTTP Status: %1\nURL: %2\nNetwork Error: %3").arg(httpCode).arg(m_url, err);
    m_isRunning = false;
    cleanup();
    emit runningChanged();
    emit downloadFailed(m_id, err, details);
}

void Downloader::cleanup() {
    if (m_throttleTimer) {
        m_throttleTimer->stop();
    }
    if (m_outputFile.isOpen()) {
        m_outputFile.flush();
        m_outputFile.close();
    }
    if (m_reply) {
        m_reply->deleteLater();
        m_reply = nullptr;
    }
}

QString Downloader::formatSpeed(double bytesPerSec) {
    if (bytesPerSec >= 1024.0 * 1024.0) {
        return QString::asprintf("%.2f MB/s", bytesPerSec / (1024.0 * 1024.0));
    } else if (bytesPerSec >= 1024.0) {
        return QString::asprintf("%.1f KB/s", bytesPerSec / 1024.0);
    } else {
        return QString::asprintf("%.0f B/s", bytesPerSec);
    }
}

QString Downloader::formatEta(double seconds) {
    int totalSec = static_cast<int>(seconds);
    int m = totalSec / 60;
    int s = totalSec % 60;
    return QString::asprintf("%02d:%02d", m, s);
}
