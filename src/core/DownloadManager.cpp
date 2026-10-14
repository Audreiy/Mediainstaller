#include "DownloadManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QUuid>
#include <QDateTime>
#include <QRegularExpression>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>
#include <QFile>
#include <QTextStream>

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

    m_speedLimitEnabled = settings.value("speedLimitEnabled", settings.value("downloadLimitEnabled", false)).toBool();
    m_speedLimitMBps = settings.value("speedLimitMBps", settings.value("maxConcurrentDownloads", 2)).toInt();
    if (m_speedLimitMBps < 1) m_speedLimitMBps = 2;
    m_sequentialMode = settings.value("sequentialMode", false).toBool();
    m_videoBackgroundEnabled = settings.value("videoBackgroundEnabled", true).toBool();
    m_language = settings.value("appLanguage", "en").toString();

    connect(&m_queueModel, &DownloadQueueModel::requestPause, this, &DownloadManager::onQueuePause);
    connect(&m_queueModel, &DownloadQueueModel::requestResume, this, &DownloadManager::onQueueResume);
    connect(&m_queueModel, &DownloadQueueModel::requestCancel, this, &DownloadManager::onQueueCancel);
    connect(&m_queueModel, &DownloadQueueModel::queueOrderChanged, this, &DownloadManager::checkNextQueueItem);
    connect(&m_urlAnalyzer, &UrlAnalyzer::analysisError, this, [this](const QString &msg) {
        logDownloadFailure("URL Analysis Error", "", msg);
    });
}

QString DownloadManager::logFilePath() {
    QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs";
    QDir().mkpath(logDir);
    return logDir + "/downloads.log";
}

void DownloadManager::logDownloadFailure(const QString &title, const QString &url, const QString &errorMessage, const QString &details) {
    QString path = logFilePath();
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out.setGenerateByteOrderMark(true);
        out << "================================================================================\n";
        out << QString("[%1] DOWNLOAD FAILURE\n").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
        out << "File:    " << title << "\n";
        if (!url.isEmpty()) {
            out << "URL:     " << url << "\n";
        }
        out << "Reason:  " << errorMessage << "\n";
        if (!details.trimmed().isEmpty()) {
            out << "Details / Logs:\n" << details.trimmed() << "\n";
        }
        out << "================================================================================\n\n";
        file.close();
    }
}

