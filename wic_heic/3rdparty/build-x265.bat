@echo off
setlocal

rem Usage:
rem   build-x265 x86
rem   build-x265 x64
rem   build-x265 x86 xp
rem   build-x265 x64 xp

set B_TMP=%1
set B_OUT=%~dp0_out\%1
if "%1"=="x86" set B_VC=Visual Studio 14
if "%1"=="x64" set B_VC=Visual Studio 14 Win64
if "%2"=="xp" set B_OUT=%~dp0_out_xp\%1
if "%2"=="xp" set B_EXTRA=-T "v140_xp" -DCMAKE_CXX_FLAGS="/DWIN32 /D_WINDOWS /W3 /GR /EHsc /Zc:threadSafeInit-"
if "%2"=="xp" set B_TMP=%1-xp

call "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat" %1
md _tmp\x265 2> nul
md _tmp\x265\%B_TMP% 2> nul
md _tmp\x265\%B_TMP%\8bit 2> nul
md _tmp\x265\%B_TMP%\10bit 2> nul
md _tmp\x265\%B_TMP%\12bit 2> nul
cd _tmp\x265\%B_TMP%

set B_STATIC1=-DCMAKE_CXX_FLAGS_RELEASE="/MT /O2 /Ob2 /DNDEBUG"
set B_STATIC2=-DCMAKE_CXX_FLAGS_RELWITHDEBINFO="/MT /Zi /O2 /Ob2 /DNDEBUG"

if "%1"=="x86" goto X86

cd 12bit
cmake %B_EXTRA% %B_STATIC1% -G "%B_VC%" -DCMAKE_INSTALL_PREFIX="%B_OUT%" -DHIGH_BIT_DEPTH=ON -DEXPORT_C_API=OFF -DENABLE_SHARED=OFF -DENABLE_CLI=OFF -DMAIN12=ON ..\..\..\..\x265\source
cmake --build . --config Release
copy /y Release\x265-static.lib ..\8bit\x265-static-main12.lib
cd ..

cd 10bit
cmake %B_EXTRA% %B_STATIC1% -G "%B_VC%" -DCMAKE_INSTALL_PREFIX="%B_OUT%" -DHIGH_BIT_DEPTH=ON -DEXPORT_C_API=OFF -DENABLE_SHARED=OFF -DENABLE_CLI=OFF ..\..\..\..\x265\source
cmake --build . --config Release
copy /y Release\x265-static.lib ..\8bit\x265-static-main10.lib
cd ..

cd 8bit
rm /y Release\x265-static.lib
cmake %B_EXTRA% %B_STATIC1% -G "%B_VC%" -DCMAKE_INSTALL_PREFIX="%B_OUT%" -DEXTRA_LIB="x265-static-main10.lib;x265-static-main12.lib" -DLINKED_10BIT=ON -DLINKED_12BIT=ON ..\..\..\..\x265\source
cmake --build . --config Release
rem MSBuild /property:Configuration="Release" x265.sln
move Release\x265-static.lib x265-static-main.lib
LIB.EXE /ignore:4006 /ignore:4221 /OUT:Release\x265-static.lib x265-static-main.lib x265-static-main10.lib x265-static-main12.lib
cmake --install .

rem cmake --build . --target install --config RelWithDebInfo
rem cmake --build . --config Debug
exit /b

:X86
:: x86 doesnt support 10/12bit
cd 8bit
cmake %B_EXTRA% %B_STATIC2% -G "%B_VC%" -DCMAKE_INSTALL_PREFIX="%B_OUT%" ..\..\..\..\x265\source
cmake --build . --target install --config RelWithDebInfo

exit /b