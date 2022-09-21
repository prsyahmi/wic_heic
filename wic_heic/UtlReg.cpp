#include "stdafx.h"
#include "wic_heic.h"
#include "UtlReg.h"
#include <strsafe.h>
#include "HeicException.h"

// Should be proper registry class

UtlReg::UtlReg()
	: m_hKey(NULL)
{
}


UtlReg::~UtlReg()
{
}

void UtlReg::CloseKey()
{
	if (m_hKey) {
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}
}

void UtlReg::CreateKeyTree(HKEY hKeyRoot, const std::string& subKey)
{
	LSTATUS status;

	CloseKey();

	status = RegCreateKeyExA(
		hKeyRoot,
		subKey.c_str(),
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&m_hKey,
		NULL);

	if (status != ERROR_SUCCESS) {
		throw HeicException(MAKE_HRESULT(0, 0, status), "[Reg] Error %X. Unable to create key at %s", status, subKey.c_str());
	}
}

void UtlReg::CreateValue(const std::string& ValueName, const std::string& data)
{
	if (!m_hKey) {
		throw HeicException(S_FALSE, "[Reg] Trying to create value, but key is not opened");
	}

	LSTATUS status = RegSetValueExA(m_hKey, ValueName.c_str(), 0, REG_SZ, (BYTE*)data.c_str(), (DWORD)data.size() + 1);
	if (status != ERROR_SUCCESS) {
		throw HeicException(MAKE_HRESULT(0, 0, status), "[Reg] Error %X.Unable to create value for SZ %s", status, ValueName.c_str());
	}
}

void UtlReg::CreateValue(const std::wstring& ValueName, const std::wstring& data)
{
	if (!m_hKey) {
		throw HeicException(S_FALSE, "[Reg] Trying to create value, but key is not opened");
	}

	LSTATUS status = RegSetValueExW(m_hKey, ValueName.c_str(), 0, REG_SZ, (BYTE*)data.c_str(), (DWORD)(data.size() + 1) * sizeof(WCHAR));
	if (status != ERROR_SUCCESS) {
		throw HeicException(MAKE_HRESULT(0, 0, status), "[Reg] Error %X.Unable to create value for SZ %s", status, ValueName.c_str());
	}
}

void UtlReg::CreateValue(const std::string& ValueName, DWORD data)
{
	if (!m_hKey) {
		throw HeicException(S_FALSE, "[Reg] Trying to create value, but key is not opened");
	}

	LSTATUS status = RegSetValueExA(m_hKey, ValueName.c_str(), 0, REG_DWORD, (BYTE*)&data, sizeof(DWORD));
	if (status != ERROR_SUCCESS) {
		throw HeicException(MAKE_HRESULT(0, 0, status), "[Reg] Error %X.Unable to create value for DWORD %s", status, ValueName.c_str());
	}
}

void UtlReg::CreateValue(const std::string& ValueName, const void* pBuf, size_t pSize)
{
	if (!m_hKey) {
		throw HeicException(S_FALSE, "[Reg] Trying to create value, but key is not opened");
	}

	LSTATUS status = RegSetValueExA(m_hKey, ValueName.c_str(), 0, REG_BINARY, (BYTE*)pBuf, (DWORD)pSize);
	if (status != ERROR_SUCCESS) {
		throw HeicException(MAKE_HRESULT(0, 0, status), "[Reg] Error %X.Unable to create value for BINARY %s", status, ValueName.c_str());
	}
}

// https://learn.microsoft.com/en-us/windows/win32/sysinfo/deleting-a-key-with-subkeys
BOOL RegDelnodeRecurse(HKEY hKeyRoot, const std::string& subkey)
{
	std::string s = subkey;
	std::string sName;

	LONG lResult;
	DWORD dwSize;
	HKEY hKey;
	DWORD index;

	if (subkey.length() == 0) {
		return FALSE;
	}

	sName.resize(MAX_PATH);

	// First, see if we can delete the key without having
	// to recurse.
	lResult = RegDeleteKeyA(hKeyRoot, subkey.c_str());
	if (lResult == ERROR_SUCCESS) {
		return TRUE;
	}

	lResult = RegOpenKeyExA(hKeyRoot, s.c_str(), 0, KEY_READ | KEY_SET_VALUE, &hKey);
	if (lResult != ERROR_SUCCESS)
	{
		if (lResult == ERROR_FILE_NOT_FOUND) {
			Log("Key not found.");
			return TRUE;
		} else {
			Log("Error opening key.");
			return FALSE;
		}
	}

	// Check for an ending slash and add one if it is missing.
	if (s.back() != '\\') s += '\\';

	index = 0;
	dwSize = MAX_PATH;
	lResult = RegEnumValueA(hKey, 0, &sName[0], &dwSize, NULL, NULL, NULL, NULL);
	if (lResult == ERROR_SUCCESS)
	{
		do {
			sName.resize(dwSize);
			std::string sub = s + sName;

			RegDeleteValueA(hKey, sName.c_str());

			dwSize = MAX_PATH;
			lResult = RegEnumValueA(hKey, 0, &sName[0], &dwSize, NULL, NULL, NULL, NULL);
		} while (lResult == ERROR_SUCCESS);
	}

	index = 0;
	dwSize = MAX_PATH;
	lResult = RegEnumKeyExA(hKey, 0, &sName[0], &dwSize, NULL, NULL, NULL, NULL);
	if (lResult == ERROR_SUCCESS)
	{
		do {
			sName.resize(dwSize);
			std::string sub = s + sName;

			if (!RegDelnodeRecurse(hKeyRoot, sub)) {
				break;
			}

			dwSize = MAX_PATH;
			lResult = RegEnumKeyExA(hKey, 0, &sName[0], &dwSize, NULL, NULL, NULL, NULL);
		} while (lResult == ERROR_SUCCESS);
	}

	RegCloseKey(hKey);

	// Try again to delete the key.
	lResult = RegDeleteKeyA(hKeyRoot, s.c_str());

	if (lResult == ERROR_SUCCESS) {
		return TRUE;
	}

	return FALSE;
}
