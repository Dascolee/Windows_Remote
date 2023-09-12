#include "ProcessHandleManager.h"

LPFN_NTQUERYSYSTEMINFORMATION   __myNtQuerySystemInformation = NULL;
LPFN_NTQUERYOBJECT              __NtQueryObject = NULL;


CProcessHandleManager::CProcessHandleManager(_CIOCPClient * IOCPClient) : CManager(IOCPClient)
{
	printf("CProcessHandleManager()\r\n");


}

CProcessHandleManager::~CProcessHandleManager()
{
	printf("CProcessHandleManager()\r\n");
}

BOOL CProcessHandleManager::SendClientProcessHandleList()
{

	BOOL IsOk = FALSE;
	char* BufferData = NULL;

	DWORD Offset = 1;
	DWORD v1 = 0;
	vector<HANDLE_ITEM_INFORMATION>::iterator i;

	//将vector中的数据写进BufferData
	BufferData = (char*)LocalAlloc(LPTR, 0x10);
	if (BufferData == NULL)
	{
		return IsOk;
	}
	//数据包的第一个字节就是数据包类型
	BufferData[0] = CLIENT_PROCESS_MANAGER_ENUM_HANDLE_REPLY;
	for (i = m_HandleItemInfoVector.begin(); i != m_HandleItemInfoVector.end(); i++)
	{
		v1 = sizeof(ULONG_PTR) + sizeof(PVOID)+ i->HandleName.length() + i->HandleType.length()+2;
		//缓冲区太小，再重新分配下
		if (LocalSize(BufferData)<(Offset + v1))
		{
			BufferData = (char*)LocalReAlloc(BufferData, (Offset + v1),
				LMEM_ZEROINIT | GMEM_MOVEABLE);
		}

		memcpy(BufferData + Offset, &(i->HandleValue), sizeof(ULONG_PTR));
		Offset += sizeof(ULONG_PTR);

		memcpy(BufferData + Offset, &(i->Object), sizeof(PVOID));
		Offset += sizeof(PVOID);

		strcpy(BufferData + Offset, i->HandleName.c_str());
		Offset += i->HandleName.length()+1;

		strcpy(BufferData + Offset, i->HandleType.c_str());
		Offset += i->HandleType.length()+1 ;
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


BOOL CProcessHandleManager::EnumHandleInfoByProcessID(HANDLE ProcessID)
{
	if (SeEnumHandleInfoByNtQuerySystemInformation(ProcessID, m_HandleItemInfoVector) == FALSE)
	{
		return FALSE;
	}
	return (m_HandleItemInfoVector.size() > 0);
}

































BOOL SeEnumHandleInfoByNtQuerySystemInformation(HANDLE ProcessID,
	vector<HANDLE_ITEM_INFORMATION>& HandleItemInfoVector)
{
	
	HMODULE NtdllModuleBase = NULL;
	NtdllModuleBase = GetModuleHandle("Ntdll.dll");
	if (NtdllModuleBase == NULL)
	{
		return FALSE;
	}
	__myNtQuerySystemInformation = (LPFN_NTQUERYSYSTEMINFORMATION)GetProcAddress(NtdllModuleBase,
		"NtQuerySystemInformation");
	__NtQueryObject = (LPFN_NTQUERYOBJECT)GetProcAddress(NtdllModuleBase, "NtQueryObject");
	if (__myNtQuerySystemInformation == NULL || __NtQueryObject == NULL)
	{
		return FALSE;
	}

	if (ProcessID == NULL)
	{
		return FALSE;
	}
	HANDLE_ITEM_INFORMATION HandleItemInfo;
	ULONG ReturnLength = 0;
	ULONG BufferLength = 1;
	PSYSTEM_HANDLES_INFORMATION SystemHandlesInfo = (PSYSTEM_HANDLES_INFORMATION)malloc(BufferLength);
	PSYSTEM_HANDLE_INFORMATION  v1 = NULL;
	NTSTATUS Status;
	while (1)
	{
		Status = __myNtQuerySystemInformation(SystemHandleInformation, SystemHandlesInfo,
			BufferLength, &ReturnLength);

		if (Status != STATUS_SUCCESS)
		{
			if (Status == STATUS_INFO_LENGTH_MISMATCH)
			{
				free(SystemHandlesInfo);
				BufferLength = ReturnLength;

				SystemHandlesInfo = (PSYSTEM_HANDLES_INFORMATION)malloc(BufferLength);
				if (!SystemHandlesInfo)
				{
					return FALSE;
				}
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			break;
		}
	}

	v1 = SystemHandlesInfo->SystemHandleInfo;
	//v1 = (PSYSTEM_HANDLE_INFORMATION)((ULONG_PTR)SystemHandlesInfo + 4);	
	LONG NumberOfHandles = SystemHandlesInfo->NumberOfHandles;
	while (NumberOfHandles > NULL)
	{
		if (v1->UniqueProcessID == (USHORT)ProcessID)
		{
			HandleItemInfo.HandleValue = v1->HandleValue;
			HandleItemInfo.Object = v1->Object;
			if (v1->GrantedAccess != 0x0012019F)
			{
				char* v5 = NULL;
				v5 = (char*)SeGetHandleNameByNtQueryObject(ProcessID, (HANDLE)v1->HandleValue, TRUE);
				if (v5 != NULL)
				{
					HandleItemInfo.HandleName = v5;
					VirtualFree(v5, NULL, MEM_RELEASE);
				}
				v5 = (char*)SeGetHandleTypeByNtQueryObject(ProcessID, (HANDLE)v1->HandleValue);
				if (v5 != NULL)
				{
					HandleItemInfo.HandleType = v5;
					VirtualFree(v5, NULL, MEM_RELEASE);
				}

			}
			HandleItemInfoVector.push_back(HandleItemInfo);
		}
		v1 = (PSYSTEM_HANDLE_INFORMATION)((ULONG_PTR)v1 + sizeof SYSTEM_HANDLE_INFORMATION);
		NumberOfHandles--;
	}
	free(SystemHandlesInfo);
	return (HandleItemInfoVector.size() > 0);
}

void* SeGetHandleNameByNtQueryObject(HANDLE ProcessID, HANDLE HandleValue, BOOL IsTranslateName)
{

	wchar_t* v1 = (wchar_t*)SeGetHandleNameByNtQueryObjectW(
		ProcessID, HandleValue, IsTranslateName);

	if (v1)
	{
		LPVOID HandleName = VirtualAlloc(NULL,
			wcslen(v1) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		WideCharToMultiByte(CP_ACP, NULL, v1, -1, (LPSTR)HandleName, (int)wcslen(v1) + 1, NULL, NULL);
		VirtualFree(v1, NULL, MEM_RELEASE);
		return HandleName;
	}
	return NULL;
}

void* SeGetHandleNameByNtQueryObjectW(HANDLE ProcessID, HANDLE HandleValue, BOOL IsTranslateName)
{

	ULONG  ReturnLength = NULL;
	HANDLE ProcessHandle = NULL;
	HANDLE v1 = NULL;
	BOOL  IsOk = FALSE;
	char v3[0x1000] = { 0 };
	POBJECT_NAME_INFORMATION ObjectNameInfo = (POBJECT_NAME_INFORMATION)v3;
	LPVOID HandleName = VirtualAlloc(NULL, 0x1000,
		MEM_COMMIT, PAGE_READWRITE);

	if (HandleName == NULL)
	{
		return NULL;
	}

	ZeroMemory(HandleName, 0x1000);
	if (ProcessID != NULL)
	{
		ProcessHandle = HanOpenProcess(PROCESS_DUP_HANDLE, FALSE, (HANDLE)ProcessID);


		if (ProcessHandle != NULL)
		{
			DuplicateHandle(ProcessHandle, HandleValue, GetCurrentProcess(),
				&v1, NULL, FALSE, DUPLICATE_SAME_ACCESS);
			if (v1 != NULL)
			{

				__NtQueryObject(v1,
					ObjectNameInformation, ObjectNameInfo, 0x1000, &ReturnLength);//没得到

				if (ObjectNameInfo->Name.Length != NULL)
				{
					wcscpy((wchar_t*)HandleName, ObjectNameInfo->Name.Buffer);

					IsOk = TRUE;
					if (IsTranslateName)
					{
						LPVOID v1 = HanDosPathToNtPathW((wchar_t*)HandleName);
						if (v1 != NULL)
						{
							VirtualFree(HandleName, NULL, MEM_RELEASE);
							HandleName = v1;
						}
					}
				}
			}
		}
	}
	if (v1 != NULL)
	{
		HanCloseHandle(v1);
		v1 = NULL;
	}
	if (ProcessHandle != NULL)
	{
		HanCloseHandle(ProcessHandle);
		ProcessHandle = NULL;
	}
	if (!IsOk)
	{
		VirtualFree(HandleName, NULL, MEM_RELEASE);
		return(NULL);
	}
	else
	{
		return(HandleName);
	}
}

void* SeGetHandleTypeByNtQueryObject(HANDLE ProcessID, HANDLE HandleValue)
{

	HANDLE ProcessHandle = NULL;
	ULONG ReturnLength = NULL;
	HANDLE v1 = NULL;
	BOOL  IsOk = FALSE;
	char v3[0x1000] = { 0 };
	LPVOID HandleType = VirtualAlloc(NULL, 0x1000,
		MEM_COMMIT, PAGE_READWRITE);

	if (HandleType == NULL)
	{
		return NULL;
	}

	ZeroMemory(HandleType, 0x1000);
	POBJECT_TYPE_INFORMATION ObjectTypeInfo = (POBJECT_TYPE_INFORMATION)v3;

	if (ProcessID != NULL)
	{
		ProcessHandle = HanOpenProcess(PROCESS_DUP_HANDLE, FALSE, (HANDLE)ProcessID);
		if (ProcessHandle != NULL)
		{
			DuplicateHandle(ProcessHandle, HandleValue, GetCurrentProcess(),
				&v1, NULL, FALSE, DUPLICATE_SAME_ACCESS);
			if (v1 != NULL)
			{
				__NtQueryObject(v1, ObjectTypeInformation, v3, 8, &ReturnLength);
				__NtQueryObject(v1, ObjectTypeInformation, v3, ReturnLength, &ReturnLength);

				if (ObjectTypeInfo->TypeName.Length != NULL)
				{

					WideCharToMultiByte(CP_ACP, NULL, (LPCWSTR)ObjectTypeInfo->TypeName.Buffer,
						-1, (LPSTR)HandleType, 0x1000, NULL, NULL);

					/*
					if (lstrcmpiA((LPCSTR)HandleType,
					"Mutant") == NULL)
					{

					}*/
					IsOk = TRUE;

				}

			}
		}

	}

	if (v1 != NULL)
	{
		HanCloseHandle(v1);
		v1 = NULL;
	}
	if (ProcessHandle != NULL)
	{
		HanCloseHandle(ProcessHandle);
		ProcessHandle = NULL;
	}
	if (!IsOk)
	{
		VirtualFree(HandleType, NULL, MEM_RELEASE);
		return(NULL);
	}
	else
	{
		return(HandleType);
	}
}

BOOL HanEnableSeDebugPrivilege(HANDLE ProcessHandle, BOOL IsEnable)
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

HANDLE HanOpenProcess(DWORD DesiredAccess, BOOL IsInheritHandle, HANDLE ProcessID)
{
	SeEnableSeDebugPrivilege(GetCurrentProcess(), TRUE);
	HANDLE ProcessHandle = OpenProcess(DesiredAccess, IsInheritHandle, (DWORD)ProcessID);
	DWORD LastError = GetLastError();
	SeEnableSeDebugPrivilege(GetCurrentProcess(), FALSE);
	SetLastError(LastError);
	return ProcessHandle;
}
BOOL HanCloseHandle(HANDLE HandleValue)
{
	DWORD HandleFlags;
	if (GetHandleInformation(HandleValue, &HandleFlags)
		&& (HandleFlags & HANDLE_FLAG_PROTECT_FROM_CLOSE) != HANDLE_FLAG_PROTECT_FROM_CLOSE)
		return !!CloseHandle(HandleValue);
	return FALSE;
}
void* HanDosPathToNtPathW(wchar_t* StringName)
{
	void* v5 = VirtualAlloc(NULL, 0x1000, MEM_COMMIT, PAGE_READWRITE);

	wchar_t VolumeDeviceName[3] = L"A:";
	wchar_t COMDeviceName[5] = L"COM0";
	int v7 = 0;
	while (VolumeDeviceName[0] <= 0x5A)  //Z
	{
		RtlZeroMemory(v5, 0x1000);

		if (QueryDosDeviceW(VolumeDeviceName, (LPWSTR)v5, MAX_PATH * 2) > NULL)
		{
			v7 = lstrlenW((LPWSTR)v5);
			lstrcatW((LPWSTR)v5, (LPCWSTR)(StringName + v7));
			if (lstrcmpiW((LPCWSTR)v5, StringName) == NULL)
			{
				RtlZeroMemory(v5, 0x1000);
				lstrcatW((LPWSTR)v5, VolumeDeviceName);
				lstrcatW((LPWSTR)v5, (LPWSTR)(StringName + v7));
				return v5;
			}
		}
		VolumeDeviceName[0]++;
	}
	while (COMDeviceName[3] <= 0x39)
	{
		RtlZeroMemory(v5, 0x1000);
		if (QueryDosDeviceW(COMDeviceName, (LPWSTR)v5, MAX_PATH * 2) > NULL)
		{
			v7 = lstrlenW((LPWSTR)v5);
			lstrcatW((LPWSTR)v5, (LPCWSTR)(StringName + v7));
			if (lstrcmpiW((LPCWSTR)v5, StringName) == NULL)
			{
				RtlZeroMemory(v5, 0x1000);
				lstrcatW((LPWSTR)v5, COMDeviceName);
				lstrcatW((LPWSTR)v5, (LPWSTR)(StringName + v7));
				return(v5);
			}
		}
		COMDeviceName[3]++;
	}
	VirtualFree(v5, NULL, MEM_RELEASE);
	return NULL;
}