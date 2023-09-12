#include "Manager.h"





CManager::CManager(_CIOCPClient * IOCPClient)
{
	m_IOCPClient = IOCPClient;
	m_IOCPClient->setManagerCallBack(this);

	m_EventOpenDialogHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
}


CManager::~CManager()
{

}

VOID CManager::WaittingForDialogOpen()
{
	WaitForSingleObject(m_EventOpenDialogHandle, INFINITE);
	//必须的Sleep,因为远程窗口从InitDialog中发送COMMAND_NEXT到显示还要一段时间
	Sleep(150);
}

VOID CManager::NotifyDialogIsOpen()
{
	SetEvent(m_EventOpenDialogHandle);
}


BOOL CManager::EnableSeDebugPrivilege(HANDLE ProcessHandle, BOOL IsEnable, LPCTSTR RequireLevel)
{
	DWORD  LastError;
	HANDLE TokenHandle = 0;

	if (!OpenProcessToken(ProcessHandle, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &TokenHandle))
	{
		LastError = GetLastError();
		if (TokenHandle)
			CloseHandle(TokenHandle);
		return LastError;
	}
	TOKEN_PRIVILEGES TokenPrivileges;
	memset(&TokenPrivileges, 0, sizeof(TOKEN_PRIVILEGES));
	LUID v1;
	if (!LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &v1))
	{
		LastError = GetLastError();
		CloseHandle(TokenHandle);
		return LastError;
	}
	TokenPrivileges.PrivilegeCount = 1;
	TokenPrivileges.Privileges[0].Luid = v1;
	if (IsEnable)
		TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		TokenPrivileges.Privileges[0].Attributes = 0;
	AdjustTokenPrivileges(TokenHandle, FALSE, &TokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	LastError = GetLastError();
	CloseHandle(TokenHandle);
	return LastError;
}