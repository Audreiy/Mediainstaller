pragma Singleton
import QtQuick

QtObject {
    id: root

    // Supported: "en", "tr", "ru" - Default: English ("en")
    property string language: (typeof downloadManager !== "undefined" && downloadManager) ? downloadManager.language : "en"

    readonly property var translations: ({
        "en": {
            // General
            "appName": "YuYuInstaller",
            "appVersion": "v1.0 Pro",
            "appSubtitle": "Media Downloader & Converter",
            
            // Sidebar
            "navDownloader": "Downloader",
            "navQueue": "Download Queue",
            "navSettings": "Settings",
            "activeDownloads": "Active",
            "ready": "Ready",

            // Dashboard
            "dashTitle": "Media Downloader & Converter",
            "dashSubtitle": "Paste any video or audio URL, select your desired quality, and download at maximum speed.",
            "urlPlaceholder": "Paste URL from YouTube, Animecix, Vidmoly, TikTok, Instagram, Twitter...",
            "pasteBtn": "Paste",
            "analyzeBtn": "Analyze",
            "analyzing": "Analyzing media stream...",
            "directFileBadge": "Direct Media Stream Detected",
            "directFileDesc": "This link contains an uncompressed stream or direct file. You can download it directly at top speed.",
            "startDownload": "Start Download",
            "chooseQuality": "Format & Quality Selection",
            "selectResolution": "Video Resolution:",
            "selectBitrate": "Audio Bitrate:",
            "qualityBest": "Best (Auto)",
            "videoAudio": "Video (MP4 / Best Quality)",
            "videoAudioDesc": "Highest resolution video + merged audio",
            "audioMp3": "Audio Only (320 kbps MP3)",
            "audioMp3Desc": "Studio-quality audio track converted with FFmpeg",
            "saveTo": "Save To",
            "changeFolder": "Change Folder",
            "feature1Title": "High-Speed Stream Engine",
            "feature1Desc": "Direct-to-disk streaming architecture ensures blazing fast speeds with zero memory overhead.",
            "feature2Title": "320 kbps MP3 Converter",
            "feature2Desc": "Integrated FFmpeg processor seamlessly extracts studio-quality MP3 and AAC audio directly from videos.",

            // Queue
            "queueTitle": "Download Queue",
            "queueSubtitle": "%1 active • Total %2 items",
            "clearFinished": "Clear Finished",
            "noDownloads": "No Downloads in Queue",
            "noDownloadsHint": "Paste a media link in the Downloader tab to begin.",
            "openFolder": "Open Folder",
            "pause": "Pause",
            "resume": "Resume",
            "cancel": "Cancel",
            "statusDownloading": "Downloading",
            "statusConverting": "Converting (MP3)",
            "statusCompleted": "Completed",
            "statusPaused": "Paused",
            "statusQueued": "Queued",
            "statusFailed": "Failed",
            "statusCancelled": "Cancelled",
            "waitingInQueue": "Waiting in queue...",
            "downloadModeConcurrent": "All at Once",
            "downloadModeSequential": "Sequential",
            "moveUp": "Move Up",
            "moveDown": "Move Down",

            // Settings
            "settingsTitle": "Settings & Preferences",
            "settingsSubtitle": "Customize language, download directory, and media processing engine.",
            "langSection": "Display Language",
            "langSubtitle": "Choose your preferred user interface language.",
            "langEn": "English",
            "langTr": "Türkçe",
            "langRu": "Русский",
            "videoBgSection": "Animated Video Background",
            "videoBgEnabled": "Playing",
            "videoBgDisabled": "Paused (Frozen Frame)",
            "storageSection": "Default Download Folder",
            "storageSubtitle": "Downloaded files will be automatically saved to this location.",
            "browse": "Browse",
            "engineStatus": "Processing Engine Status",
            "engineSubtitle": "Hardware acceleration and media decoding binaries.",
            "ffmpegTitle": "FFmpeg Media Processor",
            "ffmpegAvailable": "FFmpeg is installed and ready for media merging & audio conversion.",
            "ffmpegNotFound": "FFmpeg was not detected on this system.",
            "ytdlpTitle": "yt-dlp Video Engine",
            "ytdlpAvailable": "yt-dlp is installed and ready for YouTube & online video downloads.",
            "ytdlpNotFound": "yt-dlp was not detected on this system.",
            "readyStatus": "Ready",
            "missingStatus": "Missing",
            "aboutTitle": "About YuYuInstaller",
            "aboutDesc": "YuYuInstaller is a modern Windows media downloader and converter built with C++20, Qt 6 Quick / QML, FFmpeg, and yt-dlp for high-performance direct streaming and 320 kbps audio extraction.",
            "aboutVersion": "Version 1.0.0 Pro • Hardware-Accelerated QML Interface"
        },
        "tr": {
            // General
            "appName": "YuYuİnstaller",
            "appVersion": "v1.0 Pro",
            "appSubtitle": "Medya İndirici & Dönüştürücü",

            // Sidebar
            "navDownloader": "İndirici",
            "navQueue": "İndirme Kuyruğu",
            "navSettings": "Ayarlar",
            "activeDownloads": "Aktif",
            "ready": "Hazır",

            // Dashboard
            "dashTitle": "Medya İndirici & Dönüştürücü",
            "dashSubtitle": "Video veya ses bağlantısını yapıştırın, kaliteyi seçin ve maksimum hızda indirin.",
            "urlPlaceholder": "YouTube, Animecix, Vidmoly, TikTok, Instagram, Twitter veya medya bağlantısı yapıştırın...",
            "pasteBtn": "Yapıştır",
            "analyzeBtn": "Analiz Et",
            "analyzing": "Medya akışı taranıyor...",
            "directFileBadge": "Doğrudan Medya Akışı Algılandı",
            "directFileDesc": "Bu bağlantı doğrudan video/ses dosyası içeriyor. Son hızda indirebilirsiniz.",
            "startDownload": "İndirmeyi Başlat",
            "chooseQuality": "Format ve Kalite Seçimi",
            "selectResolution": "Video Çözünürlüğü:",
            "selectBitrate": "Ses Kalitesi (Bitrate):",
            "qualityBest": "En Yüksek (Auto)",
            "videoAudio": "Video (MP4 / En Yüksek Kalite)",
            "videoAudioDesc": "En yüksek çözünürlüklü video ve birleştirilmiş ses akışı",
            "audioMp3": "Yalnızca Ses (320 kbps MP3)",
            "audioMp3Desc": "FFmpeg ile stüdyo kalitesinde dönüştürülmüş MP3 ses parçası",
            "saveTo": "Kaydedilecek Yer",
            "changeFolder": "Klasörü Değiştir",
            "feature1Title": "Turbo Çok Kanallı İndirme",
            "feature1Desc": "Eşzamanlı parça akışı indirmeleri 10 kata kadar hızlandırır ve doğrudan diske yazar.",
            "feature2Title": "320 kbps MP3 Dönüştürücü",
            "feature2Desc": "Entegre FFmpeg işlemcisi videolardan stüdyo kalitesinde MP3 ses çıkarır.",

            // Queue
            "queueTitle": "İndirme Kuyruğu",
            "queueSubtitle": "%1 aktif • Toplam %2 öğe",
            "clearFinished": "Bitenleri Temizle",
            "noDownloads": "Kuyrukta İndirme Yok",
            "noDownloadsHint": "İndirmeye başlamak için İndirici sekmesinden bir bağlantı yapıştırın.",
            "openFolder": "Klasörü Aç",
            "pause": "Duraklat",
            "resume": "Devam Et",
            "cancel": "İptal Et",
            "statusDownloading": "İndiriliyor",
            "statusConverting": "Dönüştürülüyor (MP3)",
            "statusCompleted": "Tamamlandı",
            "statusPaused": "Duraklatıldı",
            "statusQueued": "Sırada",
            "statusFailed": "Başarısız",
            "statusCancelled": "İptal Edildi",
            "waitingInQueue": "Sırada bekliyor...",
            "downloadModeConcurrent": "Aynı Anda",
            "downloadModeSequential": "Sırayla İndir",
            "moveUp": "Yukarı Taşı",
            "moveDown": "Aşağı Taşı",

            // Settings
            "settingsTitle": "Ayarlar ve Tercihler",
            "settingsSubtitle": "Uygulama dilini, indirme klasörünü ve medya işleme araçlarını özelleştirin.",
            "langSection": "Arayüz Dili",
            "langSubtitle": "Uygulamanın görüntüleneceği dili seçin.",
            "langEn": "English",
            "langTr": "Türkçe",
            "langRu": "Русский",
            "videoBgSection": "Video Arka Plan",
            "videoBgEnabled": "Oynatılıyor",
            "videoBgDisabled": "Duraklatıldı (Sabit Kare)",
            "storageSection": "Varsayılan İndirme Klasörü",
            "storageSubtitle": "İndirilen tüm medyalar doğrudan bu klasöre kaydedilir.",
            "browse": "Gözat",
            "engineStatus": "İşleme Motoru Durumu",
            "engineSubtitle": "Donanım hızlandırma ve medya çözücü ikili araçlar.",
            "ffmpegTitle": "FFmpeg Medya İşlemcisi",
            "ffmpegAvailable": "FFmpeg kurulu ve medya birleştirme / ses dönüştürme için hazır.",
            "ffmpegNotFound": "FFmpeg bu sistemde bulunamadı.",
            "ytdlpTitle": "yt-dlp Video Motoru",
            "ytdlpAvailable": "yt-dlp kurulu ve YouTube / web indirmeleri için hazır.",
            "ytdlpNotFound": "yt-dlp bu sistemde bulunamadı.",
            "readyStatus": "Hazır",
            "missingStatus": "Bulunamadı",
            "aboutTitle": "YuYuİnstaller Hakkında",
            "aboutDesc": "YuYuİnstaller; C++20, Qt 6 Quick / QML, FFmpeg ve yt-dlp ile güçlendirilmiş, yüksek performanslı yeni nesil medya indirme ve ses dönüştürme yazılımıdır.",
            "aboutVersion": "Sürüm 1.0.0 Pro • Donanım Hızlandırmalı QML Arayüzü"
        },
        "ru": {
            // General
            "appName": "YuYuInstaller",
            "appVersion": "v1.0 Pro",
            "appSubtitle": "Загрузчик и Конвертер Медиа",

            // Sidebar
            "navDownloader": "Загрузчик",
            "navQueue": "Очередь загрузок",
            "navSettings": "Настройки",
            "activeDownloads": "Активно",
            "ready": "Готово",

            // Dashboard
            "dashTitle": "Загрузчик и Конвертер Медиа",
            "dashSubtitle": "Вставьте ссылку на видео или аудио, выберите качество и скачивайте на максимальной скорости.",
            "urlPlaceholder": "Вставьте ссылку с YouTube, Animecix, Vidmoly, TikTok, Instagram, Twitter...",
            "pasteBtn": "Вставить",
            "analyzeBtn": "Анализ",
            "analyzing": "Анализ медиапотока...",
            "directFileBadge": "Обнаружен прямой медиапоток",
            "directFileDesc": "Эта ссылка содержит прямой файл. Вы можете скачать его на максимальной скорости.",
            "startDownload": "Начать загрузку",
            "chooseQuality": "Выбор формата и качества",
            "selectResolution": "Разрешение видео:",
            "selectBitrate": "Качество звука (Битрейт):",
            "qualityBest": "Максимум (Авто)",
            "videoAudio": "Видео (MP4 / Лучшее качество)",
            "videoAudioDesc": "Видео высокого разрешения со сведенным звуком",
            "audioMp3": "Только аудио (320 kbps MP3)",
            "audioMp3Desc": "Аудиодорожка студийного качества, преобразованная через FFmpeg",
            "saveTo": "Сохранить в",
            "changeFolder": "Изменить папку",
            "feature1Title": "Турбо загрузка потоков",
            "feature1Desc": "Прямая запись на диск обеспечивает молниеносную скорость без нагрузки на память.",
            "feature2Title": "Конвертер MP3 320 кбит/с",
            "feature2Desc": "Встроенный FFmpeg без потерь извлекает MP3 и AAC напрямую из видео.",

            // Queue
            "queueTitle": "Очередь загрузок",
            "queueSubtitle": "%1 активно • Всего %2 файлов",
            "clearFinished": "Очистить завершенные",
            "noDownloads": "В очереди нет загрузок",
            "noDownloadsHint": "Вставьте ссылку во вкладке «Загрузчик», чтобы начать.",
            "openFolder": "Открыть папку",
            "pause": "Пауза",
            "resume": "Продолжить",
            "cancel": "Отмена",
            "statusDownloading": "Загрузка",
            "statusConverting": "Конвертация (MP3)",
            "statusCompleted": "Завершено",
            "statusPaused": "Приостановлено",
            "statusQueued": "В очереди",
            "statusFailed": "Ошибка",
            "statusCancelled": "Отменено",
            "waitingInQueue": "Ожидание очереди...",
            "downloadModeConcurrent": "Одновременно",
            "downloadModeSequential": "По очереди",
            "moveUp": "Вверх",
            "moveDown": "Вниз",

            // Settings
            "settingsTitle": "Настройки и Параметры",
            "settingsSubtitle": "Настройте язык интерфейса, папку загрузок и движки обработки.",
            "langSection": "Язык интерфейса",
            "langSubtitle": "Выберите удобный язык приложения.",
            "langEn": "English",
            "langTr": "Türkçe",
            "langRu": "Русский",
            "videoBgSection": "Анимированный видеофон",
            "videoBgEnabled": "Воспроизводится",
            "videoBgDisabled": "Пауза (Застывший кадр)",
            "storageSection": "Папка загрузки по умолчанию",
            "storageSubtitle": "Все скачанные файлы будут автоматически сохраняться сюда.",
            "browse": "Обзор",
            "engineStatus": "Статус инструментов обработки",
            "engineSubtitle": "Бинарные модули аппаратного ускорения и декодирования.",
            "ffmpegTitle": "Медиапроцессор FFmpeg",
            "ffmpegAvailable": "FFmpeg установлен и готов к объединению потоков и конвертации аудио.",
            "ffmpegNotFound": "FFmpeg не обнаружен в системе.",
            "ytdlpTitle": "Видеодвижок yt-dlp",
            "ytdlpAvailable": "yt-dlp установлен и готов к загрузке видео с YouTube и веб-сайтов.",
            "ytdlpNotFound": "yt-dlp не обнаружен в системе.",
            "readyStatus": "Готово",
            "missingStatus": "Не найден",
            "aboutTitle": "О программе YuYuInstaller",
            "aboutDesc": "YuYuInstaller — это современный загрузчик и конвертер медиа для Windows, созданный на C++20, Qt 6 Quick / QML, FFmpeg и многопоточных стрим-модулях.",
            "aboutVersion": "Версия 1.0.0 Pro • Аппаратно-ускоренный интерфейс QML"
        }
    })

    function t(key) {
        var dict = translations[root.language];
        if (!dict) dict = translations["en"];
        if (dict && dict[key] !== undefined) {
            return dict[key];
        }
        var fallback = translations["en"];
        if (fallback && fallback[key] !== undefined) {
            return fallback[key];
        }
        return key;
    }

    function setLanguage(lang) {
        if (lang === "en" || lang === "tr" || lang === "ru") {
            root.language = lang;
            if (typeof downloadManager !== "undefined" && downloadManager) {
                downloadManager.setLanguage(lang);
            }
        }
    }
}
