#pragma once

#include "_IOCPClient.h"

class _CIOCPClient;
class CManager
{
public:
	CManager(_CIOCPClient* IOCPClient);
	~CManager();
	VOID CManager::WaittingForDialogOpen();
	VOID CManager::NotifyDialogIsOpen();

	virtual VOID CManager::HandleIO(PBYTE BufferData, ULONG BufferLength) {};
	BOOL CManager::EnableSeDebugPrivilege(HANDLE ProcessHandle, BOOL IsEnable, LPCTSTR RequireLevel);

	//ͨѶ�������ָ��
	_CIOCPClient* m_IOCPClient;
	HANDLE m_EventOpenDialogHandle;
};

