// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "wic_heic.h"
#include "ComFactory.h"
#include "ComCounter.h"
#include "UtlReg.h"

#if defined(X86)
#pragma comment (linker, "/export:DllGetClassObject=_DllGetClassObject@12,PRIVATE")
#else
#pragma comment (linker, "/export:DllCanUnloadNow,PRIVATE")
#pragma comment (linker, "/export:DllGetClassObject,PRIVATE")
#pragma comment (linker, "/export:DllRegisterServer,PRIVATE")
#pragma comment (linker, "/export:DllUnregisterServer,PRIVATE")
#endif

HMODULE GModule = NULL;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		GModule = hModule;
		DisableThreadLibraryCalls(hModule);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

HRESULT RegisterEncoder()
{
	// TODO: Rewrite this, this is a lazy way - no error checking, cleanup, transaction, etc
	// Requires Admin rights

	LSTATUS status;
	HKEY hInstance;
	HKEY hEncoder;
	HKEY hInProc;
	HKEY hFormat;

	status = RegCreateKeyExA(
		HKEY_CLASSES_ROOT,
		"CLSID\\{AC757296-3522-4E11-9862-C17BE5A1767E}\\Instance\\" S_CLSID_HEICEncoder,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hInstance,
		NULL);

	if (status != ERROR_SUCCESS) {
		Log("Unable to create key for WIC %X", status);
		return MAKE_HRESULT(0, 0, status);
	}

	const char ClsidEncoder[] = S_CLSID_HEICEncoder;
	const char FriendlyName[] = "HEIC Encoder";
	RegSetValueExA(hInstance, "CLSID", 0, REG_SZ, (BYTE*)ClsidEncoder, sizeof(ClsidEncoder));
	RegSetValueExA(hInstance, "FriendlyName", 0, REG_SZ, (BYTE*)FriendlyName, sizeof(FriendlyName));

	RegCloseKey(hInstance);

	status = RegCreateKeyExA(
		HKEY_CLASSES_ROOT,
		"CLSID\\" S_CLSID_HEICEncoder,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hEncoder,
		NULL);

	if (status != ERROR_SUCCESS)
	{
		Log("Unable to create key for HEIC Encoder %X", status);
		return MAKE_HRESULT(0, 0, status);
	}
	else
	{
		DWORD dw;

		dw = 0;
		RegSetValueExA(hEncoder, "ArbitrationPriority", 0, REG_DWORD, (BYTE*)&dw, sizeof(dw));
		dw = 0;
		RegSetValueExA(hEncoder, "SupportAnimation", 0, REG_DWORD, (BYTE*)&dw, sizeof(dw));
		dw = 0;
		RegSetValueExA(hEncoder, "SupportChromaKey", 0, REG_DWORD, (BYTE*)&dw, sizeof(dw));
		dw = 0;
		RegSetValueExA(hEncoder, "SupportLossless", 0, REG_DWORD, (BYTE*)&dw, sizeof(dw));
		dw = 1;
		RegSetValueExA(hEncoder, "SupportMultiframe", 0, REG_DWORD, (BYTE*)&dw, sizeof(dw));

		const BYTE Author[] = "prsyahmi@github";
		const BYTE ContainerFormat[] = S_GUID_ContainerFormatHEIC;
		const BYTE Description[] = "HEIC Image Encoder";
		const BYTE FriendlyName[] = "HEIC Image Encoder";
		const BYTE FileExtensions[] = ".heic";
		const BYTE MimeTypes[] = "image/heic";
		const BYTE SpecVersion[] = "1.0.0.0";
		const BYTE Vendor[] = S_GUID_VendorPrsyahmi;
		const BYTE Version[] = "1.0.0.0";
		RegSetValueExA(hEncoder, "Author", 0, REG_SZ, Author, sizeof(Author));
		RegSetValueExA(hEncoder, "ContainerFormat", 0, REG_SZ, ContainerFormat, sizeof(ContainerFormat));
		RegSetValueExA(hEncoder, "Description", 0, REG_SZ, Description, sizeof(Description));
		RegSetValueExA(hEncoder, "FriendlyName", 0, REG_SZ, FriendlyName, sizeof(FriendlyName));
		RegSetValueExA(hEncoder, "FileExtensions", 0, REG_SZ, FileExtensions, sizeof(FileExtensions));
		RegSetValueExA(hEncoder, "MimeTypes", 0, REG_SZ, MimeTypes, sizeof(MimeTypes));
		RegSetValueExA(hEncoder, "SpecVersion", 0, REG_SZ, SpecVersion, sizeof(SpecVersion));
		RegSetValueExA(hEncoder, "Vendor", 0, REG_SZ, Vendor, sizeof(Vendor));
		RegSetValueExA(hEncoder, "Version", 0, REG_SZ, Version, sizeof(Version));
	}

	RegCloseKey(hEncoder);

	status = RegCreateKeyExA(
		HKEY_CLASSES_ROOT,
		"CLSID\\" S_CLSID_HEICEncoder "\\InprocServer32",
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hInProc,
		NULL);

	if (status == ERROR_SUCCESS) {
		std::wstring s;

		DWORD copied = 0;
		do {
			s.resize(s.size() + MAX_PATH);
			copied = GetModuleFileName(GModule, &s[0], (DWORD)s.size());
		} while (copied >= s.size());
		s.resize(copied);

		Log("Registering at %ws", s.c_str());

		const char Both[] = "Both";
		RegSetValueExW(hInProc, L"", 0, REG_SZ, (BYTE*)s.c_str(), (DWORD)(s.size() * sizeof(WCHAR)));
		RegSetValueExA(hInProc, "ThreadingModel", 0, REG_SZ, (BYTE*)Both, sizeof(Both));
	}

	// Let's support GUID_WICPixelFormat24bppRGB only

	status = RegCreateKeyExA(
		HKEY_CLASSES_ROOT,
		"CLSID\\" S_CLSID_HEICEncoder "\\Formats\\{6fddc324-4e03-4bfe-b185-3d77768dc90d}",
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hFormat,
		NULL);
	RegCloseKey(hFormat);

	return S_OK;
}

