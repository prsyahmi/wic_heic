#include "stdafx.h"
#include "wic_heic.h"
#include "HeicBitmapFrameEncode.h"


CHeicBitmapFrameEncode::CHeicBitmapFrameEncode()
	: m_Count(1)
	, m_Width(0)
	, m_Height(0)
	, m_Chroma(heif_chroma_undefined)
	, m_Colorspace(heif_colorspace_RGB)
	, m_Created(false)
	, m_BitDepthPerChannel(8)
{
}


CHeicBitmapFrameEncode::~CHeicBitmapFrameEncode()
{
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameEncode::QueryInterface(REFIID riid, void **ppvObject)
{
	HRESULT hr = S_OK;

	if (!ppvObject) {
		return E_INVALIDARG;
	}

	*ppvObject = nullptr;
	if (IsEqualGUID(riid, IID_IUnknown)) {
		*ppvObject = static_cast<IUnknown*>(this);
	} else if (IsEqualGUID(riid, IID_IWICBitmapFrameDecode)) {
		*ppvObject = static_cast<IWICBitmapFrameEncode*>(this);
	} else {
		hr = E_NOINTERFACE;
	}

	if (SUCCEEDED(hr)) {
		AddRef();
	}

	return hr;
}

ULONG STDMETHODCALLTYPE CHeicBitmapFrameEncode::AddRef(void)
{
	return ++m_Count;
}

ULONG STDMETHODCALLTYPE CHeicBitmapFrameEncode::Release(void)
{
	uint32_t n = --m_Count;
	if (m_Count == 0) {
		delete this;
	}

	return n;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameEncode::Initialize(__RPC__in_opt IPropertyBag2 *pIEncoderOptions)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameEncode::SetSize(UINT uiWidth, UINT uiHeight)
{
	m_Width = uiWidth;
	m_Height = uiHeight;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameEncode::SetResolution(double dpiX, double dpiY)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameEncode::SetPixelFormat(__RPC__inout WICPixelFormatGUID *pPixelFormat)
{
	if (!pPixelFormat) {
		return E_INVALIDARG;
	}

	if (IsEqualGUID(*pPixelFormat, GUID_WICPixelFormat24bppRGB)) {
		m_PixelFormat = *pPixelFormat;
		m_Chroma = heif_chroma_interleaved_RGB;
		m_BitDepthPerChannel = 8;
	}
	else if (IsEqualGUID(*pPixelFormat, GUID_WICPixelFormat24bppBGR)) {
		m_PixelFormat = GUID_WICPixelFormat24bppRGB;
		m_Chroma = heif_chroma_interleaved_RGB;
		m_BitDepthPerChannel = 8;
	}
	else if (IsEqualGUID(*pPixelFormat, GUID_WICPixelFormat32bppRGBA)) {
		m_PixelFormat = *pPixelFormat;
		m_Chroma = heif_chroma_interleaved_RGBA;
		m_BitDepthPerChannel = 8;
	}
	else if (IsEqualGUID(*pPixelFormat, GUID_WICPixelFormat32bppBGRA)) {
		m_PixelFormat = GUID_WICPixelFormat32bppRGBA;
		m_Chroma = heif_chroma_interleaved_RGBA;
		m_BitDepthPerChannel = 8;
	}
	// All below is not tested - not supported
	else if (IsEqualGUID(*pPixelFormat, GUID_WICPixelFormat48bppBGR)) {
		m_PixelFormat = GUID_WICPixelFormat32bppRGBA1010102;
		m_Chroma = heif_chroma_interleaved_RRGGBB_BE;
		m_BitDepthPerChannel = 16;
	}
	else if (IsEqualGUID(*pPixelFormat, GUID_WICPixelFormat48bppRGB)) {
		m_PixelFormat = GUID_WICPixelFormat32bppRGBA1010102;
		m_Chroma = heif_chroma_interleaved_RRGGBB_LE;
		m_BitDepthPerChannel = 16;
	}
	else if (IsEqualGUID(*pPixelFormat, GUID_WICPixelFormat64bppBGRA)) {
		m_PixelFormat = GUID_WICPixelFormat32bppRGBA1010102;
		m_Chroma = heif_chroma_interleaved_RRGGBBAA_BE;
		m_BitDepthPerChannel = 16;
	}
	else if (IsEqualGUID(*pPixelFormat, GUID_WICPixelFormat64bppRGBA)) {
		m_PixelFormat = GUID_WICPixelFormat32bppRGBA1010102;
		m_Chroma = heif_chroma_interleaved_RRGGBBAA_LE; // !
		m_BitDepthPerChannel = 16;
	}
	else {
		*pPixelFormat = GUID_WICPixelFormat24bppRGB;
		m_PixelFormat = *pPixelFormat;
		m_Chroma = heif_chroma_interleaved_RGB;
		m_BitDepthPerChannel = 8;
	}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameEncode::SetColorContexts(UINT cCount, __RPC__in_ecount_full(cCount) IWICColorContext **ppIColorContext)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameEncode::SetPalette(__RPC__in_opt IWICPalette *pIPalette)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameEncode::SetThumbnail(__RPC__in_opt IWICBitmapSource *pIThumbnail)
{
	try
	{
		// TODO
		return S_OK;
	}
	catch (const heif::Error& ex)
	{
		Log("Call to CHeicBitmapFrameEncode::SetThumbnail failed: %s", ex.get_message().c_str());
		return E_INVALIDARG;
	}
	catch (const std::exception& ex)
	{
		Log("Call to CHeicBitmapFrameEncode::SetThumbnail failed: %s", ex.what());
		return E_INVALIDARG;
	}
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameEncode::WritePixels(UINT lineCount, UINT cbStride, UINT cbBufferSize, __RPC__in_ecount_full(cbBufferSize) BYTE *pbPixels)
{
	try
	{
		// Untested

		int stride;
		UINT copyStride;

		CreateFrame();

		uint8_t* data = m_Frame.get_plane(heif_channel_interleaved, &stride);
		if (!data) {
			return E_OUTOFMEMORY;
		}

		copyStride = (UINT)stride > cbStride ? cbStride : stride;

		uint8_t* dest = pbPixels;
		UINT copied = 0;
		for (UINT y = 0; y < lineCount; y++) {
			memcpy_s(dest, cbBufferSize - (dest - pbPixels), data, copyStride);

			data += stride;
			dest += cbStride;
		}
	}
	catch (const heif::Error& ex)
	{
		Log("Call to CHeicBitmapFrameEncode::WritePixels failed: %s", ex.get_message().c_str());
		return E_INVALIDARG;
	}
	catch (const std::exception& ex)
	{
		Log("Call to CHeicBitmapFrameEncode::WritePixels failed: %s", ex.what());
		return E_INVALIDARG;
	}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameEncode::WriteSource(__RPC__in_opt IWICBitmapSource *pIBitmapSource, __RPC__in_opt WICRect *prc)
{
	try
	{
		HRESULT hr;
		int stride;
		UINT bufferSize = 0;
		WICPixelFormatGUID srcFormat;
		IWICBitmapSource *pConverted = NULL;

		CreateFrame();

		uint8_t* data = m_Frame.get_plane(heif_channel_interleaved, &stride);
		if (!data) {
			return E_OUTOFMEMORY;
		}

		bufferSize = stride * m_Height;

		hr = pIBitmapSource->GetPixelFormat(&srcFormat);
		if (FAILED(hr)) {
			throw std::exception("Unable to get source pixel format");
		}

		if (IsEqualGUID(srcFormat, m_PixelFormat))
		{
			return pIBitmapSource->CopyPixels(prc, stride, bufferSize, data);
		}
		else
		{
			hr = WICConvertBitmapSource(m_PixelFormat, pIBitmapSource, &pConverted);
			if (FAILED(hr)) {
				throw std::exception("Unable to convert source pixel format");
			}

			pConverted->CopyPixels(prc, stride, bufferSize, data);
		}
	}
	catch (const heif::Error& ex)
	{
		Log("Call to CHeicBitmapFrameEncode::WriteSource failed: %s", ex.get_message().c_str());
		return E_INVALIDARG;
	}
	catch (const std::exception& ex)
	{
		Log("Call to CHeicBitmapFrameEncode::WriteSource failed: %s", ex.what());
		return E_INVALIDARG;
	}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameEncode::Commit(void)
{
	try
	{
		CreateFrame();
		return S_OK;
	}
	catch (const heif::Error& ex)
	{
		Log("Call to CHeicBitmapFrameEncode::Commit failed: %s", ex.get_message().c_str());
		return E_INVALIDARG;
	}
	catch (const std::exception& ex)
	{
		Log("Call to CHeicBitmapFrameEncode::Commit failed: %s", ex.what());
		return E_INVALIDARG;
	}
}

HRESULT STDMETHODCALLTYPE CHeicBitmapFrameEncode::GetMetadataQueryWriter(__RPC__deref_out_opt IWICMetadataQueryWriter **ppIMetadataQueryWriter)
{
	return E_NOTIMPL;
}

void CHeicBitmapFrameEncode::CreateFrame()
{
	if (m_Created) return;
	m_Frame.create(m_Width, m_Height, m_Colorspace, m_Chroma);
	m_Frame.add_plane(heif_channel_interleaved, m_Width, m_Height, m_BitDepthPerChannel);

	m_Created = true;
}
