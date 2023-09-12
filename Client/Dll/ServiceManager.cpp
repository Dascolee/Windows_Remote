#include "ServiceManager.h"

CServiceManager::CServiceManager(_CIOCPClient * IOCPClient) : CManager(IOCPClient)
{
	printf("CServiceManager()构造调用\r\n");
	EnableSeDebugPrivilege(GetCurrentProcess(), TRUE, SE_DEBUG_NAME);
	m_ServiceManagerHandle = NULL;
	SendClientServiceList();
}

CServiceManager::~CServiceManager()
{
	EnableSeDebugPrivilege(GetCurrentProcess(), FALSE, SE_DEBUG_NAME);
	if (m_ServiceManagerHandle!=NULL)
	{
		CloseServiceHandle(m_ServiceManagerHandle);
		m_ServiceManagerHandle = NULL;
	}
	printf("~CServiceManager()析构调用\r\n");
}

VOID CServiceManager::HandleIO(PBYTE BufferData, ULONG BufferLength)
{
	BYTE IsToken;

	switch (BufferData[0])
	{
	case CLIENT_SERVICE_MANAGER:
	{
		SendClientServiceList();
		break;
	}
	case CLIENT_SERVICE_MANAGER_CONFIG:
	{
		ConfigClientServices((LPBYTE)BufferData + 1, BufferLength - 1);
		break;
	}
	
	}
}

VOID CServiceManager::SendClientServiceList()
{
	LPBYTE BufferData = GetClientServiceList();
	if (BufferData == NULL)
	{
		return;
	}
	m_IOCPClient->OnSending((char*)BufferData, LocalSize(BufferData));
	LocalFree(BufferData);
}

LPBYTE CServiceManager::GetClientServiceList()
{
	LPENUM_SERVICE_STATUS EnumServiceStatus = NULL;
	LPQUERY_SERVICE_CONFIG QueryServiceConfige = NULL;
	DWORD	ReturnLength = 0;
	DWORD	ServicesCount = 0;//枚举服务数
	DWORD   ResumeHandle = 0;//恢复句柄
	LPBYTE  BufferData = NULL;
	DWORD   Offset = 0;

	char RunWay[256] = { 0 };
	char AutoRun[256] = { 0 };
	DWORD BufferLength = 0;

	//打开服务管理器获得句柄
	if ((m_ServiceManagerHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))==NULL)
	{
		return NULL;
	}
	EnumServiceStatus = (LPENUM_SERVICE_STATUS)LocalAlloc(LPTR, 64 * 1024);
	if (EnumServiceStatus == NULL)
	{
		CloseServiceHandle(m_ServiceManagerHandle);
		m_ServiceManagerHandle = NULL;
		return NULL;
	}
	//通过句柄枚举信息
	EnumServicesStatus(m_ServiceManagerHandle,
		SERVICE_TYPE_ALL,
		SERVICE_STATE_ALL,
		(LPENUM_SERVICE_STATUS)EnumServiceStatus,
		60 * 1024,
		&ReturnLength,
		&ServicesCount,
		&ResumeHandle
	);
	BufferData = (LPBYTE)LocalAlloc(LPTR, MAX_PATH);

	BufferData[0] = CLIENT_SERVICE_MANAGER_LIST_REPLY;
	Offset = 1;
	for (unsigned long i = 0; i < ServicesCount; i++)
	{
		SC_HANDLE ServiceHandle = NULL;
		DWORD ResumeHandle = 0;
		//打开各别服务
		ServiceHandle = OpenService(m_ServiceManagerHandle,
			EnumServiceStatus[i].lpServiceName,SERVICE_ALL_ACCESS);
		if ( ServiceHandle==NULL )
		{
			continue;
		}
		QueryServiceConfige = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LPTR, 4 * 1024);
		QueryServiceConfig(ServiceHandle, QueryServiceConfige, 4 * 1024, &ResumeHandle);
		//查询服务的启动类型
		if (EnumServiceStatus[i].ServiceStatus.dwCurrentState!=SERVICE_STOPPED)//启动状态
		{
			ZeroMemory(RunWay, sizeof(RunWay));
			lstrcat(RunWay, "启动");
		}
		else
		{
			ZeroMemory(RunWay, sizeof(RunWay));
			lstrcat(RunWay, "停止");
		}
		if (2== QueryServiceConfige->dwStartType)
		{
			ZeroMemory(AutoRun, sizeof(AutoRun));
			lstrcat(AutoRun, "自动");
		}
		if (3 == QueryServiceConfige->dwStartType)
		{
			ZeroMemory(AutoRun, sizeof(AutoRun));
			lstrcat(AutoRun, "手动");
		}
		if (4 == QueryServiceConfige->dwStartType)
		{
			ZeroMemory(AutoRun, sizeof(AutoRun));
			lstrcat(AutoRun, "禁用");
		}
		//先计算整个数据包的大小
		BufferLength = sizeof(DWORD) + lstrlen(EnumServiceStatus[i].lpDisplayName)
			+ lstrlen(QueryServiceConfige->lpBinaryPathName) + lstrlen(EnumServiceStatus[i].lpServiceName)
			+ lstrlen(RunWay) + lstrlen(AutoRun) + 1;
		//缓冲区太小，再重新分配下
		if (LocalSize(BufferData) < (Offset+ BufferLength))
		{
			BufferData = (LPBYTE)LocalReAlloc(BufferData, (Offset + BufferLength),
				LMEM_ZEROINIT|LMEM_MOVEABLE);
		}
		memcpy(BufferData + Offset, EnumServiceStatus[i].lpDisplayName,
			lstrlen(EnumServiceStatus[i].lpDisplayName) + 1);
		Offset += lstrlen(EnumServiceStatus[i].lpDisplayName) + 1; //真实名称

		memcpy(BufferData + Offset, EnumServiceStatus[i].lpServiceName,
			lstrlen(EnumServiceStatus[i].lpServiceName) + 1);
		Offset += lstrlen(EnumServiceStatus[i].lpServiceName) + 1; //显示名称

		memcpy(BufferData + Offset, QueryServiceConfige->lpBinaryPathName,
			lstrlen(QueryServiceConfige->lpBinaryPathName) + 1);
		Offset += lstrlen(QueryServiceConfige->lpBinaryPathName) + 1; //路径

		memcpy(BufferData + Offset, RunWay, lstrlen(RunWay) + 1);//运行状态
		Offset += lstrlen(RunWay) + 1;

		memcpy(BufferData + Offset, AutoRun, lstrlen(AutoRun) + 1);//自启动状态
		Offset += lstrlen(AutoRun) + 1;

		CloseServiceHandle(ServiceHandle);
		LocalFree(QueryServiceConfige);
	}

	CloseServiceHandle(m_ServiceManagerHandle);
	LocalFree(EnumServiceStatus);
	return BufferData;
}

