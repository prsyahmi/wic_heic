# Windows Imaging Component for HEIC file
This software add support to open heic files in windows or other software like IrfanView.  
This software uses libheif (https://github.com/strukturag/libheif)

### How to build

#### Requirements:
- CMake
- Visual Studio C++ Compiler v14
- nasm

#### Steps:
- This repository contains submodule for 3rd party libs, clone with `git clone --recurse-submodules <url>`
- Build 3rd party component (`wic_heic\3rdparty\build.bat`)
- Build solution
- The resulting files will be at `(Build)\[ARCH]\[TARGET]\wic_heic.dll` or `(Build)\x86\Release\wic_heic.dll`

### Installation:
- Download msi installation from: https://github.com/prsyahmi/wic_heic/releases


### Optional hardened x64 Photos opener

For a local PNG handoff into Microsoft Photos, see
[tools/photos-opener](tools/photos-opener/README.md). The optional build uses
updated pinned decoder libraries, resource limits, bounds checks, and a
separate per-user codec identity.
