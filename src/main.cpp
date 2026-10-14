#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <dwmapi.h>
#endif

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QIcon>
#include <QFont>
#include <QDebug>
#include <iostream>

#include "DownloadManager.h"
#include "DownloadQueueModel.h"
#include "UrlAnalyzer.h"
#include "FFmpegProcessor.h"
#include "MediaInfo.h"
#include "VideoBackgroundItem.h"
#include "UpdateManager.h"

void consoleHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    Q_UNUSED(type);
    Q_UNUSED(context);
    std::cout << msg.toStdString() << std::endl;
    std::cout.flush();
}

int main(int argc, char *argv[]) {
    qInstallMessageHandler(consoleHandler);
    std::cout << ">>> [DEBUG] YuYuInstaller process started! <<<" << std::endl;
    std::cout.flush();

    // Enable High DPI scaling
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough
    );

    QGuiApplication app(argc, argv);
    app.setApplicationName("YuYuInstaller");
    app.setOrganizationName("YuYuDev");
    app.setApplicationVersion("1.1.7");

    QFont defaultFont("Segoe UI", 10);
    defaultFont.setStyleHint(QFont::SansSerif);
    defaultFont.setFamilies({"Segoe UI", "Segoe UI Variable Text", "Arial", "Roboto", "Noto Sans"});
    app.setFont(defaultFont);

    QIcon appIcon;
    const QStringList iconCandidates = {
        ":/qt/qml/MediaGrab/assets/app_icon.png",
        ":/qt/qml/MediaGrab/assets/logo.png",
        QCoreApplication::applicationDirPath() + "/assets/app_icon.png",
        QCoreApplication::applicationDirPath() + "/assets/logo.png",
        QCoreApplication::applicationDirPath() + "/assets/app.ico"
    };
    for (const auto &c : iconCandidates) {
        QIcon ic(c);
        if (!ic.isNull()) {
            appIcon = ic;
            break;
        }
    }
    if (!appIcon.isNull()) {
        app.setWindowIcon(appIcon);
    }

    // Modern dark controls style
    QQuickStyle::setStyle("Basic");

    // Register custom types for QML
    qmlRegisterUncreatableType<MediaInfo>("MediaGrab", 1, 0, "MediaInfo", "MediaInfo is created by UrlAnalyzer");
    qmlRegisterType<VideoBackgroundItem>("MediaGrab", 1, 0, "VideoBackground");
    qRegisterMetaType<MediaStreamOption>("MediaStreamOption");

    DownloadManager downloadManager;
    UpdateManager updateManager;

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
    rootCtx->setContextProperty("updateManager", &updateManager);

    // Silent update check 3 seconds after application launch
    QTimer::singleShot(3000, &updateManager, [&updateManager]() {
        updateManager.checkForUpdates(true);
    });

    // Load Main QML resource path
    const QUrl url(u"qrc:/qt/qml/MediaGrab/qml/Main.qml"_qs);
    std::cout << ">>> [DEBUG] Loading QML URL: " << url.toString().toStdString() << std::endl;
    std::cout.flush();

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [&app, url, appIcon](QObject *obj, const QUrl &objUrl) {
        std::cout << ">>> [DEBUG] objectCreated fired! obj=" << (void*)obj << " url=" << objUrl.toString().toStdString() << std::endl;
        std::cout.flush();
        if (!obj && url == objUrl) {
            std::cerr << ">>> [FATAL] Failed to load root QML object: " << objUrl.toString().toStdString() << std::endl;
            std::cerr.flush();
            QCoreApplication::exit(-1);
        } else if (auto win = qobject_cast<QQuickWindow*>(obj)) {
#ifdef _WIN32
            HWND hwnd = reinterpret_cast<HWND>(win->winId());
            if (hwnd) {
                BOOL darkMode = TRUE;
                DwmSetWindowAttribute(hwnd, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &darkMode, sizeof(darkMode));
            }
#endif
            if (!appIcon.isNull()) {
                win->setIcon(appIcon);
            }
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
