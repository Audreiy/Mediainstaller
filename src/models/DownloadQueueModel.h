#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QString>
#include <QDateTime>

struct DownloadItem {
    QString id;
    QString title;
    QString url;
    QString destinationPath;
    QString status;         // "Queued", "Downloading", "Converting", "Paused", "Completed", "Failed"
    double progress = 0.0;  // 0 - 100
    QString speed;          // e.g. "8.5 MB/s"
    QString eta;            // e.g. "01:24"
    qint64 bytesDownloaded = 0;
    qint64 totalBytes = 0;
    QString downloadedFormatted;
    QString totalFormatted;
    QString remainingFormatted;
    bool isAudioOnly = false;
    QString errorMessage;
    QDateTime createdTime;
};

class DownloadQueueModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int activeDownloadCount READ activeDownloadCount NOTIFY activeCountChanged)
    Q_PROPERTY(int totalDownloadCount READ totalDownloadCount NOTIFY countChanged)

public:
    enum DownloadRoles {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        UrlRole,
        DestinationPathRole,
        StatusRole,
        ProgressRole,
        SpeedRole,
        EtaRole,
        DownloadedFormattedRole,
        TotalFormattedRole,
        RemainingFormattedRole,
        IsAudioOnlyRole,
        ErrorMessageRole
    };

    explicit DownloadQueueModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int activeDownloadCount() const;
    int totalDownloadCount() const { return m_items.size(); }

    void addItem(const DownloadItem &item);
    void updateProgress(const QString &id, double progress, const QString &speed, const QString &eta, qint64 downloaded, qint64 total);
    void updateStatus(const QString &id, const QString &status, const QString &errorMsg = QString());
    void setCompleted(const QString &id, qint64 finalSize, const QString &finalPath = QString());
    DownloadItem getItem(const QString &id) const;
    DownloadItem getItemAt(int index) const;

    Q_INVOKABLE void pauseDownload(const QString &id);
    Q_INVOKABLE void resumeDownload(const QString &id);
    Q_INVOKABLE void cancelDownload(const QString &id);
    Q_INVOKABLE void removeItem(const QString &id);
    Q_INVOKABLE void openFileFolder(const QString &id);
    Q_INVOKABLE void clearCompleted();

    Q_INVOKABLE void moveItemUp(const QString &id);
    Q_INVOKABLE void moveItemDown(const QString &id);
    Q_INVOKABLE void moveItem(int fromIndex, int toIndex);

signals:
    void activeCountChanged();
    void countChanged();
    void queueOrderChanged();
    void requestPause(const QString &id);
    void requestResume(const QString &id);
    void requestCancel(const QString &id);

private:
    int findIndexById(const QString &id) const;
    QList<DownloadItem> m_items;
};
