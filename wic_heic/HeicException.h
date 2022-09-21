#pragma once

class HeicException : public std::exception
{
private:
	va_list m_va;
	HRESULT m_hr;

public:
	HeicException(HRESULT hr, const char* szFormat, ...);
	~HeicException();

	HRESULT GetResult() const;

private:
	std::string format(const char* szFormat);
};

