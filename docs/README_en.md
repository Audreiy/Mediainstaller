# YuYuInstaller

[Türkçe](../README.md) | [Українська](README_uk.md)

YuYuInstaller is a high-performance Windows media downloader and audio converter built with modern C++20 and Qt 6. It offers up to 4K video downloads, 1-click 320 kbps MP3 extraction, and a fluid dark anime interface.

> [!NOTE]
> **Auto-Update:** Once installed, you don't need to manually download new `.exe` installers. Updates can be installed with 1-click directly from the **Updates** tab.

---

## Features

- **Multi-Platform:** Download from YouTube, Instagram, TikTok, Twitter/X, Facebook, Animecix, and hundreds of sites.
- **High Resolution:** Up to 4K UHD, 2K QHD, and 1080p FHD video; editable video titles prior to downloading.
- **320 kbps MP3:** Studio-quality audio extraction powered by FFmpeg.
- **Smart Queue:** Speed limit caps (MB/s), concurrent or sequential modes, pause & resume.
- **Multi-Language:** English, Turkish, and Ukrainian support.


---

## Quick Start

1. Download and run `YuYuInstaller_Setup.exe` from [Releases](https://github.com/Audreiy/Mediainstaller/releases/latest).
2. Paste a video or audio link and click **Analyze**.
3. Select your desired resolution or MP3 format, then click **Start Download**.

---

## Build from Source

```powershell
git clone https://github.com/Audreiy/Mediainstaller.git
cd Mediainstaller
.\build.bat        # Compiles the application
.\deploy.bat       # Deploys Qt runtime binaries
.\make_setup.bat   # Packages the installer (Optional)
```

---

## License

This project is licensed under the [MIT License](../LICENSE).
