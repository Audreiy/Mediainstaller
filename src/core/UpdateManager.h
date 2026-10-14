#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>

class UpdateManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool checking READ isChecking NOTIFY checkingChanged)
    Q_PROPERTY(bool updateAvailable READ isUpdateAvailable NOTIFY updateAvailableChanged)
    Q_PROPERTY(QString currentVersion READ currentVersion CONSTANT)
    Q_PROPERTY(QString latestVersion READ latestVersion NOTIFY latestVersionChanged)
    Q_PROPERTY(QString releaseNotes READ releaseNotes NOTIFY releaseNotesChanged)
    Q_PROPERTY(QString downloadUrl READ downloadUrl NOTIFY downloadUrlChanged)
    Q_PROPERTY(bool downloading READ isDownloading NOTIFY downloadingChanged)
    Q_PROPERTY(int downloadProgress READ downloadProgress NOTIFY downloadProgressChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    explicit UpdateManager(QObject *parent = nullptr);
    ~UpdateManager() override;

    bool isChecking() const { return m_checking; }
    bool isUpdateAvailable() const { return m_updateAvailable; }
    QString currentVersion() const { return m_currentVersion; }
    QString latestVersion() const { return m_latestVersion; }
    QString releaseNotes() const { return m_releaseNotes; }
    QString downloadUrl() const { return m_downloadUrl; }
    bool isDownloading() const { return m_downloading; }
    int downloadProgress() const { return m_downloadProgress; }
    QString statusMessage() const { return m_statusMessage; }

    Q_INVOKABLE void checkForUpdates(bool silent = false);
    Q_INVOKABLE void startDownloadAndInstall();
    Q_INVOKABLE void cancelDownload();

signals:
    void checkingChanged();
    void updateAvailableChanged();
    void latestVersionChanged();
    void releaseNotesChanged();
    void downloadUrlChanged();
    void downloadingChanged();
    void downloadProgressChanged();
    void statusMessageChanged();
    void updateCheckFinished(bool updateAvailable, const QString &version);
    void updateDownloadFinished(bool success, const QString &errorOrPath);

private slots:
    void onCheckReplyFinished();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadReadyRead();
    void onDownloadFinished();

private:
    bool isNewerVersion(const QString &latest, const QString &current);

    QNetworkAccessManager m_netManager;
    QNetworkReply *m_checkReply = nullptr;
    QNetworkReply *m_downloadReply = nullptr;
    QFile *m_tempFile = nullptr;
    QString m_tempFilePath;

    bool m_checking = false;
    bool m_updateAvailable = false;
    bool m_downloading = false;
    int m_downloadProgress = 0;
    bool m_isSilentCheck = false;

    QString m_currentVersion = "1.1.7";
    QString m_latestVersion;
    QString m_releaseNotes;
    QString m_downloadUrl;
    QString m_statusMessage;
};
