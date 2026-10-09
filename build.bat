@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
set "PATH=C:\Qt\6.8.2\msvc2022_64\bin;C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%"

echo === Configuring CMake ===
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:/Qt/6.8.2/msvc2022_64"
if %errorlevel% neq 0 exit /b %errorlevel%

echo === Building YuYuInstaller ===
cmake --build build --config Release
if %errorlevel% neq 0 exit /b %errorlevel%

copy /Y build\YuYuInstaller.exe build\MediaGrabApp.exe >nul 2>&1
copy /Y build\YuYuInstaller.exe build\MediaGrab.exe >nul 2>&1
if not exist build\assets mkdir build\assets >nul 2>&1
copy /Y assets\* build\assets\ >nul 2>&1

echo === Build Successful! ===
