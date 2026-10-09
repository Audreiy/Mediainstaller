@echo off
setlocal
echo ==============================================
echo   YuYuInstaller - Installer Packaging Script
echo ==============================================

set "ISCC="
if exist "%LOCALAPPDATA%\Programs\Inno Setup 6\ISCC.exe" set "ISCC=%LOCALAPPDATA%\Programs\Inno Setup 6\ISCC.exe"
if exist "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" set "ISCC=C:\Program Files (x86)\Inno Setup 6\ISCC.exe"
if exist "C:\Program Files\Inno Setup 6\ISCC.exe" set "ISCC=C:\Program Files\Inno Setup 6\ISCC.exe"

if not defined ISCC (
    echo [ERROR] Inno Setup compiler ISCC.exe not found!
    exit /b 1
)

echo Compiling setup package using: "%ISCC%"
"%ISCC%" installer.iss

if %errorlevel% equ 0 (
    echo.
    echo ==============================================
    echo   Setup build successful!
    echo   File: setup_output\YuYuInstaller_Setup.exe
    echo ==============================================
) else (
    echo [ERROR] Setup build failed with code %errorlevel%
)
