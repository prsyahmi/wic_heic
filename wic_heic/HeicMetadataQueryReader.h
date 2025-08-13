#pragma once

class CHeicMetadataQueryReader : public IWICMetadataQueryReader
{
private:
	std::atomic<uint32_t> m_Count;
	heif::ImageHandle m_Handle;

public:
	CHeicMetadataQueryReader(heif::ImageHandle handle);
	~CHeicMetadataQueryReader();

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE Release(void);

	virtual HRESULT STDMETHODCALLTYPE GetContainerFormat(__RPC__out GUID* pguidContainerFormat);
	virtual HRESULT STDMETHODCALLTYPE GetLocation(UINT cchMaxLength, __RPC__inout_ecount_full_opt(cchMaxLength) WCHAR* wzNamespace, __RPC__out UINT* pcchActualLength);
	virtual HRESULT STDMETHODCALLTYPE GetMetadataByName(__RPC__in LPCWSTR wzName, __RPC__inout_opt PROPVARIANT* pvarValue);
	virtual HRESULT STDMETHODCALLTYPE GetEnumerator(__RPC__deref_out_opt IEnumString** ppIEnumString);
};
