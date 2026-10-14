#include "UrlAnalyzer.h"
#include "FFmpegProcessor.h"
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QFileInfo>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QSet>
#include <algorithm>
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
    if (lower.endsWith(".m3u8") || lower.contains(".m3u8?")) {
        return UrlCategory::HlsStream;
    }
    if (lower.endsWith(".mp4") || lower.endsWith(".webm") || lower.endsWith(".mkv") ||
        lower.contains(".mp4?") || lower.contains(".webm?")) {
        return UrlCategory::DirectVideo;
    }
    if (lower.endsWith(".mp3") || lower.endsWith(".m4a") || lower.endsWith(".wav") ||
        lower.endsWith(".aac") || lower.contains(".mp3?")) {
        return UrlCategory::DirectAudio;
    }
    if (lower.contains("youtube.com") || lower.contains("youtu.be") ||
        lower.contains("spotify.com") ||
        lower.contains("animecix.") || lower.contains("vidmoly.") || lower.contains("sibnet.ru") ||
        lower.contains("twitter.com") || lower.contains("x.com") ||
        lower.contains("tiktok.com") || lower.contains("instagram.com") ||
        lower.contains("vimeo.com") || lower.contains("reddit.com") ||
        lower.contains("soundcloud.com") || lower.contains("twitch.tv") ||
        lower.contains("dailymotion.com") || lower.contains("streamtape.com") ||
        lower.contains("dood") || lower.contains("filemoon") ||
        lower.contains("facebook.com") || lower.contains("fb.watch") ||
        lower.contains("pornhub.com") || lower.contains("xvideos.com") ||
        lower.contains("xnxx.com") || lower.contains("redtube.com") ||
        lower.contains("youporn.com") || lower.contains("spankbang.com") ||
        lower.contains("eporner.com") || lower.contains("rumble.com") ||
        lower.contains("bilibili") || lower.contains("vk.com") ||
        lower.contains("streamable.com") || lower.contains("threads.net") ||
        lower.contains("bsky.app") || lower.contains("pinterest.com") ||
        lower.contains("pin.it") || lower.contains("bandcamp.com") ||
        lower.contains("mixcloud.com")) {
        return UrlCategory::GenericWebPage;
    }
    return UrlCategory::Unknown;
}

QString UrlAnalyzer::cleanUrl(const QString &rawUrl) {
    QString trimmed = rawUrl.trimmed();
    if (trimmed.isEmpty()) return trimmed;

    QUrl url(trimmed);
    if (!url.isValid()) return trimmed;

    QUrlQuery query(url);
    if (query.isEmpty()) return trimmed;

    static const QSet<QString> trackingKeys = {
        "utm_source", "utm_medium", "utm_campaign", "utm_term", "utm_content",
        "igsh", "obrf", "si", "fbclid", "subid", "subid1", "subid2", "campaign",
        "ref", "source", "feature", "context", "gclid", "dclid", "msclkid"
    };

    QList<QPair<QString, QString>> keptItems;
    for (const auto &item : query.queryItems()) {
        QString keyLower = item.first.toLower();
        if (!trackingKeys.contains(keyLower)) {
            keptItems.append(item);
        }
    }

    QUrl cleanedUrl = url;
    if (keptItems.isEmpty()) {
        cleanedUrl.setQuery(QString());
    } else {
        QUrlQuery cleanQuery;
        cleanQuery.setQueryItems(keptItems);
        cleanedUrl.setQuery(cleanQuery);
    }

    return cleanedUrl.toString(QUrl::FullyEncoded);
}

void UrlAnalyzer::clear() {
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
    if (m_currentInfo) {
        m_currentInfo->deleteLater();
        m_currentInfo = nullptr;
        emit mediaInfoChanged();
    }
    m_analyzingUrl.clear();
    m_pageReferer.clear();
    if (m_isAnalyzing) {
        m_isAnalyzing = false;
        emit analyzingChanged();
    }
}

