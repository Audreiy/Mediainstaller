#include "UrlAnalyzer.h"
#include "FFmpegProcessor.h"
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QFileInfo>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QDebug>

UrlAnalyzer::UrlAnalyzer(QObject *parent)
    : QObject(parent)
{
}

UrlAnalyzer::~UrlAnalyzer() {
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }
    if (m_ytDlpProcess) {
        m_ytDlpProcess->kill();
        m_ytDlpProcess->deleteLater();
    }
}

UrlCategory UrlAnalyzer::categorizeUrl(const QString &rawUrl) {
    QString lower = rawUrl.trimmed().toLower();
    if (lower.endsWith(".mp4") || lower.endsWith(".webm") || lower.endsWith(".mkv") ||
        lower.contains(".mp4?") || lower.contains(".webm?")) {
        return UrlCategory::DirectVideo;
    }
    if (lower.endsWith(".mp3") || lower.endsWith(".m4a") || lower.endsWith(".wav") ||
        lower.endsWith(".aac") || lower.contains(".mp3?")) {
        return UrlCategory::DirectAudio;
    }
    if (lower.endsWith(".m3u8") || lower.contains(".m3u8?")) {
        return UrlCategory::HlsStream;
    }
    if (lower.contains("youtube.com") || lower.contains("youtu.be") ||
        lower.contains("twitter.com") || lower.contains("x.com") ||
        lower.contains("tiktok.com") || lower.contains("instagram.com") ||
        lower.contains("vimeo.com") || lower.contains("reddit.com") ||
        lower.contains("soundcloud.com") || lower.contains("twitch.tv") ||
        lower.contains("facebook.com")) {
        return UrlCategory::GenericWebPage;
    }
    return UrlCategory::Unknown;
}

void UrlAnalyzer::analyzeUrl(const QString &rawUrl) {
    QString trimmed = rawUrl.trimmed();
    if (trimmed.isEmpty()) {
        emit analysisError("Please enter a valid URL.");
        return;
    }

    QUrl url(trimmed);
    if (!url.isValid() || (url.scheme() != "http" && url.scheme() != "https")) {
        emit analysisError("Invalid URL format. URL must start with 'http://' or 'https://'.");
        return;
    }

    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }

    if (m_ytDlpProcess) {
        m_ytDlpProcess->kill();
        m_ytDlpProcess->deleteLater();
        m_ytDlpProcess = nullptr;
    }

    m_analyzingUrl = trimmed;
    m_isAnalyzing = true;
    emit analyzingChanged();

    UrlCategory cat = categorizeUrl(trimmed);
    QString lower = trimmed.toLower();

    if (lower.contains("youtube.com") || lower.contains("youtu.be")) {
        startYouTubeOembedProbe(url);
    } else if (cat == UrlCategory::GenericWebPage) {
        startYtDlpProbe(trimmed);
    } else if (cat == UrlCategory::DirectVideo || cat == UrlCategory::DirectAudio) {
        startDirectProbe(url);
    } else {
        // Unknown: try yt-dlp first if available, else probe directly
        if (!FFmpegProcessor::findYtDlpExecutable().isEmpty()) {
            startYtDlpProbe(trimmed);
        } else {
            startDirectProbe(url);
        }
    }
}

void UrlAnalyzer::startYouTubeOembedProbe(const QUrl &url) {
    QString encodedUrl = QUrl::toPercentEncoding(url.toString());
    QString oembedUrlStr = "https://www.youtube.com/oembed?url=" + encodedUrl + "&format=json";

    QUrl oembedUrl(oembedUrlStr);
    QNetworkRequest request(oembedUrl);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) MediaGrab/1.0");

    m_currentReply = m_netManager.get(request);
    connect(m_currentReply, &QNetworkReply::finished, this, &UrlAnalyzer::onOembedFinished);
}

void UrlAnalyzer::onOembedFinished() {
    if (!m_currentReply) return;

    int statusCode = m_currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray data = m_currentReply->readAll();
    m_currentReply->deleteLater();
    m_currentReply = nullptr;

    if (statusCode == 200 && !data.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            QString title = obj.value("title").toString().trimmed();
            QString author = obj.value("author_name").toString().trimmed();
            QString thumb = obj.value("thumbnail_url").toString().trimmed();

            if (!title.isEmpty()) {
                m_isAnalyzing = false;
                emit analyzingChanged();

                if (m_currentInfo) {
                    m_currentInfo->deleteLater();
                }
                m_currentInfo = new MediaInfo(this);
                m_currentInfo->setUrl(m_analyzingUrl);
                m_currentInfo->setTitle(title);
                m_currentInfo->setAuthor(author);
                m_currentInfo->setThumbnailUrl(thumb);
                m_currentInfo->setIsDirectFile(false);

                populateWebStreamOptions(m_currentInfo);

                emit mediaInfoChanged();
                emit analysisSuccess(m_currentInfo);
                return;
            }
        }
    }

    // If oEmbed fails or is private/blocked, fallback to yt-dlp probe
    startYtDlpProbe(m_analyzingUrl);
}

