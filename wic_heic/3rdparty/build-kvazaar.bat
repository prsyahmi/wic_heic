@echo off
setlocal

rem Usage:
rem   build-kvazaar x86
rem   build-kvazaar x64

set B_TMP=%1
set B_OUT=%~dp0_out\%1
set B_VC=Visual Studio 17 2022
if "%1"=="x86" set B_BIT=Win32
if "%1"=="x64" set B_BIT=x64

md _tmp\kvazaar 2> nul
md _tmp\kvazaar\%B_TMP% 2> nul
cd _tmp\kvazaar\%B_TMP%

rem set B_STATIC=-DCMAKE_CXX_FLAGS_RELEASE="/MT /O2 /Ob2 /DNDEBUG"
set B_STATIC=-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded

cmake -G "%B_VC%" -A %B_BIT% -DCMAKE_INSTALL_PREFIX="%B_OUT%" %B_STATIC% -DBUILD_SHARED_LIBS=0 ..\..\..\kvazaar
cmake --build . --target install --config Release

exit /b