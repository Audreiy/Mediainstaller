#pragma once

#include <QString>
#include <QList>
#include <QObject>

struct MediaStreamOption {
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString resolution MEMBER resolution)
    Q_PROPERTY(QString container MEMBER container)
    Q_PROPERTY(QString codec MEMBER codec)
    Q_PROPERTY(qint64 sizeBytes MEMBER sizeBytes)
    Q_PROPERTY(QString sizeFormatted MEMBER sizeFormatted)
    Q_PROPERTY(bool isAudioOnly MEMBER isAudioOnly)

public:
    QString id;
    QString resolution;      // e.g. "1080p", "720p", "Audio 320kbps"
    QString container;       // e.g. "mp4", "mp3", "webm"
    QString codec;           // e.g. "H.264 / AAC"
    qint64 sizeBytes = 0;
    QString sizeFormatted;   // e.g. "145.2 MB"
    bool isAudioOnly = false;
};

class MediaInfo : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString referer READ referer WRITE setReferer NOTIFY refererChanged)
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString author READ author WRITE setAuthor NOTIFY authorChanged)
    Q_PROPERTY(QString duration READ duration WRITE setDuration NOTIFY durationChanged)
    Q_PROPERTY(QString thumbnailUrl READ thumbnailUrl WRITE setThumbnailUrl NOTIFY thumbnailUrlChanged)
    Q_PROPERTY(bool isDirectFile READ isDirectFile WRITE setIsDirectFile NOTIFY isDirectFileChanged)
    Q_PROPERTY(bool isAudioOnly READ isAudioOnly WRITE setIsAudioOnly NOTIFY isAudioOnlyChanged)
    Q_PROPERTY(int maxHeight READ maxHeight WRITE setMaxHeight NOTIFY maxHeightChanged)
    Q_PROPERTY(QVariantList availableHeights READ availableHeightsVariant WRITE setAvailableHeightsVariant NOTIFY availableHeightsChanged)
    Q_PROPERTY(QVariantList streamOptions READ streamOptionsVariant NOTIFY streamOptionsChanged)

public:
    explicit MediaInfo(QObject *parent = nullptr);

    QString referer() const { return m_referer; }
    void setReferer(const QString &ref);

    QString url() const { return m_url; }
    void setUrl(const QString &url);

    QString title() const { return m_title; }
    void setTitle(const QString &title);

    QString author() const { return m_author; }
    void setAuthor(const QString &author);

    QString duration() const { return m_duration; }
    void setDuration(const QString &duration);

    QString thumbnailUrl() const { return m_thumbnailUrl; }
    void setThumbnailUrl(const QString &url);

    bool isDirectFile() const { return m_isDirectFile; }
    void setIsDirectFile(bool direct);

    bool isAudioOnly() const { return m_isAudioOnly; }
    void setIsAudioOnly(bool audioOnly);

    int maxHeight() const { return m_maxHeight; }
    void setMaxHeight(int h);

    QList<int> availableHeights() const { return m_availableHeights; }
    void setAvailableHeights(const QList<int> &heights);

    QVariantList availableHeightsVariant() const;
    void setAvailableHeightsVariant(const QVariantList &list);

    QList<MediaStreamOption> streamOptions() const { return m_streamOptions; }
    void setStreamOptions(const QList<MediaStreamOption> &options);

    QVariantList streamOptionsVariant() const;

signals:
    void refererChanged();
    void urlChanged();
    void titleChanged();
    void authorChanged();
    void durationChanged();
    void thumbnailUrlChanged();
    void isDirectFileChanged();
    void isAudioOnlyChanged();
    void maxHeightChanged();
    void availableHeightsChanged();
    void streamOptionsChanged();

private:
    QString m_referer;
    QString m_url;
    QString m_title;
    QString m_author;
    QString m_duration;
    QString m_thumbnailUrl;
    bool m_isDirectFile = false;
    bool m_isAudioOnly = false;
    int m_maxHeight = 1080;
    QList<int> m_availableHeights;
    QList<MediaStreamOption> m_streamOptions;
};
