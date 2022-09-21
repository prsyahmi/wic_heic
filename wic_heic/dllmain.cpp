// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "wic_heic.h"
#include "ComFactory.h"
#include "ComCounter.h"
#include "UtlReg.h"
#include "HeicException.h"

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


HRESULT RegisterDecoder(const std::wstring& dllPath)
{
	try
	{
		UtlReg regInstance;
		UtlReg regDecoder;

		regInstance.CreateKeyTree(HKEY_CLASSES_ROOT, "CLSID\\{7ED96837-96F0-4812-B211-F13C24117ED3}\\Instance\\" S_CLSID_HEICDecoder);
		regInstance.CreateValue("CLSID", S_CLSID_HEICDecoder);
		regInstance.CreateValue("FriendlyName", "HEIC Decoder");

		regDecoder.CreateKeyTree(HKEY_CLASSES_ROOT, "CLSID\\" S_CLSID_HEICDecoder);
		regDecoder.CreateValue("ArbitrationPriority", 0);
		regDecoder.CreateValue("SupportAnimation", 0);
		regDecoder.CreateValue("SupportChromaKey", 0);
		regDecoder.CreateValue("SupportLossless", 0);
		regDecoder.CreateValue("SupportMultiframe", 1);
		regDecoder.CreateValue("Author", "prsyahmi@github");
		regDecoder.CreateValue("ContainerFormat", S_GUID_ContainerFormatHEIC);
		regDecoder.CreateValue("Description", "HEIC Image Encoder");
		regDecoder.CreateValue("FriendlyName", "HEIC Image Encoder");
		regDecoder.CreateValue("FileExtensions", ".heic");
		regDecoder.CreateValue("MimeTypes", "image/heic");
		regDecoder.CreateValue("SpecVersion", "1.0.0.0");
		regDecoder.CreateValue("Vendor", S_GUID_VendorPrsyahmi);
		regDecoder.CreateValue("Version", "1.0.0.0");

		regDecoder.CreateKeyTree(HKEY_CLASSES_ROOT, "CLSID\\" S_CLSID_HEICDecoder "\\InprocServer32");
		regDecoder.CreateValue(L"", dllPath.c_str());
		regDecoder.CreateValue("ThreadingModel", "Both");

		// GUID_WICPixelFormat24bppRGB
		regDecoder.CreateKeyTree(HKEY_CLASSES_ROOT, "CLSID\\" S_CLSID_HEICDecoder "\\Formats\\{6fddc324-4e03-4bfe-b185-3d77768dc90d}");

		struct TPattern {
			char* Mask;
			char* Pattern;
			DWORD Size;
			DWORD Offset;
		};

		TPattern patterns[] = {
			{
				"\xff\xff\xff\xff\xff\xff\xff\xff",
				"\x66\x74\x79\x70\x68\x65\x69\x63",
				8,
				4
			},
			{
				"\xff\xff\xff\xff\xff\xff\xff\xff",
				"\x66\x74\x79\x70\x68\x65\x69\x73",
				8,
				4
			},
			{
				"\xff\xff\xff\xff\xff\xff\xff\xff",
				"\x66\x74\x79\x70\x6D\x69\x66\x31",
				8,
				4
			},
			{
				"\xff\xff\xff\xff\xff\xff\xff\xff",
				"\x66\x74\x79\x70\x6D\x69\x66\x32",
				8,
				4
			},
			{
				"\xff\xff\xff\xff\xff\xff\xff\xff",
				"\x66\x74\x79\x70\x68\x65\x69\x78",
				8,
				4
			},
			{
				"\xff\xff\xff\xff\xff\xff\xff\xff",
				"\x66\x74\x79\x70\x68\x65\x69\x6D",
				8,
				4
			},
			{
				"\xff\xff\xff\xff\xff\xff\xff\xff",
				"\x66\x74\x79\x70\x6D\x73\x66\x31",
				8,
				4
			},
		};

		for (size_t i = 0; i < _countof(patterns); i++) {
			std::string sKey = "CLSID\\" S_CLSID_HEICDecoder "\\Patterns\\";
			sKey += std::to_string(i);

			regDecoder.CreateKeyTree(HKEY_CLASSES_ROOT, sKey);
			regDecoder.CreateValue("Mask", patterns[i].Mask, patterns[i].Size);
			regDecoder.CreateValue("Pattern", patterns[i].Pattern, patterns[i].Size);
			regDecoder.CreateValue("Length", patterns[i].Size);
			regDecoder.CreateValue("Position", patterns[i].Offset);
		}

	}
	catch (const HeicException& ex)
	{
		Log("Unable to register dll: %s", ex.what());
		return ex.GetResult();
	}
	catch (const std::exception& ex)
	{
		Log("Unable to register dll: %s", ex.what());
		return S_FALSE;
	}

	return S_OK;
}

