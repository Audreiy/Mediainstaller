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

    m_errorLogs.clear();
    m_lastErrorMessage.clear();

    QString ytDlpExe = FFmpegProcessor::findYtDlpExecutable();
    if (ytDlpExe.isEmpty()) {
        emit downloadFailed(m_id, "yt-dlp executable not found. Please ensure yt-dlp is installed.", "yt-dlp executable could not be found in application directory or system PATH.");
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
    args << "--socket-timeout" << "30";
    args << "--retries" << "5";
    if (!m_impersonateRetried) {
        args << "--impersonate" << "Chrome";
    }

    if (m_speedLimitMBps > 0) {
        args << "--limit-rate" << QString("%1M").arg(m_speedLimitMBps);
    }

    QString encodedUrl = QString::fromUtf8(QUrl::fromUserInput(m_url.trimmed()).toEncoded(QUrl::FullyEncoded));
    if (encodedUrl.isEmpty()) encodedUrl = m_url.trimmed();

    QString lowerUrl = encodedUrl.toLower();
    bool isYouTube = lowerUrl.contains("youtube.com") || lowerUrl.contains("youtu.be");
    bool isSocial = lowerUrl.contains("instagram.com") || lowerUrl.contains("tiktok.com") ||
                    lowerUrl.contains("twitter.com") || lowerUrl.contains("x.com") ||
                    lowerUrl.contains("facebook.com") || lowerUrl.contains("fb.watch");

    if (!m_referer.isEmpty()) {
        QString encodedRef = QString::fromUtf8(QUrl::fromUserInput(m_referer.trimmed()).toEncoded(QUrl::FullyEncoded));
        args << "--referer" << (encodedRef.isEmpty() ? m_referer : encodedRef);
    } else if (!isSocial && !isYouTube) {
        args << "--referer" << encodedUrl;
    }

    if (!ffmpegExe.isEmpty()) {
        args << "--ffmpeg-location" << QFileInfo(ffmpegExe).absolutePath();
    }

    if (m_formatId.startsWith("audio_mp3") || m_formatId == "audio_mp3") {
        QString bitrate = "320k";
        if (m_formatId.contains("256")) bitrate = "256k";
        else if (m_formatId.contains("192")) bitrate = "192k";
        else if (m_formatId.contains("128")) bitrate = "128k";
        args << "-f" << "bestaudio/best";
        args << "-x" << "--audio-format" << "mp3" << "--audio-quality" << bitrate;
    } else if (m_formatId == "audio_m4a") {
        args << "-f" << "bestaudio/best";
        args << "-x" << "--audio-format" << "m4a";
    } else if (m_formatId.startsWith("video_") && m_formatId != "best_video") {
        int targetHeight = 1080;
        if (m_formatId == "video_4k") {
            targetHeight = 2160;
        } else {
            QString numStr = m_formatId;
            numStr.remove("video_").remove("p");
            targetHeight = numStr.toInt();
            if (targetHeight <= 0) targetHeight = 1080;
        }
        args << "-f" << QString("bestvideo[height<=%1]+bestaudio/best[height<=%1]/bestvideo+bestaudio/best").arg(targetHeight);
        args << "--merge-output-format" << "mp4";
    } else {
        // High quality video with merged audio (Best available)
        args << "-f" << "bestvideo+bestaudio/best";
        args << "--merge-output-format" << "mp4";
    }

    // Output template: use %(ext)s so yt-dlp correctly writes the target extension without duplication
    QFileInfo fi(m_targetPath);
    QString templatePath = fi.dir().filePath(fi.completeBaseName() + ".%(ext)s");
    args << "-o" << templatePath;

    // Progress template: DOWNLOAD_PROGRESS:percent|speed|eta|downloaded_bytes|total_bytes|total_bytes_estimate
    args << "--progress-template" << "DOWNLOAD_PROGRESS:%(progress._percent_str)s|%(progress._speed_str)s|%(progress._eta_str)s|%(progress.downloaded_bytes)s|%(progress.total_bytes)s|%(progress.total_bytes_estimate)s";

    args << encodedUrl;

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
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PYTHONIOENCODING", "utf-8");
    env.insert("PYTHONUTF8", "1");
    m_process->setProcessEnvironment(env);

    connect(m_process, &QProcess::readyReadStandardOutput, this, &YtDlpDownloader::onReadyReadStandardOutput);
    connect(m_process, &QProcess::readyReadStandardError, this, &YtDlpDownloader::onReadyReadStandardError);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &YtDlpDownloader::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &YtDlpDownloader::onProcessError);

    m_progressTimer.start();
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
        if (!line.isEmpty()) {
            m_errorLogs.append(line);
            if (m_errorLogs.size() > 60) {
                m_errorLogs.removeFirst();
            }
        }
        parseOutputLine(line);
    }
}