void UrlAnalyzer::analyzeUrl(const QString &rawUrl) {
    QString trimmed = cleanUrl(rawUrl);
    if (trimmed.isEmpty()) {
        emit analysisError("Please enter a valid media URL.");
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
    m_pageReferer.clear();
    m_isAnalyzing = true;
    emit analyzingChanged();

    QString lower = trimmed.toLower();

    // 0. Animecix automated URL resolver
    if (lower.contains("animecix.tv") || lower.contains("animecix.com") || lower.contains("animecix.net")) {
        startAnimecixResolve(url);
        return;
    }

    // 0.1 Tau Video direct embed link
    if (lower.contains("tau-video.xyz/embed/")) {
        QRegularExpression tauRe(R"(tau-video\.xyz/embed/([a-f0-9]{24}))", QRegularExpression::CaseInsensitiveOption);
        auto m = tauRe.match(trimmed);
        if (m.hasMatch()) {
            startTauVideoResolve(m.captured(1), "Tau Video", 0, 0, "", "");
            return;
        }
    }

    // 0.2 Spotify music track resolver
    if (lower.contains("spotify.com/")) {
        startSpotifyResolve(url);
        return;
    }

    // 1. YouTube & Direct social media & streaming video platforms
    if (lower.contains("youtube.com") || lower.contains("youtu.be") ||
        lower.contains("instagram.com") || lower.contains("tiktok.com") ||
        lower.contains("twitter.com") || lower.contains("x.com") ||
        lower.contains("facebook.com") || lower.contains("fb.watch") ||
        lower.contains("reddit.com") || lower.contains("twitch.tv") ||
        lower.contains("vimeo.com") || lower.contains("dailymotion.com") ||
        lower.contains("rumble.com") || lower.contains("bilibili") ||
        lower.contains("vk.com") || lower.contains("streamable.com") ||
        lower.contains("threads.net") || lower.contains("bsky.app") ||
        lower.contains("pinterest.com") || lower.contains("pin.it") ||
        lower.contains("soundcloud.com") || lower.contains("bandcamp.com") ||
        lower.contains("mixcloud.com")) {
        startYtDlpProbe(trimmed);
        return;
    }

    // 2. Direct HLS Stream (.m3u8)
    if (lower.contains(".m3u8")) {
        QString baseName = QFileInfo(url.path()).completeBaseName();
        if (baseName.isEmpty() || baseName.toLower() == "master" || baseName.toLower() == "playlist" || baseName.toLower() == "index") {
            baseName = "Stream_Video_" + QString::number(QDateTime::currentSecsSinceEpoch());
        }
        setupStreamMediaInfo(trimmed, baseName, "HLS Stream (M3U8)");
        return;
    }

    // 3. Direct Video / Audio file
    if (lower.endsWith(".mp4") || lower.endsWith(".mp3") || lower.endsWith(".webm") || lower.endsWith(".mkv") || lower.endsWith(".m4a")) {
        startDirectProbe(url);
        return;
    }

    // 4. Any Web Page (Animecix, Vidmoly, Sibnet, streaming platforms, etc.)
    // First, scrape the webpage HTML with intelligent video sniffer
    startWebPageScrape(url);
}

void UrlAnalyzer::startAnimecixResolve(const QUrl &url) {
    QString urlStr = url.toString();

    int titleId = 0;
    int seasonNum = 0;
    int episodeNum = 0;

    QRegularExpression titleRe(R"(animecix\.(?:tv|com|net)/titles/(\d+))", QRegularExpression::CaseInsensitiveOption);
    auto mTitle = titleRe.match(urlStr);
    if (mTitle.hasMatch()) {
        titleId = mTitle.captured(1).toInt();
    }

    QRegularExpression seasonRe(R"(/season/(\d+))", QRegularExpression::CaseInsensitiveOption);
    auto mSeason = seasonRe.match(urlStr);
    if (mSeason.hasMatch()) {
        seasonNum = mSeason.captured(1).toInt();
    }

    QRegularExpression epRe(R"(/episode/(\d+))", QRegularExpression::CaseInsensitiveOption);
    auto mEp = epRe.match(urlStr);
    if (mEp.hasMatch()) {
        episodeNum = mEp.captured(1).toInt();
    }

    if (titleId <= 0) {
        startWebPageScrape(url);
        return;
    }

    int targetSeason = (seasonNum > 0) ? seasonNum : 1;
    int targetEp = (episodeNum > 0) ? episodeNum : 1;
    bool isEpisodeSpecific = (seasonNum > 0 || episodeNum > 0);

    // Step 1: Query batch-download/{titleId}/episodes to extract title, episode name, and poster
    QString episodesApiUrl = QString("https://animecix.tv/secure/batch-download/%1/episodes").arg(titleId);
    QNetworkRequest request((QUrl(episodesApiUrl)));
    request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36");
    request.setRawHeader("Accept", "application/json, text/plain, */*");
    request.setRawHeader("Referer", url.toString().toUtf8());
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = m_netManager.get(request);
    reply->ignoreSslErrors();
    m_currentReply = reply;

    connect(reply, &QNetworkReply::finished, this, [this, reply, titleId, targetSeason, targetEp, isEpisodeSpecific, url]() {
        if (m_currentReply == reply) {
            m_currentReply = nullptr;
        }
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray data = reply->readAll();
        reply->deleteLater();

        QString animeTitle = "Anime";
        QString episodeName;
        QString poster;
        QString tauId;

        if (statusCode == 200 && !data.isEmpty()) {
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isObject()) {
                QJsonObject rootObj = doc.object();
                QJsonObject titleObj = rootObj.value("title").toObject();
                if (!titleObj.value("name").toString().trimmed().isEmpty()) {
                    animeTitle = titleObj.value("name").toString().trimmed();
                }
                if (!titleObj.value("poster").toString().trimmed().isEmpty()) {
                    poster = titleObj.value("poster").toString().trimmed();
                }

                QJsonArray seasons = rootObj.value("seasons").toArray();
                if (!seasons.isEmpty()) {
                    for (const QJsonValue &sVal : seasons) {
                        QJsonObject sObj = sVal.toObject();
                        if (sObj.value("number").toInt() == targetSeason) {
                            QJsonArray episodes = sObj.value("episodes").toArray();
                            for (const QJsonValue &epVal : episodes) {
                                QJsonObject epObj = epVal.toObject();
                                if (epObj.value("episode").toInt() == targetEp) {
                                    episodeName = epObj.value("name").toString().trimmed();
                                    QString epPoster = epObj.value("poster").toString().trimmed();
                                    if (!epPoster.isEmpty()) {
                                        poster = epPoster;
                                    }

                                    QRegularExpression tauPosterRe(R"(tau-video/(?:poster|thb)/([a-f0-9]{24}))", QRegularExpression::CaseInsensitiveOption);
                                    auto m = tauPosterRe.match(poster);
                                    if (m.hasMatch()) {
                                        tauId = m.captured(1);
                                    }
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }

        if (!tauId.isEmpty()) {
            startTauVideoResolve(tauId, animeTitle, targetSeason, targetEp, episodeName, poster);
            return;
        }

        // Step 2: Query secure/videos?titleId={titleId}&page={page}&perPage=200 with multi-page support
        auto allCandidates = std::make_shared<QJsonArray>();
        auto fetchPage = std::make_shared<std::function<void(int)>>();
        *fetchPage = [this, fetchPage, allCandidates, titleId, animeTitle, targetSeason, targetEp, isEpisodeSpecific, episodeName, poster, url](int page) {
            QString videosApiUrl = QString("https://animecix.tv/secure/videos?titleId=%1&page=%2&perPage=200").arg(titleId).arg(page);
            QNetworkRequest vReq((QUrl(videosApiUrl)));
            vReq.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36");
            vReq.setRawHeader("Accept", "application/json, text/plain, */*");
            vReq.setRawHeader("Referer", url.toString().toUtf8());
            vReq.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

            QNetworkReply *vReply = m_netManager.get(vReq);
            vReply->ignoreSslErrors();
            m_currentReply = vReply;

            connect(vReply, &QNetworkReply::finished, this, [this, fetchPage, allCandidates, vReply, titleId, animeTitle, targetSeason, targetEp, isEpisodeSpecific, episodeName, poster, url, page]() {
                if (m_currentReply == vReply) {
                    m_currentReply = nullptr;
                }
                int vStatus = vReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                QByteArray vData = vReply->readAll();
                vReply->deleteLater();

                int lastPage = 1;
                if (vStatus == 200 && !vData.isEmpty()) {
                    QJsonDocument vDoc = QJsonDocument::fromJson(vData);
                    if (vDoc.isObject()) {
                        QJsonObject vObj = vDoc.object();
                        QJsonObject pagination = vObj.value("pagination").toObject();
                        lastPage = pagination.value("last_page").toInt(1);
                        QJsonArray items = pagination.value("data").toArray();
                        if (items.isEmpty()) {
                            items = vObj.value("data").toArray();
                        }

                        for (const QJsonValue &itemVal : items) {
                            QJsonObject item = itemVal.toObject();
                            if (item.value("category").toString().toLower() != "trailer") {
                                if (isEpisodeSpecific) {
                                    if (item.value("season_num").toInt() == targetSeason && item.value("episode_num").toInt() == targetEp) {
                                        allCandidates->append(item);
                                    }
                                } else {
                                    allCandidates->append(item);
                                }
                            }
                        }

                        // Priority 1: Check for Tau Video in filtered candidates
                        for (const QJsonValue &cVal : *allCandidates) {
                            QJsonObject cObj = cVal.toObject();
                            QString embedUrl = cObj.value("url").toString();
                            QRegularExpression tauEmbedRe(R"(tau-video\.xyz/embed/([a-f0-9]{24}))", QRegularExpression::CaseInsensitiveOption);
                            auto m = tauEmbedRe.match(embedUrl);
                            if (m.hasMatch()) {
                                QString vTauId = m.captured(1);
                                startTauVideoResolve(vTauId, animeTitle, targetSeason, targetEp, episodeName, poster);
                                return;
                            }
                        }

                        // If Tau Video was not found yet and more pages exist, fetch next page
                        if (page < lastPage && page < 6) {
                            (*fetchPage)(page + 1);
                            return;
                        }

                        // Priority 2: Check for direct supported web embeds
                        for (const QJsonValue &cVal : *allCandidates) {
                            QJsonObject cObj = cVal.toObject();
                            QString embedUrl = cObj.value("url").toString();
                            if (!embedUrl.isEmpty() &&
                                (embedUrl.contains("sibnet.ru") || embedUrl.contains("vidmoly.") || embedUrl.contains("ok.ru") ||
                                 embedUrl.contains("dailymotion.com") || embedUrl.contains("mp4upload") || embedUrl.contains("dood"))) {
                                m_analyzingUrl = embedUrl;
                                startYtDlpProbe(embedUrl);
                                return;
                            }
                        }
                    }
                }

                // Step 3: Fallback to secure/first-video?titleId={titleId}&season={targetSeason} (useful for episode 1 or standalone movies)
                QString fvUrl = QString("https://animecix.tv/secure/first-video?titleId=%1&season=%2").arg(titleId).arg(targetSeason);
                QNetworkRequest fvReq((QUrl(fvUrl)));
                fvReq.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36");
                fvReq.setRawHeader("Accept", "application/json, text/plain, */*");
                fvReq.setRawHeader("Referer", url.toString().toUtf8());
                fvReq.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

                QNetworkReply *fvReply = m_netManager.get(fvReq);
                fvReply->ignoreSslErrors();
                m_currentReply = fvReply;

                connect(fvReply, &QNetworkReply::finished, this, [this, fvReply, animeTitle, targetSeason, targetEp, isEpisodeSpecific, episodeName, poster, url]() {
                    if (m_currentReply == fvReply) {
                        m_currentReply = nullptr;
                    }
                    int fvStatus = fvReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                    QByteArray fvData = fvReply->readAll();
                    fvReply->deleteLater();

                    QString fvTauId;
                    QString fvEpName = episodeName;
                    QString fvPoster = poster;

                    if (fvStatus == 200 && !fvData.isEmpty()) {
                        QJsonDocument fvDoc = QJsonDocument::fromJson(fvData);
                        if (fvDoc.isObject()) {
                            QJsonObject fvObj = fvDoc.object();
                            QString embedUrl = fvObj.value("url").toString();
                            QRegularExpression tauEmbedRe(R"(tau-video\.xyz/embed/([a-f0-9]{24}))", QRegularExpression::CaseInsensitiveOption);
                            auto m = tauEmbedRe.match(embedUrl);
                            if (m.hasMatch()) {
                                fvTauId = m.captured(1);
                            }
                            QJsonObject epObj = fvObj.value("episode").toObject();
                            if (fvEpName.isEmpty() && !epObj.value("name").toString().isEmpty()) {
                                fvEpName = epObj.value("name").toString();
                            }
                            if (fvPoster.isEmpty() && !epObj.value("poster").toString().isEmpty()) {
                                fvPoster = epObj.value("poster").toString();
                            }
                        }
                    }

                    if (!fvTauId.isEmpty() && (targetEp == 1 || !isEpisodeSpecific)) {
                        startTauVideoResolve(fvTauId, animeTitle, targetSeason, targetEp, fvEpName, fvPoster);
                    } else {
                        m_isAnalyzing = false;
                        emit analyzingChanged();
                        emit analysisError(QString("Animecix: Video stream not found for Season %1 Episode %2.").arg(targetSeason).arg(targetEp));
                    }
                });
            });
        };

        (*fetchPage)(1);
    });
}

void UrlAnalyzer::startTauVideoResolve(const QString &tauId, const QString &animeTitle, int season, int episode, const QString &epName, const QString &poster) {
    QString tauApiUrl = QString("https://tau-video.xyz/api/video/%1").arg(tauId);
    QNetworkRequest request((QUrl(tauApiUrl)));
    request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36");
    request.setRawHeader("Referer", "https://tau-video.xyz/");
    request.setRawHeader("Accept", "application/json, text/plain, */*");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = m_netManager.get(request);
    reply->ignoreSslErrors();
    m_currentReply = reply;

    connect(reply, &QNetworkReply::finished, this, [this, reply, tauId, animeTitle, season, episode, epName, poster]() {
        if (m_currentReply == reply) {
            m_currentReply = nullptr;
        }
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray data = reply->readAll();
        reply->deleteLater();

        if (statusCode == 200 && !data.isEmpty()) {
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                QJsonArray urls = obj.value("urls").toArray();

                QString bestUrl;
                QString bestLabel;
                qint64 bestSize = 0;

                for (const QJsonValue &uVal : urls) {
                    QJsonObject uObj = uVal.toObject();
                    QString uStr = uObj.value("url").toString();
                    QString uLabel = uObj.value("label").toString().toLower();
                    qint64 uSize = uObj.value("size").toInteger();
                    if (!uStr.isEmpty()) {
                        if (bestUrl.isEmpty() || uLabel.contains("1080") || (!bestLabel.contains("1080") && uLabel.contains("720"))) {
                            bestUrl = uStr;
                            bestLabel = uObj.value("label").toString();
                            bestSize = uSize;
                        }
                    }
                }

                if (!bestUrl.isEmpty()) {
                    m_isAnalyzing = false;
                    emit analyzingChanged();

                    if (m_currentInfo) {
                        m_currentInfo->deleteLater();
                    }
                    m_currentInfo = new MediaInfo(this);
                    m_currentInfo->setUrl(bestUrl);

                    QString formattedTitle = animeTitle;
                    if (season > 0 && episode > 0) {
                        formattedTitle += QString(" - S%1E%2")
                            .arg(season, 2, 10, QChar('0'))
                            .arg(episode, 2, 10, QChar('0'));
                    }
                    if (!epName.isEmpty()) {
                        formattedTitle += " - " + epName;
                    }

                    m_currentInfo->setTitle(formattedTitle);
                    m_currentInfo->setAuthor("Animecix (Tau Video)");
                    m_currentInfo->setThumbnailUrl(poster);
                    m_currentInfo->setIsDirectFile(true);

                    double durSec = obj.value("duration").toDouble();
                    if (durSec > 0) {
                        int m = static_cast<int>(durSec) / 60;
                        int s = static_cast<int>(durSec) % 60;
                        m_currentInfo->setDuration(QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));
                    }

                    populateDirectFileOptions(m_currentInfo, bestSize, "video/mp4", formattedTitle + ".mp4");

                    emit mediaInfoChanged();
                    emit analysisSuccess(m_currentInfo);
                    return;
                }
            }
        }

        m_isAnalyzing = false;
        emit analyzingChanged();
        emit analysisError("Animecix: Video stream URL unreachable or server did not respond.");
    });
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
                m_currentInfo->setMaxHeight(1080);

                populateWebStreamOptions(m_currentInfo);

                emit mediaInfoChanged();
                emit analysisSuccess(m_currentInfo);
                return;
            }
        }
    }

    // Fallback to yt-dlp probe if oEmbed fails
    startYtDlpProbe(m_analyzingUrl);
}

void UrlAnalyzer::startWebPageScrape(const QUrl &url) {
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36");
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request.setRawHeader("Accept-Language", "tr-TR,tr;q=0.9,en-US;q=0.8,en;q=0.7");

    m_currentReply = m_netManager.get(request);
    connect(m_currentReply, &QNetworkReply::finished, this, &UrlAnalyzer::onWebPageScrapeFinished);
}

void UrlAnalyzer::onWebPageScrapeFinished() {
    if (!m_currentReply) return;

    QUrl finalUrl = m_currentReply->url();
    int httpCode = m_currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray htmlBytes = m_currentReply->readAll();
    m_currentReply->deleteLater();
    m_currentReply = nullptr;

    if (httpCode >= 200 && httpCode < 400 && !htmlBytes.isEmpty()) {
        QString html = QString::fromUtf8(htmlBytes);
        parseHtmlForMedia(html, finalUrl);
    } else {
        // Fallback to yt-dlp probe
        startYtDlpProbe(m_analyzingUrl);
    }
}

void UrlAnalyzer::parseHtmlForMedia(const QString &html, const QUrl &baseUrl) {
    m_pageReferer = baseUrl.toString();
    QString unescapedHtml = html;
    unescapedHtml.replace(R"(\/)", "/");

    // 1. Extract clean page title
    QString title;
    QRegularExpression titleRe(R"(<title[^>]*>(.*?)</title>)", QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
    auto titleMatch = titleRe.match(unescapedHtml);
    if (titleMatch.hasMatch()) {
        title = titleMatch.captured(1).trimmed();
        title.replace("&amp;", "&").replace("&#039;", "'").replace("&quot;", "\"");
        // Remove site branding suffixes (e.g. " - Animecix", " | Animecix", "izle", etc.)
        title.remove(QRegularExpression(R"(\s*[-|]\s*Animecix.*$)", QRegularExpression::CaseInsensitiveOption));
        title.remove(QRegularExpression(R"(\s*[-|]\s*Dizipal.*$)", QRegularExpression::CaseInsensitiveOption));
        title.remove(QRegularExpression(R"(\s*[-|]\s*Doeda.*$)", QRegularExpression::CaseInsensitiveOption));
        title.remove(QRegularExpression(R"(\s*izle.*$)", QRegularExpression::CaseInsensitiveOption));
        title = title.trimmed();
    }
    if (title.isEmpty()) {
        QRegularExpression ogTitleRe(R"(<meta[^>]+property=["']og:title["'][^>]+content=["']([^"']+)["'])", QRegularExpression::CaseInsensitiveOption);
        auto ogMatch = ogTitleRe.match(unescapedHtml);
        if (ogMatch.hasMatch()) {
            title = ogMatch.captured(1).trimmed();
        }
    }
    if (title.isEmpty()) {
        title = "Web_Video_" + QString::number(QDateTime::currentSecsSinceEpoch());
    }

    // 2. Extract thumbnail if available
    QString thumb;
    QRegularExpression thumbRe(R"(<meta[^>]+property=["']og:image["'][^>]+content=["']([^"']+)["'])", QRegularExpression::CaseInsensitiveOption);
    auto thumbMatch = thumbRe.match(unescapedHtml);
    if (thumbMatch.hasMatch()) {
        thumb = thumbMatch.captured(1).trimmed();
    }
    if (thumb.isEmpty()) {
        QRegularExpression posterRe(R"(poster\s*[:=]\s*["']([^"']+)["'])", QRegularExpression::CaseInsensitiveOption);
        auto posterMatch = posterRe.match(unescapedHtml);
        if (posterMatch.hasMatch()) {
            thumb = posterMatch.captured(1).trimmed();
        }
    }

    // 3. Check for mediaDefinition or playerParams (Stiefmutter, Xmilf, MindGeek, adult tube players)
    QRegularExpression mediaDefRe(R"raw("videoUrl"\s*:\s*"([^"]+)")raw", QRegularExpression::CaseInsensitiveOption);
    auto mediaDefMatch = mediaDefRe.match(unescapedHtml);
    if (mediaDefMatch.hasMatch()) {
        QString streamUrl = mediaDefMatch.captured(1).trimmed();
        if (streamUrl.startsWith("//")) streamUrl = "https:" + streamUrl;
        else if (streamUrl.startsWith("/")) streamUrl = baseUrl.resolved(QUrl(streamUrl)).toString();
        setupStreamMediaInfo(streamUrl, title, "Embedded Stream", thumb, baseUrl.toString());
        return;
    }

    // 4. Scan for direct HLS stream (.m3u8) in HTML / script tags
    QRegularExpression m3u8Re(R"((https?://[^"'\s<>]+\.m3u8[^"'\s<>]*))", QRegularExpression::CaseInsensitiveOption);
    auto m3u8Match = m3u8Re.match(unescapedHtml);
    if (m3u8Match.hasMatch()) {
        QString streamUrl = m3u8Match.captured(1).trimmed();
        setupStreamMediaInfo(streamUrl, title, "HLS Stream (M3U8)", thumb, baseUrl.toString());
        return;
    }

    // 5. Scan for direct MP4 / WebM stream in HTML / script tags
    QRegularExpression mp4Re(R"((https?://[^"'\s<>]+\.(?:mp4|webm|mkv)[^"'\s<>]*))", QRegularExpression::CaseInsensitiveOption);
    auto mp4Match = mp4Re.match(unescapedHtml);
    if (mp4Match.hasMatch()) {
        QString streamUrl = mp4Match.captured(1).trimmed();
        setupStreamMediaInfo(streamUrl, title, "Direct Video Stream", thumb, baseUrl.toString());
        return;
    }

    // 6. Scan for HTML5 <video> and <source> tags
    QRegularExpression html5Re(R"(<(?:video|source)[^>]+src=["']([^"']+)["'])", QRegularExpression::CaseInsensitiveOption);
    auto html5Match = html5Re.match(unescapedHtml);
    if (html5Match.hasMatch()) {
        QString src = html5Match.captured(1).trimmed();
        if (src.startsWith("//")) src = "https:" + src;
        else if (src.startsWith("/")) src = baseUrl.resolved(QUrl(src)).toString();
        if (src.contains(".mp4") || src.contains(".m3u8") || src.contains("/media/") || src.contains("video")) {
            setupStreamMediaInfo(src, title, "HTML5 Video Stream", thumb, baseUrl.toString());
            return;
        }
    }

    // 7. Scan for player JS configs (file: "...", player.src("..."), video_url: "...")
    QRegularExpression jsPlayerRe(R"((?:file|video_url|videoUrl|player\.src)\s*[:(]\s*["'](https?://[^"']+)["'])", QRegularExpression::CaseInsensitiveOption);
    auto jsMatch = jsPlayerRe.match(unescapedHtml);
    if (jsMatch.hasMatch()) {
        QString src = jsMatch.captured(1).trimmed();
        setupStreamMediaInfo(src, title, "Player Stream", thumb, baseUrl.toString());
        return;
    }

    // 8. Scan for meta video tags
    QRegularExpression metaVidRe(R"(<meta[^>]+(?:property|name)=["'](?:og:video|og:video:url|og:video:secure_url|twitter:player:stream)["'][^>]+content=["']([^"']+)["'])", QRegularExpression::CaseInsensitiveOption);
    auto metaVidMatch = metaVidRe.match(unescapedHtml);
    if (metaVidMatch.hasMatch()) {
        QString src = metaVidMatch.captured(1).trimmed();
        if (src.startsWith("//")) src = "https:" + src;
        else if (src.startsWith("/")) src = baseUrl.resolved(QUrl(src)).toString();
        if (src.contains(".mp4") || src.contains(".m3u8") || src.contains("/media/") || src.contains("video")) {
            setupStreamMediaInfo(src, title, "Web Video Stream", thumb, baseUrl.toString());
            return;
        }
    }

    // 9. Scan for embedded iframe players (Vidmoly, Sibnet, Streamtape, Doodstream, Filemoon, Voe, Dailymotion, generic iframes)
    QRegularExpression iframeRe(R"(<iframe[^>]+src=["']([^"']+)["'])", QRegularExpression::CaseInsensitiveOption);
    auto it = iframeRe.globalMatch(unescapedHtml);
    QString foundIframe;
    QString providerName = "Online Video Player";

    while (it.hasNext()) {
        auto m = it.next();
        QString src = m.captured(1).trimmed();
        if (src.startsWith("//")) src = "https:" + src;
        else if (src.startsWith("/")) src = baseUrl.resolved(QUrl(src)).toString();

        QString lowerSrc = src.toLower();
        if (lowerSrc.contains("vidmoly")) {
            foundIframe = src;
            providerName = "Vidmoly Player";
            break;
        } else if (lowerSrc.contains("sibnet")) {
            foundIframe = src;
            providerName = "Sibnet Player";
            break;
        } else if (lowerSrc.contains("streamtape") || lowerSrc.contains("dood") ||
                   lowerSrc.contains("filemoon") || lowerSrc.contains("voe") ||
                   lowerSrc.contains("dailymotion")) {
            foundIframe = src;
            providerName = "Embedded Player";
            break;
        } else if (foundIframe.isEmpty() && (lowerSrc.contains("embed") || lowerSrc.contains("player") || lowerSrc.contains("video"))) {
            foundIframe = src;
            providerName = "Embedded Video Player";
        }
    }

    if (!foundIframe.isEmpty()) {
        fetchIframeContent(QUrl(foundIframe), title, thumb, providerName);
        return;
    }

    // 10. Fallback: Try dynamic yt-dlp probe with referer
    startYtDlpProbe(m_analyzingUrl);
}

