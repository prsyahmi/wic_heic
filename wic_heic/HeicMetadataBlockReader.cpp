#include "stdafx.h"
#include "wic_heic.h"
#include "HeicMetadataBlockReader.h"
#include <propvarutil.h>
#pragma comment(lib, "Propsys.lib")

CHeicMetadataBlockReader::CHeicMetadataBlockReader(heif::ImageHandle handle)
	: m_Handle(handle)
{
	m_MetadataBlock = m_Handle.get_list_of_metadata_block_IDs();
}

CHeicMetadataBlockReader::~CHeicMetadataBlockReader()
{
}

ULONG STDMETHODCALLTYPE CHeicMetadataBlockReader::AddRef(void)
{
	return ++m_Count;
}

ULONG STDMETHODCALLTYPE CHeicMetadataBlockReader::Release(void)
{
	uint32_t n = --m_Count;
	if (n == 0) {
		delete this;
	}

	return n;
}

HRESULT STDMETHODCALLTYPE CHeicMetadataBlockReader::QueryInterface(REFIID riid, void** ppvObject)
{
	HRESULT hr = S_OK;

	if (!ppvObject) {
		return E_POINTER;
	}

	*ppvObject = nullptr;
	if (IsEqualGUID(riid, IID_IUnknown)) {
		*ppvObject = static_cast<IUnknown*>(this);
	}
	else if (IsEqualGUID(riid, IID_IWICMetadataBlockReader)) {
		*ppvObject = static_cast<IWICMetadataBlockReader*>(this);
	}
	else {
		hr = E_NOINTERFACE;
	}

	if (SUCCEEDED(hr)) {
		AddRef();
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE CHeicMetadataBlockReader::GetContainerFormat(__RPC__out GUID* pguidContainerFormat)
{
	DbgLog("%s", __FUNCTION__);

	if (!pguidContainerFormat) {
		return E_POINTER;
	}

	*pguidContainerFormat = GUID_ContainerFormatHEIC;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicMetadataBlockReader::GetCount(__RPC__out UINT* pcCount)
{
	DbgLog("%s", __FUNCTION__);

	if (!pcCount) {
		return E_POINTER;
	}

	*pcCount = (UINT)m_MetadataBlock.size();
	DbgLog("%s -> %u", __FUNCTION__, *pcCount);

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CHeicMetadataBlockReader::GetReaderByIndex(UINT nIndex, __RPC__deref_out_opt IWICMetadataReader** ppIMetadataReader)
{
	DbgLog("%s (nIndex=%u, ppIMetadataReader=%p)", __FUNCTION__, nIndex, ppIMetadataReader);

	if (!ppIMetadataReader) {
		return E_POINTER;
	}

	*ppIMetadataReader = nullptr;

	if (nIndex >= m_MetadataBlock.size()) {
		return E_INVALIDARG;
	}

	std::vector<uint8_t> buffer = m_Handle.get_metadata(m_MetadataBlock[nIndex]);
	std::string type = m_Handle.get_metadata_type(m_MetadataBlock[nIndex]);
	size_t payloadOffset = 0;
	GUID metadataType = GUID_MetadataFormatXMP;

	if (type == "Exif") {
		payloadOffset = 4;
		metadataType = GUID_MetadataFormatApp1;
	}

	IStream* pStream = SHCreateMemStream(buffer.data() + payloadOffset, static_cast<UINT>(buffer.size() - payloadOffset));
	if (!pStream) {
		return E_OUTOFMEMORY;
	}

	HRESULT hr = S_OK;
	IWICComponentFactory* pFactory = nullptr;

	hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));

	if (!SUCCEEDED(hr)) {
		pStream->Release();
		return hr;
	}

	hr = pFactory->CreateMetadataReader(
		metadataType,
		nullptr,
		0,
		pStream,
		ppIMetadataReader
	);

	DbgLog("%s -> hr=0x%08X, *ppIMetadataReader=%p, totalBlock=%d, payloadSize=%d, payloadOffset=%d", __FUNCTION__,
		hr, *ppIMetadataReader, (int)m_MetadataBlock.size(), (int)buffer.size(), (int)payloadOffset);

	pStream->Release();

	return hr;
}

HRESULT STDMETHODCALLTYPE CHeicMetadataBlockReader::GetEnumerator(__RPC__deref_out_opt IEnumUnknown** ppIEnumMetadata)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CHeicMetadataBlockReader::CreateQueryReader(IWICMetadataQueryReader** ppIQueryReader)
{
	HRESULT hr = S_OK;
	IWICComponentFactory* pFactory = nullptr;

	if (!ppIQueryReader) {
		return E_POINTER;
	}

	hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));

	if (!SUCCEEDED(hr)) {
		return hr;
	}

	// Create the individual Exif block reader
	hr = pFactory->CreateQueryReaderFromBlockReader(
		this,
		ppIQueryReader
	);

	pFactory->Release();

	return hr;
}
