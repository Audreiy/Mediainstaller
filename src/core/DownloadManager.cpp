#include "DownloadManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QUuid>
#include <QDateTime>
#include <QRegularExpression>
#include <QSettings>

DownloadManager::DownloadManager(QObject *parent)
    : QObject(parent)
{
    QSettings settings("YuYuDev", "YuYuInstaller");
    QString savedPath = settings.value("defaultDownloadPath").toString();
    if (!savedPath.isEmpty() && QDir(savedPath).exists()) {
        m_defaultDownloadPath = savedPath;
    } else {
        m_defaultDownloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
        if (m_defaultDownloadPath.isEmpty()) {
            m_defaultDownloadPath = QDir::homePath() + "/Downloads";
        }
    }

    m_downloadLimitEnabled = settings.value("downloadLimitEnabled", false).toBool();
    m_maxConcurrentDownloads = settings.value("maxConcurrentDownloads", 2).toInt();
    if (m_maxConcurrentDownloads < 1) m_maxConcurrentDownloads = 1;
    if (settings.contains("sequentialMode")) {
        m_sequentialMode = settings.value("sequentialMode", false).toBool();
        if (m_sequentialMode) {
            m_downloadLimitEnabled = true;
            m_maxConcurrentDownloads = 1;
        }
    } else {
        m_sequentialMode = m_downloadLimitEnabled && (m_maxConcurrentDownloads == 1);
    }
    m_videoBackgroundEnabled = settings.value("videoBackgroundEnabled", true).toBool();
    m_language = settings.value("appLanguage", "en").toString();

    connect(&m_queueModel, &DownloadQueueModel::requestPause, this, &DownloadManager::onQueuePause);
    connect(&m_queueModel, &DownloadQueueModel::requestResume, this, &DownloadManager::onQueueResume);
    connect(&m_queueModel, &DownloadQueueModel::requestCancel, this, &DownloadManager::onQueueCancel);
    connect(&m_queueModel, &DownloadQueueModel::queueOrderChanged, this, &DownloadManager::checkNextQueueItem);
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
        QSettings settings("YuYuDev", "YuYuInstaller");
        settings.setValue("defaultDownloadPath", m_defaultDownloadPath);
        emit defaultDownloadPathChanged();
    }
}

void DownloadManager::setSequentialMode(bool seq) {
    if (seq) {
        setDownloadLimitEnabled(true);
        setMaxConcurrentDownloads(1);
    } else {
        setDownloadLimitEnabled(false);
    }
}

void DownloadManager::setDownloadLimitEnabled(bool enabled) {
    if (m_downloadLimitEnabled != enabled) {
        m_downloadLimitEnabled = enabled;
        m_sequentialMode = m_downloadLimitEnabled && (m_maxConcurrentDownloads == 1);
        QSettings settings("YuYuDev", "YuYuInstaller");
        settings.setValue("downloadLimitEnabled", m_downloadLimitEnabled);
        settings.setValue("sequentialMode", m_sequentialMode);
        emit downloadLimitEnabledChanged();
        emit sequentialModeChanged();

        if (!m_downloadLimitEnabled) {
            startAllQueuedTasks();
        } else {
            checkNextQueueItem();
        }
    }
}

void DownloadManager::setMaxConcurrentDownloads(int limit) {
    if (limit < 1) limit = 1;
    if (limit > 10) limit = 10;
    if (m_maxConcurrentDownloads != limit) {
        m_maxConcurrentDownloads = limit;
        m_sequentialMode = m_downloadLimitEnabled && (m_maxConcurrentDownloads == 1);
        QSettings settings("YuYuDev", "YuYuInstaller");
        settings.setValue("maxConcurrentDownloads", m_maxConcurrentDownloads);
        settings.setValue("sequentialMode", m_sequentialMode);
        emit maxConcurrentDownloadsChanged();
        emit sequentialModeChanged();

        if (m_downloadLimitEnabled) {
            checkNextQueueItem();
        }
    }
}

void DownloadManager::setVideoBackgroundEnabled(bool enabled) {
    if (m_videoBackgroundEnabled != enabled) {
        m_videoBackgroundEnabled = enabled;
        QSettings settings("YuYuDev", "YuYuInstaller");
        settings.setValue("videoBackgroundEnabled", m_videoBackgroundEnabled);
        emit videoBackgroundEnabledChanged();
    }
}

