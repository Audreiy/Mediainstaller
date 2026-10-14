; YuYuInstaller Professional Inno Setup Script
#define MyAppName "YuYuInstaller"
#define MyAppVersion "1.1.7"
#define MyAppPublisher "YuYuDev"
#define MyAppExeName "YuYuInstaller.exe"
#define MyAppAssocName MyAppName + " Media Package"
#define MyAppAssocExt ".yuyu"
#define MyAppAssocKey StringChange(MyAppAssocName, " ", "") + MyAppAssocExt

[Setup]
AppId={{8B8CEAA8-3498-4B2E-BE39-FE6CF42F5E9B}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog commandline
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

; Ensure running app is closed before install/uninstall so files are not locked
CloseApplications=yes
CloseApplicationsFilter=YuYuInstaller.exe,*.dll

; Output
OutputDir=setup_output
OutputBaseFilename=YuYuInstaller_Setup

; Visual Aesthetics
SetupIconFile=assets\app.ico
UninstallDisplayIcon={app}\{#MyAppExeName}
WizardStyle=modern
WizardImageFile=assets\wizard_large.bmp
WizardSmallImageFile=assets\wizard_small.bmp

; Compression
Compression=lzma2/ultra64
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "turkish"; MessagesFile: "compiler:Languages\Turkish.isl"
Name: "ukrainian"; MessagesFile: "compiler:Languages\Ukrainian.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
; Main Executable & License
Source: "build\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "LICENSE"; DestDir: "{app}"; Flags: ignoreversion

; Tools (ffmpeg & yt-dlp)
Source: "build\ffmpeg.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\yt-dlp.exe"; DestDir: "{app}"; Flags: ignoreversion

; Qt 6 & Windows Runtime DLLs
Source: "build\*.dll"; DestDir: "{app}"; Flags: ignoreversion

; Application Assets (Video background, icons, logos)
Source: "build\assets\*"; DestDir: "{app}\assets"; Flags: ignoreversion recursesubdirs createallsubdirs

; Qt Plugins & QML Modules
Source: "build\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "build\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "build\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "build\tls\*"; DestDir: "{app}\tls"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "build\networkinformation\*"; DestDir: "{app}\networkinformation"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "build\generic\*"; DestDir: "{app}\generic"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "build\qml\*"; DestDir: "{app}\qml"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\assets\app.ico"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\assets\app.ico"; Check: ShouldCreateDesktopIcon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
Filename: "{app}\{#MyAppExeName}"; Flags: nowait; Check: WizardSilent

[UninstallDelete]
Type: filesandordirs; Name: "{app}"

[Code]
// If a desktop shortcut already exists, DO NOT recreate it so Windows preserves its exact screen location!
function ShouldCreateDesktopIcon(): Boolean;
begin
  if FileExists(ExpandConstant('{autodesktop}\{#MyAppName}.lnk')) or
     FileExists(ExpandConstant('{userdesktop}\{#MyAppName}.lnk')) or
     FileExists(ExpandConstant('{commondesktop}\{#MyAppName}.lnk')) then
  begin
    Result := False;
  end
  else
  begin
    Result := True;
  end;
end;

// Automatically terminate running app before uninstall so Windows file-locks are released
function InitializeUninstall(): Boolean;
var
  ResultCode: Integer;
begin
  Result := True;
  Exec('taskkill.exe', '/f /im YuYuInstaller.exe', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
  Sleep(500);
end;

// Clean up entire folder and any runtime generated cache
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usPostUninstall then
  begin
    DelTree(ExpandConstant('{app}'), True, True, True);
  end;
end;
