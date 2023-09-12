#include"stdafx.h"
#include "_CriticalSection.h"

_CCriticalSection::_CCriticalSection(CRITICAL_SECTION& CriticalSection)
{
	m_CriticalSection = &CriticalSection;
	Lock();
}

_CCriticalSection::~_CCriticalSection()
{
	UnLock();
}

void _CCriticalSection::Lock()
{
	EnterCriticalSection(m_CriticalSection);
}
void _CCriticalSection::UnLock()
{
	LeaveCriticalSection(m_CriticalSection);

}