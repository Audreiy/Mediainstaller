#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QIcon>
#include <QDebug>
#include <iostream>

#include "DownloadManager.h"
#include "DownloadQueueModel.h"
#include "UrlAnalyzer.h"
#include "FFmpegProcessor.h"
#include "MediaInfo.h"

void consoleHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    Q_UNUSED(type);
    Q_UNUSED(context);
    std::cout << msg.toStdString() << std::endl;
    std::cout.flush();
}

int main(int argc, char *argv[]) {
    qInstallMessageHandler(consoleHandler);
    std::cout << ">>> [DEBUG] MediaGrabApp process started! <<<" << std::endl;
    std::cout.flush();

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

    // Connect to warnings to log all QML runtime messages
    QObject::connect(&engine, &QQmlApplicationEngine::warnings, [](const QList<QQmlError> &warnings) {
        for (const auto &w : warnings) {
            std::cout << "[QML WARNING] " << w.toString().toStdString() << std::endl;
            std::cout.flush();
        }
    });

    // Inject core singletons into QML context
    QQmlContext *rootCtx = engine.rootContext();
    rootCtx->setContextProperty("downloadManager", &downloadManager);
    rootCtx->setContextProperty("queueModel", downloadManager.queueModel());
    rootCtx->setContextProperty("urlAnalyzer", downloadManager.urlAnalyzer());
    rootCtx->setContextProperty("ffmpegProcessor", downloadManager.ffmpegProcessor());

    // Load Main QML resource path
    const QUrl url(u"qrc:/qt/qml/MediaGrab/qml/Main.qml"_qs);
    std::cout << ">>> [DEBUG] Loading QML URL: " << url.toString().toStdString() << std::endl;
    std::cout.flush();

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        std::cout << ">>> [DEBUG] objectCreated fired! obj=" << (void*)obj << " url=" << objUrl.toString().toStdString() << std::endl;
        std::cout.flush();
        if (!obj && url == objUrl) {
            std::cerr << ">>> [FATAL] Failed to load root QML object: " << objUrl.toString().toStdString() << std::endl;
            std::cerr.flush();
            QCoreApplication::exit(-1);
        } else if (auto win = qobject_cast<QQuickWindow*>(obj)) {
            std::cout << ">>> [DEBUG] Invoking win->show(), raise(), requestActivate() <<<" << std::endl;
            std::cout.flush();
            win->show();
            win->raise();
            win->requestActivate();
        }
    }, Qt::QueuedConnection);

    engine.load(url);

    std::cout << ">>> [DEBUG] Entering app.exec()... <<<" << std::endl;
    std::cout.flush();

    return app.exec();
}
