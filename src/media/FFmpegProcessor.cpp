#include "FFmpegProcessor.h"
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QRegularExpression>

FFmpegProcessor::FFmpegProcessor(QObject *parent)
    : QObject(parent)
{
}

FFmpegProcessor::~FFmpegProcessor() {
    cancel();
}

#include <QDirIterator>

QString FFmpegProcessor::findFFmpegExecutable() {
    // 1. Check in application directory
    QString localApp = QCoreApplication::applicationDirPath() + "/ffmpeg.exe";
    if (QFileInfo::exists(localApp)) {
        return QDir::toNativeSeparators(localApp);
    }

    // 2. Check in current directory or build subdirectory
    QString currentApp = QDir::currentPath() + "/ffmpeg.exe";
    if (QFileInfo::exists(currentApp)) {
        return QDir::toNativeSeparators(currentApp);
    }
    QString currentBuild = QDir::currentPath() + "/build/ffmpeg.exe";
    if (QFileInfo::exists(currentBuild)) {
        return QDir::toNativeSeparators(currentBuild);
    }

    // 3. Check in parent build directory
    QString parentBuild = QCoreApplication::applicationDirPath() + "/../build/ffmpeg.exe";
    if (QFileInfo::exists(parentBuild)) {
        return QDir::cleanPath(parentBuild);
    }

    // 4. Check in PATH
    QString pathExe = QStandardPaths::findExecutable("ffmpeg");
    if (!pathExe.isEmpty()) {
        return pathExe;
    }

    // 5. Check WinGet Links and common install locations
    QStringList potentialPaths = {
        QDir::homePath() + "/AppData/Local/Microsoft/WinGet/Links/ffmpeg.exe",
        "C:/ffmpeg/bin/ffmpeg.exe",
        "C:/Program Files/ffmpeg/bin/ffmpeg.exe",
        QDir::homePath() + "/AppData/Local/Programs/ffmpeg/bin/ffmpeg.exe"
    };

    for (const QString &p : potentialPaths) {
        if (QFileInfo::exists(p)) {
            return QDir::toNativeSeparators(p);
        }
    }

    // 6. Search WinGet Packages directory
    QDir wingetDir(QDir::homePath() + "/AppData/Local/Microsoft/WinGet/Packages");
    if (wingetDir.exists()) {
        QDirIterator it(wingetDir.absolutePath(), QStringList() << "ffmpeg.exe", QDir::Files, QDirIterator::Subdirectories);
        if (it.hasNext()) {
            return QDir::toNativeSeparators(it.next());
        }
    }

    return QString();
}

QString FFmpegProcessor::findYtDlpExecutable() {
    // 1. Check in application directory
    QString localApp = QCoreApplication::applicationDirPath() + "/yt-dlp.exe";
    if (QFileInfo::exists(localApp)) {
        return QDir::toNativeSeparators(localApp);
    }

    // 2. Check in current directory or build subdirectory
    QString currentApp = QDir::currentPath() + "/yt-dlp.exe";
    if (QFileInfo::exists(currentApp)) {
        return QDir::toNativeSeparators(currentApp);
    }
    QString currentBuild = QDir::currentPath() + "/build/yt-dlp.exe";
    if (QFileInfo::exists(currentBuild)) {
        return QDir::toNativeSeparators(currentBuild);
    }

    // 3. Check in parent build directory
    QString parentBuild = QCoreApplication::applicationDirPath() + "/../build/yt-dlp.exe";
    if (QFileInfo::exists(parentBuild)) {
        return QDir::cleanPath(parentBuild);
    }

    // 4. Check in PATH
    QString pathExe = QStandardPaths::findExecutable("yt-dlp");
    if (!pathExe.isEmpty()) {
        return pathExe;
    }

    // 5. Check WinGet Links
    QString wingetLink = QDir::homePath() + "/AppData/Local/Microsoft/WinGet/Links/yt-dlp.exe";
    if (QFileInfo::exists(wingetLink)) {
        return QDir::toNativeSeparators(wingetLink);
    }

    // 6. Search WinGet Packages directory
    QDir wingetDir(QDir::homePath() + "/AppData/Local/Microsoft/WinGet/Packages");
    if (wingetDir.exists()) {
        QDirIterator it(wingetDir.absolutePath(), QStringList() << "yt-dlp.exe", QDir::Files, QDirIterator::Subdirectories);
        if (it.hasNext()) {
            return QDir::toNativeSeparators(it.next());
        }
    }

    return QString();
}

bool FFmpegProcessor::isFFmpegAvailable() const {
    return !findFFmpegExecutable().isEmpty();
}

bool FFmpegProcessor::isYtDlpAvailable() const {
    return !findYtDlpExecutable().isEmpty();
}

void FFmpegProcessor::refreshTools() {
    emit toolsStatusChanged();
}

