#include "UpdateManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QRegularExpression>

UpdateManager::UpdateManager(QObject *parent)
    : QObject(parent)
{
    m_tempFilePath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/YuYuInstaller_Setup.exe";
}

UpdateManager::~UpdateManager() {
    cancelDownload();
    if (m_checkReply) {
        m_checkReply->abort();
        m_checkReply->deleteLater();
        m_checkReply = nullptr;
    }
}

void UpdateManager::checkForUpdates(bool silent) {
    if (m_checking || m_downloading) {
        return;
    }

    m_isSilentCheck = silent;
    m_checking = true;
    emit checkingChanged();

    m_statusMessage = "Checking for updates...";
    emit statusMessageChanged();

    QUrl apiUrl("https://api.github.com/repos/Audreiy/Mediainstaller/releases/latest");
    QNetworkRequest request(apiUrl);
    request.setHeader(QNetworkRequest::UserAgentHeader, "YuYuInstaller/1.0.0 (Windows NT 10.0; Win64; x64)");
    request.setRawHeader("Accept", "application/vnd.github.v3+json");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    if (m_checkReply) {
        m_checkReply->abort();
        m_checkReply->deleteLater();
    }

    m_checkReply = m_netManager.get(request);
    connect(m_checkReply, &QNetworkReply::finished, this, &UpdateManager::onCheckReplyFinished);
}

void UpdateManager::onCheckReplyFinished() {
    if (!m_checkReply) return;

    m_checking = false;
    emit checkingChanged();

    if (m_checkReply->error() != QNetworkReply::NoError) {
        int httpCode = m_checkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (httpCode == 404) {
            // No releases created yet on repository
            m_statusMessage = "Your version is up to date.";
            m_updateAvailable = false;
        } else {
            m_statusMessage = "Could not reach update server.";
            m_updateAvailable = false;
        }
        emit statusMessageChanged();
        emit updateAvailableChanged();
        emit updateCheckFinished(false, m_currentVersion);
        m_checkReply->deleteLater();
        m_checkReply = nullptr;
        return;
    }

    QByteArray data = m_checkReply->readAll();
    m_checkReply->deleteLater();
    m_checkReply = nullptr;

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        m_statusMessage = "Invalid update response format.";
        emit statusMessageChanged();
        emit updateCheckFinished(false, m_currentVersion);
        return;
    }

    QJsonObject rootObj = doc.object();
    QString tagName = rootObj.value("tag_name").toString().trimmed();
    QString body = rootObj.value("body").toString();
    QJsonArray assets = rootObj.value("assets").toArray();

    QString foundDownloadUrl;
    for (const auto &val : assets) {
        QJsonObject asset = val.toObject();
        QString name = asset.value("name").toString();
        if (name.endsWith(".exe", Qt::CaseInsensitive)) {
            foundDownloadUrl = asset.value("browser_download_url").toString();
            if (name.contains("Setup", Qt::CaseInsensitive)) {
                break; // Preferred setup installer
            }
        }
    }

    m_latestVersion = tagName;
    emit latestVersionChanged();

    m_releaseNotes = body;
    emit releaseNotesChanged();

    m_downloadUrl = foundDownloadUrl;
    emit downloadUrlChanged();

    if (isNewerVersion(tagName, m_currentVersion)) {
        m_updateAvailable = true;
        m_statusMessage = QString("Update %1 is available!").arg(tagName);
    } else {
        m_updateAvailable = false;
        m_statusMessage = "Your application is up to date.";
    }

    emit updateAvailableChanged();
    emit statusMessageChanged();
    emit updateCheckFinished(m_updateAvailable, m_latestVersion);
}

bool UpdateManager::isNewerVersion(const QString &latestStr, const QString &currentStr) {
    auto cleanVersion = [](const QString &v) -> QList<int> {
        QString s = v;
        if (s.startsWith("v", Qt::CaseInsensitive)) {
            s.remove(0, 1);
        }
        QStringList parts = s.split(QRegularExpression("[^0-9]+"), Qt::SkipEmptyParts);
        QList<int> nums;
        for (const QString &p : parts) {
            nums.append(p.toInt());
        }
        while (nums.size() < 3) nums.append(0);
        return nums;
    };

    QList<int> lat = cleanVersion(latestStr);
    QList<int> cur = cleanVersion(currentStr);

    for (int i = 0; i < 3; ++i) {
        if (lat[i] > cur[i]) return true;
        if (lat[i] < cur[i]) return false;
    }
    return false;
}

