#include "MediaInfo.h"
#include <QVariant>

MediaInfo::MediaInfo(QObject *parent)
    : QObject(parent)
{
}

void MediaInfo::setReferer(const QString &ref) {
    if (m_referer != ref) {
        m_referer = ref;
        emit refererChanged();
    }
}

void MediaInfo::setUrl(const QString &url) {
    if (m_url != url) {
        m_url = url;
        emit urlChanged();
    }
}

void MediaInfo::setTitle(const QString &title) {
    if (m_title != title) {
        m_title = title;
        emit titleChanged();
    }
}

void MediaInfo::setAuthor(const QString &author) {
    if (m_author != author) {
        m_author = author;
        emit authorChanged();
    }
}

void MediaInfo::setDuration(const QString &duration) {
    if (m_duration != duration) {
        m_duration = duration;
        emit durationChanged();
    }
}

void MediaInfo::setThumbnailUrl(const QString &url) {
    if (m_thumbnailUrl != url) {
        m_thumbnailUrl = url;
        emit thumbnailUrlChanged();
    }
}

void MediaInfo::setIsDirectFile(bool direct) {
    if (m_isDirectFile != direct) {
        m_isDirectFile = direct;
        emit isDirectFileChanged();
    }
}

void MediaInfo::setIsAudioOnly(bool audioOnly) {
    if (m_isAudioOnly != audioOnly) {
        m_isAudioOnly = audioOnly;
        emit isAudioOnlyChanged();
    }
}

void MediaInfo::setMaxHeight(int h) {
    if (m_maxHeight != h) {
        m_maxHeight = h;
        emit maxHeightChanged();
    }
}

void MediaInfo::setAvailableHeights(const QList<int> &heights) {
    if (m_availableHeights != heights) {
        m_availableHeights = heights;
        emit availableHeightsChanged();
    }
}

QVariantList MediaInfo::availableHeightsVariant() const {
    QVariantList list;
    for (int h : m_availableHeights) {
        list.append(h);
    }
    return list;
}

void MediaInfo::setAvailableHeightsVariant(const QVariantList &list) {
    QList<int> heights;
    for (const QVariant &v : list) {
        heights.append(v.toInt());
    }
    setAvailableHeights(heights);
}

void MediaInfo::setStreamOptions(const QList<MediaStreamOption> &options) {
    m_streamOptions = options;
    emit streamOptionsChanged();
}

QVariantList MediaInfo::streamOptionsVariant() const {
    QVariantList list;
    for (const auto &opt : m_streamOptions) {
        list.append(QVariant::fromValue(opt));
    }
    return list;
}

