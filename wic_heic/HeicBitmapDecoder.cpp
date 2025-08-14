#include "stdafx.h"
#include "wic_heic.h"
#include "HeifStreamReader.h"
#include "HeicBitmapDecoder.h"
#include "HeicBitmapFrameDecode.h"

CHeicBitmapDecoder::CHeicBitmapDecoder()
	: m_Count(1)
	, m_Reader(nullptr)
{
}


CHeicBitmapDecoder::~CHeicBitmapDecoder()
{
	if (m_Reader) {
		delete m_Reader;
	}
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
	DbgLog("CHeicBitmapDecoder::QueryCapability()");

	// https://learn.microsoft.com/en-us/windows/win32/wic/-wic-imp-iwicbitmapdecoder
	if (!pdwCapability) {
		return E_INVALIDARG;
	}

	*pdwCapability = WICBitmapDecoderCapabilitySameEncoder | WICBitmapDecoderCapabilityCanDecodeSomeImages | WICBitmapDecoderCapabilityCanDecodeThumbnail;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapDecoder::Initialize(__RPC__in_opt IStream *pIStream, WICDecodeOptions cacheOptions)
{
	// https://github.com/strukturag/libheif/issues/83
	try
	{
		if (m_Reader) {
			return WINCODEC_ERR_WRONGSTATE;
		}

		m_Reader = new CHeifStreamReader(pIStream);
		m_Context.read_from_reader(*m_Reader);
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
	DbgLog("CHeicBitmapDecoder::GetContainerFormat()");

	if (!pguidContainerFormat) {
		return E_INVALIDARG;
	}

	*pguidContainerFormat = GUID_ContainerFormatHEIC;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapDecoder::GetDecoderInfo(__RPC__deref_out_opt IWICBitmapDecoderInfo **ppIDecoderInfo)
{
	DbgLog("CHeicBitmapDecoder::GetDecoderInfo()");
	return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapDecoder::CopyPalette(__RPC__in_opt IWICPalette *pIPalette)
{
	return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapDecoder::GetMetadataQueryReader(__RPC__deref_out_opt IWICMetadataQueryReader **ppIMetadataQueryReader)
{
	DbgLog("CHeicBitmapDecoder::GetMetadataQueryReader()");
	return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapDecoder::GetPreview(__RPC__deref_out_opt IWICBitmapSource **ppIBitmapSource)
{
	try
	{
		DbgLog("GetPreview@1");
		int index = 0;

		if (!m_Reader) {
			return WINCODEC_ERR_WRONGSTATE;
		}

		if (index >= m_Context.get_number_of_top_level_images()) {
			return WINCODEC_ERR_FRAMEMISSING;
		}

		if (!ppIBitmapSource) {
			return E_INVALIDARG;
		}

		std::vector<heif_item_id> ids = m_Context.get_list_of_top_level_image_IDs();

		heif::ImageHandle handle = m_Context.get_image_handle(ids[index]);
		if (handle.empty()) {
			return WINCODEC_ERR_FRAMEMISSING;
		}

		std::vector<heif_item_id> thumbIds = handle.get_list_of_thumbnail_IDs();
		if (!thumbIds.size()) {
			return WINCODEC_ERR_FRAMEMISSING;
		}

		heif::ImageHandle thumbHandle = handle.get_thumbnail(thumbIds[0]);

		DbgLog("GetPreview@handle: hasAlpha=%d, chromaBpp=%d, lumaBpp=%d, width=%d, height=%d, primary=%d",
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

		*ppIBitmapSource = decoder;
	}
	catch (const heif::Error& ex)
	{
		Log("GetPreview@Exception: %s", ex.get_message().c_str());
		return E_INVALIDARG;
	}
	catch (const std::exception& ex)
	{
		Log("GetPreview@Exception: %s", ex.what());
		return E_INVALIDARG;
	}
	catch (...)
	{
		Log("GetPreview@Exception: Unknown");
		return E_INVALIDARG;
	}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapDecoder::GetColorContexts(UINT cCount, __RPC__inout_ecount_full_opt(cCount) IWICColorContext **ppIColorContexts, __RPC__out UINT *pcActualCount)
{
	return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapDecoder::GetThumbnail(__RPC__deref_out_opt IWICBitmapSource **ppIThumbnail)
{
	try
	{
		DbgLog("GetThumbnail@1");

		if (!m_Reader) {
			return WINCODEC_ERR_WRONGSTATE;
		}

		heif::ImageHandle handle = m_Context.get_primary_image_handle();

		if (!ppIThumbnail) {
			return E_INVALIDARG;
		}

		std::vector<heif_item_id> thumbIds = handle.get_list_of_thumbnail_IDs();
		if (!thumbIds.size()) {
			return WINCODEC_ERR_FRAMEMISSING;
		}

		heif::ImageHandle thumbHandle = handle.get_thumbnail(thumbIds[0]);

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
		return WINCODEC_ERR_FRAMEMISSING;
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

HRESULT STDMETHODCALLTYPE CHeicBitmapDecoder::GetFrameCount(__RPC__out UINT *pCount)
{
	if (!pCount) {
		return E_INVALIDARG;
	}

	DbgLog("GetFrameCount %d %d",
		m_Context.get_number_of_top_level_images(),
		m_Context.get_list_of_top_level_image_IDs().size());


	*pCount = static_cast<UINT>(m_Context.get_number_of_top_level_images());
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapDecoder::GetFrame(UINT index, __RPC__deref_out_opt IWICBitmapFrameDecode **ppIBitmapFrame)
{
	try
	{
		Log("GetFrame: %d", index);

		if (index >= m_Context.get_number_of_top_level_images()) {
			return WINCODEC_ERR_FRAMEMISSING;
		}

		if (!ppIBitmapFrame) {
			return E_INVALIDARG;
		}

		std::vector<heif_item_id> ids = m_Context.get_list_of_top_level_image_IDs();

		heif::ImageHandle handle = m_Context.get_image_handle(ids[index]);
		if (handle.empty()) {
			return WINCODEC_ERR_FRAMEMISSING;
		}

		DbgLog("GetFrame@handle: hasAlpha=%d, chromaBpp=%d, lumaBpp=%d, width=%d, height=%d, primary=%d",
			handle.has_alpha_channel(),
			handle.get_chroma_bits_per_pixel(),
			handle.get_luma_bits_per_pixel(),
			handle.get_width(),
			handle.get_height(),
			handle.is_primary_image()
		);

		CHeicBitmapFrameDecode* decoder = new(std::nothrow) CHeicBitmapFrameDecode(handle);
		if (!decoder) {
			return E_OUTOFMEMORY;
		}

		*ppIBitmapFrame = decoder;
	}
	catch (const heif::Error& ex)
	{
		Log("GetFrame@Exception: %s", ex.get_message().c_str());
		return E_INVALIDARG;
	}
	catch (const std::exception& ex)
	{
		Log("GetFrame@Exception: %s", ex.what());
		return E_INVALIDARG;
	}
	catch (...)
	{
		Log("GetFrame@Exception: Unknown");
		return E_INVALIDARG;
	}
	return S_OK;
}
