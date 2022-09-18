@echo off
setlocal

rem Usage:
rem   build-libheif x86
rem   build-libheif x64
rem   build-libheif x86 xp
rem   build-libheif x64 xp

set B_TMP=%1
set B_OUT=%~dp0_out\%1
if "%1"=="x86" set B_VC=Visual Studio 14
if "%1"=="x64" set B_VC=Visual Studio 14 Win64
if "%2"=="xp" set B_OUT=%~dp0_out_xp\%1
if "%2"=="xp" set B_EXTRA=-T "v140_xp" -DCMAKE_CXX_FLAGS="/DWIN32 /D_WINDOWS /W3 /GR /EHsc /Zc:threadSafeInit-"
if "%2"=="xp" set B_TMP=%1-xp

call "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat" %1
md _tmp\libheif 2> nul
md _tmp\libheif\%B_TMP% 2> nul
cd _tmp\libheif\%B_TMP%
cmake %B_EXTRA% -G "%B_VC%" -DCMAKE_INSTALL_PREFIX="%B_OUT%" -DBUILD_SHARED_LIBS=0 ..\..\..\libheif

cmake --build . --target install --config RelWithDebInfo
cmake --build . --config Debug
rem copy Debug\leveldb.lib "%B_OUT%\lib\leveldb_dbg.lib"
exit /b
