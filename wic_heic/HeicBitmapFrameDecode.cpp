#include "stdafx.h"
#include "wic_heic.h"
#include "HeicBitmapFrameDecode.h"


CHeicBitmapFrameDecode::CHeicBitmapFrameDecode(heif::ImageHandle handle)
	: m_Count(1)
	, m_Handle(handle)
	, m_Bpp(0)
	, m_Plane(nullptr)
	, m_Stride(0)
{
	// Set both to undefined to use original one
	m_Image = handle.decode_image(heif_colorspace_RGB, heif_chroma_interleaved_RGB);
	m_Bpp = m_Image.get_bits_per_pixel(heif_channel_interleaved);
	m_Plane = m_Image.get_plane(heif_channel_interleaved, &m_Stride);

	if (!m_Plane || !m_Stride) {
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

	heif_chroma cformat = m_Image.get_chroma_format();
	DbgLog("GetPixelFormat: %d", cformat);

	switch (cformat)
	{
	case heif_chroma_monochrome:
		// I'm unsure about this... Does heif_chroma_monochrome has fixed BPP?
		if (m_Bpp == 1) {
			*pPixelFormat = GUID_WICPixelFormatBlackWhite;
		} else if (m_Bpp == 2) {
			*pPixelFormat = GUID_WICPixelFormat2bppGray;
		} else if (m_Bpp == 4) {
			*pPixelFormat = GUID_WICPixelFormat4bppGray;
		} else if (m_Bpp == 8) {
			*pPixelFormat = GUID_WICPixelFormat8bppGray;
		}
		break;
	case heif_chroma_420:
	case heif_chroma_422:
	case heif_chroma_444:
		//*pPixelFormat = ;
		break;
	case heif_chroma_interleaved_RGB:
		*pPixelFormat = GUID_WICPixelFormat24bppRGB;
		break;
	case heif_chroma_interleaved_RGBA:
		*pPixelFormat = GUID_WICPixelFormat32bppRGBA;
		break;
	case heif_chroma_interleaved_RRGGBB_BE:
		*pPixelFormat = GUID_WICPixelFormat48bppBGR;
		break;
	case heif_chroma_interleaved_RRGGBB_LE:
		*pPixelFormat = GUID_WICPixelFormat48bppRGB;
		break;
	case heif_chroma_interleaved_RRGGBBAA_BE:
		*pPixelFormat = GUID_WICPixelFormat64bppBGRA;
		break;
	case heif_chroma_interleaved_RRGGBBAA_LE:
		*pPixelFormat = GUID_WICPixelFormat64bppRGBA;
		break;
	}

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

	DbgLog("CopyPixels: (X=%d,Y=%d,W=%d,H=%d), outStride=%u, inStride=%d",
		rc.X, rc.Y, rc.Width, rc.Height,
		cbStride, m_Stride);

	uint8_t* plane = m_Plane;
	plane += (m_Stride * rc.Y);

	uint8_t* out = pbBuffer;
	for (int height = 0; height < rc.Height; height++)
	{
		memcpy_s(out, cbBufferSize, plane + (rc.X * m_Bpp), cbStride);
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

		decoder->AddRef();
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