void YtDlpDownloader::parseOutputLine(const QString &line) {
    if (line.isEmpty()) return;

    // Detect errors from yt-dlp output
    if (line.contains("ERROR:", Qt::CaseInsensitive)) {
        int idx = line.indexOf("ERROR:", 0, Qt::CaseInsensitive);
        if (idx != -1) {
            QString err = line.mid(idx + 6).trimmed();
            if (err.startsWith("[")) {
                int endBracket = err.indexOf("]");
                if (endBracket != -1) {
                    err = err.mid(endBracket + 1).trimmed();
                }
            }
            if (!err.isEmpty()) {
                m_lastErrorMessage = err;
            }
        }
    }

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
            if (!totOk && parts.size() > 5) {
                tot = parts[5].trimmed().toLongLong(&totOk);
            }

            if (pctOk && pct >= 0.0) {
                m_progress = pct;
            }
            if (!speed.isEmpty() && speed != "NA" && speed != "Unknown B/s") {
                m_speedString = speed;
            }
            if (!eta.isEmpty() && eta != "NA") {
                m_etaString = eta;
            }

            if (m_progressTimer.hasExpired(250) || m_progress >= 99.0) {
                m_progressTimer.restart();
                emit progressUpdated(m_progress, m_speedString, m_etaString,
                                     downOk ? down : -1,
                                     totOk ? tot : -1);
            }
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
        if (m_progressTimer.hasExpired(250) || m_progress >= 99.0) {
            m_progressTimer.restart();
            emit progressUpdated(m_progress, m_speedString, m_etaString, -1, -1);
        }
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

        // Verify destination file exists and get real completed size
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

        qint64 finalSize = QFileInfo(finalPath).size();
        if (finalSize > 0) {
            emit progressUpdated(100.0, "Done", "00:00", finalSize, finalSize);
            emit downloadCompleted(m_id, finalPath);
        } else {
            if (QFile::exists(finalPath)) {
                QFile::remove(finalPath);
            }
            QString errorReason = "Download failed: Resulting media file is empty (0 bytes).";
            QString details = m_errorLogs.join("\n");
            emit downloadFailed(m_id, errorReason, details);
        }
    } else {
        if (!m_impersonateRetried) {
            m_impersonateRetried = true;
            m_isRunning = true;
            if (m_process) {
                m_process->deleteLater();
                m_process = nullptr;
            }
            start();
            return;
        }

        QString errorReason = m_lastErrorMessage;
        if (errorReason.isEmpty()) {
            errorReason = QString("Download failed (Process exit code: %1)").arg(exitCode);
        }
        QString details = m_errorLogs.join("\n");
        emit downloadFailed(m_id, errorReason, details);
    }

    if (m_process) {
        m_process->deleteLater();
        m_process = nullptr;
    }
}

void YtDlpDownloader::onProcessError(QProcess::ProcessError error) {
    if (m_isPaused) return;

    m_isRunning = false;
    emit runningChanged();

    QString errorReason = "yt-dlp encountered an execution error.";
    if (error == QProcess::FailedToStart) {
        errorReason = "yt-dlp failed to start (file missing or blocked by antivirus).";
    } else if (error == QProcess::Crashed) {
        errorReason = "yt-dlp crashed unexpectedly.";
    } else if (error == QProcess::Timedout) {
        errorReason = "yt-dlp operation timed out.";
    }

    if (!m_lastErrorMessage.isEmpty()) {
        errorReason += " (" + m_lastErrorMessage + ")";
    }

    QString details = m_errorLogs.join("\n");
    emit downloadFailed(m_id, errorReason, details);

    if (m_process) {
        m_process->deleteLater();
        m_process = nullptr;
    }
}