void UrlAnalyzer::startYtDlpProbe(const QString &rawUrl) {
    QString ytDlpExe = FFmpegProcessor::findYtDlpExecutable();
    if (ytDlpExe.isEmpty()) {
        // Fallback to direct probe if yt-dlp isn't installed
        startDirectProbe(QUrl(rawUrl));
        return;
    }

    m_ytDlpProcess = new QProcess(this);
    QStringList args;
    args << "--print" << "%(title)s|||%(uploader)s|||%(duration_string)s|||%(thumbnail)s";
    args << "--no-warnings";
    args << "--no-playlist";
    args << rawUrl;

    connect(m_ytDlpProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &UrlAnalyzer::onYtDlpFinished);

    m_ytDlpProcess->start(ytDlpExe, args);
}

void UrlAnalyzer::onYtDlpFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    m_isAnalyzing = false;
    emit analyzingChanged();

    if (!m_ytDlpProcess) return;

    QString output = QString::fromUtf8(m_ytDlpProcess->readAllStandardOutput()).trimmed();
    m_ytDlpProcess->deleteLater();
    m_ytDlpProcess = nullptr;

    if (exitStatus == QProcess::NormalExit && exitCode == 0 && !output.isEmpty()) {
        QStringList parts = output.split("|||");
        QString title = parts.value(0).trimmed();
        QString author = (parts.size() > 1) ? parts.value(1).trimmed() : "";
        QString duration = (parts.size() > 2) ? parts.value(2).trimmed() : "";
        QString thumbnail = (parts.size() > 3) ? parts.value(3).trimmed() : "";

        if (!title.isEmpty()) {
            if (m_currentInfo) {
                m_currentInfo->deleteLater();
            }
            m_currentInfo = new MediaInfo(this);
            m_currentInfo->setUrl(m_analyzingUrl);
            m_currentInfo->setTitle(title);
            m_currentInfo->setAuthor(author);
            m_currentInfo->setDuration(duration);
            m_currentInfo->setThumbnailUrl(thumbnail);
            m_currentInfo->setIsDirectFile(false);

            populateWebStreamOptions(m_currentInfo);

            emit mediaInfoChanged();
            emit analysisSuccess(m_currentInfo);
            return;
        }
    }

    // If both failed, try direct probe as final fallback
    startDirectProbe(QUrl(m_analyzingUrl));
}

void UrlAnalyzer::startDirectProbe(const QUrl &url) {
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) Chrome/128.0.0.0 Safari/537.36");

    m_currentReply = m_netManager.head(request);
    connect(m_currentReply, &QNetworkReply::finished, this, &UrlAnalyzer::onHeadFinished);
}

void UrlAnalyzer::onHeadFinished() {
    if (!m_currentReply) return;

    m_isAnalyzing = false;
    emit analyzingChanged();

    if (m_currentReply->error() != QNetworkReply::NoError) {
        int httpCode = m_currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (httpCode == 405) {
            QUrl targetUrl = m_currentReply->url();
            m_currentReply->deleteLater();
            m_currentReply = nullptr;

            QNetworkRequest getReq(targetUrl);
            getReq.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64)");
            getReq.setRawHeader("Range", "bytes=0-1024");
            getReq.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

            m_isAnalyzing = true;
            emit analyzingChanged();
            m_currentReply = m_netManager.get(getReq);
            connect(m_currentReply, &QNetworkReply::finished, this, &UrlAnalyzer::onHeadFinished);
            return;
        }

        QString errStr = m_currentReply->errorString();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
        emit analysisError("URL analysis failed: " + errStr);
        return;
    }

    QUrl finalUrl = m_currentReply->url();
    qint64 contentLength = m_currentReply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
    QString contentType = m_currentReply->header(QNetworkRequest::ContentTypeHeader).toString();
    QString contentDisposition = m_currentReply->rawHeader("Content-Disposition");

    m_currentReply->deleteLater();
    m_currentReply = nullptr;

    if (m_currentInfo) {
        m_currentInfo->deleteLater();
    }
    m_currentInfo = new MediaInfo(this);
    m_currentInfo->setUrl(finalUrl.toString());

    QString detectedFileName = extractFileName(finalUrl, contentDisposition);
    m_currentInfo->setTitle(detectedFileName);
    m_currentInfo->setIsDirectFile(true);

    populateDirectFileOptions(m_currentInfo, contentLength, contentType, detectedFileName);

    emit mediaInfoChanged();
    emit analysisSuccess(m_currentInfo);
}

