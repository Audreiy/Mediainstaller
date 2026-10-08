#include "YtDlpDownloader.h"
#include "FFmpegProcessor.h"
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>
#include <QDebug>

YtDlpDownloader::YtDlpDownloader(const QString &id,
                                 const QString &url,
                                 const QString &targetPath,
                                 const QString &formatId,
                                 QObject *parent)
    : QObject(parent)
    , m_id(id)
    , m_url(url)
    , m_targetPath(targetPath)
    , m_formatId(formatId)
{
}

YtDlpDownloader::~YtDlpDownloader() {
    cancel();
}

void YtDlpDownloader::start() {
    if (m_isRunning) return;

    QString ytDlpExe = FFmpegProcessor::findYtDlpExecutable();
    if (ytDlpExe.isEmpty()) {
        emit downloadFailed(m_id, "yt-dlp executable not found. Please ensure yt-dlp is installed.");
        return;
    }

    QString ffmpegExe = FFmpegProcessor::findFFmpegExecutable();

    m_isRunning = true;
    m_isPaused = false;
    emit runningChanged();

    QStringList args;
    args << "--newline";
    args << "--no-warnings";
    args << "--no-playlist";
    args << "--no-check-certificates";
    args << "--user-agent" << "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36";

    if (!m_referer.isEmpty()) {
        args << "--referer" << m_referer;
    }

    if (!ffmpegExe.isEmpty()) {
        args << "--ffmpeg-location" << QFileInfo(ffmpegExe).absolutePath();
    }

    if (m_formatId == "audio_mp3") {
        args << "-x" << "--audio-format" << "mp3" << "--audio-quality" << "320k";
    } else if (m_formatId == "audio_m4a") {
        args << "-x" << "--audio-format" << "m4a";
    } else {
        // High quality video with merged audio or best direct stream
        args << "-f" << "bestvideo[ext=mp4]+bestaudio[ext=m4a]/bestvideo+bestaudio/best[ext=mp4]/best";
    }

    // Output template: use %(ext)s so yt-dlp correctly writes the target extension without duplication
    QFileInfo fi(m_targetPath);
    QString templatePath = fi.dir().filePath(fi.completeBaseName() + ".%(ext)s");
    args << "-o" << templatePath;

    // Progress template: DOWNLOAD_PROGRESS:percent|speed|eta|downloaded_bytes|total_bytes
    args << "--progress-template" << "DOWNLOAD_PROGRESS:%(progress._percent_str)s|%(progress._speed_str)s|%(progress._eta_str)s|%(progress.downloaded_bytes)s|%(progress.total_bytes)s";

    args << m_url;

    m_process = new QProcess(this);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &YtDlpDownloader::onReadyReadStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &YtDlpDownloader::onReadyReadStandardError);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &YtDlpDownloader::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &YtDlpDownloader::onProcessError);

    m_process->start(ytDlpExe, args);
}

void YtDlpDownloader::pause() {
    if (!m_isRunning || !m_process) return;
    m_isPaused = true;
    m_isRunning = false;
    emit runningChanged();
    m_process->kill();
    m_process->waitForFinished(500);
}

void YtDlpDownloader::resume() {
    if (m_isRunning) return;
    start();
}

void YtDlpDownloader::cancel() {
    if (m_process) {
        if (m_process->state() != QProcess::NotRunning) {
            m_process->kill();
            m_process->waitForFinished(500);
        }
        m_process->deleteLater();
        m_process = nullptr;
    }
    m_isRunning = false;
    emit runningChanged();

    // Clean up partial files if any
    QString partFile = m_targetPath + ".part";
    if (QFile::exists(partFile)) {
        QFile::remove(partFile);
    }

    emit downloadCancelled(m_id);
}

void YtDlpDownloader::onReadyReadStandardOutput() {
    if (!m_process) return;
    while (m_process->canReadLine()) {
        QString line = QString::fromUtf8(m_process->readLine()).trimmed();
        parseOutputLine(line);
    }
}

