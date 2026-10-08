#pragma once

#include <QObject>
#include <QString>
#include <QProcess>

enum class AudioFormat {
    MP3,
    AAC,
    WAV,
    M4A
};

class FFmpegProcessor : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isProcessing READ isProcessing NOTIFY processingChanged)
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusChanged)
    Q_PROPERTY(bool isFFmpegAvailable READ isFFmpegAvailable NOTIFY toolsStatusChanged)
    Q_PROPERTY(QString ffmpegPath READ findFFmpegExecutable NOTIFY toolsStatusChanged)
    Q_PROPERTY(bool isYtDlpAvailable READ isYtDlpAvailable NOTIFY toolsStatusChanged)
    Q_PROPERTY(QString ytDlpPath READ findYtDlpExecutable NOTIFY toolsStatusChanged)

public:
    explicit FFmpegProcessor(QObject *parent = nullptr);
    ~FFmpegProcessor() override;

    bool isProcessing() const { return m_isProcessing; }
    double progress() const { return m_progress; }
    QString statusMessage() const { return m_statusMessage; }
    bool isFFmpegAvailable() const;
    bool isYtDlpAvailable() const;

    Q_INVOKABLE void extractAudio(const QString &inputFilePath, const QString &outputFilePath, const QString &format = "mp3", int bitrateKbps = 320);
    Q_INVOKABLE void mergeStreams(const QString &videoPath, const QString &audioPath, const QString &outputPath);
    Q_INVOKABLE void cancel();
    Q_INVOKABLE void refreshTools();

    static QString findFFmpegExecutable();
    static QString findYtDlpExecutable();

signals:
    void processingChanged();
    void progressChanged(double percent);
    void statusChanged(const QString &message);
    void finished(bool success, const QString &outputPath, const QString &errorMsg);
    void toolsStatusChanged();

private slots:
    void onProcessReadyRead();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);

private:
    void parseDurationFromOutput(const QString &text);
    void parseTimeFromOutput(const QString &text);

    QProcess *m_process = nullptr;
    bool m_isProcessing = false;
    double m_progress = 0.0;
    QString m_statusMessage;
    QString m_currentOutputFile;
    double m_totalDurationSec = 0.0;
};
