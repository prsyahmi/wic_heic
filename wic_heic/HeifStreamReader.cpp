#include "stdafx.h"
#include "wic_heic.h"
#include "HeifStreamReader.h"


CHeifStreamReader::CHeifStreamReader(IStream* stream)
	: m_Stream(stream)
	, m_LastPos(0)
	, m_Size(0)
{
	DbgLog("CHeifStreamReader");
	m_Stream->AddRef();

	STATSTG st;
	if (S_OK == m_Stream->Stat(&st, STATFLAG_NONAME)) {
		m_Size = st.cbSize.QuadPart;
	}
}


CHeifStreamReader::~CHeifStreamReader()
{
	DbgLog("~CHeifStreamReader");
	m_Stream->Release();
}

int64_t CHeifStreamReader::get_position() const
{
	DbgLog("CHeifStreamReader::get_position() -> %zu", m_LastPos);
	return m_LastPos;
}

int CHeifStreamReader::read(void* data, size_t size)
{
	ULARGE_INTEGER uli;
	ULONG nRead;
	HRESULT hr = m_Stream->Read(data, static_cast<ULONG>(size), &nRead);
	if (hr != S_OK) {
		return -1;
	}

	hr = m_Stream->Seek({ 0 }, STREAM_SEEK_CUR, &uli);
	if (hr == S_OK) {
		m_LastPos = static_cast<int64_t>(uli.QuadPart);
	} else {
		m_LastPos += nRead;
	}
	DbgLog("CHeifStreamReader::read(readSize=%zu) -> totalRead=%zu, lastPos=%zu", size, nRead, m_LastPos);

	return 0;
}

int CHeifStreamReader::seek(int64_t position)
{
	LARGE_INTEGER li;
	ULARGE_INTEGER uli;
	li.QuadPart = position;

	if (m_Size && (uint64_t)position > m_Size) {
		return -1;
	}

	HRESULT hr = m_Stream->Seek(li, STREAM_SEEK_SET, &uli);
	DbgLog("CHeifStreamReader::seek(%lld) -> %lld -> hr=%X", position, uli.QuadPart, hr);
	if (hr != S_OK) {
		return -1;
	}

	m_LastPos = static_cast<int64_t>(uli.QuadPart);
	return 0;
}

heif_reader_grow_status CHeifStreamReader::wait_for_file_size(int64_t target_size)
{
	DbgLog("CHeifStreamReader::wait_for_file_size()");
	if (m_Size && (uint64_t)target_size > m_Size) {
		return heif_reader_grow_status_size_beyond_eof;
	}

	return heif_reader_grow_status_size_reached;
}