STDAPI DllRegisterServer()
{
	// TODO: Rewrite this, this is a lazy way - no error checking, cleanup, transaction, etc
	// Requires Admin rights

	LSTATUS status;
	HKEY hInstance;
	HKEY hDecoder;
	HKEY hInProc;
	HKEY hFormat;
	HKEY hPatterns;

	status = RegCreateKeyExA(
		HKEY_CLASSES_ROOT,
		"CLSID\\{7ED96837-96F0-4812-B211-F13C24117ED3}\\Instance\\" S_CLSID_HEICDecoder,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hInstance,
		NULL);

	if (status != ERROR_SUCCESS) {
		Log("Unable to create key for WIC %X", status);
		return MAKE_HRESULT(0, 0, status);
	}

	const char ClsidDecoder[] = S_CLSID_HEICDecoder;
	const char FriendlyName[] = "HEIC Decoder";
	RegSetValueExA(hInstance, "CLSID", 0, REG_SZ, (BYTE*)ClsidDecoder, sizeof(ClsidDecoder));
	RegSetValueExA(hInstance, "FriendlyName", 0, REG_SZ, (BYTE*)FriendlyName, sizeof(FriendlyName));

	RegCloseKey(hInstance);

	status = RegCreateKeyExA(
		HKEY_CLASSES_ROOT,
		"CLSID\\" S_CLSID_HEICDecoder,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hDecoder,
		NULL);

	if (status != ERROR_SUCCESS)
	{
		Log("Unable to create key for HEIC Decoder %X", status);
		return MAKE_HRESULT(0, 0, status);
	}
	else
	{
		DWORD dw;

		dw = 0;
		RegSetValueExA(hDecoder, "ArbitrationPriority", 0, REG_DWORD, (BYTE*)&dw, sizeof(dw));
		dw = 0;
		RegSetValueExA(hDecoder, "SupportAnimation", 0, REG_DWORD, (BYTE*)&dw, sizeof(dw));
		dw = 0;
		RegSetValueExA(hDecoder, "SupportChromaKey", 0, REG_DWORD, (BYTE*)&dw, sizeof(dw));
		dw = 0;
		RegSetValueExA(hDecoder, "SupportLossless", 0, REG_DWORD, (BYTE*)&dw, sizeof(dw));
		dw = 1;
		RegSetValueExA(hDecoder, "SupportMultiframe", 0, REG_DWORD, (BYTE*)&dw, sizeof(dw));

		const BYTE Author[] = "prsyahmi@github";
		const BYTE ColorManagementVersion[] = "1.0.0.0";
		const BYTE ContainerFormat[] = S_GUID_ContainerFormatHEIC;
		const BYTE Description[] = "HEIC Image Decoder";
		const BYTE FriendlyName[] = "HEIC Image Decoder";
		const BYTE FileExtensions[] = ".heic";
		const BYTE MimeTypes[] = "image/heic";
		const BYTE SpecVersion[] = "1.0.0.0";
		const BYTE Vendor[] = S_GUID_VendorPrsyahmi;
		const BYTE Version[] = "1.0.0.0";
		RegSetValueExA(hDecoder, "Author", 0, REG_SZ, Author, sizeof(Author));
		RegSetValueExA(hDecoder, "ColorManagementVersion", 0, REG_SZ, ColorManagementVersion, sizeof(ColorManagementVersion));
		RegSetValueExA(hDecoder, "ContainerFormat", 0, REG_SZ, ContainerFormat, sizeof(ContainerFormat));
		RegSetValueExA(hDecoder, "Description", 0, REG_SZ, Description, sizeof(Description));
		RegSetValueExA(hDecoder, "FriendlyName", 0, REG_SZ, FriendlyName, sizeof(FriendlyName));
		RegSetValueExA(hDecoder, "FileExtensions", 0, REG_SZ, FileExtensions, sizeof(FileExtensions));
		RegSetValueExA(hDecoder, "MimeTypes", 0, REG_SZ, MimeTypes, sizeof(MimeTypes));
		RegSetValueExA(hDecoder, "SpecVersion", 0, REG_SZ, SpecVersion, sizeof(SpecVersion));
		RegSetValueExA(hDecoder, "Vendor", 0, REG_SZ, Vendor, sizeof(Vendor));
		RegSetValueExA(hDecoder, "Version", 0, REG_SZ, Version, sizeof(Version));
	}

	RegCloseKey(hDecoder);

	status = RegCreateKeyExA(
		HKEY_CLASSES_ROOT,
		"CLSID\\" S_CLSID_HEICDecoder "\\InprocServer32",
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hInProc,
		NULL);

	if (status == ERROR_SUCCESS) {
		std::wstring s;

		DWORD copied = 0;
		do {
			s.resize(s.size() + MAX_PATH);
			copied = GetModuleFileName(GModule, &s[0], (DWORD)s.size());
		} while (copied >= s.size());
		s.resize(copied);

		Log("Registering at %ws", s.c_str());

		const char Both[] = "Both";
		RegSetValueExW(hInProc, L"", 0, REG_SZ, (BYTE*)s.c_str(), (DWORD)(s.size() * sizeof(WCHAR)));
		RegSetValueExA(hInProc, "ThreadingModel", 0, REG_SZ, (BYTE*)Both, sizeof(Both));
	}

	// Let's support GUID_WICPixelFormat24bppRGB only

	status = RegCreateKeyExA(
		HKEY_CLASSES_ROOT,
		"CLSID\\" S_CLSID_HEICDecoder "\\Formats\\{6fddc324-4e03-4bfe-b185-3d77768dc90d}",
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hFormat,
		NULL);
	RegCloseKey(hFormat);

	status = RegCreateKeyExA(
		HKEY_CLASSES_ROOT,
		"CLSID\\" S_CLSID_HEICDecoder "\\Patterns\\0",
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hPatterns,
		NULL);

	if (status == ERROR_SUCCESS) {
		DWORD dw;
		const BYTE Mask[] = "\xff\xff\xff\xff\xff\xff\xff\xff";
		const BYTE Pattern[] = "\x66\x74\x79\x70\x68\x65\x69\x63";

		dw = 8;
		RegSetValueExA(hPatterns, "Length", 0, REG_DWORD, (BYTE*)&dw, sizeof(dw));
		dw = 4;
		RegSetValueExA(hPatterns, "Position", 0, REG_DWORD, (BYTE*)&dw, sizeof(dw));

		RegSetValueExA(hPatterns, "Mask", 0, REG_BINARY, Mask, sizeof(Mask) - 1);
		RegSetValueExA(hPatterns, "Pattern", 0, REG_BINARY, Pattern, sizeof(Pattern) - 1);

		RegCloseKey(hPatterns);
	}

	RegisterEncoder();

	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
	return S_OK;
}