void DownloadManager::setLanguage(const QString &lang) {
    if (m_language != lang && (lang == "en" || lang == "tr" || lang == "uk")) {
        m_language = lang;
        QSettings settings("YuYuDev", "YuYuInstaller");
        settings.setValue("appLanguage", m_language);
        emit languageChanged();
    }
}

bool DownloadManager::hasActiveDownload() const {
    if (!m_pendingAudioConversion.isEmpty()) return true;

    for (const auto &d : m_activeHttpDownloads) {
        if (d && d->isRunning()) return true;
    }
    for (const auto &d : m_activeYtDlpDownloads) {
        if (d && d->isRunning()) return true;
    }
    return false;
}

void DownloadManager::pauseActiveDownloadsExcept(const QString &keepId) {
    for (auto it = m_activeHttpDownloads.begin(); it != m_activeHttpDownloads.end(); ++it) {
        if (it.key() != keepId && it.value() && it.value()->isRunning()) {
            it.value()->pause();
            m_queueModel.updateStatus(it.key(), "Queued");
            m_queueModel.updateProgress(it.key(), -1, "", "--:--", -1, -1);
        }
    }
    for (auto it = m_activeYtDlpDownloads.begin(); it != m_activeYtDlpDownloads.end(); ++it) {
        if (it.key() != keepId && it.value() && it.value()->isRunning()) {
            it.value()->pause();
            m_queueModel.updateStatus(it.key(), "Queued");
            m_queueModel.updateProgress(it.key(), -1, "", "--:--", -1, -1);
        }
    }
}

void DownloadManager::startTask(const QueuedTask &task) {
    m_queueModel.updateStatus(task.id, "Downloading");
    m_queueModel.updateProgress(task.id, 0.0, "Starting...", "--:--", -1, -1);

    if (task.useYtDlp) {
        YtDlpDownloader *ytDownloader = new YtDlpDownloader(task.id, task.url, task.finalFilePath, task.formatId, this);
        m_activeYtDlpDownloads[task.id] = ytDownloader;

        connect(ytDownloader, &YtDlpDownloader::progressUpdated, this, [this, id = task.id](double pct, const QString &speed, const QString &eta, qint64 down, qint64 tot) {
            m_queueModel.updateProgress(id, pct, speed, eta, down, tot);
        });

        connect(ytDownloader, &YtDlpDownloader::conversionStarted, this, [this, id = task.id]() {
            m_queueModel.updateStatus(id, "Converting");
        });

        connect(ytDownloader, &YtDlpDownloader::downloadCompleted, this, &DownloadManager::handleDownloadCompleted);
        connect(ytDownloader, &YtDlpDownloader::downloadFailed, this, &DownloadManager::handleDownloadFailed);
        connect(ytDownloader, &YtDlpDownloader::downloadCancelled, this, &DownloadManager::handleDownloadCancelled);

        ytDownloader->start();
    } else {
        // Direct HTTP download
        QString downloadPath = task.finalFilePath;
        QString targetDir = QFileInfo(task.finalFilePath).absolutePath();
        if (task.isAudioOnly && !task.url.toLower().endsWith(".mp3")) {
            downloadPath = targetDir + "/temp_" + task.id + ".mp4";
            m_pendingAudioConversion[task.id] = true;
        }

        Downloader *downloader = new Downloader(task.id, task.url, downloadPath, this);
        m_activeHttpDownloads[task.id] = downloader;

        connect(downloader, &Downloader::progressUpdated, this, [this, id = task.id](double pct, const QString &speed, const QString &eta, qint64 down, qint64 tot) {
            m_queueModel.updateProgress(id, pct, speed, eta, down, tot);
        });

        connect(downloader, &Downloader::downloadCompleted, this, &DownloadManager::handleDownloadCompleted);
        connect(downloader, &Downloader::downloadFailed, this, &DownloadManager::handleDownloadFailed);
        connect(downloader, &Downloader::downloadCancelled, this, &DownloadManager::handleDownloadCancelled);

        downloader->start();
    }

    emit downloadStarted(task.id, task.title);
}

