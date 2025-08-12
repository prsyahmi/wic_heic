// wic_heic.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "wic_heic.h"

#pragma comment (lib, "heif.lib")
#pragma comment (lib, "libde265.lib")
#pragma comment (lib, "libkvazaar.lib")

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
