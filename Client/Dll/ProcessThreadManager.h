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
	//��Ϣ����
	VOID CProcessThreadManager::HandleIO(PBYTE BufferData, ULONG BufferLength);
	//ö���߳���Ϣ
	BOOL CProcessThreadManager::SeEnumThreadInfoByProcessID(HANDLE ProcessID);
	//ɱ���߳�
	VOID CProcessThreadManager::KillThreader(LPBYTE BufferData, UINT BufferLength);
	//�����߳�
	VOID CProcessThreadManager::SuspendThreader(LPBYTE BufferData, UINT BufferLength);
	//�ָ��߳�
	VOID CProcessThreadManager::ResumeThreader(LPBYTE BufferData, UINT BufferLength);


	vector<_THREAD_ITEM_INFORMATION_>m_ThreadItemInfoVector;
};


