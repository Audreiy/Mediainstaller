#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QIcon>

#include "DownloadManager.h"
#include "DownloadQueueModel.h"
#include "UrlAnalyzer.h"
#include "FFmpegProcessor.h"
#include "MediaInfo.h"

int main(int argc, char *argv[]) {
    // Enable High DPI scaling
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough
    );

    QGuiApplication app(argc, argv);
    app.setApplicationName("MediaGrab");
    app.setOrganizationName("MediaGrabDev");
    app.setApplicationVersion("1.0.0");

    // Modern dark controls style
    QQuickStyle::setStyle("Basic");

    // Register custom types for QML
    qmlRegisterUncreatableType<MediaInfo>("MediaGrab", 1, 0, "MediaInfo", "MediaInfo is created by UrlAnalyzer");
    qRegisterMetaType<MediaStreamOption>("MediaStreamOption");

    DownloadManager downloadManager;

    QQmlApplicationEngine engine;

    // Inject core singletons into QML context
    QQmlContext *rootCtx = engine.rootContext();
    rootCtx->setContextProperty("downloadManager", &downloadManager);
    rootCtx->setContextProperty("queueModel", downloadManager.queueModel());
    rootCtx->setContextProperty("urlAnalyzer", downloadManager.urlAnalyzer());
    rootCtx->setContextProperty("ffmpegProcessor", downloadManager.ffmpegProcessor());

    // Load Main QML module
    const QUrl url(u"qrc:/MediaGrab/qml/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);

    engine.loadFromModule("MediaGrab", "Main");

    return app.exec();
}