void UrlAnalyzer::fetchIframeContent(const QUrl &iframeUrl, const QString &title, const QString &thumb, const QString &providerName) {
    m_pendingIframeUrl = iframeUrl;
    m_pendingTitle = title;
    m_pendingThumb = thumb;
    m_pendingProvider = providerName;

    QNetworkRequest request(iframeUrl);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, true);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36");
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request.setRawHeader("Referer", m_analyzingUrl.toUtf8());

    m_currentReply = m_netManager.get(request);
    connect(m_currentReply, &QNetworkReply::finished, this, &UrlAnalyzer::onIframeFetchFinished);
}

void UrlAnalyzer::onIframeFetchFinished() {
    if (!m_currentReply) return;

    QUrl finalUrl = m_currentReply->url();
    int httpCode = m_currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray htmlBytes = m_currentReply->readAll();
    m_currentReply->deleteLater();
    m_currentReply = nullptr;

    if (httpCode >= 200 && httpCode < 400 && !htmlBytes.isEmpty()) {
        QString html = QString::fromUtf8(htmlBytes);
        QString unescaped = html;
        unescaped.replace(R"(\/)", "/");

        // 1. Check for mediaDefinition in iframe
        QRegularExpression mediaDefRe(R"raw("videoUrl"\s*:\s*"([^"]+)")raw", QRegularExpression::CaseInsensitiveOption);
        auto mediaDefMatch = mediaDefRe.match(unescaped);
        if (mediaDefMatch.hasMatch()) {
            QString streamUrl = mediaDefMatch.captured(1).trimmed();
            if (streamUrl.startsWith("//")) streamUrl = "https:" + streamUrl;
            else if (streamUrl.startsWith("/")) streamUrl = finalUrl.resolved(QUrl(streamUrl)).toString();
            setupStreamMediaInfo(streamUrl, m_pendingTitle, m_pendingProvider, m_pendingThumb, finalUrl.toString());
            return;
        }

        // 2. Check for .m3u8 stream in iframe HTML
        QRegularExpression m3u8Re(R"((https?://[^"'\s<>]+\.m3u8[^"'\s<>]*))", QRegularExpression::CaseInsensitiveOption);
        auto m3u8Match = m3u8Re.match(unescaped);
        if (m3u8Match.hasMatch()) {
            setupStreamMediaInfo(m3u8Match.captured(1).trimmed(), m_pendingTitle, m_pendingProvider, m_pendingThumb, finalUrl.toString());
            return;
        }

        // 3. Check for .mp4 stream in iframe HTML
        QRegularExpression mp4Re(R"((https?://[^"'\s<>]+\.(?:mp4|webm|mkv)[^"'\s<>]*))", QRegularExpression::CaseInsensitiveOption);
        auto mp4Match = mp4Re.match(unescaped);
        if (mp4Match.hasMatch()) {
            setupStreamMediaInfo(mp4Match.captured(1).trimmed(), m_pendingTitle, m_pendingProvider, m_pendingThumb, finalUrl.toString());
            return;
        }

        // 4. Check for sources: [{file: "..."}] or player.src(...)
        QRegularExpression sourcesRe(R"((?:file|src)\s*:\s*["']([^"']+)["'])", QRegularExpression::CaseInsensitiveOption);
        auto sourcesMatch = sourcesRe.match(unescaped);
        if (sourcesMatch.hasMatch()) {
            QString src = sourcesMatch.captured(1).trimmed();
            if (src.startsWith("//")) src = "https:" + src;
            else if (src.startsWith("/")) src = finalUrl.resolved(QUrl(src)).toString();
            setupStreamMediaInfo(src, m_pendingTitle, m_pendingProvider, m_pendingThumb, finalUrl.toString());
            return;
        }
    }

    // Method 2 fallback: dynamic probe via yt-dlp on the iframe URL with referer
    m_pageReferer = m_analyzingUrl;
    startYtDlpProbe(m_pendingIframeUrl.toString());
}

