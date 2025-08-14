#pragma once

class CComCounter
{
public:
	std::atomic<LONG> m_ObjLock;
	std::atomic<LONG> m_ServerLock;

public:
	CComCounter();
	virtual ~CComCounter();

	static CComCounter& GetInstance() {
		static CComCounter counter;
		return counter;
	}
};

