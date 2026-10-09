#include "DownloadManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QUuid>
#include <QDateTime>
#include <QRegularExpression>

DownloadManager::DownloadManager(QObject *parent)
    : QObject(parent)
{
    m_defaultDownloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (m_defaultDownloadPath.isEmpty()) {
        m_defaultDownloadPath = QDir::homePath() + "/Downloads";
    }

    connect(&m_queueModel, &DownloadQueueModel::requestPause, this, &DownloadManager::onQueuePause);
    connect(&m_queueModel, &DownloadQueueModel::requestResume, this, &DownloadManager::onQueueResume);
    connect(&m_queueModel, &DownloadQueueModel::requestCancel, this, &DownloadManager::onQueueCancel);
}

DownloadManager::~DownloadManager() {
    for (auto d : m_activeHttpDownloads) {
        if (d) d->cancel();
    }
    for (auto d : m_activeYtDlpDownloads) {
        if (d) d->cancel();
    }
}

QString DownloadManager::getDefaultDownloadDirectory() const {
    return m_defaultDownloadPath;
}

void DownloadManager::setDefaultDownloadPath(const QString &path) {
    if (m_defaultDownloadPath != path && !path.isEmpty()) {
        m_defaultDownloadPath = path;
        emit defaultDownloadPathChanged();
    }
}

QString DownloadManager::generateUniqueId() const {
    return QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
}

QString DownloadManager::sanitizeFileName(const QString &name) const {
    QString safe = name.trimmed();
    static const QString illegalChars = R"(<>:"/\|?*)";
    for (const QChar &ch : illegalChars) {
        safe.replace(ch, '_');
    }

    // Strip out common redundant extensions from raw title to avoid name.mp4.mp4
    if (safe.endsWith(".mp4", Qt::CaseInsensitive)) safe.chop(4);
    else if (safe.endsWith(".mp3", Qt::CaseInsensitive)) safe.chop(4);
    else if (safe.endsWith(".m4a", Qt::CaseInsensitive)) safe.chop(4);
    else if (safe.endsWith(".webm", Qt::CaseInsensitive)) safe.chop(5);
    else if (safe.endsWith(".mkv", Qt::CaseInsensitive)) safe.chop(4);

    safe = safe.trimmed();
    if (safe.isEmpty() || safe.toLower() == "watch") {
        safe = "Media_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    }
    return safe;
}

void DownloadManager::addDownload(const QString &url, const QString &title, const QString &formatId, const QString &customFolder) {
    QString targetDir = customFolder.trimmed().isEmpty() ? m_defaultDownloadPath : customFolder;
    QDir().mkpath(targetDir);

    QString id = generateUniqueId();
    bool isAudioOnly = (formatId == "audio_mp3" || formatId == "audio_m4a");
    QString extension = (formatId == "audio_mp3") ? ".mp3" : ((formatId == "audio_m4a") ? ".m4a" : ".mp4");

    QString safeBaseName = sanitizeFileName(title);

    // Collision check: prevent overwriting existing files with same name
    QString finalFileName = safeBaseName + extension;
    int counter = 1;
    while (QFile::exists(targetDir + "/" + finalFileName)) {
        finalFileName = QString("%1 (%2)%3").arg(safeBaseName).arg(counter).arg(extension);
        counter++;
    }

    QString finalFilePath = targetDir + "/" + finalFileName;

    QString lowerUrl = url.trimmed().toLower();
    bool isWebPlatform = lowerUrl.contains("youtube.com") || lowerUrl.contains("youtu.be") ||
                         lowerUrl.contains("twitter.com") || lowerUrl.contains("x.com") ||
                         lowerUrl.contains("tiktok.com") || lowerUrl.contains("instagram.com") ||
                         lowerUrl.contains("vimeo.com") || lowerUrl.contains("reddit.com") ||
                         lowerUrl.contains("soundcloud.com") || lowerUrl.contains("twitch.tv") ||
                         lowerUrl.contains("facebook.com");
    bool isDirectFile = lowerUrl.endsWith(".mp4") || lowerUrl.endsWith(".mp3") ||
                        lowerUrl.endsWith(".m4a") || lowerUrl.endsWith(".wav") ||
                        lowerUrl.endsWith(".webm") || lowerUrl.endsWith(".mkv");

    bool useYtDlp = isWebPlatform || (!isDirectFile && !FFmpegProcessor::findYtDlpExecutable().isEmpty());

    DownloadItem item;
    item.id = id;
    item.title = finalFileName;
    item.url = url;
    item.destinationPath = finalFilePath;
    item.status = "Downloading";
    item.progress = 0.0;
    item.speed = "Starting...";
    item.eta = "--:--";
    item.isAudioOnly = isAudioOnly;
    item.createdTime = QDateTime::currentDateTime();

    m_queueModel.addItem(item);

    if (useYtDlp) {
        YtDlpDownloader *ytDownloader = new YtDlpDownloader(id, url, finalFilePath, formatId, this);
        m_activeYtDlpDownloads[id] = ytDownloader;

        connect(ytDownloader, &YtDlpDownloader::progressUpdated, this, [this, id](double pct, const QString &speed, const QString &eta, qint64 down, qint64 tot) {
            m_queueModel.updateProgress(id, pct, speed, eta, down, tot);
        });

        connect(ytDownloader, &YtDlpDownloader::conversionStarted, this, [this, id]() {
            m_queueModel.updateStatus(id, "Converting");
        });

        connect(ytDownloader, &YtDlpDownloader::downloadCompleted, this, &DownloadManager::handleDownloadCompleted);
        connect(ytDownloader, &YtDlpDownloader::downloadFailed, this, &DownloadManager::handleDownloadFailed);
        connect(ytDownloader, &YtDlpDownloader::downloadCancelled, this, &DownloadManager::handleDownloadCancelled);

        ytDownloader->start();
    } else {
        // Direct HTTP download
        QString downloadPath = finalFilePath;
        if (isAudioOnly && !lowerUrl.endsWith(".mp3")) {
            downloadPath = targetDir + "/temp_" + id + ".mp4";
            m_pendingAudioConversion[id] = true;
        }

        Downloader *downloader = new Downloader(id, url, downloadPath, this);
        m_activeHttpDownloads[id] = downloader;

        connect(downloader, &Downloader::progressUpdated, this, [this, id](double pct, const QString &speed, const QString &eta, qint64 down, qint64 tot) {
            m_queueModel.updateProgress(id, pct, speed, eta, down, tot);
        });

        connect(downloader, &Downloader::downloadCompleted, this, &DownloadManager::handleDownloadCompleted);
        connect(downloader, &Downloader::downloadFailed, this, &DownloadManager::handleDownloadFailed);
        connect(downloader, &Downloader::downloadCancelled, this, &DownloadManager::handleDownloadCancelled);

        downloader->start();
    }

    emit downloadStarted(id, finalFileName);
}

