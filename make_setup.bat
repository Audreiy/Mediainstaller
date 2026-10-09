@echo off
echo ==============================================
echo   YuYuInstaller - Installer Packaging Script
echo ==============================================

set "ISCC=C:\Users\serbu\AppData\Local\Programs\Inno Setup 6\ISCC.exe"
if not exist "%ISCC%" (
    set "ISCC=C:\Program Files (x86)\Inno Setup 6\ISCC.exe"
)

if not exist "%ISCC%" (
    echo [ERROR] Inno Setup compiler (ISCC.exe) not found!
    pause
    exit /b 1
)

echo Compiling setup package...
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
pause
