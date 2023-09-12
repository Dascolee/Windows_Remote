#pragma once

#include<iostream>
#include<Windows.h>
using namespace std;




class _CCriticalSection
{
public:
	_CCriticalSection(CRITICAL_SECTION& CriticalSection);
	~_CCriticalSection();
	void Lock();
	void UnLock();

private:
	CRITICAL_SECTION* m_CriticalSection;

};

