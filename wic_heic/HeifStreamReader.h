#pragma once

class CHeifStreamReader : public heif::Context::Reader
{
private:
	IStream* m_Stream;
	int64_t m_LastPos;
	uint64_t m_Size;

public:
	CHeifStreamReader(IStream* stream);
	~CHeifStreamReader();

	int64_t get_position() const override;
	int read(void* data, size_t size) override;
	int seek(int64_t position) override;
	heif_reader_grow_status wait_for_file_size(int64_t target_size) override;
};

