#pragma once

#include <QQuickPaintedItem>
#include <QPainter>
#include <QTimer>
#include <QImage>
#include <QString>

struct IMFSourceReader;

class VideoBackgroundItem : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(bool playing READ isPlaying WRITE setPlaying NOTIFY playingChanged)
    Q_PROPERTY(int targetFps READ targetFps WRITE setTargetFps NOTIFY targetFpsChanged)

public:
    explicit VideoBackgroundItem(QQuickItem *parent = nullptr);
    ~VideoBackgroundItem() override;

    QString source() const { return m_source; }
    void setSource(const QString &source);

    bool isPlaying() const { return m_playing; }
    void setPlaying(bool play);

    int targetFps() const { return m_targetFps; }
    void setTargetFps(int fps);

    void paint(QPainter *painter) override;

signals:
    void sourceChanged();
    void playingChanged();
    void targetFpsChanged();

private slots:
    void onTimerTick();

private:
    void openMedia();
    void closeMedia();
    void readNextFrame();

    QString m_source;
    bool m_playing = true;
    int m_targetFps = 45;
    int m_frameIntervalMs = 22;
    QTimer m_timer;
    QImage m_currentFrame;
    int m_videoWidth = 0;
    int m_videoHeight = 0;

    IMFSourceReader *m_pReader = nullptr;
    bool m_mfInitialized = false;
};
