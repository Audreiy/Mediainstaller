#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>
#include "MediaInfo.h"

enum class UrlCategory {
    DirectVideo,     // .mp4, .webm, .mkv
    DirectAudio,     // .mp3, .m4a, .wav, .aac
    HlsStream,       // .m3u8
    GenericWebPage,  // Web video platform (YouTube, Animecix, Vidmoly, etc.)
    Unknown
};

class UrlAnalyzer : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isAnalyzing READ isAnalyzing NOTIFY analyzingChanged)
    Q_PROPERTY(MediaInfo* currentMediaInfo READ currentMediaInfo NOTIFY mediaInfoChanged)

public:
    explicit UrlAnalyzer(QObject *parent = nullptr);
    ~UrlAnalyzer() override;

    bool isAnalyzing() const { return m_isAnalyzing; }
    MediaInfo* currentMediaInfo() const { return m_currentInfo; }

    Q_INVOKABLE void analyzeUrl(const QString &rawUrl);
    Q_INVOKABLE void clear();
    static UrlCategory categorizeUrl(const QString &rawUrl);

signals:
    void analyzingChanged();
    void mediaInfoChanged();
    void analysisSuccess(MediaInfo *info);
    void analysisError(const QString &message);

private slots:
    void onHeadFinished();
    void onOembedFinished();
    void onWebPageScrapeFinished();
    void onYtDlpFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onSpotifyOembedFinished();
    void onSpotifySearchFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void startSpotifyResolve(const QUrl &url);
    void startYouTubeOembedProbe(const QUrl &url);
    void startAnimecixResolve(const QUrl &url);
    void startTauVideoResolve(const QString &tauId, const QString &animeTitle, int season, int episode, const QString &epName, const QString &poster);
    void startWebPageScrape(const QUrl &url);
    void parseHtmlForMedia(const QString &html, const QUrl &baseUrl);
    void startYtDlpProbe(const QString &rawUrl);
    void startDirectProbe(const QUrl &url);
    void setupStreamMediaInfo(const QString &streamUrl, const QString &title, const QString &sourceName, const QString &thumbnail = QString());
    void populateDirectFileOptions(MediaInfo *info, qint64 sizeBytes, const QString &mimeType, const QString &fileName);
    void populateWebStreamOptions(MediaInfo *info);
    QString extractFileName(const QUrl &url, const QString &contentDisposition);

    bool m_isAnalyzing = false;
    MediaInfo *m_currentInfo = nullptr;
    QNetworkAccessManager m_netManager;
    QNetworkReply *m_currentReply = nullptr;
    QProcess *m_ytDlpProcess = nullptr;
    QString m_analyzingUrl;
    QString m_spotifyTitle;
    QString m_spotifyThumb;
};
