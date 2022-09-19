#pragma once

class UtlReg
{
public:
	UtlReg();
	~UtlReg();
};

BOOL RegDelnodeRecurse(HKEY hKeyRoot, const std::string& subkey);
