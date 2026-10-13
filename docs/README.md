# YuYuInstaller

[English](README_en.md) | [Українська](README_uk.md)

YuYuInstaller, modern ve yüksek performanslı bir Windows medya indirme ve dönüştürme yazılımıdır. 4K'ya kadar video indirme, tek tıkla 320 kbps MP3 dönüştürme ve akıcı bir anime arayüzü sunar.

> [!NOTE]
> **Otomatik Güncelleme:** Uygulamayı bir kez kurduktan sonra yeni sürümleri uygulama içindeki **Güncellemeler** sekmesinden tek tıkla otomatik olarak alabilirsiniz.

---

## Özellikler

- **Çoklu Platform:** YouTube, Instagram, TikTok, Twitter/X, Facebook, Animecix ve yüzlerce siteden medya indirme.
- **Yüksek Kalite:** 4K UHD, 2K QHD ve 1080p FHD video seçenekleri; video başlığını indirmeden önce düzenleyebilme.
- **320 kbps MP3:** FFmpeg motoru ile stüdyo kalitesinde kayıpsız ses ayıklama.
- **Akıllı Kuyruk:** İndirme hız sınırı (MB/s), eşzamanlı / sıralı indirme modları ve duraklat/devam et kontrolleri.
- **Çok Dilli:** Türkçe, İngilizce ve Ukraynaca arayüz desteği.
- **Sessiz Güncelleme:** Masaüstü kısayol yerini koruyan, sıfır UAC arka plan güncelleyicisi.

---

## Hızlı Başlangıç

1. [Releases](https://github.com/Audreiy/Mediainstaller/releases/latest) sayfasından `YuYuInstaller_Setup.exe` dosyasını indirip kurun.
2. İndirmek istediğiniz video veya ses linkini yapıştırıp **Analiz Et** butonuna basın.
3. İstediğiniz kaliteyi veya MP3 seçeneğini belirleyip **İndirmeyi Başlat**'a tıklayın.

---

## Kaynak Koddan Derleme

```powershell
git clone https://github.com/Audreiy/Mediainstaller.git
cd Mediainstaller
.\build.bat        # Uygulamayı derler
.\deploy.bat       # Qt bağımlılıklarını hazırlar
.\make_setup.bat   # Kurulum paketini üretir (Opsiyonel)
```

---

## Lisans

Bu proje [MIT Lisansı](../LICENSE) ile korunmaktadır.