STDAPI DllUnregisterServer()
{
	RegDelnodeRecurse(HKEY_CLASSES_ROOT, "CLSID\\{7ED96837-96F0-4812-B211-F13C24117ED3}\\Instance\\" S_CLSID_HEICDecoder);
	RegDelnodeRecurse(HKEY_CLASSES_ROOT, "CLSID\\" S_CLSID_HEICDecoder);

	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
	return S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppvObj)
{
	HRESULT hr = E_OUTOFMEMORY;

	if (!ppvObj) {
		return E_INVALIDARG;
	}

	if (!IsEqualGUID(riid, IID_IClassFactory)) {
		return E_INVALIDARG;
	}

	*ppvObj = NULL;

	if (IsEqualCLSID(rclsid, CLSID_HEICDecoder) || IsEqualCLSID(rclsid, CLSID_HEICEncoder)) {
		CComFactory *pClassFactory = new CComFactory();
		if (pClassFactory)
		{
			hr = pClassFactory->QueryInterface(riid, ppvObj);
			pClassFactory->Release();
		}
		else
		{
			return E_OUTOFMEMORY;
		}
	}
	else
	{
		return CLASS_E_CLASSNOTAVAILABLE;
	}

	return S_OK;
}

STDAPI DllCanUnloadNow()
{
	CComCounter& counter = CComCounter::GetInstance();
	if (counter.m_ObjLock == 0 && counter.m_ServerLock == 0) {
		return S_OK;
	}

	return S_FALSE;
}
