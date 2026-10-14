#pragma once

#include <QObject>
#include <QString>
#include <QProcess>
#include <QElapsedTimer>

class YtDlpDownloader : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(double progress READ progress NOTIFY progressUpdated)

public:
    explicit YtDlpDownloader(const QString &id,
                             const QString &url,
                             const QString &targetPath,
                             const QString &formatId,
                             QObject *parent = nullptr);
    ~YtDlpDownloader() override;

    QString id() const { return m_id; }
    QString url() const { return m_url; }
    QString targetPath() const { return m_targetPath; }
    bool isRunning() const { return m_isRunning; }
    double progress() const { return m_progress; }

    void setReferer(const QString &ref) { m_referer = ref; }
    QString referer() const { return m_referer; }

    void setSpeedLimitMBps(int mbps) { m_speedLimitMBps = mbps; }
    int speedLimitMBps() const { return m_speedLimitMBps; }

    void start();
    void pause();
    void resume();
    void cancel();

signals:
    void runningChanged();
    void progressUpdated(double percent, const QString &speedStr, const QString &etaStr, qint64 downloaded, qint64 total);
    void conversionStarted(const QString &id);
    void downloadCompleted(const QString &id, const QString &filePath);
    void downloadFailed(const QString &id, const QString &errorMessage, const QString &details = QString());
    void downloadCancelled(const QString &id);

private slots:
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);

private:
    void parseOutputLine(const QString &line);

    QString m_id;
    QString m_url;
    QString m_targetPath;
    QString m_formatId;
    QString m_referer;
    int m_speedLimitMBps = 0;
    bool m_isRunning = false;
    bool m_isPaused = false;
    bool m_impersonateRetried = false;
    double m_progress = 0.0;
    QString m_speedString;
    QString m_etaString;
    QElapsedTimer m_progressTimer;
    QStringList m_errorLogs;
    QString m_lastErrorMessage;
    QProcess *m_process = nullptr;
};
