// wic_heic.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "wic_heic.h"
#ifdef WIC_LOCAL_DECODE_ONLY
#include <libde265/de265.h>
#endif

#pragma comment (lib, "heif.lib")
#pragma comment (lib, "libde265.lib")

#ifndef WIC_LOCAL_DECODE_ONLY
#pragma comment (lib, "libkvazaar.lib")
#endif

#ifdef WIC_LOCAL_DECODE_ONLY
extern "C" __declspec(dllexport) const char* __stdcall WicHeicLocalGetBuildInfo()
{
    static const std::string info = std::string("WicHeic Local 1.1.17.0; libheif ") +
        heif_get_version() + "; libde265 " + de265_get_version();
    return info.c_str();
}

#endif

void Log(const char* szFormat, ...)
{
	va_list args;
	va_start(args, szFormat);

	std::string out;

	int len = _vscprintf(szFormat, args);
	if (len >= 0) {
		out.resize(len);

		if (vsprintf_s(&out[0], out.length() + 1, szFormat, args) == -1) {
			out.clear();
		}
	}


	va_end(args);

	out += "\n";
	OutputDebugStringA(out.c_str());
}