HRESULT RegisterEncoder(const std::wstring& dllPath)
{
	try
	{
		UtlReg regInstance;
		UtlReg regEncoder;

		regInstance.CreateKeyTree(HKEY_CLASSES_ROOT, "CLSID\\{AC757296-3522-4E11-9862-C17BE5A1767E}\\Instance\\" S_CLSID_HEICEncoder);
		regInstance.CreateValue("CLSID", S_CLSID_HEICEncoder);
		regInstance.CreateValue("FriendlyName", "HEIC Encoder");

		regEncoder.CreateKeyTree(HKEY_CLASSES_ROOT, "CLSID\\" S_CLSID_HEICEncoder);
		regEncoder.CreateValue("ArbitrationPriority", 0);
		regEncoder.CreateValue("SupportAnimation", 0);
		regEncoder.CreateValue("SupportChromaKey", 0);
		regEncoder.CreateValue("SupportLossless", 0);
		regEncoder.CreateValue("SupportMultiframe", 1);
		regEncoder.CreateValue("Author", "prsyahmi@github");
		regEncoder.CreateValue("ContainerFormat", S_GUID_ContainerFormatHEIC);
		regEncoder.CreateValue("Description", "HEIC Image Encoder");
		regEncoder.CreateValue("FriendlyName", "HEIC Image Encoder");
		regEncoder.CreateValue("FileExtensions", ".heic");
		regEncoder.CreateValue("MimeTypes", "image/heic");
		regEncoder.CreateValue("SpecVersion", "1.0.0.0");
		regEncoder.CreateValue("Vendor", S_GUID_VendorPrsyahmi);
		regEncoder.CreateValue("Version", "1.0.0.0");

		regEncoder.CreateKeyTree(HKEY_CLASSES_ROOT, "CLSID\\" S_CLSID_HEICEncoder "\\InprocServer32");
		regEncoder.CreateValue(L"", dllPath.c_str());
		regEncoder.CreateValue("ThreadingModel", "Both");

		// GUID_WICPixelFormat24bppRGB
		regEncoder.CreateKeyTree(HKEY_CLASSES_ROOT, "CLSID\\" S_CLSID_HEICEncoder "\\Formats\\{6fddc324-4e03-4bfe-b185-3d77768dc90d}");
	}
	catch (const HeicException& ex)
	{
		Log("Unable to register dll: %s", ex.what());
		return ex.GetResult();
	}
	catch (const std::exception& ex)
	{
		Log("Unable to register dll: %s", ex.what());
		return S_FALSE;
	}

	return S_OK;
}

STDAPI DllRegisterServer()
{
	// Requires Admin rights

	std::wstring dllPath;

	DWORD copied = 0;
	do {
		dllPath.resize(dllPath.size() + MAX_PATH);
		copied = GetModuleFileName(GModule, &dllPath[0], (DWORD)dllPath.size());
	} while (copied >= dllPath.size());
	dllPath.resize(copied);

	HRESULT hr;
	
	hr = RegisterDecoder(dllPath);
	if (hr != S_OK) {
		return hr;
	}

	hr = RegisterEncoder(dllPath);
	if (hr != S_OK) {
		return hr;
	}

	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
	return S_OK;
}

STDAPI DllUnregisterServer()
{
	RegDelnodeRecurse(HKEY_CLASSES_ROOT, "CLSID\\{7ED96837-96F0-4812-B211-F13C24117ED3}\\Instance\\" S_CLSID_HEICDecoder);
	RegDelnodeRecurse(HKEY_CLASSES_ROOT, "CLSID\\" S_CLSID_HEICDecoder);
	RegDelnodeRecurse(HKEY_CLASSES_ROOT, "CLSID\\{AC757296-3522-4E11-9862-C17BE5A1767E}\\Instance\\" S_CLSID_HEICEncoder);
	RegDelnodeRecurse(HKEY_CLASSES_ROOT, "CLSID\\" S_CLSID_HEICEncoder);

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
