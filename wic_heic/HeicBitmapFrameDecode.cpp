#include "stdafx.h"
#include "wic_heic.h"
#include "HeicBitmapFrameDecode.h"


CHeicBitmapFrameDecode::CHeicBitmapFrameDecode(heif::ImageHandle handle)
	: m_Count(1)
	, m_Handle(handle)
	, m_Bpp(0)
	, m_PlaneInterleaved(nullptr)
	, m_Stride(0)
{
	int bpc = handle.get_chroma_bits_per_pixel();
	if (bpc == -1) {
		bpc = handle.get_luma_bits_per_pixel();
	}

	m_chroma = heif_chroma_interleaved_RGB;
	m_PixelFormat = GUID_WICPixelFormat24bppRGB;
	m_Bpp = 32;

	if (bpc == 8 && !m_Handle.has_alpha_channel())
	{
		m_PixelFormat = GUID_WICPixelFormat24bppRGB;
		m_chroma = heif_chroma_interleaved_RGB;
		m_Bpp = 32;
	}
	else if (bpc == 8 && m_Handle.has_alpha_channel())
	{
		m_PixelFormat = GUID_WICPixelFormat32bppRGBA;
		m_chroma = heif_chroma_interleaved_RGBA;
		m_Bpp = 32;
	}
	else if (bpc == 10 && !m_Handle.has_alpha_channel())
	{
		m_PixelFormat = GUID_WICPixelFormat48bppRGB;
		m_chroma = heif_chroma_interleaved_RRGGBB_BE;
		m_Bpp = 32;
	}
	else if (bpc == 10 && m_Handle.has_alpha_channel())
	{
		m_PixelFormat = GUID_WICPixelFormat64bppRGBA;
		m_chroma = heif_chroma_interleaved_RRGGBBAA_BE;
		m_Bpp = 64;
	}
	else if (bpc == 12 && !m_Handle.has_alpha_channel())
	{
		m_PixelFormat = GUID_WICPixelFormat48bppRGB;
		m_chroma = heif_chroma_interleaved_RRGGBB_BE;
		m_Bpp = 48;
	}
	else if (bpc == 12 && m_Handle.has_alpha_channel())
	{
		m_PixelFormat = GUID_WICPixelFormat64bppRGBA;
		m_chroma = heif_chroma_interleaved_RRGGBBAA_BE;
		m_Bpp = 64;
	}

	// No HDR support for now
	m_chroma = heif_chroma_interleaved_RGB;
	m_PixelFormat = GUID_WICPixelFormat24bppRGB;
	m_Bpp = 32;

	// Set both to undefined to use original one
	m_Image = handle.decode_image(heif_colorspace_RGB, m_chroma);
	m_PlaneInterleaved = m_Image.get_plane2(heif_channel_interleaved, &m_Stride);

	DbgLog("Decode: (chroma=%d, bpp=%d, bpc=%d, stride=%d)",
		m_chroma, m_Bpp, bpc, m_Stride);

	if (!m_PlaneInterleaved || !m_Stride) {
		throw std::exception("Unable to get plane");
	}
}


