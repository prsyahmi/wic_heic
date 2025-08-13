@echo off
setlocal

rem Usage:
rem   build-libde265 x86
rem   build-libde265 x64
rem   build-libde265 x86 xp
rem   build-libde265 x64 xp

set B_TMP=%1
set B_OUT=%~dp0_out\%1
set B_VC=Visual Studio 17 2022
if "%1"=="x86" set B_BIT=Win32
if "%1"=="x64" set B_BIT=x64

rem set B_STATIC=-DCMAKE_CXX_FLAGS_RELEASE="/MT /Zi /O2 /Ob2 /DNDEBUG"
set B_STATIC=-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded -DCMAKE_CXX_FLAGS_RELEASE="/MT /O2 /Ob2 /DNDEBUG"

copy libde265\extra\libde265\de265-version.h libde265\libde265

md _tmp\libde265 2> nul
md _tmp\libde265\%B_TMP% 2> nul
cd _tmp\libde265\%B_TMP%

cmake %B_EXTRA% %B_STATIC% -G "%B_VC%" -A %B_BIT% -DCMAKE_INSTALL_PREFIX="%B_OUT%" -DBUILD_SHARED_LIBS=0 ..\..\..\libde265
cmake --build . --target install --config Release
rem cmake --build . --config Debug

exit /b
