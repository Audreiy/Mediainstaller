#include "DownloadQueueModel.h"
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QProcess>

static QString formatBytes(qint64 bytes) {
    if (bytes <= 0) return QString();
    double b = static_cast<double>(bytes);
    if (b < 1024.0 * 1024.0) {
        return QString::asprintf("%.1f KB", b / 1024.0);
    } else if (b < 1024.0 * 1024.0 * 1024.0) {
        return QString::asprintf("%.1f MB", b / (1024.0 * 1024.0));
    } else {
        return QString::asprintf("%.2f GB", b / (1024.0 * 1024.0 * 1024.0));
    }
}

DownloadQueueModel::DownloadQueueModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int DownloadQueueModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return 0;
    return m_items.size();
}

QVariant DownloadQueueModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return QVariant();

    const auto &item = m_items.at(index.row());
    switch (role) {
    case IdRole: return item.id;
    case TitleRole: return item.title;
    case UrlRole: return item.url;
    case DestinationPathRole: return item.destinationPath;
    case StatusRole: return item.status;
    case ProgressRole: return item.progress;
    case SpeedRole: return item.speed;
    case EtaRole: return item.eta;
    case DownloadedFormattedRole: return item.downloadedFormatted;
    case TotalFormattedRole: return item.totalFormatted;
    case RemainingFormattedRole: return item.remainingFormatted;
    case IsAudioOnlyRole: return item.isAudioOnly;
    case ErrorMessageRole: return item.errorMessage;
    default: return QVariant();
    }
}

QHash<int, QByteArray> DownloadQueueModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[TitleRole] = "title";
    roles[UrlRole] = "url";
    roles[DestinationPathRole] = "destinationPath";
    roles[StatusRole] = "status";
    roles[ProgressRole] = "progress";
    roles[SpeedRole] = "speed";
    roles[EtaRole] = "eta";
    roles[DownloadedFormattedRole] = "downloadedFormatted";
    roles[TotalFormattedRole] = "totalFormatted";
    roles[RemainingFormattedRole] = "remainingFormatted";
    roles[IsAudioOnlyRole] = "isAudioOnly";
    roles[ErrorMessageRole] = "errorMessage";
    return roles;
}

int DownloadQueueModel::activeDownloadCount() const {
    int count = 0;
    for (const auto &item : m_items) {
        if (item.status == "Downloading" || item.status == "Converting") {
            count++;
        }
    }
    return count;
}

int DownloadQueueModel::findIndexById(const QString &id) const {
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i).id == id)
            return i;
    }
    return -1;
}

void DownloadQueueModel::addItem(const DownloadItem &item) {
    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items.append(item);
    endInsertRows();
    emit countChanged();
    emit activeCountChanged();
}

void DownloadQueueModel::updateProgress(const QString &id, double progress, const QString &speed, const QString &eta, qint64 downloaded, qint64 total) {
    int idx = findIndexById(id);
    if (idx < 0) return;

    auto &it = m_items[idx];
    if (progress >= 0.0) {
        it.progress = progress;
    }
    if (!speed.isEmpty()) {
        it.speed = speed;
    }
    if (!eta.isEmpty()) {
        it.eta = eta;
    }
    if (downloaded >= 0) {
        it.bytesDownloaded = downloaded;
    }

    qint64 effTotal = total;
    if (effTotal <= 0 && it.totalBytes > 0) {
        effTotal = it.totalBytes;
    }
    // Estimate total from progress if not known yet
    if (effTotal <= 0 && it.progress > 0.5 && it.bytesDownloaded > 0) {
        effTotal = static_cast<qint64>((it.bytesDownloaded * 100.0) / it.progress);
    }

    if (it.bytesDownloaded > 0) {
        it.downloadedFormatted = formatBytes(it.bytesDownloaded);
    }
    if (effTotal > 0) {
        it.totalBytes = effTotal;
        it.totalFormatted = formatBytes(effTotal);
        if (effTotal > it.bytesDownloaded) {
            it.remainingFormatted = formatBytes(effTotal - it.bytesDownloaded);
        } else {
            it.remainingFormatted = "";
        }
    }

    QModelIndex modelIndex = index(idx, 0);
    emit dataChanged(modelIndex, modelIndex, {ProgressRole, SpeedRole, EtaRole, DownloadedFormattedRole, TotalFormattedRole, RemainingFormattedRole});
}

