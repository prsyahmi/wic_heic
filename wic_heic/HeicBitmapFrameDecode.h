#pragma once

class CHeicBitmapFrameDecode : public IWICBitmapFrameDecode
{
private:
	std::atomic<uint32_t> m_Count;
	heif::ImageHandle m_Handle;
	heif::Image m_Image;
	heif_chroma m_chroma;
	WICPixelFormatGUID m_PixelFormat;
	int m_Bpp;
	size_t m_Stride;
	uint8_t* m_PlaneInterleaved;

public:
	CHeicBitmapFrameDecode(heif::ImageHandle handle);
	virtual ~CHeicBitmapFrameDecode();

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE Release(void);
	virtual HRESULT STDMETHODCALLTYPE GetSize(__RPC__out UINT *puiWidth, __RPC__out UINT *puiHeight);
	virtual HRESULT STDMETHODCALLTYPE GetPixelFormat(__RPC__out WICPixelFormatGUID *pPixelFormat);
	virtual HRESULT STDMETHODCALLTYPE GetResolution(__RPC__out double *pDpiX, __RPC__out double *pDpiY);
	virtual HRESULT STDMETHODCALLTYPE CopyPalette(__RPC__in_opt IWICPalette *pIPalette);
	virtual HRESULT STDMETHODCALLTYPE CopyPixels(__RPC__in_opt const WICRect *prc, UINT cbStride, UINT cbBufferSize, __RPC__out_ecount_full(cbBufferSize) BYTE *pbBuffer);
	virtual HRESULT STDMETHODCALLTYPE GetMetadataQueryReader(__RPC__deref_out_opt IWICMetadataQueryReader **ppIMetadataQueryReader);
	virtual HRESULT STDMETHODCALLTYPE GetColorContexts(UINT cCount, __RPC__inout_ecount_full_opt(cCount) IWICColorContext **ppIColorContexts, __RPC__out UINT *pcActualCount);
	virtual HRESULT STDMETHODCALLTYPE GetThumbnail(__RPC__deref_out_opt IWICBitmapSource **ppIThumbnail);
};

