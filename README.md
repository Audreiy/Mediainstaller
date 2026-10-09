# YuYuİnstaller

YuYuİnstaller, C++20 ve Qt 6 Quick / QML kullanılarak geliştirilmiş, donanım hızlandırmalı, çok dilli (Türkçe 🇹🇷, İngilizce 🇬🇧, Rusça 🇷🇺), modern koyu temalı bir Windows medya indirme ve ses dönüştürme uygulamasıdır.

## 🚀 Temel Özellikler

- **Modern & Akıcı QML Arayüzü:** Donanım hızlandırmalı (DirectX/Vulkan) anime temalı karanlık tasarım, mikro-animasyonlar ve kart tabanlı dinamik arayüz.
- **Çoklu Dil Desteği (i18n):** Ayarlar sayfasından anlık olarak Türkçe, İngilizce ve Rusça dilleri arasında kesintisiz geçiş.
- **Otomatik Akış & Platform Çözümleyici:** Animecix (Tau Video, vb.), YouTube, Vidmoly, Sibnet, TikTok, Instagram ve doğrudan medya linklerini otomatik olarak algılayıp en yüksek kalitede çözer.
- **Doğrudan Medya İndirme:** HTTP/HTTPS üzerinden yüksek hızlı, parçalı ve doğrudan diske akışlı (RAM şişirmeyen) indirme motoru.
- **İlerleme, Hız ve ETA Takibi:** Anlık indirme hızı (MB/s), tahmini kalan süre ve animasyonlu ilerleme çubuğu.
- **FFmpeg ile Ses Dönüştürme:** Videolardan 320 kbps kalitede MP3 çıkarma ve format dönüştürme.
- **Kuyruk Yönetimi:** İndirmeleri duraklatma, devam ettirme, iptal etme, bitenleri temizleme ve tamamlanan dosyayı doğrudan Windows Gezgini'nde açma.

## 🛠️ Teknoloji Yığını

- **Dil:** C++20
- **Arayüz:** Qt 6.8 (Qt Quick / QML)
- **Derleyici & Araçlar:** MSVC 2026, CMake, Ninja
- **Medya Motoru:** FFmpeg, yt-dlp
- **Ağ Motoru:** Qt Network

## 📁 Proje Yapısı

```
YuYuInstaller/
├── CMakeLists.txt
├── README.md
├── build.bat
├── deploy.bat
├── app.rc
├── assets/
│   ├── app.ico
│   └── logo.png
├── src/
│   ├── main.cpp
│   ├── core/
│   │   ├── UrlAnalyzer.h / .cpp       # URL doğrulama, Animecix & TauVideo çözümleyici
│   │   ├── Downloader.h / .cpp        # Disk akışlı asenkron indirme
│   │   ├── YtDlpDownloader.h / .cpp   # yt-dlp akış indirici
│   │   └── DownloadManager.h / .cpp   # Kuyruk ve indirme yöneticisi
│   ├── media/
│   │   ├── FFmpegProcessor.h / .cpp   # FFmpeg ses çıkarma ve akış birleştirme
│   │   └── AudioConverter.h           # Ses formatı yapılandırmaları
│   └── models/
│       ├── MediaInfo.h / .cpp         # Medya üst veri modeli
│       └── DownloadQueueModel.h / .cpp# QML ListView için reaktif C++ modeli
└── qml/
    ├── Main.qml                       # Ana pencere
    ├── Theme.qml                      # Tasarım sistemi & renk paleti
    ├── I18n.qml                       # Reaktif çoklu dil yönetim singleton'ı
    ├── components/                    # Yeniden kullanılabilir QML bileşenleri
    └── views/                         # Sayfa görünümleri (İndirici, Kuyruk, Ayarlar)
```
