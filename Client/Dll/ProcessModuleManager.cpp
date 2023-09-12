#include "ProcessModuleManager.h"



CProcessModuleManager::CProcessModuleManager(_CIOCPClient * IOCPClient) : CManager(IOCPClient)
{
	printf("CProcessModuleManager()\r\n");
}


CProcessModuleManager::~CProcessModuleManager()
{
	printf("~CProcessModuleManager()\r\n");
}

BOOL CProcessModuleManager::SendClientProcessModuleList()
{
	BOOL IsOk = FALSE;
	char* BufferData = NULL;

	DWORD Offset = 1;
	DWORD v1 = 0;
	vector<MODULE_INFORMATION>::iterator i;

	//将vector中的数据写进BufferData
	BufferData = (char*)LocalAlloc(LPTR, 0x10);
	if (BufferData == NULL)
	{
		return IsOk;
	}
	//数据包的第一个字节就是数据包类型
	BufferData[0] = CLIENT_PROCESS_MANAGER_ENUM_MODULE_REPLY;
	for (i = m_ModuleItemInfoVector.begin(); i != m_ModuleItemInfoVector.end(); i++)
	{
		v1 = sizeof(MODULEINFO) + i->ModuleName.length()+1;
		//缓冲区太小，再重新分配下

		if (LocalSize(BufferData)<(Offset + v1))
		{
			BufferData = (char*)LocalReAlloc(BufferData, (Offset + v1),
				LMEM_ZEROINIT | GMEM_MOVEABLE);
		}

		memcpy(BufferData + Offset, &(i->ModuleInfo.lpBaseOfDll), sizeof(LPVOID));
		Offset += sizeof(LPVOID);

		memcpy(BufferData + Offset, &(i->ModuleInfo.SizeOfImage), sizeof(DWORD));
		Offset += sizeof(DWORD);

		memcpy(BufferData + Offset, &(i->ModuleInfo.EntryPoint), sizeof(LPVOID));
		Offset += sizeof(LPVOID);

		strcpy(BufferData + Offset, i->ModuleName.c_str());
		Offset += i->ModuleName.length()+1;
	}

	m_IOCPClient->OnSending(BufferData, LocalSize(BufferData));
	IsOk = TRUE;
	if (BufferData != NULL)
	{
		LocalFree(BufferData);
		BufferData = NULL;
	}
	return IsOk;
}

BOOL CProcessModuleManager::EnumModuleInfoByProcessID(HANDLE ProcessID)
{
	if (EnumMoudleByPsapiEx(ProcessID, m_ModuleItemInfoVector) == FALSE)
	{
		return FALSE;
	}
	return (m_ModuleItemInfoVector.size() > 0);
}

BOOL  CProcessModuleManager::EnumMoudleByPsapiEx(HANDLE ProcessID, vector<MODULE_INFORMATION>& ModuleInfo)
{
	HANDLE  ProcessHandle = 0;
	BOOL    IsOk = FALSE;

	ProcessHandle = ModOpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, ProcessID);
	if (ProcessHandle)
	{
		IsOk = EnumMoudleByPsapi(ProcessHandle, ModuleInfo);
		ModCloseHandle(ProcessHandle);
		return IsOk;
	}
	return FALSE;
}
BOOL CProcessModuleManager::EnumMoudleByPsapi(HANDLE ProcessHandle, vector<MODULE_INFORMATION>& ModuleInfo)
{
	int i;
	DWORD ReturnLength = NULL;
	MODULE_INFORMATION v1 = { 0 };

	HMODULE ModulesHandle[1024] = { 0 };

	if (EnumProcessModules(ProcessHandle, ModulesHandle, sizeof(ModulesHandle), &ReturnLength))
	{
		for (i = 0; i < (int)(ReturnLength / sizeof(HMODULE)); i++)
		{
			if (GetModuleInformation(ProcessHandle, (HMODULE)ModulesHandle[i], &v1.ModuleInfo, sizeof(MODULEINFO)))
			{
				char ModuleName[MAX_PATH] = { 0 };
				if (!GetModuleBaseNameA(ProcessHandle, (HMODULE)ModulesHandle[i], ModuleName, MAX_PATH))
				{
					v1.ModuleName = "Unknow模块";
				}
				else
				{
					v1.ModuleName = ModuleName;
				}

				ModuleInfo.push_back(v1);
			}

		}
	}

	if (ModuleInfo.size() == 0)
	{
		return FALSE;
	}
	return TRUE;
}
BOOL  ModEnableSeDebugPrivilege(HANDLE ProcessHandle, BOOL IsEnable)
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
	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &v1))
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
HANDLE  ModOpenProcess(DWORD DesiredAccess, BOOL IsInheritHandle, HANDLE ProcessID)
{
	ModEnableSeDebugPrivilege(GetCurrentProcess(), TRUE);
	HANDLE ProcessHandle = OpenProcess(DesiredAccess, IsInheritHandle, (DWORD)ProcessID);
	DWORD LastError = GetLastError();
	ModEnableSeDebugPrivilege(GetCurrentProcess(), FALSE);
	SetLastError(LastError);
	return ProcessHandle;
}
BOOL ModCloseHandle(HANDLE HandleValue)
{
	DWORD HandleFlags;
	if (GetHandleInformation(HandleValue, &HandleFlags)
		&& (HandleFlags & HANDLE_FLAG_PROTECT_FROM_CLOSE) != HANDLE_FLAG_PROTECT_FROM_CLOSE)
		return !!CloseHandle(HandleValue);
	return FALSE;
}