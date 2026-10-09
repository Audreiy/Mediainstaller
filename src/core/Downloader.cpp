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

    if (!m_outputFile.open(QIODevice::WriteOnly)) {
        emit downloadFailed(m_id, QString::fromUtf8("Dosya oluşturulamadı: ") + m_outputFile.errorString());
        return;
    }

    QNetworkRequest request;
    request.setUrl(QUrl(m_url));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) MediaGrab/1.0");

    m_reply = m_netManager.get(request);

    connect(m_reply, &QNetworkReply::readyRead, this, &Downloader::onReadyRead);
    connect(m_reply, &QNetworkReply::downloadProgress, this, &Downloader::onDownloadProgress);
    connect(m_reply, &QNetworkReply::finished, this, &Downloader::onFinished);
    connect(m_reply, &QNetworkReply::errorOccurred, this, &Downloader::onErrorOccurred);

    m_isRunning = true;
    m_speedTimer.start();
    m_totalTimer.start();
    m_lastSampleBytes = 0;
    emit runningChanged();
}

void Downloader::pause() {
    // For basic pause, abort the reply, remember offset
    m_isPaused = true;
    if (m_reply) {
        m_reply->abort();
    }
}

void Downloader::resume() {
    m_isPaused = false;
    // Future range resume support
    start();
}

void Downloader::cancel() {
    if (!m_isRunning) return;
    m_isRunning = false;
    if (m_reply) {
        m_reply->abort();
    }
    cleanup();
    // Remove partially downloaded file
    m_outputFile.remove();
    emit downloadCancelled(m_id);
    emit runningChanged();
}

void Downloader::onReadyRead() {
    if (!m_reply || !m_outputFile.isOpen()) return;

    // Stream chunk directly to disk - memory efficient
    QByteArray chunk = m_reply->readAll();
    m_outputFile.write(chunk);
    m_downloadedBytes += chunk.size();
}

void Downloader::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    m_totalBytes = bytesTotal;

    double percent = 0.0;
    if (bytesTotal > 0) {
        percent = (double(bytesReceived) / double(bytesTotal)) * 100.0;
    }
    m_progress = percent;

    qint64 elapsedMs = m_speedTimer.elapsed();
    if (elapsedMs >= 500) { // Update speed twice per second
        qint64 deltaBytes = bytesReceived - m_lastSampleBytes;
        double speedBps = (double(deltaBytes) / double(elapsedMs)) * 1000.0;
        m_speedString = formatSpeed(speedBps);

        QString etaStr = "--:--";
        if (speedBps > 1024 && bytesTotal > bytesReceived) {
            double remainingSec = double(bytesTotal - bytesReceived) / speedBps;
            etaStr = formatEta(remainingSec);
        }

        m_lastSampleBytes = bytesReceived;
        m_speedTimer.restart();

        emit progressUpdated(percent, m_speedString, etaStr, bytesReceived, bytesTotal);
    }
}

void Downloader::onFinished() {
    if (!m_reply) return;

    if (m_reply->error() == QNetworkReply::NoError) {
        m_outputFile.flush();
        m_outputFile.close();
        m_isRunning = false;
        emit runningChanged();
        emit progressUpdated(100.0, "0 KB/s", "00:00", m_totalBytes, m_totalBytes);
        emit downloadCompleted(m_id, m_targetPath);
    }
    cleanup();
}

void Downloader::onErrorOccurred(QNetworkReply::NetworkError code) {
    Q_UNUSED(code);
    if (!m_reply) return;

    if (m_reply->error() != QNetworkReply::OperationCanceledError) {
        QString err = m_reply->errorString();
        m_isRunning = false;
        cleanup();
        emit runningChanged();
        emit downloadFailed(m_id, err);
    }
}

void Downloader::cleanup() {
    if (m_outputFile.isOpen()) {
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
