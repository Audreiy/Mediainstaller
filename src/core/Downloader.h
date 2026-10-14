#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QFile>
#include <QElapsedTimer>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class Downloader : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(double progress READ progress NOTIFY progressUpdated)
    Q_PROPERTY(QString speedString READ speedString NOTIFY progressUpdated)

public:
    explicit Downloader(const QString &id, const QString &url, const QString &targetPath, QObject *parent = nullptr);
    ~Downloader() override;

    QString id() const { return m_id; }
    QString url() const { return m_url; }
    QString targetPath() const { return m_targetPath; }
    bool isRunning() const { return m_isRunning; }
    double progress() const { return m_progress; }
    QString speedString() const { return m_speedString; }

    void setSpeedLimitMBps(int mbps);
    int speedLimitMBps() const { return m_speedLimitMBps; }

    void setReferer(const QString &ref) { m_referer = ref; }
    QString referer() const { return m_referer; }

    void start();
    void pause();
    void resume();
    void cancel();

signals:
    void runningChanged();
    void progressUpdated(double percent, const QString &speedStr, const QString &etaStr, qint64 downloaded, qint64 total);
    void downloadCompleted(const QString &id, const QString &filePath);
    void downloadFailed(const QString &id, const QString &errorMessage, const QString &details = QString());
    void downloadCancelled(const QString &id);

private slots:
    void onReadyRead();
    void onThrottleTimeout();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onFinished();
    void onErrorOccurred(QNetworkReply::NetworkError code);

private:
    void cleanup();
    void processThrottledRead();
    QString formatSpeed(double bytesPerSec);
    QString formatEta(double seconds);

    QString m_id;
    QString m_url;
    QString m_targetPath;
    QString m_referer;
    bool m_isRunning = false;
    bool m_isPaused = false;
    double m_progress = 0.0;
    QString m_speedString;

    int m_speedLimitMBps = 0;
    qint64 m_tokenBucket = 0;
    QTimer *m_throttleTimer = nullptr;
    QElapsedTimer m_rateTimer;

    QNetworkAccessManager m_netManager;
    QNetworkReply *m_reply = nullptr;
    QFile m_outputFile;

    qint64 m_downloadedBytes = 0;
    qint64 m_totalBytes = 0;
    qint64 m_lastSampleBytes = 0;
    QElapsedTimer m_speedTimer;
    QElapsedTimer m_totalTimer;
};
