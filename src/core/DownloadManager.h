#pragma once

#include <QObject>
#include <QString>
#include <QMap>
#include <QPointer>
#include "DownloadQueueModel.h"
#include "UrlAnalyzer.h"
#include "Downloader.h"
#include "YtDlpDownloader.h"
#include "FFmpegProcessor.h"

struct QueuedTask {
    QString id;
    QString url;
    QString title;
    QString formatId;
    QString customFolder;
    QString finalFilePath;
    bool isAudioOnly = false;
    bool useYtDlp = false;
};

class DownloadManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(DownloadQueueModel* queueModel READ queueModel CONSTANT)
    Q_PROPERTY(UrlAnalyzer* urlAnalyzer READ urlAnalyzer CONSTANT)
    Q_PROPERTY(FFmpegProcessor* ffmpegProcessor READ ffmpegProcessor CONSTANT)
    Q_PROPERTY(QString defaultDownloadPath READ defaultDownloadPath NOTIFY defaultDownloadPathChanged)
    Q_PROPERTY(bool sequentialMode READ sequentialMode WRITE setSequentialMode NOTIFY sequentialModeChanged)
    Q_PROPERTY(bool videoBackgroundEnabled READ videoBackgroundEnabled WRITE setVideoBackgroundEnabled NOTIFY videoBackgroundEnabledChanged)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)

public:
    explicit DownloadManager(QObject *parent = nullptr);
    ~DownloadManager() override;

    DownloadQueueModel* queueModel() { return &m_queueModel; }
    UrlAnalyzer* urlAnalyzer() { return &m_urlAnalyzer; }
    FFmpegProcessor* ffmpegProcessor() { return &m_ffmpegProcessor; }
    QString defaultDownloadPath() const { return m_defaultDownloadPath; }
    bool sequentialMode() const { return m_sequentialMode; }
    bool videoBackgroundEnabled() const { return m_videoBackgroundEnabled; }
    QString language() const { return m_language; }

    Q_INVOKABLE void addDownload(const QString &url, const QString &title, const QString &formatId, const QString &customFolder = QString());
    Q_INVOKABLE QString getDefaultDownloadDirectory() const;
    Q_INVOKABLE void setDefaultDownloadPath(const QString &path);
    Q_INVOKABLE void setSequentialMode(bool seq);
    Q_INVOKABLE void setVideoBackgroundEnabled(bool enabled);
    Q_INVOKABLE void setLanguage(const QString &lang);

signals:
    void defaultDownloadPathChanged();
    void sequentialModeChanged();
    void videoBackgroundEnabledChanged();
    void languageChanged();
    void downloadStarted(const QString &id, const QString &title);
    void downloadFinished(const QString &id, const QString &title);

public slots:
    void checkNextQueueItem();

private slots:
    void handleProgressUpdated(double percent, const QString &speedStr, const QString &etaStr, qint64 downloaded, qint64 total);
    void handleDownloadCompleted(const QString &id, const QString &filePath);
    void handleDownloadFailed(const QString &id, const QString &errorMsg);
    void handleDownloadCancelled(const QString &id);

    void onQueuePause(const QString &id);
    void onQueueResume(const QString &id);
    void onQueueCancel(const QString &id);

private:
    QString generateUniqueId() const;
    QString sanitizeFileName(const QString &name) const;
    bool hasActiveDownload() const;
    void startTask(const QueuedTask &task);
    void startAllQueuedTasks();
    void pauseActiveDownloadsExcept(const QString &keepId = QString());

    DownloadQueueModel m_queueModel;
    UrlAnalyzer m_urlAnalyzer;
    FFmpegProcessor m_ffmpegProcessor;
    QString m_defaultDownloadPath;
    bool m_sequentialMode = false;
    bool m_videoBackgroundEnabled = true;
    QString m_language = "en";

    QMap<QString, QueuedTask> m_queuedTasks;
    QMap<QString, QPointer<Downloader>> m_activeHttpDownloads;
    QMap<QString, QPointer<YtDlpDownloader>> m_activeYtDlpDownloads;
    QMap<QString, bool> m_pendingAudioConversion;
};