void UrlAnalyzer::setupStreamMediaInfo(const QString &streamUrl, const QString &title, const QString &sourceName, const QString &thumbnail, const QString &referer) {
    m_isAnalyzing = false;
    emit analyzingChanged();

    if (m_currentInfo) {
        m_currentInfo->deleteLater();
    }
    m_currentInfo = new MediaInfo(this);
    m_currentInfo->setUrl(streamUrl);
    m_currentInfo->setTitle(title);
    m_currentInfo->setAuthor(sourceName);
    m_currentInfo->setThumbnailUrl(thumbnail);
    m_currentInfo->setIsDirectFile(false);
    m_currentInfo->setReferer(!referer.isEmpty() ? referer : m_pageReferer);

    populateWebStreamOptions(m_currentInfo);

    emit mediaInfoChanged();
    emit analysisSuccess(m_currentInfo);
}

void UrlAnalyzer::startYtDlpProbe(const QString &rawUrl) {
    m_analyzingUrl = rawUrl;
    QString ytDlpExe = FFmpegProcessor::findYtDlpExecutable();
    if (ytDlpExe.isEmpty()) {
        startDirectProbe(QUrl(rawUrl));
        return;
    }

    m_ytDlpProcess = new QProcess(this);
    QStringList args;
    args << "--print" << "%(title)s|||%(uploader)s|||%(duration_string)s|||%(thumbnail)s|||%(height)s|||%(formats.:.height)#j";
    args << "--no-warnings";
    args << "--no-playlist";
    args << "--no-check-certificates";
    args << "--socket-timeout" << "25";
    args << "--retries" << "5";
    args << "--impersonate" << "Chrome";

    QString lower = rawUrl.toLower();
    bool isYouTube = lower.contains("youtube.com") || lower.contains("youtu.be");
    bool isSocial = lower.contains("instagram.com") || lower.contains("tiktok.com") ||
                    lower.contains("twitter.com") || lower.contains("x.com") ||
                    lower.contains("facebook.com") || lower.contains("fb.watch");

    if (!isSocial) {
        QString ref = !m_pageReferer.isEmpty() ? m_pageReferer : rawUrl;
        args << "--referer" << ref;
    }

    args << rawUrl;

    connect(m_ytDlpProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &UrlAnalyzer::onYtDlpFinished);

    m_ytDlpProcess->start(ytDlpExe, args);
}