void DownloadManager::startAllQueuedTasks() {
    auto keys = m_queuedTasks.keys();
    for (const QString &id : keys) {
        if (m_queuedTasks.contains(id)) {
            startTask(m_queuedTasks.take(id));
        }
    }
    for (auto it = m_activeHttpDownloads.begin(); it != m_activeHttpDownloads.end(); ++it) {
        if (it.value() && !it.value()->isRunning()) {
            m_queueModel.updateStatus(it.key(), "Downloading");
            it.value()->resume();
        }
    }
    for (auto it = m_activeYtDlpDownloads.begin(); it != m_activeYtDlpDownloads.end(); ++it) {
        if (it.value() && !it.value()->isRunning()) {
            m_queueModel.updateStatus(it.key(), "Downloading");
            it.value()->resume();
        }
    }
}

int DownloadManager::countActiveDownloads() const {
    int count = 0;
    for (int i = 0; i < m_queueModel.rowCount(); ++i) {
        DownloadItem it = m_queueModel.getItemAt(i);
        if (it.status == "Downloading" || it.status == "Converting") {
            count++;
        }
    }
    return count;
}

void DownloadManager::checkNextQueueItem() {
    if (!m_downloadLimitEnabled || m_maxConcurrentDownloads <= 0) {
        startAllQueuedTasks();
        return;
    }

    int activeCount = countActiveDownloads();

    // If active count exceeds the limit (e.g. user lowered limit), pause excess from the bottom
    if (activeCount > m_maxConcurrentDownloads) {
        for (int i = m_queueModel.rowCount() - 1; i >= 0 && activeCount > m_maxConcurrentDownloads; --i) {
            DownloadItem it = m_queueModel.getItemAt(i);
            if (it.status == "Downloading") {
                onQueuePause(it.id);
                activeCount--;
            }
        }
    }

    // If active count is below limit, start next queued/paused tasks from top to bottom
    for (int i = 0; i < m_queueModel.rowCount() && activeCount < m_maxConcurrentDownloads; ++i) {
        DownloadItem it = m_queueModel.getItemAt(i);
        if (it.status == "Queued" || it.status == "Paused") {
            if (m_queuedTasks.contains(it.id)) {
                startTask(m_queuedTasks.take(it.id));
                activeCount++;
            } else if (m_activeHttpDownloads.contains(it.id) && m_activeHttpDownloads[it.id]) {
                m_queueModel.updateStatus(it.id, "Downloading");
                m_queueModel.updateProgress(it.id, -1, "Resuming...", "--:--", -1, -1);
                m_activeHttpDownloads[it.id]->resume();
                activeCount++;
            } else if (m_activeYtDlpDownloads.contains(it.id) && m_activeYtDlpDownloads[it.id]) {
                m_queueModel.updateStatus(it.id, "Downloading");
                m_queueModel.updateProgress(it.id, -1, "Resuming...", "--:--", -1, -1);
                m_activeYtDlpDownloads[it.id]->resume();
                activeCount++;
            }
        }
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
    bool isAudioOnly = formatId.startsWith("audio_");
    QString extension = isAudioOnly ? (formatId.contains("m4a") ? ".m4a" : ".mp3") : ".mp4";

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
    bool isDirectFile = lowerUrl.contains(".mp4") || lowerUrl.contains(".mp3") ||
                        lowerUrl.contains(".m4a") || lowerUrl.contains(".wav") ||
                        lowerUrl.contains(".webm") || lowerUrl.contains(".mkv") ||
                        lowerUrl.contains("tau-video");

    bool useYtDlp = isWebPlatform || (!isDirectFile && !FFmpegProcessor::findYtDlpExecutable().isEmpty());

    QueuedTask task;
    task.id = id;
    task.url = url;
    task.title = finalFileName;
    task.formatId = formatId;
    task.customFolder = customFolder;
    task.finalFilePath = finalFilePath;
    task.isAudioOnly = isAudioOnly;
    task.useYtDlp = useYtDlp;

    DownloadItem item;
    item.id = id;
    item.title = finalFileName;
    item.url = url;
    item.destinationPath = finalFilePath;
    item.progress = 0.0;
    item.isAudioOnly = isAudioOnly;
    item.createdTime = QDateTime::currentDateTime();

    int activeCount = countActiveDownloads();
    if (m_downloadLimitEnabled && m_maxConcurrentDownloads > 0 && activeCount >= m_maxConcurrentDownloads) {
        item.status = "Queued";
        item.speed = "";
        item.eta = "--:--";
        m_queuedTasks[id] = task;
        m_queueModel.addItem(item);
    } else {
        item.status = "Downloading";
        item.speed = "Starting...";
        item.eta = "--:--";
        m_queueModel.addItem(item);
        startTask(task);
    }
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
            if (m_activeHttpDownloads.contains(id)) {
                if (m_activeHttpDownloads[id]) m_activeHttpDownloads[id]->deleteLater();
                m_activeHttpDownloads.remove(id);
            }
            if (m_activeYtDlpDownloads.contains(id)) {
                if (m_activeYtDlpDownloads[id]) m_activeYtDlpDownloads[id]->deleteLater();
                m_activeYtDlpDownloads.remove(id);
            }
            QMetaObject::invokeMethod(this, &DownloadManager::checkNextQueueItem, Qt::QueuedConnection);
        }, Qt::SingleShotConnection);

        m_ffmpegProcessor.extractAudio(filePath, finalMp3Path, "mp3", 320);
        return;
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

    QMetaObject::invokeMethod(this, &DownloadManager::checkNextQueueItem, Qt::QueuedConnection);
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
    QMetaObject::invokeMethod(this, &DownloadManager::checkNextQueueItem, Qt::QueuedConnection);
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
    QMetaObject::invokeMethod(this, &DownloadManager::checkNextQueueItem, Qt::QueuedConnection);
}

