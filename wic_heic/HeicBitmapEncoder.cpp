#include "stdafx.h"
#include "wic_heic.h"
#include "HeicBitmapFrameEncode.h"
#include "HeicBitmapEncoder.h"
#include "HeifStreamWriter.h"


CHeicBitmapEncoder::CHeicBitmapEncoder()
	: m_Count(1)
	, m_Stream(nullptr)
{
}


CHeicBitmapEncoder::~CHeicBitmapEncoder()
{
	for (auto it = m_Frames.begin(); it != m_Frames.end(); ++it) {
		(*it)->Release();
	}

	if (m_Stream) {
		m_Stream->Release();
	}
}

HRESULT STDMETHODCALLTYPE CHeicBitmapEncoder::QueryInterface(REFIID riid, void **ppvObject)
{
	HRESULT hr = S_OK;

	if (!ppvObject) {
		return E_INVALIDARG;
	}

	*ppvObject = nullptr;
	if (IsEqualGUID(riid, IID_IUnknown)) {
		*ppvObject = static_cast<IUnknown*>(this);
	} else if (IsEqualGUID(riid, IID_IWICBitmapEncoder)) {
		*ppvObject = static_cast<IWICBitmapEncoder*>(this);
	} else {
		hr = E_NOINTERFACE;
	}

	if (SUCCEEDED(hr)) {
		AddRef();
	}

	return hr;
}

ULONG STDMETHODCALLTYPE CHeicBitmapEncoder::AddRef(void)
{
	return ++m_Count;
}

ULONG STDMETHODCALLTYPE CHeicBitmapEncoder::Release(void)
{
	uint32_t n = --m_Count;
	if (m_Count == 0) {
		delete this;
	}

	return n;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapEncoder::Initialize(__RPC__in_opt IStream *pIStream, WICBitmapEncoderCacheOption cacheOption)
{
	m_Stream = pIStream;
	m_Stream->AddRef();

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapEncoder::GetContainerFormat(__RPC__out GUID *pguidContainerFormat)
{
	if (!pguidContainerFormat) {
		return E_INVALIDARG;
	}

	*pguidContainerFormat = GUID_ContainerFormatHEIC;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapEncoder::GetEncoderInfo(__RPC__deref_out_opt IWICBitmapEncoderInfo **ppIEncoderInfo)
{
	return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapEncoder::SetColorContexts(UINT cCount, __RPC__in_ecount_full(cCount) IWICColorContext **ppIColorContext)
{
	return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapEncoder::SetPalette(__RPC__in_opt IWICPalette *pIPalette)
{
	return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapEncoder::SetThumbnail(__RPC__in_opt IWICBitmapSource *pIThumbnail)
{
	return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapEncoder::SetPreview(__RPC__in_opt IWICBitmapSource *pIPreview)
{
	return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapEncoder::CreateNewFrame(__RPC__deref_out_opt IWICBitmapFrameEncode **ppIFrameEncode, __RPC__deref_opt_inout_opt IPropertyBag2 **ppIEncoderOptions)
{
	try
	{
		if (!ppIFrameEncode) {
			return E_INVALIDARG;
		}

		CHeicBitmapFrameEncode* pFrame = new CHeicBitmapFrameEncode();
		*ppIFrameEncode = pFrame;

		pFrame->AddRef();
		m_Frames.push_back(pFrame);
	}
	catch (const heif::Error& ex)
	{
		Log("Call to CHeicBitmapEncoder::CreateNewFrame failed: %s", ex.get_message().c_str());
		return WINCODEC_ERR_GENERIC_ERROR;
	}
	catch (const std::exception& ex)
	{
		Log("Call to CHeicBitmapEncoder::CreateNewFrame failed: %s", ex.what());
		return WINCODEC_ERR_GENERIC_ERROR;
	}
	catch (...)
	{
		Log("Call to CHeicBitmapEncoder::CreateNewFrame failed: Unknown exception");
		return WINCODEC_ERR_GENERIC_ERROR;
	}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapEncoder::Commit(void)
{
	try
	{
		heif::Encoder encoder(heif_compression_HEVC);
		CHeifStreamWriter writer(m_Stream);

		if (!m_Stream) {
			return WINCODEC_ERR_NOTINITIALIZED;
		}

		for (auto it = m_Frames.begin(); it != m_Frames.end(); ++it) {
			m_Context.encode_image((*it)->m_Frame, encoder);
		}

		m_Context.write(writer);
	}
	catch (const heif::Error& ex)
	{
		Log("Call to CHeicBitmapEncoder::Commit failed: %s", ex.get_message().c_str());
		return WINCODEC_ERR_INVALIDPARAMETER;
	}
	catch (const std::exception& ex)
	{
		Log("Call to CHeicBitmapEncoder::Commit failed: %s", ex.what());
		return WINCODEC_ERR_GENERIC_ERROR;
	}
	catch (...)
	{
		Log("Call to CHeicBitmapEncoder::Commit failed: Unknown exception");
		return WINCODEC_ERR_GENERIC_ERROR;
	}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicBitmapEncoder::GetMetadataQueryWriter(__RPC__deref_out_opt IWICMetadataQueryWriter **ppIMetadataQueryWriter)
{
	return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}
