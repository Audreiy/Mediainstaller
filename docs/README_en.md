# YuYuInstaller

[Türkçe](README.md) | [Українська](README_uk.md)

YuYuInstaller is a next-generation Windows media downloader and audio converter built with modern C++20 and Qt 6 Quick / QML. It features a hardware-accelerated anime video background, multi-language support (English, Turkish, Ukrainian), and a high-performance streaming engine.

---

## Key Features

- **Modern & Fluid QML User Interface:**
  - DirectX/Direct3D/Vulkan hardware-accelerated dark anime theme.
  - Video background engine capped at 45 FPS; when paused, it freezes on the last displayed frame just like a YouTube video player with 0% idle CPU utilization.
- **High-Speed Direct Stream Engine:**
  - Direct-to-disk streaming architecture over HTTP/HTTPS ensuring blazing-fast download speeds without memory overhead.
- **Studio-Quality 320 kbps MP3 Conversion via FFmpeg:**
  - Lossless audio extraction from online videos converted to crisp 320 kbps MP3 or AAC tracks.
- **Multi-Platform & Stream Resolving:**
  - Automatically identifies and resolves media streams from YouTube, Animecix (Tau Video), Vidmoly, Sibnet, TikTok, Instagram, Twitter (X), and direct media links.
- **Advanced Download Queue:**
  - Concurrent (all-at-once), sequential, and custom download limit modes.
  - Real-time download speed meters (MB/s), estimated time remaining (ETA), and animated progress bars.
  - Pause, resume, cancel, clear finished downloads, and direct file reveal in Windows Explorer.
- **Multi-Language System (i18n):**
  - Seamless, instant language switching between English (default), Turkish, and Ukrainian from the Settings tab.
- **Automatic & Secure Self-Updating:**
  - SSL/HTTPS-encrypted update checking directly against GitHub Releases. When a new version is released, downloads the setup package and updates seamlessly in silent mode with auto-restart.
- **Clean Windows Installer:**
  - Packaged with Inno Setup 6 featuring modern aesthetics, desktop shortcut position preservation, and complete cleanup on uninstallation.

---

## User Guide

Using YuYuInstaller is fast and effortless:

### 1. Adding a Media Link
* Copy any video or music link from a supported platform (YouTube, Animecix, TikTok, Instagram, etc.).
* Paste it into the link input box in the **Downloader** tab and click **Analyze**.

### 2. Choosing Format & Quality
* Once analysis is complete, media metadata and options will be displayed:
  * **Video (MP4 / Best Quality):** Highest resolution video merged with full-fidelity audio.
  * **Audio Only (320 kbps MP3):** Pure audio track converted to studio-grade 320 kbps MP3 via FFmpeg.
* Optionally specify a custom destination folder via the **Change Folder** button.

### 3. Downloading & Managing the Queue
* Click **Start Download**.
* The download task is automatically added to the **Download Queue** tab.
* In the Queue tab, you can:
  * Monitor real-time download speeds (MB/s) and time remaining (ETA).
  * Pause, resume, or cancel downloads at any time.
  * Once completed, click **Open Folder** to reveal the downloaded file directly in Windows Explorer.

### 4. Customizing Settings
* From the **Settings** tab:
  * **Interface Language:** Switch between English (EN), Turkish (TR), or Ukrainian (UA).
  * **Video Background:** Toggle the background animation on/off with a single switch. Pausing freezes the video frame cleanly with zero CPU impact.
  * **Default Download Folder:** Choose where downloads are saved by default.
  * **Engine Status:** Verify that external tools (FFmpeg and yt-dlp) are detected and operational.

---

## Technology Stack

* **Core Language:** C++20 (MSVC 2026)
* **User Interface:** Qt 6.8 (Qt Quick, QML, Qt Quick Controls 2)
* **Video Rendering:** Windows Media Foundation (`mfplat`, `mfreadwrite`), QQuickFramebufferObject
* **Media Binaries:** FFmpeg 7.x, yt-dlp
* **Packaging:** Inno Setup 6 (Modern Wizard, LZMA2 Ultra Compression)

---

## Repository Structure

```text
YuYuInstaller/
├── CMakeLists.txt              # CMake build definitions and module configuration
├── installer.iss               # Inno Setup installation script
├── build.bat                   # Release build automation script (Ninja & MSVC)
├── deploy.bat                  # Qt runtime dependency deployment (windeployqt)
├── make_setup.bat              # One-click installer generation script
├── app.rc                      # Windows application icon and metadata
├── assets/                     # Logos, application icons, and background video
├── src/
│   ├── main.cpp                # Application entry point and QML engine registration
│   ├── core/                   # Download workers, stream analyzers, and manager
│   ├── media/                  # Media Foundation video playback & FFmpeg converter
│   └── models/                 # Reactive list models for QML
└── qml/
    ├── Main.qml                # Main application window
    ├── Theme.qml               # Design tokens, color palette, and styling constants
    ├── I18n.qml                # Multi-language localization singleton
    ├── components/             # Reusable UI components
    └── views/                  # Primary views (Dashboard, Queue, Settings, Updates)
```

---

## Building from Source

### Prerequisites
* Windows 10 / 11 (64-bit)
* Visual Studio 2022 / 2026 (with Desktop C++ workload)
* Qt 6.8+ (MSVC 2022 64-bit)
* CMake 3.20+ & Ninja
* Inno Setup 6 (required for installer generation)

### Build Instructions

1. **Clone the Repository:**
   ```powershell
   git clone https://github.com/Audreiy/Mediainstaller.git
   cd Mediainstaller
   ```

2. **Build the Application:**
   ```powershell
   .\build.bat
   ```

3. **Deploy Qt Dependencies:**
   ```powershell
   .\deploy.bat
   ```

4. **Package the Installer (Optional):**
   ```powershell
   .\make_setup.bat
   ```
   *The compiled installer will be located at `setup_output\YuYuInstaller_Setup.exe`.*
