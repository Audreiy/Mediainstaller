# MediaGrab

MediaGrab, C++20 ve Qt 6 Quick / QML kullanılarak geliştirilmiş, donanım hızlandırmalı, modern koyu temalı bir Windows medya indirme ve ses dönüştürme uygulamasıdır.

## 🚀 Temel Özellikler

- **Modern & Akıcı QML Arayüzü:** Donanım hızlandırmalı (DirectX/Vulkan) karanlık tema, mikro-animasyonlar ve kart tabanlı dinamik arayüz.
- **Doğrudan Medya İndirme:** HTTP/HTTPS üzerinden yüksek hızlı, doğrudan diske akışlı (RAM şişirmeyen) indirme motoru.
- **İlerleme, Hız ve ETA Takibi:** Anlık indirme hızı (MB/s), tahmini kalan süre ve animasyonlu ilerleme çubuğu.
- **FFmpeg ile Ses Dönüştürme:** Videolardan 320 kbps kalitede MP3 çıkarma ve format dönüştürme.
- **Kuyruk Yönetimi:** İndirmeleri duraklatma, devam ettirme, iptal etme ve tamamlanan dosyayı doğrudan Windows Gezgini'nde açma.

## 🛠️ Teknoloji Yığını

- **Dil:** C++20
- **Arayüz:** Qt 6.8 (Qt Quick / QML)
- **Derleyici & Araçlar:** MSVC 2022 (v144/v145), CMake, Ninja
- **Medya Motoru:** FFmpeg 9.0 (Gyan build)
- **Ağ Motoru:** Qt Network (ve libcurl opsiyonel desteği)

## 📁 Proje Yapısı

```
MediaGrab/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── main.cpp
│   ├── core/
│   │   ├── UrlAnalyzer.h / .cpp       # URL doğrulama, analiz ve format tespiti
│   │   ├── Downloader.h / .cpp        # Disk akışlı asenkron indirme
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
    ├── components/                    # Yeniden kullanılabilir QML bileşenleri
    └── views/                         # Sayfa görünümleri (İndirici, Kuyruk, Ayarlar)
```
