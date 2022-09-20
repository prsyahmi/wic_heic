#include "stdafx.h"
#include "HeifStreamWriter.h"


CHeifStreamWriter::CHeifStreamWriter(IStream* stream)
	: m_Stream(stream)
{
	m_Stream->AddRef();
}


CHeifStreamWriter::~CHeifStreamWriter()
{
	m_Stream->Release();
}

heif_error CHeifStreamWriter::write(const void* data, size_t size)
{
	HRESULT hr = m_Stream->Write(data, size, nullptr);
	if (FAILED(hr)) {
		return{
			heif_error_Encoding_error,
			heif_suberror_Cannot_write_output_data,
			"Unable to write"
		};
	}

	return{
		heif_error_Ok,
		heif_suberror_Unspecified,
		""
	};
}