void FFmpegProcessor::extractAudio(const QString &inputFilePath, const QString &outputFilePath, const QString &format, int bitrateKbps) {
    if (m_isProcessing) {
        emit finished(false, "", "Another conversion process is already running.");
        return;
    }

    QString ffmpegPath = findFFmpegExecutable();
    if (ffmpegPath.isEmpty()) {
        emit finished(false, "", "FFmpeg was not found. Please ensure FFmpeg is installed.");
        return;
    }

    m_currentOutputFile = outputFilePath;
    m_progress = 0.0;
    m_totalDurationSec = 0.0;
    m_isProcessing = true;
    m_statusMessage = "Converting audio to " + format.toUpper() + "...";
    emit processingChanged();
    emit statusChanged(m_statusMessage);

    QStringList args;
    args << "-i" << inputFilePath;
    args << "-vn"; // No video

    if (format.toLower() == "mp3") {
        args << "-acodec" << "libmp3lame";
        args << "-b:a" << QString::number(bitrateKbps) + "k";
    } else if (format.toLower() == "wav") {
        args << "-acodec" << "pcm_s16le";
    } else if (format.toLower() == "aac" || format.toLower() == "m4a") {
        args << "-acodec" << "aac";
        args << "-b:a" << QString::number(bitrateKbps) + "k";
    } else {
        args << "-acodec" << "copy";
    }

    args << "-y" << outputFilePath; // Overwrite

    m_process = new QProcess(this);
    connect(m_process, &QProcess::readyReadStandardError, this, &FFmpegProcessor::onProcessReadyRead);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &FFmpegProcessor::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &FFmpegProcessor::onProcessError);

    m_process->start(ffmpegPath, args);
}

void FFmpegProcessor::mergeStreams(const QString &videoPath, const QString &audioPath, const QString &outputPath) {
    if (m_isProcessing) return;

    QString ffmpegPath = findFFmpegExecutable();
    if (ffmpegPath.isEmpty()) {
        emit finished(false, "", "FFmpeg was not found.");
        return;
    }

    m_currentOutputFile = outputPath;
    m_progress = 0.0;
    m_totalDurationSec = 0.0;
    m_isProcessing = true;
    m_statusMessage = "Merging video and audio streams...";
    emit processingChanged();

    QStringList args;
    args << "-i" << videoPath;
    args << "-i" << audioPath;
    args << "-c:v" << "copy";
    args << "-c:a" << "aac";
    args << "-y" << outputPath;

    m_process = new QProcess(this);
    connect(m_process, &QProcess::readyReadStandardError, this, &FFmpegProcessor::onProcessReadyRead);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &FFmpegProcessor::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &FFmpegProcessor::onProcessError);

    m_process->start(ffmpegPath, args);
}

void FFmpegProcessor::cancel() {
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(1000);
    }
    m_isProcessing = false;
    emit processingChanged();
}

void FFmpegProcessor::onProcessReadyRead() {
    if (!m_process) return;

    QString output = QString::fromUtf8(m_process->readAllStandardError());

    if (m_totalDurationSec <= 0.0) {
        parseDurationFromOutput(output);
    }
    parseTimeFromOutput(output);
}

void FFmpegProcessor::parseDurationFromOutput(const QString &text) {
    // Look for "Duration: 00:03:45.12"
    QRegularExpression re(R"(Duration:\s*(\d{2}):(\d{2}):(\d{2}(?:\.\d+)?))");
    auto match = re.match(text);
    if (match.hasMatch()) {
        double hours = match.captured(1).toDouble();
        double mins = match.captured(2).toDouble();
        double secs = match.captured(3).toDouble();
        m_totalDurationSec = hours * 3600.0 + mins * 60.0 + secs;
    }
}

void FFmpegProcessor::parseTimeFromOutput(const QString &text) {
    // Look for "time=00:01:23.45"
    QRegularExpression re(R"(time=\s*(\d{2}):(\d{2}):(\d{2}(?:\.\d+)?))");
    auto match = re.match(text);
    if (match.hasMatch() && m_totalDurationSec > 0.0) {
        double hours = match.captured(1).toDouble();
        double mins = match.captured(2).toDouble();
        double secs = match.captured(3).toDouble();
        double currentSec = hours * 3600.0 + mins * 60.0 + secs;

        double pct = (currentSec / m_totalDurationSec) * 100.0;
        if (pct > 100.0) pct = 100.0;
        m_progress = pct;
        emit progressChanged(pct);
    }
}

void FFmpegProcessor::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    m_isProcessing = false;
    emit processingChanged();

    bool ok = (exitStatus == QProcess::NormalExit && exitCode == 0);
    if (ok) {
        m_progress = 100.0;
        emit progressChanged(100.0);
        emit finished(true, m_currentOutputFile, "");
    } else {
        emit finished(false, "", QString("Media processing failed with exit code: %1").arg(exitCode));
    }

    if (m_process) {
        m_process->deleteLater();
        m_process = nullptr;
    }
}

void FFmpegProcessor::onProcessError(QProcess::ProcessError error) {
    Q_UNUSED(error);
    m_isProcessing = false;
    emit processingChanged();
    emit finished(false, "", "An error occurred while executing FFmpeg.");

    if (m_process) {
        m_process->deleteLater();
        m_process = nullptr;
    }
}
