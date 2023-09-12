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

	//通讯类的引擎指针
	_CIOCPClient* m_IOCPClient;
	HANDLE m_EventOpenDialogHandle;
};

