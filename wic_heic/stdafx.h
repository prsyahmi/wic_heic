// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <unknwn.h>
#include <wincodec.h>
#include <Shlobj.h>

#include <cstdint>
#include <atomic>

#pragma warning(push)
#pragma warning(disable: 4800)
#define LIBHEIF_STATIC_BUILD
#define LIBDE264_STATIC_BUILD
#include <libheif/heif_cxx.h>
#pragma warning(pop)

#pragma comment (lib, "windowscodecs.lib")

void Log(const char* szFormat, ...);

#ifdef _DEBUG
#define DbgLog Log
#else
#define DbgLog
#endif
