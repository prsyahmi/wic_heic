#include "stdafx.h"
#include "HeicMetadataQueryReader.h"

class CHeicMetadataEnum : public IEnumString
{
private:
	std::atomic<uint32_t> m_Count;

public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
	{
		HRESULT hr = S_OK;

		if (!ppvObject) {
			return E_INVALIDARG;
		}

		*ppvObject = nullptr;
		if (IsEqualGUID(riid, IID_IUnknown)) {
			*ppvObject = static_cast<IUnknown*>(this);
		} else if (IsEqualGUID(riid, IID_IEnumString)) {
			*ppvObject = static_cast<CHeicMetadataEnum*>(this);
		} else {
			hr = E_NOINTERFACE;
		}

		if (SUCCEEDED(hr)) {
			AddRef();
		}

		return hr;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void)
	{
		return ++m_Count;
	}

	virtual ULONG STDMETHODCALLTYPE Release(void)
	{
		uint32_t n = --m_Count;
		if (m_Count == 0) {
			delete this;
		}

		return n;
	}

	virtual HRESULT STDMETHODCALLTYPE Next(ULONG celt, _Out_writes_to_(celt, *pceltFetched) LPOLESTR* rgelt, _Out_opt_ ULONG* pceltFetched)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE Skip(ULONG celt)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE Reset(void)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE Clone(__RPC__deref_out_opt IEnumString** ppenum)
	{
		return E_NOTIMPL;
	}
};

CHeicMetadataQueryReader::CHeicMetadataQueryReader(heif::ImageHandle handle)
	: m_Handle(handle)
{
}

CHeicMetadataQueryReader::~CHeicMetadataQueryReader()
{
}

HRESULT STDMETHODCALLTYPE CHeicMetadataQueryReader::QueryInterface(REFIID riid, void** ppvObject)
{
	HRESULT hr = S_OK;

	if (!ppvObject) {
		return E_INVALIDARG;
	}

	*ppvObject = nullptr;
	if (IsEqualGUID(riid, IID_IUnknown)) {
		*ppvObject = static_cast<IUnknown*>(this);
	} else if (IsEqualGUID(riid, IID_IWICMetadataQueryReader)) {
		*ppvObject = static_cast<IWICMetadataQueryReader*>(this);
	} else {
		hr = E_NOINTERFACE;
	}

	if (SUCCEEDED(hr)) {
		AddRef();
	}

	return hr;
}

ULONG STDMETHODCALLTYPE CHeicMetadataQueryReader::AddRef(void)
{
	return ++m_Count;
}

ULONG STDMETHODCALLTYPE CHeicMetadataQueryReader::Release(void)
{
	uint32_t n = --m_Count;
	if (m_Count == 0) {
		delete this;
	}

	return n;
}

HRESULT STDMETHODCALLTYPE CHeicMetadataQueryReader::GetContainerFormat(__RPC__out GUID* pguidContainerFormat)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CHeicMetadataQueryReader::GetLocation(UINT cchMaxLength, __RPC__inout_ecount_full_opt(cchMaxLength) WCHAR* wzNamespace, __RPC__out UINT* pcchActualLength)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CHeicMetadataQueryReader::GetMetadataByName(__RPC__in LPCWSTR wzName, __RPC__inout_opt PROPVARIANT* pvarValue)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CHeicMetadataQueryReader::GetEnumerator(__RPC__deref_out_opt IEnumString** ppIEnumString)
{
	return E_NOTIMPL;
}
