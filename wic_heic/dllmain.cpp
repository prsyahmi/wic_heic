// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "wic_heic.h"
#include "ComFactory.h"

#if defined(X86)
#pragma comment (linker, "/export:DllGetClassObject=_DllGetClassObject@12,PRIVATE")
#else
#pragma comment (linker, "/export:DllCanUnloadNow,PRIVATE")
#pragma comment (linker, "/export:DllGetClassObject,PRIVATE")
#pragma comment (linker, "/export:DllRegisterServer,PRIVATE")
#pragma comment (linker, "/export:DllUnregisterServer,PRIVATE")
#endif


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

STDAPI DllRegisterServer()
{
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
	return S_OK;
}

STDAPI DllUnregisterServer()
{
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

	if (IsEqualCLSID(rclsid, CLSID_HEICDecoder)) {
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
	return 0;
}
