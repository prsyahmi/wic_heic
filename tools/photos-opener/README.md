# Optional HEIC opener for Microsoft Photos

This x64 build decodes HEIC locally, writes a PNG cache copy, and opens that
copy through Microsoft Photos' file-activation API. It leaves the HEIC source
unchanged. It works around Photos versions that cannot load third-party WIC
components inside AppContainer; installing a WIC decoder alone does not remove
that Windows restriction.

The ordinary WicHeic MSI build keeps its existing codec identity, encoder and
metadata integration. This optional build uses a separate decoder identity,
disables WIC encoding and EXIF integration, and outputs 24-bit RGB SDR. The PNG
preserves those decoded pixels; source metadata, HDR and Live Photo behavior
are not preserved in the cache copy.

## Build and test

Requirements: Windows x64, Python 3.10 or newer, Git, Visual Studio 2022 C++
desktop tools, and the Windows 10 or 11 SDK. No Python packages are required.

```powershell
git submodule update --init --recursive
python tools/photos-opener/build.py --all
```

The build locates Visual Studio with `vswhere`, verifies the pinned libheif
and libde265 commits, compiles the portable decoder backend, runs boundary
and malformed-input tests, and tests a PNG pixel round-trip. Output is under
`build/photos-opener/`; the ZIP contains the executable files, licenses and
objects/libraries for relinking. There is no HEIC encoding backend, external
plugin loading or uncompressed codec in this build.

The build driver uses upstream CMake source lists directly with MSVC response
files. It supports this one x64 configuration; the existing solution and MSI
build remain available for other configurations. Default builds recompile all
sources. `--resume` skips existing object files and is only for resuming an
interrupted build with unchanged source and compiler options.

## Install and use

Extract the ZIP and run `WicHeicLocalSetup.exe` with its DLL and opener EXE
alongside it. Installation is for the current user and requires no elevation.
The installer verifies SHA-256 hashes embedded during the build before
registering the codec and adding **HEIC in Microsoft Photos** to Open with.

Right-click a HEIC file, select **Open with → HEIC in Microsoft Photos**, and
choose **Always**. If it is not listed, browse to
`%LOCALAPPDATA%\WicHeicLocal\1.1.17.0\WicHeicOpen.exe`.
Windows keeps control of the default-app choice; the installer does not
modify the protected `UserChoice` hash.

Cache PNGs are stored in `%TEMP%\WicHeicLocalCache-1.1.17` and can be deleted
after closing Photos. Images are processed locally with no upload or network
service. The opener validates the decoder hash before loading it.

Run the installed setup executable with `--uninstall` to unregister this build.
Close image applications before deleting its installation folder.

## Validation and limits

Regression coverage includes invalid rectangles, buffer/stride arithmetic,
short reads, oversized read requests, COM decoder lifetime and 200 malformed
HEIC inputs. The original local build passed 1,239 checks; a real HEIC image
and opening its cache copy in Photos were also verified interactively.
CI exercises the automated tests without opening a desktop window.

ASLR, DEP, Control Flow Guard and MSVC stack protection are enabled. These are
unsigned development builds and the tests are not an independent security
audit. The original HEIC file remains the authoritative copy for editing or
metadata/HDR workflows.

Windows restriction reference:
https://learn.microsoft.com/en-us/windows/win32/wic/what-s-new-in-wic-for-windows-8-1#restrictions-to-component-extensibility-in-appcontainer
