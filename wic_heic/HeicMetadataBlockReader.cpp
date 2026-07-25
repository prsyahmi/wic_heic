#include "stdafx.h"
#include "wic_heic.h"
#include "HeicMetadataBlockReader.h"


class CEnumUnknown : public IEnumUnknown
{
private:
    explicit CEnumUnknown(std::vector<IUnknown*> items)
        : m_refCount(1)
        , m_items(std::move(items))
        , m_index(0)
    {
    }

    ~CEnumUnknown()
    {
        for (IUnknown* p : m_items)
        {
            if (p) {
                p->Release();
            }
        }
    }

    ULONG m_refCount;
    std::vector<IUnknown*> m_items;
    size_t m_index;

public:
    // Takes ownership of the references already held in items
    // (i.e. caller should NOT Release() them after passing them in).
    static HRESULT CreateInstance(std::vector<IUnknown*> items, IEnumUnknown** ppEnum)
    {
        if (!ppEnum)
            return E_INVALIDARG;

        *ppEnum = new (std::nothrow) CEnumUnknown(std::move(items));
        return *ppEnum ? S_OK : E_OUTOFMEMORY;
    }

    // --- IUnknown ---

    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override
    {
        if (!ppv)
            return E_POINTER;

        if (riid == IID_IUnknown || riid == IID_IEnumUnknown)
        {
            *ppv = static_cast<IEnumUnknown*>(this);
            AddRef();
            return S_OK;
        }

        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef() override
    {
        return InterlockedIncrement(&m_refCount);
    }

    STDMETHODIMP_(ULONG) Release() override
    {
        ULONG c = InterlockedDecrement(&m_refCount);
        if (c == 0)
            delete this;
        return c;
    }

    // --- IEnumUnknown ---

    STDMETHODIMP Next(ULONG celt, IUnknown** rgelt, ULONG* pceltFetched) override
    {
        if (!rgelt)
            return E_POINTER;

        ULONG fetched = 0;
        while (fetched < celt && m_index < m_items.size())
        {
            IUnknown* pItem = m_items[m_index];
            pItem->AddRef();
            rgelt[fetched] = pItem;
            ++m_index;
            ++fetched;
        }

        if (pceltFetched)
            *pceltFetched = fetched;

        for (ULONG i = fetched; i < celt; ++i)
            rgelt[i] = nullptr;

        return (fetched == celt) ? S_OK : S_FALSE;
    }

    STDMETHODIMP Skip(ULONG celt) override
    {
        size_t remaining = m_items.size() - m_index;
        if (celt > remaining)
        {
            m_index = m_items.size();
            return S_FALSE;
        }
        m_index += celt;
        return S_OK;
    }

    STDMETHODIMP Reset() override
    {
        m_index = 0;
        return S_OK;
    }

    STDMETHODIMP Clone(IEnumUnknown** ppEnum) override
    {
        if (!ppEnum)
            return E_POINTER;

        // Clone shares the same underlying objects; AddRef each for the new list.
        std::vector<IUnknown*> cloned;
        cloned.reserve(m_items.size());
        for (IUnknown* p : m_items)
        {
            p->AddRef();
            cloned.push_back(p);
        }

        HRESULT hr = CreateInstance(std::move(cloned), ppEnum);
        if (SUCCEEDED(hr))
        {
            static_cast<CEnumUnknown*>(*ppEnum)->m_index = m_index;
        }
        return hr;
    }
};

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

    *pguidContainerFormat = GUID_ContainerFormatJpeg;
	//*pguidContainerFormat = GUID_ContainerFormatHEIC;
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
        WICPersistOptionDefault,
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
    // This section is made by claude
    DbgLog("%s (ppIMetadataReader=%p)", __FUNCTION__, ppIEnumMetadata);

    if (!ppIEnumMetadata) {
        return E_POINTER;
    }

    *ppIEnumMetadata = nullptr;

    UINT cCount = 0;
    HRESULT hr = GetCount(&cCount);
    if (FAILED(hr)) {
        return hr;
    }

    std::vector<IUnknown*> items;
    items.reserve(cCount);

    for (UINT i = 0; i < cCount; ++i)
    {
        IWICMetadataReader* pReader = nullptr;
        hr = GetReaderByIndex(i, &pReader);
        if (FAILED(hr))
        {
            // Release anything already collected before bailing out.
            for (IUnknown* p : items) {
                p->Release();
            }
            return hr;
        }

        // pReader already holds the one reference GetReaderByIndex returned;
        // CEnumUnknown takes ownership of it, so no extra AddRef here.
        items.push_back(pReader);
    }

    return CEnumUnknown::CreateInstance(std::move(items), ppIEnumMetadata);
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
