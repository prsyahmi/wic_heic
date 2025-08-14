#pragma once

class CHeicBitmapEncoder : public IWICBitmapEncoder
{
private:
	std::atomic<uint32_t> m_Count;
	heif::Context m_Context;
	std::vector<CHeicBitmapFrameEncode*> m_Frames;
	IStream* m_Stream;

public:
	CHeicBitmapEncoder();
	virtual ~CHeicBitmapEncoder();

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE Release(void);
	virtual HRESULT STDMETHODCALLTYPE Initialize(__RPC__in_opt IStream *pIStream, WICBitmapEncoderCacheOption cacheOption);
	virtual HRESULT STDMETHODCALLTYPE GetContainerFormat(__RPC__out GUID *pguidContainerFormat);
	virtual HRESULT STDMETHODCALLTYPE GetEncoderInfo(__RPC__deref_out_opt IWICBitmapEncoderInfo **ppIEncoderInfo);
	virtual HRESULT STDMETHODCALLTYPE SetColorContexts(UINT cCount, __RPC__in_ecount_full(cCount) IWICColorContext **ppIColorContext);
	virtual HRESULT STDMETHODCALLTYPE SetPalette(__RPC__in_opt IWICPalette *pIPalette);
	virtual HRESULT STDMETHODCALLTYPE SetThumbnail(__RPC__in_opt IWICBitmapSource *pIThumbnail);
	virtual HRESULT STDMETHODCALLTYPE SetPreview(__RPC__in_opt IWICBitmapSource *pIPreview);
	virtual HRESULT STDMETHODCALLTYPE CreateNewFrame(__RPC__deref_out_opt IWICBitmapFrameEncode **ppIFrameEncode, __RPC__deref_opt_inout_opt IPropertyBag2 **ppIEncoderOptions);
	virtual HRESULT STDMETHODCALLTYPE Commit(void);
	virtual HRESULT STDMETHODCALLTYPE GetMetadataQueryWriter(__RPC__deref_out_opt IWICMetadataQueryWriter **ppIMetadataQueryWriter);
};

