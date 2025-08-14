#pragma once

class CHeifStreamWriter : public heif::Context::Writer
{
private:
	IStream* m_Stream;

public:
	CHeifStreamWriter(IStream* stream);
	virtual ~CHeifStreamWriter();

	heif_error write(const void* data, size_t size) override;
};