void CServiceManager::ConfigClientServices(PBYTE BufferData,ULONG BufferLength)
{
	BYTE IsMethod = BufferData[0];
	char* ServiceName = (char*)(BufferData + 1);

	switch (IsMethod)
	{
	case 1:
	{
		SC_HANDLE ServiceManagerHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (NULL!= ServiceManagerHandle)
		{
			SC_HANDLE ServiceHandle = OpenService(ServiceManagerHandle,
				ServiceName,SERVICE_ALL_ACCESS);
			if (NULL!= ServiceHandle)
			{
				StartService(ServiceHandle, NULL, NULL);
				CloseServiceHandle(ServiceHandle);
			}
			CloseServiceHandle(ServiceManagerHandle);
		}
		Sleep(500);
		SendClientServiceList();
		break;
	}
	case 2:
	{
		SC_HANDLE ServiceManagerHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (NULL != ServiceManagerHandle)
		{
			SC_HANDLE ServiceHandle = OpenService(ServiceManagerHandle,
				ServiceName, SERVICE_ALL_ACCESS);
			if (NULL != ServiceHandle)
			{
				SERVICE_STATUS Status;
				BOOL IsOk = ControlService(ServiceHandle,SERVICE_CONTROL_STOP,&Status);
				CloseServiceHandle(ServiceHandle);
			}
			CloseServiceHandle(ServiceManagerHandle);
		}
		Sleep(500);
		SendClientServiceList();
		break;
	}

	case 3:
	{
		SC_HANDLE ServiceManagerHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (NULL != ServiceManagerHandle)
		{
			SC_HANDLE ServiceHandle = OpenService(ServiceManagerHandle,
				ServiceName, SERVICE_ALL_ACCESS);
			if (NULL != ServiceHandle)
			{
				SC_LOCK v1 =LockServiceDatabase(ServiceManagerHandle);
				BOOL IsOk = ChangeServiceConfig(
					ServiceHandle,
					SERVICE_NO_CHANGE,
					SERVICE_AUTO_START,
					SERVICE_NO_CHANGE,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL);
				UnlockServiceDatabase(v1);
				CloseServiceHandle(ServiceHandle);
			}
			CloseServiceHandle(ServiceManagerHandle);
		}
		Sleep(500);
		SendClientServiceList();
		break;
	}
	case 4:
	{
		SC_HANDLE ServiceManagerHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (NULL != ServiceManagerHandle)
		{
			SC_HANDLE ServiceHandle = OpenService(ServiceManagerHandle,
				ServiceName, SERVICE_ALL_ACCESS);
			if (NULL != ServiceHandle)
			{
				SC_LOCK v1 = LockServiceDatabase(ServiceManagerHandle);
				BOOL IsOk = ChangeServiceConfig(
					ServiceHandle,
					SERVICE_NO_CHANGE,
					SERVICE_DEMAND_START,
					SERVICE_NO_CHANGE,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL);
				UnlockServiceDatabase(v1);
				CloseServiceHandle(ServiceHandle);
			}
			CloseServiceHandle(ServiceManagerHandle);
		}
		Sleep(500);
		SendClientServiceList();
		break;
	}
	default:
		break;
	}
}