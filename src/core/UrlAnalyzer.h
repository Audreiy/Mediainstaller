#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "MediaInfo.h"

enum class UrlCategory {
    DirectVideo,     // .mp4, .webm, .mkv
    DirectAudio,     // .mp3, .m4a, .wav, .aac
    HlsStream,       // .m3u8
    GenericWebPage,  // Web video landing page
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
    static UrlCategory categorizeUrl(const QString &rawUrl);

signals:
    void analyzingChanged();
    void mediaInfoChanged();
    void analysisSuccess(MediaInfo *info);
    void analysisError(const QString &message);

private slots:
    void onHeadFinished();

private:
    void startDirectProbe(const QUrl &url);
    void populateDirectFileOptions(MediaInfo *info, qint64 sizeBytes, const QString &mimeType, const QString &fileName);
    QString extractFileName(const QUrl &url, const QString &contentDisposition);

    bool m_isAnalyzing = false;
    MediaInfo *m_currentInfo = nullptr;
    QNetworkAccessManager m_netManager;
    QNetworkReply *m_currentReply = nullptr;
};
