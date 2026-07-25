#pragma once

class CHeicMetadataBlockReader : public IWICMetadataBlockReader
{
private:
	std::atomic<uint32_t> m_Count;
	heif::ImageHandle m_Handle;
	std::vector<heif_item_id> m_MetadataBlock;

public:
	CHeicMetadataBlockReader(heif::ImageHandle handle);
	virtual ~CHeicMetadataBlockReader();

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE Release(void);

    virtual HRESULT STDMETHODCALLTYPE GetContainerFormat(__RPC__out GUID* pguidContainerFormat);
    virtual HRESULT STDMETHODCALLTYPE GetCount(__RPC__out UINT* pcCount);
    virtual HRESULT STDMETHODCALLTYPE GetReaderByIndex(UINT nIndex, __RPC__deref_out_opt IWICMetadataReader** ppIMetadataReader);
	virtual HRESULT STDMETHODCALLTYPE GetEnumerator(__RPC__deref_out_opt IEnumUnknown** ppIEnumMetadata);

	HRESULT STDMETHODCALLTYPE CreateQueryReader(IWICMetadataQueryReader** ppIQueryReader);
};