void DownloadManager::onQueuePause(const QString &id) {
    if (m_activeHttpDownloads.contains(id) && m_activeHttpDownloads[id]) {
        m_activeHttpDownloads[id]->pause();
        m_queueModel.updateStatus(id, "Paused");
        m_queueModel.updateProgress(id, -1, "0 KB/s", "--:--", -1, -1);
    } else if (m_activeYtDlpDownloads.contains(id) && m_activeYtDlpDownloads[id]) {
        m_activeYtDlpDownloads[id]->pause();
        m_queueModel.updateStatus(id, "Paused");
        m_queueModel.updateProgress(id, -1, "0 KB/s", "--:--", -1, -1);
    }
}

void DownloadManager::onQueueResume(const QString &id) {
    if (m_downloadLimitEnabled && countActiveDownloads() >= m_maxConcurrentDownloads) {
        for (int i = m_queueModel.rowCount() - 1; i >= 0; --i) {
            DownloadItem it = m_queueModel.getItemAt(i);
            if (it.id != id && it.status == "Downloading") {
                onQueuePause(it.id);
                break;
            }
        }
    }
    if (m_queuedTasks.contains(id)) {
        startTask(m_queuedTasks.take(id));
        return;
    }
    if (m_activeHttpDownloads.contains(id) && m_activeHttpDownloads[id]) {
        m_queueModel.updateStatus(id, "Downloading");
        m_queueModel.updateProgress(id, -1, "Connecting...", "--:--", -1, -1);
        m_activeHttpDownloads[id]->resume();
    } else if (m_activeYtDlpDownloads.contains(id) && m_activeYtDlpDownloads[id]) {
        m_queueModel.updateStatus(id, "Downloading");
        m_queueModel.updateProgress(id, -1, "Resuming...", "--:--", -1, -1);
        m_activeYtDlpDownloads[id]->resume();
    }
}

void DownloadManager::onQueueCancel(const QString &id) {
    if (m_queuedTasks.contains(id)) {
        m_queuedTasks.remove(id);
        m_queueModel.removeItem(id);
        QMetaObject::invokeMethod(this, &DownloadManager::checkNextQueueItem, Qt::QueuedConnection);
        return;
    }
    if (m_activeHttpDownloads.contains(id) && m_activeHttpDownloads[id]) {
        m_activeHttpDownloads[id]->cancel();
    }
    if (m_activeYtDlpDownloads.contains(id) && m_activeYtDlpDownloads[id]) {
        m_activeYtDlpDownloads[id]->cancel();
    }
}
