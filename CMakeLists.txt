cmake_minimum_required(VERSION 3.20)
project(MediaGrab VERSION 1.1.7 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Default to C:/Qt/6.8.2/msvc2022_64 if Qt6_DIR is not explicitly passed
if(NOT Qt6_DIR AND EXISTS "C:/Qt/6.8.2/msvc2022_64/lib/cmake/Qt6")
    list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.8.2/msvc2022_64")
endif()

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Quick Qml QuickControls2 Network)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(MediaGrab
    WIN32
    src/main.cpp
    src/models/MediaInfo.h
    src/models/MediaInfo.cpp
    src/models/DownloadQueueModel.h
    src/models/DownloadQueueModel.cpp
    src/core/UrlAnalyzer.h
    src/core/UrlAnalyzer.cpp
    src/core/Downloader.h
    src/core/Downloader.cpp
    src/core/YtDlpDownloader.h
    src/core/YtDlpDownloader.cpp
    src/core/DownloadManager.h
    src/core/DownloadManager.cpp
    src/core/UpdateManager.h
    src/core/UpdateManager.cpp
    src/media/FFmpegProcessor.h
    src/media/FFmpegProcessor.cpp
    src/media/VideoBackgroundItem.h
    src/media/VideoBackgroundItem.cpp
    app.rc
)

set_source_files_properties(app.rc PROPERTIES
    OBJECT_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/app.ico"
)

target_include_directories(MediaGrab PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/models
    ${CMAKE_CURRENT_SOURCE_DIR}/src/media
)

set_source_files_properties(qml/Theme.qml qml/I18n.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(MediaGrab
    URI MediaGrab
    VERSION 1.0
    RESOURCES
        assets/logo.png
        assets/ffmpeg.png
        assets/ytdlp.png
    QML_FILES
        qml/Main.qml
        qml/Theme.qml
        qml/I18n.qml
        qml/components/Sidebar.qml
        qml/components/UrlInputField.qml
        qml/components/QualitySelector.qml
        qml/components/DownloadCard.qml
        qml/components/ProgressBarCustom.qml
        qml/views/DashboardView.qml
        qml/views/QueueView.qml
        qml/views/UpdatesView.qml
        qml/views/SettingsView.qml
)

set_target_properties(MediaGrab PROPERTIES
    OUTPUT_NAME "YuYuInstaller"
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(MediaGrab PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Qml
    Qt6::QuickControls2
    Qt6::Network
    mfplat
    mfreadwrite
    mfuuid
    propsys
    dwmapi
)

# Performance & Compiler Optimizations (MSVC)
if(MSVC)
    add_compile_options(
        /arch:AVX2
        /Oi
        /Ot
        /GF
        /Gy
    )
    add_link_options(
        /OPT:REF
        /OPT:ICF
    )
endif()

target_precompile_headers(MediaGrab PRIVATE
    <QtCore>
    <QtGui>
    <QtQuick>
    <QtNetwork>
    <QObject>
    <QString>
    <QUrl>
)

# Optional libcurl linking if found
find_package(CURL QUIET)
if(CURL_FOUND)
    target_link_libraries(MediaGrab PRIVATE CURL::libcurl)
    target_compile_definitions(MediaGrab PRIVATE HAVE_LIBCURL=1)
    message(STATUS "libcurl found and linked")
else()
    message(STATUS "libcurl not found yet; will use Qt Network or bundled curl worker")
endif()