void DownloadQueueModel::updateStatus(const QString &id, const QString &status, const QString &errorMsg) {
    int idx = findIndexById(id);
    if (idx < 0) return;

    auto &it = m_items[idx];
    it.status = status;
    it.errorMessage = errorMsg;

    QModelIndex modelIndex = index(idx, 0);
    emit dataChanged(modelIndex, modelIndex, {StatusRole, ErrorMessageRole});
    emit activeCountChanged();
}

void DownloadQueueModel::setCompleted(const QString &id, qint64 finalSize, const QString &finalPath) {
    int idx = findIndexById(id);
    if (idx < 0) return;

    auto &it = m_items[idx];
    it.status = "Completed";
    it.progress = 100.0;
    it.speed = "";
    it.eta = "";
    it.errorMessage = "";
    if (!finalPath.isEmpty()) {
        it.destinationPath = finalPath;
    }
    if (finalSize > 0) {
        it.bytesDownloaded = finalSize;
        it.totalBytes = finalSize;
        it.downloadedFormatted = formatBytes(finalSize);
        it.totalFormatted = formatBytes(finalSize);
    } else if (it.bytesDownloaded > 0) {
        it.totalBytes = it.bytesDownloaded;
        it.totalFormatted = it.downloadedFormatted;
    }
    it.remainingFormatted = "";

    QModelIndex modelIndex = index(idx, 0);
    emit dataChanged(modelIndex, modelIndex, {StatusRole, ProgressRole, SpeedRole, EtaRole, DownloadedFormattedRole, TotalFormattedRole, RemainingFormattedRole, DestinationPathRole});
    emit activeCountChanged();
}

DownloadItem DownloadQueueModel::getItem(const QString &id) const {
    int idx = findIndexById(id);
    if (idx >= 0) {
        return m_items.at(idx);
    }
    return DownloadItem();
}

void DownloadQueueModel::pauseDownload(const QString &id) {
    emit requestPause(id);
}

void DownloadQueueModel::resumeDownload(const QString &id) {
    emit requestResume(id);
}

void DownloadQueueModel::cancelDownload(const QString &id) {
    emit requestCancel(id);
    removeItem(id);
}

void DownloadQueueModel::removeItem(const QString &id) {
    int idx = findIndexById(id);
    if (idx >= 0) {
        beginRemoveRows(QModelIndex(), idx, idx);
        m_items.removeAt(idx);
        endRemoveRows();
        emit countChanged();
        emit activeCountChanged();
    }
}

void DownloadQueueModel::openFileFolder(const QString &id) {
    int idx = findIndexById(id);
    if (idx < 0) return;

    QString path = m_items.at(idx).destinationPath;
    QFileInfo fi(path);
#ifdef Q_OS_WIN
    // Select the file in Windows Explorer
    QStringList args;
    args << "/select," << QDir::toNativeSeparators(fi.absoluteFilePath());
    QProcess::startDetached("explorer.exe", args);
#else
    QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absolutePath()));
#endif
}

void DownloadQueueModel::clearCompleted() {
    for (int i = m_items.size() - 1; i >= 0; --i) {
        const QString &st = m_items.at(i).status;
        if (st.compare("Completed", Qt::CaseInsensitive) == 0 ||
            st.compare("Failed", Qt::CaseInsensitive) == 0 ||
            st.compare("Cancelled", Qt::CaseInsensitive) == 0 ||
            st.compare("Canceled", Qt::CaseInsensitive) == 0) {
            beginRemoveRows(QModelIndex(), i, i);
            m_items.removeAt(i);
            endRemoveRows();
        }
    }
    emit countChanged();
    emit activeCountChanged();
}

DownloadItem DownloadQueueModel::getItemAt(int index) const {
    if (index >= 0 && index < m_items.size()) {
        return m_items.at(index);
    }
    return DownloadItem();
}

void DownloadQueueModel::moveItem(int fromIndex, int toIndex) {
    if (fromIndex < 0 || fromIndex >= m_items.size() ||
        toIndex < 0 || toIndex >= m_items.size() ||
        fromIndex == toIndex) {
        return;
    }

    int destChild = toIndex > fromIndex ? toIndex + 1 : toIndex;
    if (beginMoveRows(QModelIndex(), fromIndex, fromIndex, QModelIndex(), destChild)) {
        m_items.move(fromIndex, toIndex);
        endMoveRows();
        emit queueOrderChanged();
    }
}

void DownloadQueueModel::moveItemUp(const QString &id) {
    int idx = findIndexById(id);
    if (idx > 0) {
        moveItem(idx, idx - 1);
    }
}

void DownloadQueueModel::moveItemDown(const QString &id) {
    int idx = findIndexById(id);
    if (idx >= 0 && idx < m_items.size() - 1) {
        moveItem(idx, idx + 1);
    }
}

