# Windows Imaging Component for HEIC file
This software add support to open heic files in windows or other software like IrfanView.  
This software uses libheif (https://github.com/strukturag/libheif)

### How to build

#### Requirements:
- CMake
- Visual Studio C++ Compiler v14

#### Steps:
- Build 3rd party component (`wic_heic\3rdparty\build.bat`)
- Build solution
- The resulting files will be at `(Build)\[ARCH]\[TARGET]\wic_heic.dll` or `(Build)\x86\Release\wic_heic.dll`

### Installation:
- For now there is no installer
- Download dll file from here: https://github.com/prsyahmi/wic_heic/releases
- Copy *.dll files into folder of your choice
- Open command as admin
- Execute `_install.bat` or by directly `regsvr32.exe wic_heic.dll`
- To uninstall execute `_uninstall.bat` or `regsvr32.exe /u wic_heic.dll`
