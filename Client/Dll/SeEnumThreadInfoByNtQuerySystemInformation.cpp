#include "SeEnumThreadInfoByNtQuerySystemInformation.h"



LPFN_NTQUERYSYSTEMINFORMATION __NtQuerySystemInformation;
LPFN_NTQUERYINFORMATIONTHREAD __NtQueryInformationThread;

vector<THREAD_ITEM_INFORMATION> __ThreadItemInfoVector;  
BOOL __IsEnableDebugPrivilege;


BOOL SeEnumThreadInfoByNtQuerySystemInformation(HANDLE ProcessID,vector<THREAD_ITEM_INFORMATION>& ThreadItemInfoVector)
{

	HMODULE NtdllModuleBase = NULL;
	NtdllModuleBase = GetModuleHandle("Ntdll.dll");
	if (NtdllModuleBase == NULL)
	{
		return FALSE;
	}
	__NtQuerySystemInformation = (LPFN_NTQUERYSYSTEMINFORMATION)GetProcAddress(NtdllModuleBase,
		"NtQuerySystemInformation");
	__NtQueryInformationThread = (LPFN_NTQUERYINFORMATIONTHREAD)GetProcAddress(NtdllModuleBase,
		"NtQueryInformationThread");
	if (__NtQuerySystemInformation == NULL || __NtQueryInformationThread == NULL)
	{
		return FALSE;
	}

	if (ProcessID == NULL)
	{
		return FALSE;
	}
	THREAD_ITEM_INFORMATION ThreadItemInfo;   //自己定义的结构
	
	ULONG ReturnLength = 0;
	ULONG BufferLength = 1;
	PSYSTEM_PROCESS_INFORMATION SystemProcessInfo = (PSYSTEM_PROCESS_INFORMATION)malloc(BufferLength);
	PSYSTEM_PROCESS_INFORMATION v1;
	PSYSTEM_THREAD_INFORMATION  SystemThreadInfo = NULL;

	if (__NtQuerySystemInformation(SystemProcessInformation,
		SystemProcessInfo, BufferLength, &ReturnLength) == STATUS_INFO_LENGTH_MISMATCH)
	{
		free(SystemProcessInfo);
		BufferLength = ReturnLength + sizeof(SYSTEM_PROCESS_INFORMATION);
		
		SystemProcessInfo = (PSYSTEM_PROCESS_INFORMATION)malloc(BufferLength);
		if (!SystemProcessInfo)
		{
			return FALSE;
		}
		if (__NtQuerySystemInformation(SystemProcessInformation, SystemProcessInfo, BufferLength, &ReturnLength) != STATUS_SUCCESS)
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}

	v1 = SystemProcessInfo;

	//所有进程所有线程
	while (TRUE)
	{
		if (v1->UniqueProcessId == ProcessID)
		{
			SystemThreadInfo = &v1->Threads[0];
			for (ULONG i = 0; i < v1->NumberOfThreads; i++)
			{
				THREAD_ITEM_INFORMATION  ThreadItemInfo;
				ZeroMemory(&ThreadItemInfo, sizeof(THREAD_ITEM_INFORMATION));

				ThreadItemInfo.BasePriority = SystemThreadInfo->BasePriority;
				ThreadItemInfo.ContextSwitches = SystemThreadInfo->ContextSwitches;
				ThreadItemInfo.Priority = SystemThreadInfo->Priority;
				ThreadItemInfo.BasePriority = SystemThreadInfo->BasePriority;

				ThreadItemInfo.ThreadState = SystemThreadInfo->ThreadState;
				ThreadItemInfo.WaitReason = SystemThreadInfo->WaitReason;
				ThreadItemInfo.WaitTime = SystemThreadInfo->WaitTime;
				ThreadItemInfo.ThreadID = SystemThreadInfo->ClientId.UniqueThread;

				HANDLE ThreadHandle = SeOpenThread(THREAD_ALL_ACCESS, FALSE, ThreadItemInfo.ThreadID);
		
				if (ThreadHandle)
				{
					ThreadItemInfo.Teb = SeGetThreadTebByThreadHandle(ThreadHandle);  

					PVOID ThreadStartAddress = 0;
					if (__NtQueryInformationThread(ThreadHandle,
						SHINE::ThreadQuerySetWin32StartAddress, &ThreadStartAddress, sizeof(PVOID), NULL) == STATUS_SUCCESS)
					{
						ThreadItemInfo.ThreadStartAddress = ThreadStartAddress;
					}

					ThreadItemInfo.ThreadHandle = ThreadHandle;
				    
				}

				ThreadItemInfoVector.push_back(ThreadItemInfo);
			

				SystemThreadInfo++;
			}

			break;
		}

		if (v1->NextEntryOffset == 0)
		{
			break;
		}
		else
		{
			v1 = (PSYSTEM_PROCESS_INFORMATION)((DWORD_PTR)v1 + (DWORD_PTR)v1->NextEntryOffset);
		}
	}
	free(SystemProcessInfo);
	return (ThreadItemInfoVector.size() > 0);
}


HANDLE SeOpenThread(DWORD DesiredAccess, BOOL IsInheritHandle, HANDLE ThreadID)
{
	if (__IsEnableDebugPrivilege)
	{
		SeEnableSeDebugPrivilege(GetCurrentProcess(), TRUE);
	}
	
	HANDLE ThreadHandle = OpenThread(DesiredAccess, IsInheritHandle, (DWORD)ThreadID);
	DWORD LastError = GetLastError();
	if (__IsEnableDebugPrivilege)
		SeEnableSeDebugPrivilege(GetCurrentProcess(), FALSE);
	
	SetLastError(LastError);
	return ThreadHandle;
}


void*  SeGetThreadTebByThreadHandle(HANDLE ThreadHandle)
{
	ULONG ReturnLength = 0;
	void* Teb = 0;
	THREAD_BASIC_INFORMATION ThreadBasicInfo[5] = { 0 };

	if (__NtQueryInformationThread(ThreadHandle, 
		SHINE::ThreadBasicInformation, ThreadBasicInfo, sizeof(THREAD_BASIC_INFORMATION), &ReturnLength) == STATUS_SUCCESS)
	{
		Teb = (void*)ThreadBasicInfo->TebBaseAddress;
	}
	else
	{
		if (__NtQueryInformationThread(ThreadHandle, SHINE::ThreadBasicInformation, ThreadBasicInfo, 
			ReturnLength, &ReturnLength) == STATUS_SUCCESS)
		{
			Teb = (void*)ThreadBasicInfo->TebBaseAddress;
		}
	}
	return Teb;
}



BOOL SeEnableSeDebugPrivilege(HANDLE ProcessHandle, BOOL IsEnable)
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