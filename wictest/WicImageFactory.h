#pragma once

class CWicImageFactory
{
private:
	static std::shared_ptr<CWicImageFactory> m_pInstance;
	CWicImageFactory();

public:
	~CWicImageFactory();


};

