#include "stdafx.h"
#include "wic_heic.h"
#include "UtlReg.h"
#include <strsafe.h>

// Should be proper registry class

UtlReg::UtlReg()
{
}


UtlReg::~UtlReg()
{
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
