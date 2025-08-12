@echo off
setlocal

rem Usage:
rem   build-libheif x86
rem   build-libheif x64
rem   build-libheif x86 xp
rem   build-libheif x64 xp

set B_TMP=%1
set B_OUT=%~dp0_out\%1
set B_VC=Visual Studio 17 2022
if "%1"=="x86" set B_BIT=Win32
if "%1"=="x64" set B_BIT=x64

rem RELEASE = /O2 /Ob2 /DNDEBUG
rem RELWITHDEBINFO = /Zi /O2 /Ob1 /DNDEBUG
set B_STATIC=-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded -DCMAKE_CXX_FLAGS_RELEASE="/DLIBDE265_STATIC_BUILD=1 /DKVZ_STATIC_LIB=1 /MT /O2 /Ob2 /DNDEBUG"

md _tmp\libheif 2> nul
md _tmp\libheif\%B_TMP% 2> nul
cd _tmp\libheif\%B_TMP%

cmake --preset=release-noplugins -G "%B_VC%" -A %B_BIT% %B_STATIC% -DCMAKE_INSTALL_PREFIX="%B_OUT%" -DBUILD_DOCUMENTATION=0 -DBUILD_SHARED_LIBS=0 -DWITH_KVAZAAR=1 -DWITH_EXAMPLES=0 -DBUILD_TESTING=0 ..\..\..\libheif
rem set CL=/DLIBDE265_STATIC_BUILD=1 /DKVZ_STATIC_LIB=1 %CL%
cmake --build . --target install --config Release
rem cmake --install . --config RelWithDebInfo

rem cmake --build . --target install --config RelWithDebInfo
rem cmake --build . --config Debug

exit /b
