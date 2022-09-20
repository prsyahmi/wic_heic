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

set B_STATIC=-DCMAKE_CXX_FLAGS_RELWITHDEBINFO="/MT /Zi /O2 /Ob2 /DNDEBUG"

call "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat" %1
md _tmp\libheif 2> nul
md _tmp\libheif\%B_TMP% 2> nul
cd _tmp\libheif\%B_TMP%

cmake %B_EXTRA% %B_STATIC% -G "%B_VC%" -DCMAKE_INSTALL_PREFIX="%B_OUT%" -DBUILD_SHARED_LIBS=0 -DLIBDE265_INCLUDE_DIR="%B_OUT%\include" -DLIBDE265_LIBRARY="%B_OUT%\lib\libde265.lib" -DX265_INCLUDE_DIR="%B_OUT%\include" -DX265_LIBRARY="%B_OUT%\lib\x265-static.lib" -DCMAKE_CXXFLAGS='-DLIBDE265_STATIC_BUILD=1' ..\..\..\libheif
set CL=/DLIBDE265_STATIC_BUILD=1
MSBuild /property:Configuration="RelWithDebInfo" libheif.sln
cmake --install . --config RelWithDebInfo

rem cmake --build . --target install --config RelWithDebInfo
rem cmake --build . --config Debug

exit /b
