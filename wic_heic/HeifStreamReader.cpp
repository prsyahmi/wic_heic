#include "stdafx.h"
#include "wic_heic.h"
#include "HeifStreamReader.h"


CHeifStreamReader::CHeifStreamReader(IStream* stream)
	: m_Stream(stream)
	, m_LastPos(0)
{
	m_Stream->AddRef();
}


CHeifStreamReader::~CHeifStreamReader()
{
	m_Stream->Release();
}

int64_t CHeifStreamReader::get_position() const
{
	return m_LastPos;
}

int CHeifStreamReader::read(void* data, size_t size)
{
	ULARGE_INTEGER uli;
	ULONG nRead;
	HRESULT hr = m_Stream->Read(data, static_cast<ULONG>(size), &nRead);
	if (FAILED(hr)) {
		return 1;
	}

	hr = m_Stream->Seek({ 0 }, STREAM_SEEK_CUR, &uli);
	if (SUCCEEDED(hr)) {
		m_LastPos = static_cast<int64_t>(uli.QuadPart);
	} else {
		m_LastPos += nRead;
	}

	return 0;
}

int CHeifStreamReader::seek(int64_t position)
{
	LARGE_INTEGER li;
	ULARGE_INTEGER uli;
	li.QuadPart = position;

	HRESULT hr = m_Stream->Seek(li, STREAM_SEEK_SET, &uli);
	if (FAILED(hr)) {
		return 1;
	}

	m_LastPos = static_cast<int64_t>(uli.QuadPart);
	return 0;
}

heif_reader_grow_status CHeifStreamReader::wait_for_file_size(int64_t target_size)
{
	return heif_reader_grow_status_size_reached;
}
