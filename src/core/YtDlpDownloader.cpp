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
    if (m_process) {
        if (m_process->state() != QProcess::NotRunning) {
            m_process->kill();
            m_process->waitForFinished(300);
        }
        delete m_process;
        m_process = nullptr;
    }
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
    args << "--continue";
    args << "--user-agent" << "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36";
    args << "--extractor-args" << "generic:impersonate";

    if (!m_referer.isEmpty()) {
        args << "--referer" << m_referer;
    }

    if (!ffmpegExe.isEmpty()) {
        args << "--ffmpeg-location" << QFileInfo(ffmpegExe).absolutePath();
    }

    if (m_formatId.startsWith("audio_mp3") || m_formatId == "audio_mp3") {
        QString bitrate = "320k";
        if (m_formatId.contains("256")) bitrate = "256k";
        else if (m_formatId.contains("192")) bitrate = "192k";
        else if (m_formatId.contains("128")) bitrate = "128k";
        args << "-x" << "--audio-format" << "mp3" << "--audio-quality" << bitrate;
    } else if (m_formatId == "audio_m4a") {
        args << "-x" << "--audio-format" << "m4a";
    } else if (m_formatId == "video_1080p") {
        args << "-f" << "bestvideo[height<=1080][ext=mp4]+bestaudio[ext=m4a]/bestvideo[height<=1080]+bestaudio/best[height<=1080]/best";
    } else if (m_formatId == "video_720p") {
        args << "-f" << "bestvideo[height<=720][ext=mp4]+bestaudio[ext=m4a]/bestvideo[height<=720]+bestaudio/best[height<=720]/best";
    } else if (m_formatId == "video_480p") {
        args << "-f" << "bestvideo[height<=480][ext=mp4]+bestaudio[ext=m4a]/bestvideo[height<=480]+bestaudio/best[height<=480]/best";
    } else if (m_formatId == "video_360p") {
        args << "-f" << "bestvideo[height<=360][ext=mp4]+bestaudio[ext=m4a]/bestvideo[height<=360]+bestaudio/best[height<=360]/best";
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

    if (m_process) {
        disconnect(m_process, nullptr, this, nullptr);
        if (m_process->state() != QProcess::NotRunning) {
            m_process->kill();
            m_process->waitForFinished(300);
        }
        delete m_process;
        m_process = nullptr;
    }

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

    disconnect(m_process, nullptr, this, nullptr);
    m_process->kill();
    m_process->waitForFinished(500);
    delete m_process;
    m_process = nullptr;
}

void YtDlpDownloader::resume() {
    if (m_isRunning) return;
    m_isPaused = false;
    start();
}

void YtDlpDownloader::cancel() {
    m_isPaused = false;
    m_isRunning = false;
    emit runningChanged();

    if (m_process) {
        disconnect(m_process, nullptr, this, nullptr);
        if (m_process->state() != QProcess::NotRunning) {
            m_process->kill();
            m_process->waitForFinished(500);
        }
        delete m_process;
        m_process = nullptr;
    }

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
            bool pctOk = false;
            double pct = pctStr.toDouble(&pctOk);
            QString speed = parts[1].trimmed();
            QString eta = parts[2].trimmed();
            bool downOk = false;
            qint64 down = parts[3].trimmed().toLongLong(&downOk);
            bool totOk = false;
            qint64 tot = parts[4].trimmed().toLongLong(&totOk);

            if (pctOk && pct >= 0.0) {
                m_progress = pct;
            }
            if (!speed.isEmpty() && speed != "NA" && speed != "Unknown B/s") {
                m_speedString = speed;
            }
            if (!eta.isEmpty() && eta != "NA") {
                m_etaString = eta;
            }

            emit progressUpdated(m_progress, m_speedString, m_etaString,
                                 downOk ? down : -1,
                                 totOk ? tot : -1);
            return;
        }
    }

    // Pattern 2: Standard yt-dlp [download] 45.2% of ~12.34MiB at 2.45MiB/s ETA 00:05
    static const QRegularExpression dlRegex(R"(\[download\]\s+([\d\.]+)%\s+of\s+~?\s*([^\s]+)\s+at\s+([^\s]+)\s+ETA\s+([^\s]+))");
    auto dlMatch = dlRegex.match(line);
    if (dlMatch.hasMatch()) {
        bool pctOk = false;
        double pct = dlMatch.captured(1).toDouble(&pctOk);
        if (pctOk) m_progress = pct;
        m_speedString = dlMatch.captured(3);
        m_etaString = dlMatch.captured(4);
        emit progressUpdated(m_progress, m_speedString, m_etaString, -1, -1);
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
