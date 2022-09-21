#include "stdafx.h"
#include "HeicException.h"


HeicException::HeicException(HRESULT hr, const char* szFormat, ...)
	: m_hr(hr)
	, std::exception(format((va_start(m_va, szFormat), szFormat)).c_str())
{
	va_end(m_va);
}


HeicException::~HeicException()
{
}

HRESULT HeicException::GetResult() const
{
	return m_hr;
}

std::string HeicException::format(const char* szFormat)
{
	std::string out;

	int len = _vscprintf(szFormat, m_va);
	if (len >= 0) {
		out.resize(len);

		if (vsprintf_s(&out[0], out.length() + 1, szFormat, m_va) == -1) {
			out.clear();
		}
	}

	return out;
}
