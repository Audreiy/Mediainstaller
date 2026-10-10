# YuYuInstaller Development & Release Standards

## Critical Rules for Auto-Updates & Installers

### 1. Desktop Icon Position Preservation (Masaüstü İkonunu Asla Değiştirmeme)
- **Problem**: When Inno Setup updates an existing installation, deleting and recreating the desktop `.lnk` shortcut causes Windows Explorer to treat it as a new file and moves it to the first free grid slot (top-left).
- **Mandatory Rule**: `installer.iss` MUST ALWAYS include `Check: ShouldCreateDesktopIcon` on `{autodesktop}`. If `YuYuInstaller.lnk` already exists in `{autodesktop}`, `{userdesktop}`, or `{commondesktop}` (Public Desktop), Inno Setup MUST NOT recreate or touch the shortcut.
- The existing shortcut target automatically points to the updated `.exe` because the executable is updated in-place. The user's custom desktop position remains 100% untouched.

---

### 2. Zero-UAC Silent Updates (Discord / Spotify Model)
- `installer.iss` MUST set `PrivilegesRequired=lowest` and `PrivilegesRequiredOverridesAllowed=dialog commandline`.
- Default install directory is `{autopf}\{#MyAppName}`, which resolves to `%LOCALAPPDATA%\Programs\YuYuInstaller` for non-elevated user installations.
- `UpdateManager.cpp` MUST launch updates with:
  `/VERYSILENT /SUPPRESSMSGBOXES /NORESTART /CLOSEAPPLICATIONS /CURRENTUSER`
- This ensures updates install 100% silently in the background without any Windows UAC permission prompt.

---

### 3. Release Version History & Changelog Continuity
- Whenever a new version is released:
  1. **Version Bumps**:
     - `CMakeLists.txt` (`project(MediaGrab VERSION X.Y.Z LANGUAGES CXX)`)
     - `src/main.cpp` (`app.setApplicationVersion("X.Y.Z")`)
     - `src/core/UpdateManager.h` (`m_currentVersion = "X.Y.Z"`)
     - `installer.iss` (`#define MyAppVersion "X.Y.Z"`)
     - `qml/I18n.qml` (`appVersion`, `aboutVersion`)
  2. **Changelog Keys**:
     - Add localized release notes keys in `qml/I18n.qml` (`en`, `tr`, `uk` - e.g. `logXYZ_1..4`).
  3. **Timeline UI**:
     - Add the new version block marked with `CURRENT` badge at the top of the timeline in `qml/views/UpdatesView.qml`.
     - Change the previous version's badge from `CURRENT` to standard `Theme.bgElevated`.
     - Never remove past version logs from the timeline so users can view the full release history.

---

### 4. Build, Tag & GitHub Release Workflow (Otomasyon & Kurallar)
- **Agent Görevleri (Otomatik Yapılanlar)**:
  1. `build.bat` ile Release binary yerel olarak derlenir (`build\YuYuInstaller.exe`).
     - *Not*: Yerel derleme işlemi internet harcamaz.
  2. `make_setup.bat` ile `setup_output\YuYuInstaller_Setup.exe` kurulum paketi hazırlanır.
  3. Değişiklikler Git'e commit'lenir:
     ```powershell
     git add -A; git commit -m "Release vX.Y.Z: ..."
     ```
  4. İlgili sürüm için Git tag'i oluşturulur ve GitHub'a pushlanır:
     ```powershell
     git tag -a vX.Y.Z -m "Release vX.Y.Z: ..."
     git push origin main
     git push origin vX.Y.Z
     ```
- **Kullanıcı Görevleri (Manuel Yapılanlar)**:
  - Agent `YuYuInstaller_Setup.exe` kurulum dosyasını GitHub Release'e **ASLA OTOMATİK YÜKLEMEZ** (kullanıcının internet kota ve token kontrolü için).
  - Kurulum dosyası GitHub Releases sayfasına kullanıcı tarafından **manuel** olarak yüklenir:
    1. GitHub -> Repository (`Audreiy/Mediainstaller`) -> **Releases** sekmesine gidilir.
    2. Yeni oluşturulan `vX.Y.Z` etiketi seçilir ("Draft a new release").
    3. Başlık olarak `YuYuInstaller vX.Y.Z` yazılır ve sürüm notları eklenir.
    4. `setup_output\YuYuInstaller_Setup.exe` dosyası sürüm varlıklarına (assets) sürüklenip bırakılır.
    5. **Publish release** butonuna basılarak yayınlanır.