void DownloadManager::openLogFile() {
    QString path = logFilePath();
    QFileInfo fi(path);
    if (!fi.exists()) {
        QDir().mkpath(fi.dir().absolutePath());
        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "YuYuInstaller Download & Diagnostics Log\n";
            out << "Created: " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n";
            out << "No errors recorded yet.\n\n";
            file.close();
        }
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

QString DownloadManager::getLogFilePath() const {
    return logFilePath();
}

void DownloadManager::clearLogFile() {
    QString path = logFilePath();
    QFile file(path);
    if (file.exists()) {
        file.remove();
    }
}

void DownloadManager::retryDownload(const QString &id) {
    if (!m_taskRegistry.contains(id)) return;
    QueuedTask task = m_taskRegistry.value(id);
    m_queueModel.updateStatus(id, "Downloading");
    m_queueModel.updateProgress(id, 0.0, "Retrying...", "--:--", -1, -1);
    startTask(task);
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

int DownloadManager::effectiveConcurrentLimit() const {
    if (m_sequentialMode) {
        return 1;
    }
    return 0; // 0 means unlimited (All at Once)
}

void DownloadManager::setSequentialMode(bool seq) {
    if (m_sequentialMode != seq) {
        m_sequentialMode = seq;
        QSettings settings("YuYuDev", "YuYuInstaller");
        settings.setValue("sequentialMode", m_sequentialMode);
        emit sequentialModeChanged();
        checkNextQueueItem();
    }
}

void DownloadManager::setSpeedLimitEnabled(bool enabled) {
    if (m_speedLimitEnabled != enabled) {
        m_speedLimitEnabled = enabled;
        QSettings settings("YuYuDev", "YuYuInstaller");
        settings.setValue("speedLimitEnabled", m_speedLimitEnabled);
        settings.setValue("downloadLimitEnabled", m_speedLimitEnabled);
        emit speedLimitEnabledChanged();
        emit downloadLimitEnabledChanged();

        int effectiveLimit = m_speedLimitEnabled ? m_speedLimitMBps : 0;
        for (auto it = m_activeHttpDownloads.begin(); it != m_activeHttpDownloads.end(); ++it) {
            if (it.value()) it.value()->setSpeedLimitMBps(effectiveLimit);
        }
        for (auto it = m_activeYtDlpDownloads.begin(); it != m_activeYtDlpDownloads.end(); ++it) {
            if (it.value()) it.value()->setSpeedLimitMBps(effectiveLimit);
        }
    }
}

void DownloadManager::setSpeedLimitMBps(int limit) {
    if (limit < 1) limit = 1;
    if (limit > 100) limit = 100;
    if (m_speedLimitMBps != limit) {
        m_speedLimitMBps = limit;
        QSettings settings("YuYuDev", "YuYuInstaller");
        settings.setValue("speedLimitMBps", m_speedLimitMBps);
        settings.setValue("maxConcurrentDownloads", m_speedLimitMBps);
        emit speedLimitMBpsChanged();
        emit maxConcurrentDownloadsChanged();

        if (m_speedLimitEnabled) {
            for (auto it = m_activeHttpDownloads.begin(); it != m_activeHttpDownloads.end(); ++it) {
                if (it.value()) it.value()->setSpeedLimitMBps(m_speedLimitMBps);
            }
            for (auto it = m_activeYtDlpDownloads.begin(); it != m_activeYtDlpDownloads.end(); ++it) {
                if (it.value()) it.value()->setSpeedLimitMBps(m_speedLimitMBps);
            }
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

    int effectiveLimit = m_speedLimitEnabled ? m_speedLimitMBps : 0;

    if (task.useYtDlp) {
        YtDlpDownloader *ytDownloader = new YtDlpDownloader(task.id, task.url, task.finalFilePath, task.formatId, this);
        ytDownloader->setSpeedLimitMBps(effectiveLimit);
        if (!task.referer.isEmpty()) {
            ytDownloader->setReferer(task.referer);
        }
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
        downloader->setSpeedLimitMBps(effectiveLimit);
        if (!task.referer.isEmpty()) {
            downloader->setReferer(task.referer);
        }
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
    int limit = effectiveConcurrentLimit();
    if (limit <= 0) {
        startAllQueuedTasks();
        return;
    }

    int activeCount = countActiveDownloads();

    // If active count exceeds the limit (e.g. user lowered limit or switched to sequential), pause excess from the bottom
    if (activeCount > limit) {
        for (int i = m_queueModel.rowCount() - 1; i >= 0 && activeCount > limit; --i) {
            DownloadItem it = m_queueModel.getItemAt(i);
            if (it.status == "Downloading") {
                onQueuePause(it.id);
                activeCount--;
            }
        }
    }

    // If active count is below limit, start next queued/paused tasks from top to bottom
    for (int i = 0; i < m_queueModel.rowCount() && activeCount < limit; ++i) {
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
    QUrl parsedUrl(url.trimmed());
    QString pathLower = parsedUrl.path().toLower();

    bool isWebPlatform = lowerUrl.contains("youtube.com") || lowerUrl.contains("youtu.be") ||
                         lowerUrl.contains("twitter.com") || lowerUrl.contains("x.com") ||
                         lowerUrl.contains("tiktok.com") || lowerUrl.contains("instagram.com") ||
                         lowerUrl.contains("vimeo.com") || lowerUrl.contains("reddit.com") ||
                         lowerUrl.contains("soundcloud.com") || lowerUrl.contains("twitch.tv") ||
                         lowerUrl.contains("facebook.com") || lowerUrl.contains("fb.watch") ||
                         lowerUrl.contains("pornhub.com") || lowerUrl.contains("xvideos.com") ||
                         lowerUrl.contains("xnxx.com") || lowerUrl.contains("redtube.com") ||
                         lowerUrl.contains("youporn.com") || lowerUrl.contains("spankbang.com") ||
                         lowerUrl.contains("eporner.com") || lowerUrl.contains("rumble.com") ||
                         lowerUrl.contains("bilibili") || lowerUrl.contains("vk.com") ||
                         lowerUrl.contains("streamable.com") || lowerUrl.contains("threads.net") ||
                         lowerUrl.contains("bsky.app") || lowerUrl.contains("pinterest.com") ||
                         lowerUrl.contains("pin.it") || lowerUrl.contains("bandcamp.com") ||
                         lowerUrl.contains("mixcloud.com");

    bool isDirectFile = !isWebPlatform && (pathLower.endsWith(".mp4") || pathLower.endsWith(".mp3") ||
                        pathLower.endsWith(".m4a") || pathLower.endsWith(".wav") ||
                        pathLower.endsWith(".webm") || pathLower.endsWith(".mkv") ||
                        lowerUrl.contains("tau-video"));

    bool useYtDlp = isWebPlatform || (!isDirectFile && !FFmpegProcessor::findYtDlpExecutable().isEmpty());

    QString referer;
    if (m_urlAnalyzer.currentMediaInfo()) {
        referer = m_urlAnalyzer.currentMediaInfo()->referer();
    }

    QueuedTask task;
    task.id = id;
    task.url = url;
    task.title = finalFileName;
    task.formatId = formatId;
    task.customFolder = customFolder;
    task.finalFilePath = finalFilePath;
    task.referer = referer;
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

    m_taskRegistry[id] = task;

    int activeCount = countActiveDownloads();
    int limit = effectiveConcurrentLimit();
    if (limit > 0 && activeCount >= limit) {
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
    qint64 finalSize = QFileInfo(filePath).size();

    if (m_pendingAudioConversion.value(id, false)) {
        m_pendingAudioConversion.remove(id);
        m_queueModel.updateStatus(id, "Converting");

        DownloadItem item = m_queueModel.getItem(id);
        QString finalMp3Path = item.destinationPath;

        connect(&m_ffmpegProcessor, &FFmpegProcessor::finished, this, [this, id, filePath, finalMp3Path](bool ok, const QString &out, const QString &err) {
            Q_UNUSED(out);
            if (ok) {
                QFile::remove(filePath);
                qint64 mp3Size = QFileInfo(finalMp3Path).size();
                m_queueModel.setCompleted(id, mp3Size, finalMp3Path);
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
        m_queueModel.setCompleted(id, finalSize, filePath);
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

void DownloadManager::handleDownloadFailed(const QString &id, const QString &errorMsg, const QString &details) {
    if (m_taskRegistry.contains(id) && !m_taskRegistry[id].useYtDlp && !FFmpegProcessor::findYtDlpExecutable().isEmpty()) {
        // Fallback: try yt-dlp with Chrome impersonation if direct HTTP download failed
        QueuedTask task = m_taskRegistry[id];
        task.useYtDlp = true;
        m_taskRegistry[id] = task;
        m_queueModel.updateStatus(id, "Downloading");
        m_queueModel.updateProgress(id, 0.0, "Switching engine...", "--:--", -1, -1);
        if (m_activeHttpDownloads.contains(id)) {
            if (m_activeHttpDownloads[id]) m_activeHttpDownloads[id]->deleteLater();
            m_activeHttpDownloads.remove(id);
        }
        startTask(task);
        return;
    }

    m_queueModel.updateStatus(id, "Failed", errorMsg);

    QString title = id;
    QString url = "";
    if (m_taskRegistry.contains(id)) {
        title = m_taskRegistry[id].title;
        url = m_taskRegistry[id].url;
    } else {
        DownloadItem item = m_queueModel.getItem(id);
        title = item.title;
        url = item.url;
    }
    logDownloadFailure(title, url, errorMsg, details);

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
    DownloadItem item = m_queueModel.getItem(id);
    if (item.status == "Failed") {
        retryDownload(id);
        return;
    }

    int limit = effectiveConcurrentLimit();
    if (limit > 0 && countActiveDownloads() >= limit) {
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
    m_taskRegistry.remove(id);
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
