#pragma once
#include "Manager.h"
#include"SeEnumThreadInfoByNtQuerySystemInformation.h"


class CProcessThreadManager :
	public CManager
{
public:
	CProcessThreadManager(_CIOCPClient * IOCPClient);
	~CProcessThreadManager();
	BOOL CProcessThreadManager::SendClientProcessThreaderList();
	//消息处理
	VOID CProcessThreadManager::HandleIO(PBYTE BufferData, ULONG BufferLength);
	//枚举线程信息
	BOOL CProcessThreadManager::SeEnumThreadInfoByProcessID(HANDLE ProcessID);
	//杀死线程
	VOID CProcessThreadManager::KillThreader(LPBYTE BufferData, UINT BufferLength);
	//挂起线程
	VOID CProcessThreadManager::SuspendThreader(LPBYTE BufferData, UINT BufferLength);
	//恢复线程
	VOID CProcessThreadManager::ResumeThreader(LPBYTE BufferData, UINT BufferLength);


	vector<_THREAD_ITEM_INFORMATION_>m_ThreadItemInfoVector;
};