void DownloadManager::handleProgressUpdated(double percent, const QString &speedStr, const QString &etaStr, qint64 downloaded, qint64 total) {
    Q_UNUSED(percent);
    Q_UNUSED(speedStr);
    Q_UNUSED(etaStr);
    Q_UNUSED(downloaded);
    Q_UNUSED(total);
}

void DownloadManager::handleDownloadCompleted(const QString &id, const QString &filePath) {
    if (m_pendingAudioConversion.value(id, false)) {
        m_pendingAudioConversion.remove(id);
        m_queueModel.updateStatus(id, "Converting");

        DownloadItem item = m_queueModel.getItem(id);
        QString finalMp3Path = item.destinationPath;

        connect(&m_ffmpegProcessor, &FFmpegProcessor::finished, this, [this, id, filePath, finalMp3Path](bool ok, const QString &out, const QString &err) {
            Q_UNUSED(out);
            if (ok) {
                QFile::remove(filePath);
                m_queueModel.updateStatus(id, "Completed");
            } else {
                m_queueModel.updateStatus(id, "Failed", err);
            }
        });

        m_ffmpegProcessor.extractAudio(filePath, finalMp3Path, "mp3", 320);
    } else {
        m_queueModel.updateStatus(id, "Completed");
    }

    if (m_activeHttpDownloads.contains(id)) {
        if (m_activeHttpDownloads[id]) m_activeHttpDownloads[id]->deleteLater();
        m_activeHttpDownloads.remove(id);
    }
    if (m_activeYtDlpDownloads.contains(id)) {
        if (m_activeYtDlpDownloads[id]) m_activeYtDlpDownloads[id]->deleteLater();
        m_activeYtDlpDownloads.remove(id);
    }
}

void DownloadManager::handleDownloadFailed(const QString &id, const QString &errorMsg) {
    m_queueModel.updateStatus(id, "Failed", errorMsg);
    if (m_activeHttpDownloads.contains(id)) {
        if (m_activeHttpDownloads[id]) m_activeHttpDownloads[id]->deleteLater();
        m_activeHttpDownloads.remove(id);
    }
    if (m_activeYtDlpDownloads.contains(id)) {
        if (m_activeYtDlpDownloads[id]) m_activeYtDlpDownloads[id]->deleteLater();
        m_activeYtDlpDownloads.remove(id);
    }
}

void DownloadManager::handleDownloadCancelled(const QString &id) {
    DownloadItem item = m_queueModel.getItem(id);
    if (item.status != "Completed") {
        m_queueModel.updateStatus(id, "Cancelled");
    }
    if (m_activeHttpDownloads.contains(id)) {
        if (m_activeHttpDownloads[id]) m_activeHttpDownloads[id]->deleteLater();
        m_activeHttpDownloads.remove(id);
    }
    if (m_activeYtDlpDownloads.contains(id)) {
        if (m_activeYtDlpDownloads[id]) m_activeYtDlpDownloads[id]->deleteLater();
        m_activeYtDlpDownloads.remove(id);
    }
}

void DownloadManager::onQueuePause(const QString &id) {
    if (m_activeHttpDownloads.contains(id) && m_activeHttpDownloads[id]) {
        m_activeHttpDownloads[id]->pause();
        m_queueModel.updateStatus(id, "Paused");
    } else if (m_activeYtDlpDownloads.contains(id) && m_activeYtDlpDownloads[id]) {
        m_activeYtDlpDownloads[id]->pause();
        m_queueModel.updateStatus(id, "Paused");
    }
}

void DownloadManager::onQueueResume(const QString &id) {
    if (m_activeHttpDownloads.contains(id) && m_activeHttpDownloads[id]) {
        m_activeHttpDownloads[id]->resume();
        m_queueModel.updateStatus(id, "Downloading");
    } else if (m_activeYtDlpDownloads.contains(id) && m_activeYtDlpDownloads[id]) {
        m_activeYtDlpDownloads[id]->resume();
        m_queueModel.updateStatus(id, "Downloading");
    }
}

void DownloadManager::onQueueCancel(const QString &id) {
    if (m_activeHttpDownloads.contains(id) && m_activeHttpDownloads[id]) {
        m_activeHttpDownloads[id]->cancel();
    }
    if (m_activeYtDlpDownloads.contains(id) && m_activeYtDlpDownloads[id]) {
        m_activeYtDlpDownloads[id]->cancel();
    }
}
