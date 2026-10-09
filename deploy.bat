@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
set "PATH=C:\Qt\6.8.2\msvc2022_64\bin;%PATH%"

echo Deploying Qt binaries to build directory...
windeployqt.exe build\YuYuInstaller.exe --qmldir qml
echo Deployment complete!