void UrlAnalyzer::onYtDlpFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    m_isAnalyzing = false;
    emit analyzingChanged();

    if (!m_ytDlpProcess) return;

    QString output = QString::fromUtf8(m_ytDlpProcess->readAllStandardOutput()).trimmed();
    QString errOutput = QString::fromUtf8(m_ytDlpProcess->readAllStandardError()).trimmed();
    m_ytDlpProcess->deleteLater();
    m_ytDlpProcess = nullptr;

    if (exitStatus == QProcess::NormalExit && exitCode == 0 && !output.isEmpty()) {
        QStringList parts = output.split("|||");
        QString title = parts.value(0).trimmed();
        QString author = (parts.size() > 1) ? parts.value(1).trimmed() : "";
        QString duration = (parts.size() > 2) ? parts.value(2).trimmed() : "";
        QString thumbnail = (parts.size() > 3) ? parts.value(3).trimmed() : "";
        int maxHeight = (parts.size() > 4) ? parts.value(4).trimmed().toInt() : 1080;
        if (maxHeight <= 0) maxHeight = 1080;

        QList<int> detectedHeights;
        if (parts.size() > 5) {
            QString jsonStr = parts.mid(5).join("|||").trimmed();
            QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
            if (doc.isArray()) {
                QSet<int> uniqueHeights;
                for (const QJsonValue &v : doc.array()) {
                    int h = v.toInt();
                    if (h >= 144) {
                        uniqueHeights.insert(h);
                    }
                }
                QList<int> sorted = uniqueHeights.values();
                std::sort(sorted.begin(), sorted.end(), std::greater<int>());
                detectedHeights = sorted;
                if (!detectedHeights.isEmpty() && detectedHeights.first() > 0) {
                    maxHeight = detectedHeights.first();
                }
            }
        }

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
            m_currentInfo->setMaxHeight(maxHeight);
            m_currentInfo->setAvailableHeights(detectedHeights);
            m_currentInfo->setReferer(!m_pageReferer.isEmpty() ? m_pageReferer : m_analyzingUrl);

            populateWebStreamOptions(m_currentInfo);

            emit mediaInfoChanged();
            emit analysisSuccess(m_currentInfo);
            return;
        }
    }

    // Fallback to oEmbed probe if yt-dlp failed on YouTube
    if (m_analyzingUrl.contains("youtube.com") || m_analyzingUrl.contains("youtu.be")) {
        startYouTubeOembedProbe(QUrl(m_analyzingUrl));
        return;
    }

    // Informative guidance error if extraction failed
    QString cleanErr = "Could not detect media stream from this page.";
    if (!errOutput.isEmpty()) {
        QRegularExpression errRe(R"(ERROR:\s*(?:\[[^\]]+\]\s*)?([^\r\n]+))");
        auto match = errRe.match(errOutput);
        if (match.hasMatch()) {
            cleanErr = match.captured(1).trimmed();
        }
    }
    emit analysisError(cleanErr);
}

