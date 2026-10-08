#pragma once

#include <QObject>
#include <QString>
#include <QMap>
#include <QPointer>
#include "DownloadQueueModel.h"
#include "UrlAnalyzer.h"
#include "Downloader.h"
#include "FFmpegProcessor.h"

class DownloadManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(DownloadQueueModel* queueModel READ queueModel CONSTANT)
    Q_PROPERTY(UrlAnalyzer* urlAnalyzer READ urlAnalyzer CONSTANT)
    Q_PROPERTY(FFmpegProcessor* ffmpegProcessor READ ffmpegProcessor CONSTANT)
    Q_PROPERTY(QString defaultDownloadPath READ defaultDownloadPath NOTIFY defaultDownloadPathChanged)

public:
    explicit DownloadManager(QObject *parent = nullptr);
    ~DownloadManager() override;

    DownloadQueueModel* queueModel() { return &m_queueModel; }
    UrlAnalyzer* urlAnalyzer() { return &m_urlAnalyzer; }
    FFmpegProcessor* ffmpegProcessor() { return &m_ffmpegProcessor; }
    QString defaultDownloadPath() const { return m_defaultDownloadPath; }

    Q_INVOKABLE void addDownload(const QString &url, const QString &title, const QString &formatId, const QString &customFolder = QString());
    Q_INVOKABLE QString getDefaultDownloadDirectory() const;
    Q_INVOKABLE void setDefaultDownloadPath(const QString &path);

signals:
    void defaultDownloadPathChanged();
    void downloadStarted(const QString &id, const QString &title);
    void downloadFinished(const QString &id, const QString &title);

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

    DownloadQueueModel m_queueModel;
    UrlAnalyzer m_urlAnalyzer;
    FFmpegProcessor m_ffmpegProcessor;
    QString m_defaultDownloadPath;

    QMap<QString, QPointer<Downloader>> m_activeDownloads;
    QMap<QString, bool> m_pendingAudioConversion;
};
