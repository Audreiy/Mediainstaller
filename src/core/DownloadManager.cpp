#include "DownloadManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QUuid>
#include <QDateTime>

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
    for (auto d : m_activeDownloads) {
        if (d) {
            d->cancel();
        }
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

void DownloadManager::addDownload(const QString &url, const QString &title, const QString &formatId, const QString &customFolder) {
    QString targetDir = customFolder.trimmed().isEmpty() ? m_defaultDownloadPath : customFolder;
    QDir().mkpath(targetDir);

    QString id = generateUniqueId();
    bool needAudioConvert = (formatId == "audio_mp3");

    QString safeTitle = title.trimmed();
    if (safeTitle.isEmpty()) {
        safeTitle = "download_" + QString::number(QDateTime::currentSecsSinceEpoch());
    }

    // Sanitize file name for Windows
    static const QString illegalChars = R"(<>:"/\|?*)";
    for (const QChar &ch : illegalChars) {
        safeTitle.replace(ch, '_');
    }

    QString extension = needAudioConvert ? ".mp3" : ".mp4";
    if (!safeTitle.endsWith(".mp4", Qt::CaseInsensitive) &&
        !safeTitle.endsWith(".mp3", Qt::CaseInsensitive) &&
        !safeTitle.endsWith(".webm", Qt::CaseInsensitive) &&
        !safeTitle.endsWith(".mkv", Qt::CaseInsensitive)) {
        safeTitle += extension;
    }

    QString finalFilePath = targetDir + "/" + safeTitle;

    // Temporary download path if we need to convert to MP3 afterwards
    QString downloadFilePath = finalFilePath;
    if (needAudioConvert) {
        downloadFilePath = targetDir + "/temp_" + id + ".mp4";
        m_pendingAudioConversion[id] = true;
    }

    DownloadItem item;
    item.id = id;
    item.title = safeTitle;
    item.url = url;
    item.destinationPath = finalFilePath;
    item.status = "Downloading";
    item.progress = 0.0;
    item.speed = "0 KB/s";
    item.eta = "--:--";
    item.isAudioOnly = needAudioConvert;
    item.createdTime = QDateTime::currentDateTime();

    m_queueModel.addItem(item);

    Downloader *downloader = new Downloader(id, url, downloadFilePath, this);
    m_activeDownloads[id] = downloader;

    connect(downloader, &Downloader::progressUpdated, this, [this, id](double pct, const QString &speed, const QString &eta, qint64 down, qint64 tot) {
        m_queueModel.updateProgress(id, pct, speed, eta, down, tot);
    });

    connect(downloader, &Downloader::downloadCompleted, this, &DownloadManager::handleDownloadCompleted);
    connect(downloader, &Downloader::downloadFailed, this, &DownloadManager::handleDownloadFailed);
    connect(downloader, &Downloader::downloadCancelled, this, &DownloadManager::handleDownloadCancelled);

    downloader->start();
    emit downloadStarted(id, safeTitle);
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
        // Need to convert to MP3
        m_pendingAudioConversion.remove(id);
        m_queueModel.updateStatus(id, "Converting");

        DownloadItem item = m_queueModel.getItem(id);
        QString finalMp3Path = item.destinationPath;

        // Run conversion
        connect(&m_ffmpegProcessor, &FFmpegProcessor::finished, this, [this, id, filePath, finalMp3Path](bool ok, const QString &out, const QString &err) {
            Q_UNUSED(out);
            if (ok) {
                // Delete temp video file
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

    if (m_activeDownloads.contains(id)) {
        if (m_activeDownloads[id]) {
            m_activeDownloads[id]->deleteLater();
        }
        m_activeDownloads.remove(id);
    }
}

void DownloadManager::handleDownloadFailed(const QString &id, const QString &errorMsg) {
    m_queueModel.updateStatus(id, "Failed", errorMsg);
    if (m_activeDownloads.contains(id)) {
        if (m_activeDownloads[id]) {
            m_activeDownloads[id]->deleteLater();
        }
        m_activeDownloads.remove(id);
    }
}

void DownloadManager::handleDownloadCancelled(const QString &id) {
    m_queueModel.updateStatus(id, "Cancelled");
    if (m_activeDownloads.contains(id)) {
        if (m_activeDownloads[id]) {
            m_activeDownloads[id]->deleteLater();
        }
        m_activeDownloads.remove(id);
    }
}

void DownloadManager::onQueuePause(const QString &id) {
    if (m_activeDownloads.contains(id) && m_activeDownloads[id]) {
        m_activeDownloads[id]->pause();
        m_queueModel.updateStatus(id, "Paused");
    }
}

void DownloadManager::onQueueResume(const QString &id) {
    if (m_activeDownloads.contains(id) && m_activeDownloads[id]) {
        m_activeDownloads[id]->resume();
        m_queueModel.updateStatus(id, "Downloading");
    }
}

void DownloadManager::onQueueCancel(const QString &id) {
    if (m_activeDownloads.contains(id) && m_activeDownloads[id]) {
        m_activeDownloads[id]->cancel();
    }
}
