#include "MediaInfo.h"
#include <QVariant>

MediaInfo::MediaInfo(QObject *parent)
    : QObject(parent)
{
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