CHeicBitmapFrameDecode::~CHeicBitmapFrameDecode()
{
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameDecode::QueryInterface(REFIID riid, void **ppvObject)
{
	HRESULT hr = S_OK;

	if (!ppvObject) {
		return E_INVALIDARG;
	}

	*ppvObject = nullptr;
	if (IsEqualGUID(riid, IID_IUnknown)) {
		*ppvObject = static_cast<IUnknown*>(this);
	} else if (IsEqualGUID(riid, IID_IWICBitmapSource)) {
		*ppvObject = static_cast<IWICBitmapSource*>(this);
	} else if (IsEqualGUID(riid, IID_IWICBitmapFrameDecode)) {
		*ppvObject = static_cast<IWICBitmapFrameDecode*>(this);
	} else {
		hr = E_NOINTERFACE;
	}

	if (SUCCEEDED(hr)) {
		AddRef();
	}

	return hr;
}

ULONG STDMETHODCALLTYPE CHeicBitmapFrameDecode::AddRef(void)
{
	return ++m_Count;
}

ULONG STDMETHODCALLTYPE CHeicBitmapFrameDecode::Release(void)
{
	uint32_t n = --m_Count;
	if (m_Count == 0) {
		delete this;
	}

	return n;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameDecode::GetSize(__RPC__out UINT *puiWidth, __RPC__out UINT *puiHeight)
{
	if (!puiWidth || !puiHeight) {
		return E_INVALIDARG;
	}

	*puiWidth = static_cast<UINT>(m_Handle.get_width());
	*puiHeight = static_cast<UINT>(m_Handle.get_height());

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameDecode::GetPixelFormat(__RPC__out WICPixelFormatGUID *pPixelFormat)
{
	if (!pPixelFormat) {
		return E_INVALIDARG;
	}

	DbgLog("GetPixelFormat");
	*pPixelFormat = m_PixelFormat;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameDecode::GetResolution(__RPC__out double *pDpiX, __RPC__out double *pDpiY)
{
	if (pDpiX == nullptr || pDpiY == nullptr) {
		return E_INVALIDARG;
	}

	*pDpiX = 96;
	*pDpiY = 96;

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameDecode::CopyPalette(__RPC__in_opt IWICPalette *pIPalette)
{
	return WINCODEC_ERR_PALETTEUNAVAILABLE;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameDecode::CopyPixels(__RPC__in_opt const WICRect *prc, UINT cbStride, UINT cbBufferSize, __RPC__out_ecount_full(cbBufferSize) BYTE *pbBuffer)
{
	if (!pbBuffer) {
		return E_INVALIDARG;
	}

	WICRect rc;
	rc.X = 0;
	rc.Y = 0;
	rc.Width = m_Handle.get_width();
	rc.Height = m_Handle.get_height();

	if (prc) {
		rc = *prc;
	}

	DbgLog("CopyPixels: (X=%d,Y=%d,W=%d,H=%d), outStride=%u, inStride=%d, bpp=%d",
		rc.X, rc.Y, rc.Width, rc.Height,
		cbStride, m_Stride, m_Bpp);

	int bytesPerPixel = m_Bpp / 8;

	uint8_t* plane = m_PlaneInterleaved;
	plane += (m_Stride * rc.Y);

	uint8_t* out = pbBuffer;

	for (int height = 0; height < rc.Height; height++)
	{
		uint8_t offsetX = rc.X * bytesPerPixel;
		memcpy_s(out, cbBufferSize - offsetX, plane + offsetX, cbStride - offsetX);
		out += cbStride;
		plane += m_Stride;
	}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameDecode::GetMetadataQueryReader(__RPC__deref_out_opt IWICMetadataQueryReader **ppIMetadataQueryReader)
{
	return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameDecode::GetColorContexts(UINT cCount, __RPC__inout_ecount_full_opt(cCount) IWICColorContext **ppIColorContexts, __RPC__out UINT *pcActualCount)
{
	return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameDecode::GetThumbnail(__RPC__deref_out_opt IWICBitmapSource **ppIThumbnail)
{
	try
	{
		DbgLog("Frame::GetThumbnail@1");
		int index = 0;

		if (!ppIThumbnail) {
			return E_INVALIDARG;
		}

		std::vector<heif_item_id> ids = m_Handle.get_list_of_thumbnail_IDs();
		if (!ids.size()) {
			return WINCODEC_ERR_FRAMEMISSING;
		}

		heif::ImageHandle thumbHandle = m_Handle.get_thumbnail(ids[index]);
		if (thumbHandle.empty()) {
			return WINCODEC_ERR_FRAMEMISSING;
		}

		DbgLog("GetThumbnail@handle: hasAlpha=%d, chromaBpp=%d, lumaBpp=%d, width=%d, height=%d, primary=%d",
			thumbHandle.has_alpha_channel(),
			thumbHandle.get_chroma_bits_per_pixel(),
			thumbHandle.get_luma_bits_per_pixel(),
			thumbHandle.get_width(),
			thumbHandle.get_height(),
			thumbHandle.is_primary_image()
		);

		CHeicBitmapFrameDecode* decoder = new(std::nothrow) CHeicBitmapFrameDecode(thumbHandle);
		if (!decoder) {
			return E_OUTOFMEMORY;
		}

		*ppIThumbnail = decoder;
	}
	catch (const heif::Error& ex)
	{
		Log("GetThumbnail@Exception: %s", ex.get_message().c_str());
		return E_INVALIDARG;
	}
	catch (const std::exception& ex)
	{
		Log("GetThumbnail@Exception: %s", ex.what());
		return E_INVALIDARG;
	}
	catch (...)
	{
		Log("GetThumbnail@Exception: Unknown");
		return E_INVALIDARG;
	}

	return S_OK;
}
