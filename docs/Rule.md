# YuYuInstaller Development & Release Standards

## Critical Rules for Auto-Updates & Installers

### 1. Desktop Icon Position Preservation (Masaüstü İkonunu Asla Değiştirmeme)
- **Problem**: When Inno Setup updates an existing installation, deleting and recreating the desktop `.lnk` shortcut causes Windows Explorer to treat it as a new file and moves it to the first free grid slot (top-left).
- **Mandatory Rule**: `installer.iss` MUST ALWAYS include `Check: ShouldCreateDesktopIcon` on `{autodesktop}`. If `YuYuInstaller.lnk` already exists in `{autodesktop}`, `{userdesktop}`, or `{commondesktop}` (Public Desktop), Inno Setup MUST NOT recreate or touch the shortcut.
- The existing shortcut target automatically points to the updated `.exe` because the executable is updated in-place. The user's custom desktop position remains 100% untouched.

### 2. Zero-UAC Silent Updates (Discord / Spotify Model)
- `installer.iss` MUST set `PrivilegesRequired=lowest` and `PrivilegesRequiredOverridesAllowed=dialog commandline`.
- Default install directory is `{autopf}\{#MyAppName}`, which resolves to `%LOCALAPPDATA%\Programs\YuYuInstaller` for non-elevated user installations.
- `UpdateManager.cpp` MUST launch updates with:
  `/VERYSILENT /SUPPRESSMSGBOXES /NORESTART /CLOSEAPPLICATIONS /CURRENTUSER`
- This ensures updates install 100% silently in the background without any Windows UAC permission prompt.

### 3. Release Version History & Changelog Continuity
- Whenever a new version is released:
  1. Update version in `CMakeLists.txt`, `src/main.cpp`, `src/core/UpdateManager.h`, `installer.iss`.
  2. Add localized release notes keys in `qml/I18n.qml` (`en`, `tr`, `uk`).
  3. Add the new version block marked with `CURRENT` badge at the top of the timeline in `qml/views/UpdatesView.qml`.
  4. Change the previous version's badge from `CURRENT` to standard `Theme.bgElevated`.
  5. Never remove past version logs from the timeline so users can view the full release history.
