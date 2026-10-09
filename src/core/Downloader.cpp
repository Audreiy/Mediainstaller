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
}

Downloader::~Downloader() {
    cleanup();
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
            emit downloadFailed(m_id, QString::fromUtf8("Dosya açılamadı: ") + m_outputFile.errorString());
            return;
        }
        m_downloadedBytes = existingBytes;
    } else {
        if (!m_outputFile.open(QIODevice::WriteOnly)) {
            emit downloadFailed(m_id, QString::fromUtf8("Dosya oluşturulamadı: ") + m_outputFile.errorString());
            return;
        }
        m_downloadedBytes = 0;
    }

    QNetworkRequest request;
    request.setUrl(QUrl(m_url));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36");

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

    QByteArray chunk = m_reply->readAll();
    m_outputFile.write(chunk);
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

    if (m_isPaused) {
        cleanup();
        return;
    }

    int statusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    bool isOk = (m_reply->error() == QNetworkReply::NoError) || (statusCode == 200 || statusCode == 206);

    if (isOk) {
        m_outputFile.flush();
        m_outputFile.close();
        m_isRunning = false;
        emit runningChanged();
        emit progressUpdated(100.0, "Done", "00:00", m_totalBytes, m_totalBytes);
        emit downloadCompleted(m_id, m_targetPath);
    } else {
        m_isRunning = false;
        emit runningChanged();
    }
    cleanup();
}

void Downloader::onErrorOccurred(QNetworkReply::NetworkError code) {
    Q_UNUSED(code);
    if (!m_reply) return;

    if (m_isPaused || m_reply->error() == QNetworkReply::OperationCanceledError) {
        return;
    }

    QString err = m_reply->errorString();
    m_isRunning = false;
    cleanup();
    emit runningChanged();
    emit downloadFailed(m_id, err);
}

void Downloader::cleanup() {
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