QString UrlAnalyzer::extractFileName(const QUrl &url, const QString &contentDisposition) {
    if (!contentDisposition.isEmpty()) {
        QRegularExpression re(R"(filename\*?=(?:UTF-8'')?["']?([^"';]+)["']?)");
        auto match = re.match(contentDisposition);
        if (match.hasMatch()) {
            return match.captured(1).trimmed();
        }
    }

    QString path = url.path();
    QFileInfo fi(path);
    QString base = fi.fileName().trimmed();

    // Never use "watch" or empty base name for video files!
    if (!base.isEmpty() && base.toLower() != "watch" && base.toLower() != "watch.mp4") {
        return base;
    }

    // If it's a YouTube URL with ?v=..., use the video ID
    QUrlQuery query(url);
    if (query.hasQueryItem("v")) {
        return "YouTube_Video_" + query.queryItemValue("v");
    }

    return "Downloaded_Media_" + QString::number(QDateTime::currentSecsSinceEpoch());
}

void UrlAnalyzer::populateDirectFileOptions(MediaInfo *info, qint64 sizeBytes, const QString &mimeType, const QString &fileName) {
    QList<MediaStreamOption> options;

    QString sizeStr;
    if (sizeBytes > 0) {
        sizeStr = QString::asprintf("%.1f MB", sizeBytes / (1024.0 * 1024.0));
    } else {
        sizeStr = "Direct File";
    }

    QString ext = QFileInfo(fileName).suffix().toLower();
    if (ext.isEmpty()) {
        if (mimeType.contains("mp4")) ext = "mp4";
        else if (mimeType.contains("audio/mpeg")) ext = "mp3";
        else ext = "mp4";
    }

    // Option 1: Original stream
    MediaStreamOption original;
    original.id = "original";
    original.resolution = "Original Quality (Highest)";
    original.container = ext;
    original.codec = mimeType.isEmpty() ? "Standard" : mimeType;
    original.sizeBytes = sizeBytes;
    original.sizeFormatted = sizeStr;
    original.isAudioOnly = (ext == "mp3" || ext == "m4a" || ext == "wav" || ext == "aac");
    options.append(original);

    // Option 2: Extract audio as MP3
    if (!original.isAudioOnly) {
        MediaStreamOption audioOpt;
        audioOpt.id = "audio_mp3";
        audioOpt.resolution = "Audio Only (320 kbps MP3)";
        audioOpt.container = "mp3";
        audioOpt.codec = "MP3 (320 kbps)";
        audioOpt.sizeBytes = sizeBytes > 0 ? (sizeBytes / 4) : 0;
        audioOpt.sizeFormatted = sizeBytes > 0 ? QString::asprintf("~%.1f MB", (sizeBytes / 4) / (1024.0 * 1024.0)) : "Estimated";
        audioOpt.isAudioOnly = true;
        options.append(audioOpt);
    }

    info->setStreamOptions(options);
}

void UrlAnalyzer::populateWebStreamOptions(MediaInfo *info) {
    QList<MediaStreamOption> options;

    // Option 1: Highest resolution video
    MediaStreamOption bestVideo;
    bestVideo.id = "best_video";
    bestVideo.resolution = "Video (MP4 / Best Quality)";
    bestVideo.container = "mp4";
    bestVideo.codec = "H.264 / AAC (HD/4K)";
    bestVideo.sizeFormatted = "Best Quality";
    bestVideo.isAudioOnly = false;
    options.append(bestVideo);

    // Option 2: Studio Quality Audio (MP3 320 kbps)
    MediaStreamOption audioMp3;
    audioMp3.id = "audio_mp3";
    audioMp3.resolution = "Audio Only (320 kbps MP3)";
    audioMp3.container = "mp3";
    audioMp3.codec = "MP3 (320 kbps HQ)";
    audioMp3.sizeFormatted = "Studio Quality";
    audioMp3.isAudioOnly = true;
    options.append(audioMp3);

    // Option 3: Original Audio Stream (M4A)
    MediaStreamOption audioM4a;
    audioM4a.id = "audio_m4a";
    audioM4a.resolution = "Audio Only (Original M4A)";
    audioM4a.container = "m4a";
    audioM4a.codec = "AAC Original";
    audioM4a.sizeFormatted = "Original Stream";
    audioM4a.isAudioOnly = true;
    options.append(audioM4a);

    info->setStreamOptions(options);
}
