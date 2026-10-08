#include "UrlAnalyzer.h"
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QFileInfo>
#include <QUrlQuery>

UrlAnalyzer::UrlAnalyzer(QObject *parent)
    : QObject(parent)
{
}

UrlAnalyzer::~UrlAnalyzer() {
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }
}

UrlCategory UrlAnalyzer::categorizeUrl(const QString &rawUrl) {
    QString lower = rawUrl.trimmed().toLower();
    if (lower.endsWith(".mp4") || lower.endsWith(".webm") || lower.endsWith(".mkv") || lower.contains(".mp4?") || lower.contains(".webm?")) {
        return UrlCategory::DirectVideo;
    }
    if (lower.endsWith(".mp3") || lower.endsWith(".m4a") || lower.endsWith(".wav") || lower.endsWith(".aac") || lower.contains(".mp3?")) {
        return UrlCategory::DirectAudio;
    }
    if (lower.endsWith(".m3u8") || lower.contains(".m3u8?")) {
        return UrlCategory::HlsStream;
    }
    if (lower.contains("youtube.com") || lower.contains("youtu.be") ||
        lower.contains("twitter.com") || lower.contains("x.com") ||
        lower.contains("tiktok.com") || lower.contains("instagram.com") ||
        lower.contains("vimeo.com")) {
        return UrlCategory::GenericWebPage;
    }
    return UrlCategory::Unknown;
}

void UrlAnalyzer::analyzeUrl(const QString &rawUrl) {
    QString trimmed = rawUrl.trimmed();
    if (trimmed.isEmpty()) {
        emit analysisError(QString::fromUtf8("Lütfen geçerli bir URL girin."));
        return;
    }

    QUrl url(trimmed);
    if (!url.isValid() || (url.scheme() != "http" && url.scheme() != "https")) {
        emit analysisError(QString::fromUtf8("Geçersiz URL formatı. 'http://' veya 'https://' ile başlamalıdır."));
        return;
    }

    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }

    m_isAnalyzing = true;
    emit analyzingChanged();

    startDirectProbe(url);
}

void UrlAnalyzer::startDirectProbe(const QUrl &url) {
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36");

    m_currentReply = m_netManager.head(request);
    connect(m_currentReply, &QNetworkReply::finished, this, &UrlAnalyzer::onHeadFinished);
}

void UrlAnalyzer::onHeadFinished() {
    if (!m_currentReply) return;

    m_isAnalyzing = false;
    emit analyzingChanged();

    if (m_currentReply->error() != QNetworkReply::NoError) {
        // If HEAD is blocked by server (405 Method Not Allowed), retry with GET range 0-0
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
        emit analysisError(QString::fromUtf8("URL analizi başarısız oldu: ") + errStr);
        return;
    }

    // Extract headers
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
    QString base = fi.fileName();
    if (!base.isEmpty()) {
        return base;
    }

    return "downloaded_media_" + QString::number(QDateTime::currentSecsSinceEpoch());
}

void UrlAnalyzer::populateDirectFileOptions(MediaInfo *info, qint64 sizeBytes, const QString &mimeType, const QString &fileName) {
    QList<MediaStreamOption> options;

    QString sizeStr;
    if (sizeBytes > 0) {
        sizeStr = QString::asprintf("%.1f MB", sizeBytes / (1024.0 * 1024.0));
    } else {
        sizeStr = QString::fromUtf8("Bilinmiyor");
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
    original.resolution = QString::fromUtf8("Orijinal Kalite (En Yüksek)");
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
        audioOpt.resolution = QString::fromUtf8("Yalnızca Ses (MP3 Dönüştür)");
        audioOpt.container = "mp3";
        audioOpt.codec = "MP3 (320 kbps)";
        audioOpt.sizeBytes = sizeBytes > 0 ? (sizeBytes / 4) : 0;
        audioOpt.sizeFormatted = sizeBytes > 0 ? QString::asprintf("~%.1f MB", (sizeBytes / 4) / (1024.0 * 1024.0)) : QString::fromUtf8("Tahmini");
        audioOpt.isAudioOnly = true;
        options.append(audioOpt);
    }

    info->setStreamOptions(options);
}