void UpdateManager::startDownloadAndInstall() {
    if (m_downloading) return;
    if (m_downloadUrl.isEmpty()) {
        m_statusMessage = "Download package not found in this release.";
        emit statusMessageChanged();
        return;
    }

    if (QFile::exists(m_tempFilePath)) {
        QFile::remove(m_tempFilePath);
    }

    m_tempFile = new QFile(m_tempFilePath, this);
    if (!m_tempFile->open(QIODevice::WriteOnly)) {
        m_statusMessage = "Failed to create temporary file for update.";
        emit statusMessageChanged();
        delete m_tempFile;
        m_tempFile = nullptr;
        return;
    }

    m_downloading = true;
    m_downloadProgress = 0;
    emit downloadingChanged();
    emit downloadProgressChanged();

    m_statusMessage = "Downloading update package...";
    emit statusMessageChanged();

    QUrl packageUrl(m_downloadUrl);
    QNetworkRequest request(packageUrl);
    request.setHeader(QNetworkRequest::UserAgentHeader, "YuYuInstaller/1.0.0 (Windows NT 10.0; Win64; x64)");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    m_downloadReply = m_netManager.get(request);
    connect(m_downloadReply, &QNetworkReply::downloadProgress, this, &UpdateManager::onDownloadProgress);
    connect(m_downloadReply, &QNetworkReply::readyRead, this, &UpdateManager::onDownloadReadyRead);
    connect(m_downloadReply, &QNetworkReply::finished, this, &UpdateManager::onDownloadFinished);
}

void UpdateManager::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    if (bytesTotal > 0) {
        int pct = static_cast<int>((bytesReceived * 100) / bytesTotal);
        if (pct != m_downloadProgress) {
            m_downloadProgress = pct;
            emit downloadProgressChanged();
        }
    }
}

void UpdateManager::onDownloadReadyRead() {
    if (m_downloadReply && m_tempFile && m_tempFile->isOpen()) {
        m_tempFile->write(m_downloadReply->readAll());
    }
}

void UpdateManager::onDownloadFinished() {
    if (!m_downloadReply) return;

    bool success = false;
    QString errString;

    if (m_downloadReply->error() == QNetworkReply::NoError && m_tempFile) {
        m_tempFile->flush();
        m_tempFile->close();

        qint64 size = m_tempFile->size();
        if (size > 1024 * 500) { // Valid installer > 500 KB
            success = true;
        } else {
            errString = "Downloaded file is corrupted or too small.";
        }
    } else {
        errString = m_downloadReply->errorString();
    }

    m_downloadReply->deleteLater();
    m_downloadReply = nullptr;

    delete m_tempFile;
    m_tempFile = nullptr;

    m_downloading = false;
    emit downloadingChanged();

    if (success) {
        m_downloadProgress = 100;
        emit downloadProgressChanged();

        m_statusMessage = "Launching installer and restarting...";
        emit statusMessageChanged();
        emit updateDownloadFinished(true, m_tempFilePath);

        // Execute installer with silent and close parameters
        QString nativePath = QDir::toNativeSeparators(m_tempFilePath);
        QStringList args;
        args << "/VERYSILENT" << "/SUPPRESSMSGBOXES" << "/NORESTART" << "/CLOSEAPPLICATIONS" << "/CURRENTUSER";

        bool started = QProcess::startDetached(nativePath, args);
        if (started) {
            // Close app to allow seamless update
            QTimer::singleShot(1500, []() {
                QCoreApplication::quit();
            });
        } else {
            m_statusMessage = "Failed to launch installer process.";
            emit statusMessageChanged();
        }
    } else {
        m_statusMessage = "Update download failed: " + errString;
        emit statusMessageChanged();
        emit updateDownloadFinished(false, errString);
        if (QFile::exists(m_tempFilePath)) {
            QFile::remove(m_tempFilePath);
        }
    }
}

void UpdateManager::cancelDownload() {
    if (m_downloadReply) {
        m_downloadReply->abort();
        m_downloadReply->deleteLater();
        m_downloadReply = nullptr;
    }
    if (m_tempFile) {
        m_tempFile->close();
        delete m_tempFile;
        m_tempFile = nullptr;
        if (QFile::exists(m_tempFilePath)) {
            QFile::remove(m_tempFilePath);
        }
    }
    if (m_downloading) {
        m_downloading = false;
        m_downloadProgress = 0;
        emit downloadingChanged();
        emit downloadProgressChanged();
        m_statusMessage = "Update download cancelled.";
        emit statusMessageChanged();
    }
}
