# YuYuInstaller 🎥⚡

[🇬🇧 English](README_en.md) | [🇺🇦 Українська](README_uk.md)

YuYuInstaller, modern C++20 ve Qt 6 Quick / QML mimarisiyle geliştirilmiş; donanım hızlandırmalı anime video arka planına, çok dilli arayüze (İngilizce 🇬🇧, Türkçe 🇹🇷, Ukraynaca 🇺🇦) ve yüksek performanslı akış motoruna sahip yeni nesil bir Windows medya indirme ve ses dönüştürme yazılımıdır.

---

## 🚀 Öne Çıkan Özellikler

- **🎨 Modern & Akıcı QML Arayüzü:** 
  - DirectX/Direct3D/Vulkan donanım hızlandırmalı modern koyu anime teması.
  - Arka planda 45 FPS tavanlı, duraklatıldığında YouTube oynatıcısı gibi son karede sabit kalan ve %0 CPU harcayan akıllı video arka plan motoru.
- **⚡ Yüksek Hızlı Doğrudan Akış Motoru:**
  - HTTP/HTTPS üzerinden parçalı ve doğrudan diske yazan (RAM şişirmeyen) turbo indirme mimarisi.
- **🎵 FFmpeg ile 320 kbps Stüdyo Kalitesinde MP3 Dönüştürme:**
  - Videolardan kayıpsız ses ayıklama ve ultra yüksek kaliteli MP3/AAC dönüştürme.
- **🌐 Çoklu Platform & Akış Çözümleme:**
  - YouTube, Animecix (Tau Video), Vidmoly, Sibnet, TikTok, Instagram, Twitter (X) ve doğrudan medya bağlantılarını otomatik algılama ve çözümleme.
- **📊 Gelişmiş İndirme Kuyruğu:**
  - Eşzamanlı (aynı anda) veya sıralı indirme modları.
  - Anlık hız (MB/s), kalan süre (ETA) ve animasyonlu ilerleme çubukları.
  - Duraklatma, devam ettirme, iptal etme, bitenleri temizleme ve doğrudan Windows Gezgini'nde açma.
- **🌍 Çok Dilli Altyapı (i18n):**
  - Ayarlar sayfasından anlık olarak İngilizce (varsayılan), Türkçe ve Ukraynaca arasında kesintisiz geçiş.
- **🔄 Otomatik & Güvenli Güncelleme Motoru:**
  - GitHub Releases üzerinden SSL/HTTPS korumalı otomatik sürüm denetimi. Yeni sürüm çıktığında tek tıkla arka planda indirip sessizce kendini güncelleme ve yeniden başlatma desteği.
- **📦 Profesyonel Windows Kurulum Paketi:**
  - Inno Setup 6 ile paketlenmiş, tek tıkla kurulan ve arkasında artık dosya bırakmayan temiz kaldırıcı desteği.

---

## 📖 Kullanım Kılavuzu

Uygulamayı kullanmak son derece basittir:

### 1. Medya Bağlantısını Ekleme
* Desteklenen bir platformdan (YouTube, Animecix, TikTok, Instagram vb.) video veya müzik bağlantısını kopyalayın.
* **İndirici (Downloader)** sekmesindeki bağlantı kutusuna yapıştırın ve **Analiz Et (Analyze)** butonuna tıklayın.

### 2. Format ve Kalite Seçimi
* Analiz tamamlandığında medya bilgileri kart olarak görüntülenir.
* İhtiyacınıza göre format seçin:
  * **Video (MP4 / En Yüksek Kalite):** En yüksek çözünürlüklü video ile birleştirilmiş ses akışı.
  * **Yalnızca Ses (320 kbps MP3):** FFmpeg motoru ile stüdyo kalitesinde dönüştürülmüş saf ses dosyası.
* İsteğe bağlı olarak **Klasörü Değiştir (Change Folder)** seçeneğiyle indirme hedefini belirleyin.

