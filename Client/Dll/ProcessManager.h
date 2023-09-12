#pragma once
#include "Manager.h"
#include "common.h"
#include <vector>
#include <Psapi.h>
#pragma comment(lib,"Psapi.lib")
#include <TlHelp32.h>
#include "ProcessThreadManager.h"
#include "ProcessHandleManager.h"
#include "ProcessModuleManager.h"



typedef BOOL(__stdcall* LPFN_ISWOW64PROCESS)(HANDLE ProcessHandle, BOOL* IsResult);

#pragma pack(1)
typedef struct _PROCESS_ITEM_INFORMATION_
{
	HANDLE ProcessID;
	char   ProcessImageName[MAX_PATH];
	char   ProcessFullPath[MAX_PATH];
	char   IsWow64Process[20];
}PROCESS_ITEM_INFORMATION, *PPROCESS_ITEM_INFORMATION;


class CProcessManager :
	public CManager
{
public:
	CProcessManager(_CIOCPClient* IOCPClient);
	~CProcessManager();
	
	//Client���ܵ��Ĺ��ڽ��̵���Ϣ���⴦��
	VOID CProcessManager::HandleIO(PBYTE BufferData, ULONG BufferLength);

	//ö���µ�ϵͳ���̷�װ���ݰ�����
	BOOL CProcessManager::SendClientProcessList();
	BOOL CProcessManager::SeEnumProcessByToolHelp32(
		vector<PROCESS_ITEM_INFORMATION>& ProcessItemInfoVector);
	BOOL  CProcessManager::SeIsWow64Process(HANDLE ProcessHandle, BOOL* IsResult);
	BOOL  CProcessManager::SeIsValidWritePoint(LPVOID VirtualAddress);
	BOOL CProcessManager::OpenSpecifyProcess(PBYTE BufferData, ULONG_PTR BufferLength);
	
	//ɱ������
	VOID CProcessManager::KillProcess(LPBYTE BufferData, UINT BufferLength);
	
public:
	LPFN_ISWOW64PROCESS	m_IsWow64Process;


	HANDLE  m_ThreadHandle[0x1000];
	ULONG   m_ThreadHandleCount;
};


DWORD WINAPI ProcessThreadManagerProcedure(LPVOID ParemeterData);
DWORD WINAPI ProcessHandleManagerProcedure(LPVOID ParemeterData);
DWORD WINAPI ProcessModuleManagerProcedure(LPVOID ParemeterData);