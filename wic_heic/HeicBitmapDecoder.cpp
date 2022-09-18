#include "stdafx.h"
#include "wic_heic.h"
#include "HeicBitmapDecoder.h"
#include "HeifStreamReader.h"
#include "HeicBitmapFrameDecode.h"

CHeicBitmapDecoder::CHeicBitmapDecoder()
	: m_Count(0)
{
}


CHeicBitmapDecoder::~CHeicBitmapDecoder()
{
}

HRESULT STDMETHODCALLTYPE CHeicBitmapDecoder::QueryInterface(REFIID riid, void **ppvObject)
{
	HRESULT hr = S_OK;

	if (!ppvObject) {
		return E_INVALIDARG;
	}

	*ppvObject = nullptr;
	if (IsEqualGUID(riid, IID_IUnknown)) {
		*ppvObject = static_cast<IUnknown*>(this);
	} else if (IsEqualGUID(riid, IID_IWICBitmapDecoder)) {
		*ppvObject = static_cast<IWICBitmapDecoder*>(this);
	} else {
		hr = E_NOINTERFACE;
	}

	if (SUCCEEDED(hr)) {
		AddRef();
	}

	return hr;
}

ULONG STDMETHODCALLTYPE CHeicBitmapDecoder::AddRef(void)
{
	return ++m_Count;
}

ULONG STDMETHODCALLTYPE CHeicBitmapDecoder::Release(void)
{
	uint32_t n = --m_Count;
	if (m_Count == 0) {
		delete this;
	}

	return n;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapDecoder::QueryCapability(__RPC__in_opt IStream *pIStream, __RPC__out DWORD *pdwCapability)
{
	// https://learn.microsoft.com/en-us/windows/win32/wic/-wic-imp-iwicbitmapdecoder
	return WICBitmapDecoderCapabilityCanDecodeSomeImages;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapDecoder::Initialize(__RPC__in_opt IStream *pIStream, WICDecodeOptions cacheOptions)
{
	// https://github.com/strukturag/libheif/issues/83
	try
	{
		CHeifStreamReader reader(pIStream);
		m_Context.read_from_reader(reader);
	}
	catch (const std::exception& ex)
	{
		return E_OUTOFMEMORY;
	}
	catch (...)
	{
		return E_OUTOFMEMORY;
	}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapDecoder::GetContainerFormat(__RPC__out GUID *pguidContainerFormat)
{
	if (!pguidContainerFormat) {
		return E_INVALIDARG;
	}

	*pguidContainerFormat = GUID_ContainerFormatHEIC;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapDecoder::GetDecoderInfo(__RPC__deref_out_opt IWICBitmapDecoderInfo **ppIDecoderInfo)
{
	return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapDecoder::CopyPalette(__RPC__in_opt IWICPalette *pIPalette)
{
	return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapDecoder::GetMetadataQueryReader(__RPC__deref_out_opt IWICMetadataQueryReader **ppIMetadataQueryReader)
{
	return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapDecoder::GetPreview(__RPC__deref_out_opt IWICBitmapSource **ppIBitmapSource)
{
	return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapDecoder::GetColorContexts(UINT cCount, __RPC__inout_ecount_full_opt(cCount) IWICColorContext **ppIColorContexts, __RPC__out UINT *pcActualCount)
{
	return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapDecoder::GetThumbnail(__RPC__deref_out_opt IWICBitmapSource **ppIThumbnail)
{
	return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapDecoder::GetFrameCount(__RPC__out UINT *pCount)
{
	if (!pCount) {
		return E_INVALIDARG;
	}

	*pCount = static_cast<UINT>(m_Context.get_number_of_top_level_images());
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapDecoder::GetFrame(UINT index, __RPC__deref_out_opt IWICBitmapFrameDecode **ppIBitmapFrame)
{
	if (index >= m_Context.get_number_of_top_level_images()) {
		return WINCODEC_ERR_FRAMEMISSING;
	}

	if (!ppIBitmapFrame) {
		return E_INVALIDARG;
	}

	heif::ImageHandle handle = m_Context.get_image_handle(index);
	if (handle.empty()) {
		return WINCODEC_ERR_FRAMEMISSING;
	}

	CHeicBitmapFrameDecode* decoder = new(std::nothrow) CHeicBitmapFrameDecode(handle);
	if (!decoder) {
		return E_OUTOFMEMORY;
	}

	decoder->AddRef();
	*ppIBitmapFrame = decoder;

	return S_OK;
}
