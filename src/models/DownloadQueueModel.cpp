#include "DownloadQueueModel.h"
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QProcess>

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
    beginInsertRows(QModelIndex(), 0, 0);
    m_items.prepend(item);
    endInsertRows();
    emit countChanged();
    emit activeCountChanged();
}

void DownloadQueueModel::updateProgress(const QString &id, double progress, const QString &speed, const QString &eta, qint64 downloaded, qint64 total) {
    int idx = findIndexById(id);
    if (idx < 0) return;

    auto &it = m_items[idx];
    it.progress = progress;
    it.speed = speed;
    it.eta = eta;
    it.bytesDownloaded = downloaded;
    it.totalBytes = total;
    if (total > 0) {
        it.downloadedFormatted = QString::asprintf("%.1f MB", downloaded / (1024.0 * 1024.0));
        it.totalFormatted = QString::asprintf("%.1f MB", total / (1024.0 * 1024.0));
    }

    QModelIndex modelIndex = index(idx, 0);
    emit dataChanged(modelIndex, modelIndex, {ProgressRole, SpeedRole, EtaRole, DownloadedFormattedRole, TotalFormattedRole});
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
        if (m_items.at(i).status == "Completed" || m_items.at(i).status == "Failed") {
            beginRemoveRows(QModelIndex(), i, i);
            m_items.removeAt(i);
            endRemoveRows();
        }
    }
    emit countChanged();
    emit activeCountChanged();
}
