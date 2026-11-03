#include "stdafx.h"
#include "wic_heic.h"
#include "HeifStreamReader.h"

CHeifStreamReader::CHeifStreamReader(IStream* stream)
    : m_Stream(stream), m_LastPos(0), m_Size(0)
{
    if (!stream) throw std::invalid_argument("Null stream");
    STATSTG st = {};
    if (FAILED(stream->Stat(&st, STATFLAG_NONAME)) || st.cbSize.QuadPart == 0 ||
        st.cbSize.QuadPart > 512ULL * 1024 * 1024) {
        throw std::invalid_argument("Invalid or oversized HEIC stream");
    }
    ULARGE_INTEGER pos = {};
    if (FAILED(stream->Seek({}, STREAM_SEEK_SET, &pos)) || pos.QuadPart != 0)
        throw std::invalid_argument("HEIC stream is not seekable");
    m_Size = st.cbSize.QuadPart;
    stream->AddRef();
}

CHeifStreamReader::~CHeifStreamReader() { m_Stream->Release(); }
int64_t CHeifStreamReader::get_position() const { return m_LastPos; }

int CHeifStreamReader::read(void* data, size_t size)
{
    if ((!data && size) || size > ULONG_MAX || m_LastPos < 0 ||
        uint64_t(m_LastPos) > m_Size || size > m_Size - uint64_t(m_LastPos)) return -1;
    ULONG actual = 0;
    HRESULT hr = m_Stream->Read(data, static_cast<ULONG>(size), &actual);
    m_LastPos += actual;
    return SUCCEEDED(hr) && actual == size ? 0 : -1;
}

int CHeifStreamReader::seek(int64_t position)
{
    if (position < 0 || uint64_t(position) > m_Size) return -1;
    LARGE_INTEGER target = {}; target.QuadPart = position;
    ULARGE_INTEGER actual = {};
    if (FAILED(m_Stream->Seek(target, STREAM_SEEK_SET, &actual)) || actual.QuadPart != uint64_t(position)) return -1;
    m_LastPos = position;
    return 0;
}

heif_reader_grow_status CHeifStreamReader::wait_for_file_size(int64_t target_size)
{
    return target_size < 0 || uint64_t(target_size) > m_Size
        ? heif_reader_grow_status_size_beyond_eof : heif_reader_grow_status_size_reached;
}