### 3. İndirme ve Kuyruk Yönetimi
* **İndirmeyi Başlat (Start Download)** butonuna basın.
* İndirme işlemi otomatik olarak **İndirme Kuyruğu (Queue)** sekmesine aktarılır.
* Kuyruk sekmesinde:
  * Anlık indirme hızınızı (MB/s) ve kalan süreyi (ETA) takip edebilirsiniz.
  * İndirmeleri dilediğiniz zaman duraklatabilir, sürdürebilir veya iptal edebilirsiniz.
  * İndirme tamamlandığında **Klasörü Aç (Open Folder)** butonuna tıklayarak dosyaya doğrudan Windows Gezgini üzerinden ulaşabilirsiniz.

### 4. Ayarları Özelleştirme
* **Ayarlar (Settings)** sekmesinden:
  * **Arayüz Dili:** İngilizce (EN), Türkçe (TR) veya Ukraynaca (UA) dilleri arasında geçiş yapabilirsiniz.
  * **Video Arka Plan:** Arka plan animasyonunu dilediğiniz zaman tek anahtarla duraklatabilir (sabit kare) veya tekrar oynatabilirsiniz.
  * **Varsayılan İndirme Klasörü:** Dosyaların kaydedileceği varsayılan dizini değiştirebilirsiniz.
  * **Motor Durumu:** FFmpeg ve yt-dlp araçlarının sistemdeki durumunu kontrol edebilirsiniz.

---

## 🛠️ Teknoloji Mimarisi

* **Çekirdek Dil:** C++20 (MSVC 2026)
* **Kullanıcı Arayüzü:** Qt 6.8 (Qt Quick, QML, Qt Quick Controls 2)
* **Video İşleme & Render:** Windows Media Foundation (`mfplat`, `mfreadwrite`), QQuickFramebufferObject
* **Harici Araçlar:** FFmpeg 7.x, yt-dlp
* **Kurulum Sistemi:** Inno Setup 6 (Modern Wizard, LZMA2 Ultra Compression)

---

## 📁 Dizin Yapısı

```text
YuYuInstaller/
├── CMakeLists.txt              # CMake derleme tanımları ve modül yapılandırması
├── installer.iss               # Inno Setup profesyonel kurulum betiği
├── build.bat                   # Ninja & MSVC ile Release derleme scripti
├── deploy.bat                  # Qt DLL ve eklentilerini toplama scripti (windeployqt)
├── make_setup.bat              # Kurulum dosyasını (Setup.exe) derleme scripti
├── app.rc                      # Windows uygulama ikonu ve meta verileri
├── assets/                     # Uygulama simgeleri, logolar ve arka plan videosu
├── src/
│   ├── main.cpp                # Başlangıç noktası ve QML motor kaydı
│   ├── core/                   # İndiriciler, analizciler ve kuyruk yöneticisi
│   ├── media/                  # Media Foundation video render ve FFmpeg işleyicisi
│   └── models/                 # QML reaktif liste modelleri
└── qml/
    ├── Main.qml                # Ana kabuk pencere
    ├── Theme.qml               # Tasarım sistemi, renkler ve stil sabitleri
    ├── I18n.qml                # Çoklu dil (EN/TR/UA) yönetim singleton'ı
    ├── components/             # Reusable QML bileşenleri
    └── views/                  # Sayfa görünümleri (Dashboard, Queue, Settings)
```

---

## 🔨 Kaynak Koddan Derleme

### Gereksinimler
* Windows 10 / 11 (64-bit)
* Visual Studio 2022 / 2026 (C++ Masaüstü Geliştirme araçları ile)
* Qt 6.8+ (MSVC 2022 64-bit bileşenleri)
* CMake 3.20+ ve Ninja
* Inno Setup 6 (Kurulum paketi üretimi için)

### Adımlar

1. **Depoyu Klonlayın:**
   ```powershell
   git clone https://github.com/Audreiy/Mediainstaller.git
   cd Mediainstaller
   ```

2. **Uygulamayı Derleyin:**
   ```powershell
   .\build.bat
   ```

3. **Qt Bağımlılıklarını Dağıtın (Deploy):**
   ```powershell
   .\deploy.bat
   ```

4. **Kurulum Dosyasını Oluşturun (Opsiyonel):**
   ```powershell
   .\make_setup.bat
   ```
   *Çıktı dosyası `setup_output\YuYuInstaller_Setup.exe` konumunda oluşturulacaktır.*
