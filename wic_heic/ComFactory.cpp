#include "stdafx.h"
#include "wic_heic.h"
#include "ComFactory.h"
#include "HeifStreamReader.h"
#include "HeicBitmapDecoder.h"


CComFactory::CComFactory()
	: m_Count(1)
{
}


CComFactory::~CComFactory()
{
}

ULONG STDMETHODCALLTYPE CComFactory::AddRef(void)
{
	return ++m_Count;
}

ULONG STDMETHODCALLTYPE CComFactory::Release(void)
{
	uint32_t n = --m_Count;
	if (n == 0) {
		delete this;
	}

	return n;
}

HRESULT STDMETHODCALLTYPE CComFactory::QueryInterface(REFIID riid, void **ppvObject)
{
	HRESULT hr = S_OK;

	if (!ppvObject) {
		return E_INVALIDARG;
	}

	*ppvObject = nullptr;
	if (IsEqualGUID(riid, IID_IUnknown)) {
		*ppvObject = static_cast<IUnknown*>(this);
	} else if (IsEqualGUID(riid, IID_IClassFactory)) {
		*ppvObject = static_cast<IClassFactory*>(this);
	} else {
		hr = E_NOINTERFACE;
	}

	if (SUCCEEDED(hr)) {
		AddRef();
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE CComFactory::CreateInstance(_In_opt_ IUnknown *pUnkOuter, _In_ REFIID riid, _COM_Outptr_ void **ppvObject)
{
	HRESULT hr = S_OK;

	if (!ppvObject) {
		return E_INVALIDARG;
	}

	*ppvObject = nullptr;
	if (IsEqualGUID(riid, IID_IWICBitmapDecoder)) {
		CHeicBitmapDecoder* decoder = new(std::nothrow) CHeicBitmapDecoder();
		if (!decoder) {
			return E_OUTOFMEMORY;
		}

		hr = decoder->QueryInterface(riid, ppvObject);
		decoder->Release();

		if (FAILED(hr)) {
			return hr;
		}
	} else {
		hr = E_NOINTERFACE;
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE CComFactory::LockServer(BOOL fLock)
{
	return CoLockObjectExternal(this, fLock, TRUE);
}