void YtDlpDownloader::onReadyReadStandardError() {
    if (!m_process) return;
    while (m_process->canReadLine()) {
        QString line = QString::fromUtf8(m_process->readLine()).trimmed();
        parseOutputLine(line);
    }
}

void YtDlpDownloader::parseOutputLine(const QString &line) {
    if (line.isEmpty()) return;

    // Check for post-processing conversions: [ExtractAudio] or [Merger]
    if (line.contains("[ExtractAudio]") || line.contains("[Merger]") || line.contains("[Fixup")) {
        m_progress = 99.0;
        m_speedString = (m_formatId == "audio_mp3") ? "Encoding 320k MP3..." : "Merging Audio & Video...";
        m_etaString = "Finishing";
        emit conversionStarted(m_id);
        emit progressUpdated(99.0, m_speedString, m_etaString, 0, 0);
        return;
    }

    // Pattern 1: Custom progress template
    if (line.startsWith("DOWNLOAD_PROGRESS:")) {
        QString data = line.mid(18).trimmed();
        QStringList parts = data.split('|');
        if (parts.size() >= 5) {
            QString pctStr = parts[0].trimmed().remove('%').trimmed();
            double pct = pctStr.toDouble();
            QString speed = parts[1].trimmed();
            QString eta = parts[2].trimmed();
            qint64 down = parts[3].trimmed().toLongLong();
            qint64 tot = parts[4].trimmed().toLongLong();

            if (pct >= 0.0) {
                m_progress = pct;
                m_speedString = speed;
                m_etaString = eta;
                emit progressUpdated(m_progress, m_speedString, m_etaString, down, tot);
                return;
            }
        }
    }

    // Pattern 2: Standard yt-dlp [download] 45.2% of ~12.34MiB at 2.45MiB/s ETA 00:05
    static const QRegularExpression dlRegex(R"(\[download\]\s+([\d\.]+)%\s+of\s+~?\s*([^\s]+)\s+at\s+([^\s]+)\s+ETA\s+([^\s]+))");
    auto dlMatch = dlRegex.match(line);
    if (dlMatch.hasMatch()) {
        double pct = dlMatch.captured(1).toDouble();
        QString speed = dlMatch.captured(3);
        QString eta = dlMatch.captured(4);
        m_progress = pct;
        m_speedString = speed;
        m_etaString = eta;
        emit progressUpdated(m_progress, m_speedString, m_etaString, 0, 0);
    }
}

void YtDlpDownloader::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    m_isRunning = false;
    emit runningChanged();

    if (m_isPaused) {
        return;
    }

    bool success = (exitStatus == QProcess::NormalExit && exitCode == 0);

    if (success) {
        m_progress = 100.0;
        emit progressUpdated(100.0, "Done", "00:00", 0, 0);

        // Verify destination file exists
        QString finalPath = m_targetPath;
        if (!QFile::exists(finalPath)) {
            QFileInfo fi(m_targetPath);
            QDir dir = fi.dir();
            QString baseName = fi.completeBaseName();
            QString expectedExt = (m_formatId == "audio_mp3") ? "mp3" : ((m_formatId == "audio_m4a") ? "m4a" : "mp4");
            QString preferredFile = dir.filePath(baseName + "." + expectedExt);

            if (QFile::exists(preferredFile)) {
                finalPath = preferredFile;
            } else {
                QStringList matches = dir.entryList(QStringList() << (baseName + ".*"), QDir::Files);
                if (!matches.isEmpty()) {
                    finalPath = dir.filePath(matches.first());
                }
            }
        }

        emit downloadCompleted(m_id, finalPath);
    } else {
        emit downloadFailed(m_id, QString("Download failed (Process exit code: %1)").arg(exitCode));
    }

    if (m_process) {
        m_process->deleteLater();
        m_process = nullptr;
    }
}

void YtDlpDownloader::onProcessError(QProcess::ProcessError error) {
    Q_UNUSED(error);
    if (m_isPaused) return;

    m_isRunning = false;
    emit runningChanged();
    emit downloadFailed(m_id, "yt-dlp encountered an execution error.");

    if (m_process) {
        m_process->deleteLater();
        m_process = nullptr;
    }
}