void UrlAnalyzer::startDirectProbe(const QUrl &url) {
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, true);
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36");
    if (!m_pageReferer.isEmpty()) {
        request.setRawHeader("Referer", m_pageReferer.toUtf8());
    } else {
        request.setRawHeader("Referer", url.toString().toUtf8());
    }

    m_currentReply = m_netManager.head(request);
    connect(m_currentReply, &QNetworkReply::finished, this, &UrlAnalyzer::onHeadFinished);
}

void UrlAnalyzer::onHeadFinished() {
    if (!m_currentReply) return;

    m_isAnalyzing = false;
    emit analyzingChanged();

    if (m_currentReply->error() != QNetworkReply::NoError) {
        int httpCode = m_currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (httpCode == 403 || httpCode == 405 || httpCode == 400 || httpCode == 0) {
            QUrl targetUrl = m_currentReply->url();
            m_currentReply->deleteLater();
            m_currentReply = nullptr;

            QNetworkRequest getReq(targetUrl);
            getReq.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
            getReq.setAttribute(QNetworkRequest::Http2AllowedAttribute, true);
            getReq.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36");
            getReq.setRawHeader("Range", "bytes=0-1024");
            if (!m_pageReferer.isEmpty()) {
                getReq.setRawHeader("Referer", m_pageReferer.toUtf8());
            } else {
                getReq.setRawHeader("Referer", targetUrl.toString().toUtf8());
            }

            m_isAnalyzing = true;
            emit analyzingChanged();
            m_currentReply = m_netManager.get(getReq);
            connect(m_currentReply, &QNetworkReply::finished, this, &UrlAnalyzer::onHeadFinished);
            return;
        }

        // If direct media file format, allow setting up stream rather than failing
        QString pathLower = m_analyzingUrl.toLower();
        if (pathLower.contains(".mp4") || pathLower.contains(".m3u8") || pathLower.contains(".mp3") || pathLower.contains(".webm")) {
            QString detectedName = extractFileName(QUrl(m_analyzingUrl), "");
            m_currentReply->deleteLater();
            m_currentReply = nullptr;
            setupStreamMediaInfo(m_analyzingUrl, detectedName, "Direct Video Stream", "", m_pageReferer);
            return;
        }

        QString errStr = m_currentReply->errorString();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
        emit analysisError(QString("Media analysis failed: ") + errStr);
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
    m_currentInfo->setReferer(!m_pageReferer.isEmpty() ? m_pageReferer : finalUrl.toString());

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

    if (!base.isEmpty() && base.toLower() != "watch" && base.toLower() != "watch.mp4") {
        return base;
    }

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

    MediaStreamOption original;
    original.id = "original";
    original.resolution = "Original Quality (Highest)";
    original.container = ext;
    original.codec = mimeType.isEmpty() ? "Standard" : mimeType;
    original.sizeBytes = sizeBytes;
    original.sizeFormatted = sizeStr;
    original.isAudioOnly = (ext == "mp3" || ext == "m4a" || ext == "wav" || ext == "aac");
    options.append(original);

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

    MediaStreamOption bestVideo;
    bestVideo.id = "best_video";
    bestVideo.resolution = "Video (MP4 / Best Quality)";
    bestVideo.container = "mp4";
    bestVideo.codec = "H.264 / AAC (HD/4K)";
    bestVideo.sizeFormatted = "Full Quality";
    bestVideo.isAudioOnly = false;
    options.append(bestVideo);

    MediaStreamOption audioMp3;
    audioMp3.id = "audio_mp3";
    audioMp3.resolution = "Audio Only (320 kbps MP3)";
    audioMp3.container = "mp3";
    audioMp3.codec = "MP3 (320 kbps HQ)";
    audioMp3.sizeFormatted = "Studio Quality";
    audioMp3.isAudioOnly = true;
    options.append(audioMp3);

    info->setStreamOptions(options);
}

void UrlAnalyzer::startSpotifyResolve(const QUrl &url) {
    m_spotifyTitle.clear();
    m_spotifyThumb.clear();

    QString urlStr = url.toString();
    QRegularExpression trackIdRe(R"(track/([a-zA-Z0-9]+))");
    auto match = trackIdRe.match(urlStr);
    QString trackId = match.hasMatch() ? match.captured(1) : "";

    QUrl targetUrl;
    if (!trackId.isEmpty()) {
        targetUrl = QUrl(QString("https://open.spotify.com/embed/track/%1").arg(trackId));
    } else {
        targetUrl = url;
    }

    QNetworkRequest request(targetUrl);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36");
    request.setRawHeader("Accept-Language", "en-US,en;q=0.9");

    m_currentReply = m_netManager.get(request);
    connect(m_currentReply, &QNetworkReply::finished, this, &UrlAnalyzer::onSpotifyOembedFinished);
}

void UrlAnalyzer::onSpotifyOembedFinished() {
    if (!m_currentReply) return;

    QNetworkReply *reply = m_currentReply;
    m_currentReply = nullptr;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit analysisError("Spotify track could not be resolved or is DRM-protected. Please enter a direct media link.");
        m_isAnalyzing = false;
        emit analyzingChanged();
        return;
    }

    QByteArray data = reply->readAll();
    QString html = QString::fromUtf8(data);

    QString artist;

    // 1. Try Spotify embed JSON format: "artists":[{"name":"yayaheart"
    QRegularExpression embedArtistRe(R"raw("artists"\s*:\s*\[\s*\{\s*"name"\s*:\s*"([^"]+)")raw", QRegularExpression::CaseInsensitiveOption);
    auto mEmbedArtist = embedArtistRe.match(html);
    if (mEmbedArtist.hasMatch()) {
        artist = mEmbedArtist.captured(1).trimmed();
    }

    // Embed name/title
    QRegularExpression embedNameRe(R"raw("name"\s*:\s*"([^"]+)")raw", QRegularExpression::CaseInsensitiveOption);
    auto mEmbedName = embedNameRe.match(html);
    if (mEmbedName.hasMatch() && m_spotifyTitle.isEmpty()) {
        m_spotifyTitle = mEmbedName.captured(1).trimmed();
    }

    // Embed cover image
    QRegularExpression embedImgRe(R"raw("coverArt"\s*:\s*\{[^\}]*"url"\s*:\s*"([^"]+)")raw", QRegularExpression::CaseInsensitiveOption);
    auto mEmbedImg = embedImgRe.match(html);
    if (mEmbedImg.hasMatch() && m_spotifyThumb.isEmpty()) {
        m_spotifyThumb = mEmbedImg.captured(1).trimmed();
    }

    // 2. Standard page meta tags fallback
    if (artist.isEmpty()) {
        QRegularExpression artistRe(R"(name=["']music:musician_description["']\s+content=["']([^"']+)["'])", QRegularExpression::CaseInsensitiveOption);
        auto mArtist = artistRe.match(html);
        if (mArtist.hasMatch()) {
            artist = mArtist.captured(1).trimmed();
        }
    }
    if (m_spotifyTitle.isEmpty()) {
        QRegularExpression titleRe(R"(property=["']og:title["']\s+content=["']([^"']+)["'])", QRegularExpression::CaseInsensitiveOption);
        auto mTitle = titleRe.match(html);
        if (mTitle.hasMatch()) {
            m_spotifyTitle = mTitle.captured(1).trimmed();
        }
    }
    if (m_spotifyThumb.isEmpty()) {
        QRegularExpression imgRe(R"(property=["']og:image["']\s+content=["']([^"']+)["'])", QRegularExpression::CaseInsensitiveOption);
        auto mImg = imgRe.match(html);
        if (mImg.hasMatch()) {
            m_spotifyThumb = mImg.captured(1).trimmed();
        }
    }

    // 3. Title tag fallback
    if (artist.isEmpty() || m_spotifyTitle.isEmpty()) {
        QRegularExpression titleTagRe(R"(<title>([^<]+)\s*-\s*song\s*(?:and\s*lyrics)?\s*by\s*([^<|]+)\s*\|\s*Spotify</title>)", QRegularExpression::CaseInsensitiveOption);
        auto mTitleTag = titleTagRe.match(html);
        if (mTitleTag.hasMatch()) {
            if (m_spotifyTitle.isEmpty()) m_spotifyTitle = mTitleTag.captured(1).trimmed();
            if (artist.isEmpty()) artist = mTitleTag.captured(2).trimmed();
        }
    }

    if (m_spotifyTitle.isEmpty()) {
        emit analysisError("Spotify track info could not be read. Please enter a direct YouTube or media link.");
        m_isAnalyzing = false;
        emit analyzingChanged();
        return;
    }

    QString searchQuery = artist.isEmpty() ? m_spotifyTitle : QString("%1 %2").arg(artist, m_spotifyTitle);
    QString displayTitle = artist.isEmpty() ? m_spotifyTitle : QString("%1 - %2").arg(artist, m_spotifyTitle);
    m_spotifyTitle = displayTitle;

    QString ytDlpExe = FFmpegProcessor::findYtDlpExecutable();
    if (ytDlpExe.isEmpty()) {
        emit analysisError("yt-dlp video engine was not found.");
        m_isAnalyzing = false;
        emit analyzingChanged();
        return;
    }

    m_ytDlpProcess = new QProcess(this);
    QStringList args;
    args << "--print" << "%(title)s|||%(uploader)s|||%(duration_string)s|||%(thumbnail)s|||%(webpage_url)s";
    args << "--no-warnings";
    args << "--no-playlist";
    args << "--no-check-certificates";
    args << QString("ytsearch1:%1").arg(searchQuery);

    connect(m_ytDlpProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &UrlAnalyzer::onSpotifySearchFinished);
    m_ytDlpProcess->start(ytDlpExe, args);
}

void UrlAnalyzer::onSpotifySearchFinished(int exitCode, QProcess::ExitStatus exitStatus) {
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
        QString ytThumb = (parts.size() > 3) ? parts.value(3).trimmed() : "";
        QString webpageUrl = (parts.size() > 4) ? parts.value(4).trimmed() : "";

        if (m_currentInfo) {
            m_currentInfo->deleteLater();
        }
        m_currentInfo = new MediaInfo(this);
        m_currentInfo->setUrl(webpageUrl.isEmpty() ? m_analyzingUrl : webpageUrl);
        m_currentInfo->setTitle(m_spotifyTitle.isEmpty() ? title : m_spotifyTitle);
        m_currentInfo->setAuthor(author.isEmpty() ? "Spotify" : QString("Spotify • %1").arg(author));
        m_currentInfo->setDuration(duration);
        m_currentInfo->setThumbnailUrl(m_spotifyThumb.isEmpty() ? ytThumb : m_spotifyThumb);
        m_currentInfo->setIsDirectFile(false);
        m_currentInfo->setIsAudioOnly(true);

        populateWebStreamOptions(m_currentInfo);

        emit mediaInfoChanged();
        emit analysisSuccess(m_currentInfo);
        return;
    }

    emit analysisError(QString("Spotify track found (%1), but audio stream could not be retrieved.").arg(m_spotifyTitle));
}
