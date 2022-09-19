# Windows Imaging Component for HEIC file

### How to build

#### Requirements:
- CMake
- Visual Studio C++ Compiler v14

#### Steps:
- Build 3rd party component (wic_heic\3rdparty\build.bat)
- Build solution

### Installation:
- For now there is no installer
- Copy wic_heic\3rdparty\_out\[ARCH]\bin\*.dll to (Build)\[ARCH]\[TARGET]\
- Adjust wic_heic.dll path inside `wic_heic.reg`
- Execute `wic_heic.reg`
- Open `*.HEIC` file
