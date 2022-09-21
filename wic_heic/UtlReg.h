#pragma once

class UtlReg
{
private:
	HKEY m_hKey;

public:
	UtlReg();
	~UtlReg();

	void CloseKey();
	void CreateKeyTree(HKEY hKeyRoot, const std::string& subKey);
	void CreateValue(const std::string& ValueName, const std::string& data);
	void CreateValue(const std::wstring& ValueName, const std::wstring& data);
	void CreateValue(const std::string& ValueName, DWORD data);
	void CreateValue(const std::string& ValueName, const void* pBuf, size_t pSize);
};

BOOL RegDelnodeRecurse(HKEY hKeyRoot, const std::string& subkey);
