#pragma once

class CHeicBitmapFrameEncode : public IWICBitmapFrameEncode
{
private:
	friend class CHeicBitmapEncoder;

	std::atomic<uint32_t> m_Count;
	UINT m_Width, m_Height;
	heif_chroma m_Chroma;
	heif_colorspace m_Colorspace;
	bool m_Created;
	WICPixelFormatGUID m_PixelFormat;

protected:
	heif::Image m_Frame;

public:
	CHeicBitmapFrameEncode();
	~CHeicBitmapFrameEncode();

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE Release(void);
	virtual HRESULT STDMETHODCALLTYPE Initialize(__RPC__in_opt IPropertyBag2 *pIEncoderOptions);
	virtual HRESULT STDMETHODCALLTYPE SetSize(UINT uiWidth, UINT uiHeight);
	virtual HRESULT STDMETHODCALLTYPE SetResolution(double dpiX, double dpiY);
	virtual HRESULT STDMETHODCALLTYPE SetPixelFormat(__RPC__inout WICPixelFormatGUID *pPixelFormat);
	virtual HRESULT STDMETHODCALLTYPE SetColorContexts(UINT cCount, __RPC__in_ecount_full(cCount) IWICColorContext **ppIColorContext);
	virtual HRESULT STDMETHODCALLTYPE SetPalette(__RPC__in_opt IWICPalette *pIPalette);
	virtual HRESULT STDMETHODCALLTYPE SetThumbnail(__RPC__in_opt IWICBitmapSource *pIThumbnail);
	virtual HRESULT STDMETHODCALLTYPE WritePixels(UINT lineCount, UINT cbStride, UINT cbBufferSize, __RPC__in_ecount_full(cbBufferSize) BYTE *pbPixels);
	virtual HRESULT STDMETHODCALLTYPE WriteSource(__RPC__in_opt IWICBitmapSource *pIBitmapSource, __RPC__in_opt WICRect *prc);
	virtual HRESULT STDMETHODCALLTYPE Commit(void);
	virtual HRESULT STDMETHODCALLTYPE GetMetadataQueryWriter(__RPC__deref_out_opt IWICMetadataQueryWriter **ppIMetadataQueryWriter);

private:
	void CreateFrame();
};

