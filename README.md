# Windows Imaging Component for HEIC file

This software uses libheif (https://github.com/strukturag/libheif)

### How to build

#### Requirements:
- CMake
- Visual Studio C++ Compiler v14

#### Steps:
- Build 3rd party component (wic_heic\3rdparty\build.bat)
- Build solution
- Copy wic_heic\3rdparty\_out\[ARCH]\bin\*.dll to (Build)\[ARCH]\[TARGET]\

### Installation:
- For now there is no installer
- Copy *.dll files into folder of your choice
- Open command as admin
- Execute `_install.bat` or by directly `regsvr32.exe wic_heic.dll`
- To uninstall execute `_uninstall.bat` or `regsvr32.exe /u wic_heic.dll`
