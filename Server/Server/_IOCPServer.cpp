#include "stdafx.h"
#include "_IOCPServer.h"

_CIOCPServer::_CIOCPServer()
{
	//初始化套接字
	WSADATA WsaData;
	if (WSAStartup(MAKEWORD(2, 2), &WsaData)!= 0)
	{
		return;
	}
	memcpy(m_PacketHeaderFlag, "Shine", PACKET_FLAG_LENGHT);
	m_ListenSocket = INVALID_SOCKET;
	m_ListenThreadHandle = NULL;
	m_KillEventHandle = NULL;
	m_ListenEventHandle = WSA_INVALID_EVENT;

	InitializeCriticalSection(&m_CriticalSection);
	m_CompletionPortHandle = INVALID_HANDLE_VALUE;

	m_ThreadPoolMin = 0;
	m_ThreadPoolMax = 0;
	m_WorkThreadCount = 0;
	m_CurrentThreadCount = 0;
	m_BusyThreadCount = 0;
	m_IsWorking = FALSE;

}

//服务器的主DIALOG关闭
_CIOCPServer::~_CIOCPServer()
{

	Sleep(1);
	//触发事件使其正常退出监听线程的循环
	SetEvent(m_KillEventHandle);
	//等待监听线程的退出
	WaitForSingleObject(m_ListenThreadHandle, INFINITE);

	if (m_ListenSocket != INVALID_SOCKET)
	{
		closesocket(m_ListenSocket);
		m_ListenSocket = INVALID_SOCKET;
	}

	if (m_ListenEventHandle != NULL)
	{
		CloseHandle(m_ListenEventHandle);
		m_ListenEventHandle = INVALID_HANDLE_VALUE;
	}

	if (m_CompletionPortHandle != NULL)
	{
		CloseHandle(m_CompletionPortHandle);
		m_CompletionPortHandle = INVALID_HANDLE_VALUE;
	}

	if (m_KillEventHandle != NULL)
	{
		CloseHandle(m_KillEventHandle);
		m_KillEventHandle = INVALID_HANDLE_VALUE;
	}

	/*
	CloseHandle(m_EventHandle);
	m_EventHandle2 = INVALID_HANDLE_VALUE;
	DeleteCriticalSection(&m_FreeBufferObjectListLock);
	DeleteCriticalSection(&m_FreeContextObjectListLock);
	DeleteCriticalSection(&m_PendingAcceptListLock);
	*/

	m_ThreadPoolMin = 0;
	m_ThreadPoolMax = 0;

	DeleteCriticalSection(&m_CriticalSection);
	WSACleanup();
}

BOOL _CIOCPServer::ServerRun(USHORT ListenPort,LPFN_WINDOWNOTIFYPROCEDURE WindowNotifyProcedure)
{
	m_WindowNotifyProcedure = WindowNotifyProcedure;

	//创建退出线程的事件
	m_KillEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	if (m_KillEventHandle == NULL)
	{
		return FALSE;
	}
	//创建一个监听套接字
	m_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (m_ListenSocket == INVALID_SOCKET)
	{
		return FALSE;
	}
	//创建一个监听事件
	m_ListenEventHandle = WSACreateEvent();
	if (m_ListenEventHandle == WSA_INVALID_EVENT)
	{
		closesocket(m_ListenSocket);
		m_ListenSocket = INVALID_SOCKET;
		return FALSE;
	}
	//事件选择模型
	BOOL IsOk = WSAEventSelect(m_ListenSocket, //监听套接字与事件进行关联并授予FD_CLOSE与FD_ACCEPT属性
		m_ListenEventHandle,
		FD_ACCEPT | FD_CLOSE);
	 
	if (IsOk == SOCKET_ERROR)
	{
		WSACloseEvent(m_ListenEventHandle);
		m_ListenEventHandle = WSA_INVALID_EVENT;
		closesocket(m_ListenSocket);
		m_ListenSocket = INVALID_SOCKET;
		return FALSE;
	}
	//初始化Server端网卡
	SOCKADDR_IN ServerAddress;
	ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_port = htons(ListenPort);
	ServerAddress.sin_addr.S_un.S_addr = INADDR_ANY;
	//绑定套接字
	if (bind(m_ListenSocket, (sockaddr*)&ServerAddress, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		WSACloseEvent(m_ListenEventHandle);
		m_ListenEventHandle = WSA_INVALID_EVENT;
		closesocket(m_ListenSocket);
		m_ListenSocket = INVALID_SOCKET;
		return FALSE;
	}
	//监听
	IsOk = listen(m_ListenSocket, SOMAXCONN);
	if (IsOk == SOCKET_ERROR)
	{
		WSACloseEvent(m_ListenEventHandle);
		m_ListenEventHandle = WSA_INVALID_EVENT;
		closesocket(m_ListenSocket);
		m_ListenSocket = INVALID_SOCKET;
		return FALSE;
	}
	//创建监听线程
	m_ListenThreadHandle = CreateThread(NULL,
		0,
		ListenThreadProcedure, 
		this,
		0,
		NULL); 

	if (m_ListenThreadHandle == INVALID_HANDLE_VALUE)
	{
		WSACloseEvent(m_ListenEventHandle);
		m_ListenEventHandle = WSA_INVALID_EVENT;
		closesocket(m_ListenSocket);
		m_ListenSocket = INVALID_SOCKET;
		return FALSE;
	}
	
	//初始化IOCP(当异步请求完成的时候)
	//创建一个完成端口
	//启动工作线程（守候完成端口上等待异步请求的完成）
	// IO_RECEICE

	InitializeIOCP();
	return TRUE;
}

DWORD _CIOCPServer::ListenThreadProcedure(LPVOID ParameterData)
{
	_CIOCPServer* This = (_CIOCPServer*)ParameterData;

	int EventIndex = 0;
	WSANETWORKEVENTS NetWorkEvents;
	
	while (1)
	{
		EventIndex = WaitForSingleObject(This->m_KillEventHandle, 100);
		EventIndex = EventIndex - WAIT_OBJECT_0;
		if (EventIndex==0)
		{
			break;
		}
		DWORD v1;
		//等待接听事件授信（监听套接字授信） 事件模型
		v1 = WSAWaitForMultipleEvents(1, &This->m_ListenEventHandle,
			FALSE, 100, FALSE);

		if (v1==WSA_WAIT_TIMEOUT)
		{
			//该事件没有授信
			continue;
		}
		//发生了FD_ACCEPT与FD_CLOSE事件
		//如果事件授信我们 就该将该事件转化成一个网络事件进行判断
		v1 = WSAEnumNetworkEvents(This->m_ListenSocket, This->m_ListenEventHandle,
			&NetWorkEvents);
		if (v1 == SOCKET_ERROR)
		{
			break;
		}
		if (NetWorkEvents.lNetworkEvents & FD_ACCEPT)
		{
			if (NetWorkEvents.iErrorCode[FD_ACCEPT_BIT]==0)
			{
				//将客户端上线请求进行处理
				//如果是一个链接请求我们就进入     函数进行处理
				
				This->OnAccept();
			}
			else
			{
				break;
			}
		}
		else
		{
			//break;
		}
	}
	return 0;
}

void _CIOCPServer::OnAccept()
{
	int Result = 0;
	//保存上线用户
	SOCKET ClientSocket = INVALID_SOCKET;
	SOCKADDR_IN ClientAddress = { 0 };
	int  ClientAddressLength = sizeof(SOCKADDR_IN);
	//我们通过一个套接字来生成一个与信号通讯的套接字 （为客户端生成一个套接字）
	ClientSocket = accept(m_ListenSocket, (sockaddr*)&ClientAddress,
		&ClientAddressLength);
	if (ClientSocket==SOCKET_ERROR)
	{
		return;
	}
	//我们在这里为每一个到达的信号维护了一个与之相关联的数据结构这里简称为用户的上下背景文
	
	PCONTEXT_OBJECT ContextObject = AllocateContextObject();

	if (ContextObject == NULL)
	{
		closesocket(ClientSocket);
		ClientSocket = INVALID_SOCKET;
		return;
	}

	//成员赋值
	ContextObject->ClientSocket = ClientSocket;
	//关联内存
	ContextObject->ReceiveWsaBuffer.buf = (char*)(ContextObject->BufferData);
	ContextObject->ReceiveWsaBuffer.len = sizeof(ContextObject->BufferData);

	//将生成得套接字与完成端口句柄相关联
	HANDLE Handle = CreateIoCompletionPort((HANDLE)ClientSocket,
		m_CompletionPortHandle, (ULONG_PTR)ContextObject, 0);

	if (Handle != m_CompletionPortHandle)
	{
		delete ContextObject;
		ContextObject = NULL;
		if (ClientSocket!=INVALID_SOCKET)
		{
			closesocket(ClientSocket);
			ClientSocket = INVALID_SOCKET;
		}
		return;
	}

	//保活机制

	//为了保持与客户端的连接，若2小时内在此套接口的任一方向都没有数据交换，则TCP自动给客户发一个数据包
	//用来检测客户端主机是否崩溃
	//若客户端发生网线或者断电等非正常断开的现象，若服务器没有设置SO_KEEPALIVE选项，
	//则会一直不关闭该套接字，故须开启保活机制。默认检测时间为2小时

	//设置套接字的选项卡 setsockopt  开启保活机制 SO_KEEPALIVE
	m_KeepAliveTime = 3;
	BOOL IsOk = TRUE;
	if (setsockopt(ContextObject->ClientSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&IsOk, sizeof(IsOk))!= 0)
	{
		MessageBox(NULL,"0","setsockopt",NULL);
	}
	//设置超时详细信息
	tcp_keepalive v1;
	v1.onoff = 1;		//启用保活
	v1.keepalivetime = m_KeepAliveTime;	//超过时长未发送数据，则发送探测包
	v1.keepaliveinterval = 1000 * 10;	//若探测没有回应，重试间隔为10s
	WSAIoctl
	(
		ContextObject->ClientSocket,
		SIO_KEEPALIVE_VALS,
		&v1,
		sizeof(v1),
		NULL,
		0,
		(unsigned long *)&IsOk,
		0,
		NULL
	);

	_CCriticalSection CriticalSection(m_CriticalSection);
	m_ConnectionContextObjectList.AddTail(ContextObject);  //插入到我们的内存列表中

	OVERLAPPEDEX *OverlappedEx = new OVERLAPPEDEX(IO_INITIALIZE);

	BOOL IsOK = FALSE;
	//想完成端口投递一个请求
	//工程线程
	IsOK = PostQueuedCompletionStatus(m_CompletionPortHandle,
		0, (ULONG_PTR)ContextObject, &OverlappedEx->m_Overlapped);//工作线程 一个通知
	
	//int Lasterror = GetLastError();
	/*
	if ((!IsOk) && (GetLastError() != ERROR_IO_PENDING))  //如果投递失败
	{
		RemoteContextObject(ContextObject);
		return;
	}*/


	//该上线用户已完成了上线的请求
	//服务器向该用户投递PostRecv请求
	PostReceive(ContextObject);

}

PCONTEXT_OBJECT _CIOCPServer::AllocateContextObject()
{
	PCONTEXT_OBJECT ContextObject = NULL;
	//进入临界区
	_CCriticalSection CriticalSection(m_CriticalSection);
	//EnterCriticalSection(&m_FreeContextObjectListLock);

	//判断内存池是否为空
	if (m_FreeContextObjectList.IsEmpty() == FALSE)
	{
		ContextObject = m_FreeContextObjectList.RemoveHead();
	}
	else
	{
		// 申请新的内存
		ContextObject = new CONTEXT_OBJECT;
	}

	if (ContextObject != NULL)
	{
		ContextObject-> InitMember();  //初始化成员变量
	}
	return ContextObject;
}

VOID _CIOCPServer::RemoteContextObject(CONTEXT_OBJECT * ContextObject)
{
	_CCriticalSection CriticalSection(m_CriticalSection);
	if (m_ConnectionContextObjectList.Find(ContextObject))
	{
		CancelIo((HANDLE)ContextObject->ClientSocket);
		//关闭套接字
		closesocket(ContextObject->ClientSocket);
		ContextObject->ClientSocket = INVALID_SOCKET;
		while (!HasOverlappedIoCompleted((LPOVERLAPPED)ContextObject))
		{
			Sleep(0);
		}
		MoveContextObjectToFreePool(ContextObject);
	}
}

VOID _CIOCPServer::MoveContextObjectToFreePool(CONTEXT_OBJECT * ContextObject)
{
	_CCriticalSection CriticalSection(m_CriticalSection);

	POSITION Position = m_ConnectionContextObjectList.Find(ContextObject);

	if (Position)
	{
		ContextObject->m_ReceivedDecompressdBufferData.ClearBuffer();
		ContextObject->m_ReceivedCompressdBufferData.ClearBuffer();
		ContextObject->m_SendCompressdBufferData.ClearBuffer();

		memset(ContextObject->BufferData, 0, PACKET_LENGTH);
		m_FreeContextObjectList.AddTail(ContextObject);
		m_ConnectionContextObjectList.RemoveAt(Position);
	}

}

VOID _CIOCPServer::PostReceive(CONTEXT_OBJECT * ContextObject)
{
	//向我们的刚上线的用户的投递一个接受数据的请求
	//如果用户的第一个数据包到达也就是被控制的登录请求到达我们的工作线程(请求得到完成)
	//工作线程(守候在完成端口)会响应平调用HANdleIO函数
	OVERLAPPEDEX *OverlappedEx = new OVERLAPPEDEX(IO_RECEIVE);

	DWORD ReturnLenght;
	ULONG Flags = MSG_PARTIAL;
	int IsOK = WSARecv(ContextObject->ClientSocket,
		&ContextObject->ReceiveWsaBuffer,
		1,  
		&ReturnLenght,
		&Flags,
		&OverlappedEx->m_Overlapped,
		NULL);

	if (IsOK==SOCKET_ERROR&&WSAGetLastError()!=WSA_IO_PENDING)
	{
		//请求发送错误
		RemoteContextObject(ContextObject);
	}

}

BOOL _CIOCPServer::InitializeIOCP(VOID)
{
	m_CompletionPortHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_CompletionPortHandle==NULL)
	{
		return FALSE;
	}
	//线程池
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	
	m_ThreadPoolMin = 1;
	m_ThreadPoolMax = SystemInfo.dwNumberOfProcessors * 2;
	
	//ProcessorLowThreadsHold
	ULONG WorkThreadCount = 2;
	//= m_ThreadPoolMax;调试
	HANDLE WorkThreadHandle = NULL;


	for (int i = 0; i < WorkThreadCount; i++)
	{
		WorkThreadHandle = (HANDLE)CreateThread(NULL,
			0,
			(LPTHREAD_START_ROUTINE)WorThreadProcedure,
			(void*)this,
			0,
			NULL);
		if (WorkThreadHandle == NULL)
		{
			CloseHandle(m_CompletionPortHandle);
			return FALSE;
		}
		m_WorkThreadCount++;
		CloseHandle(WorkThreadHandle);
	}
	return TRUE;
}

DWORD _CIOCPServer::WorThreadProcedure(LPVOID ParameterData)
{
	_CIOCPServer *This = (_CIOCPServer*)(ParameterData);
	HANDLE CompletionPortHandle = This->m_CompletionPortHandle;
	
	DWORD ReturnLength = 0;
	PCONTEXT_OBJECT ContextObject = NULL;
	LPOVERLAPPED	Overlapped	  = NULL;
	OVERLAPPEDEX*	OverlappedEx  = NULL;
	ULONG v3 = 0;    //BusyThreadCount
	BOOL v1 = FALSE;

	InterlockedIncrement(&This->m_CurrentThreadCount);//1
	InterlockedIncrement(&This->m_BusyThreadCount);//1
	
	
	while (This->m_IsWorking==FALSE)
	{
		InterlockedDecrement(&This->m_BusyThreadCount);
		//该函数是阻塞函数  如果该函数返回说明有请求得到了完成
 		BOOL IsOk = GetQueuedCompletionStatus(
			CompletionPortHandle,
			&ReturnLength,
			(PULONG_PTR)&ContextObject,
			&Overlapped,
			60000);
		
/*		OVERLAPPEDEX *OverlappedEx = new OVERLAPPEDEX(IO_INITIALIZE);
		BOOL IsOK = FALSE;
		//想完成端口投递一个请求
		//工程线程
		IsOK = PostQueuedCompletionStatus(m_CompletionPortHandle,
		0, (ULONG_PTR)ContextObject, &OverlappedEx->m_Overlapped);//工作线程 一个通知*/
		
		
		DWORD LastError = GetLastError();
		OverlappedEx = CONTAINING_RECORD(Overlapped, OVERLAPPEDEX, m_Overlapped);
		
		v3 = InterlockedIncrement(&This->m_BusyThreadCount);//3
															//1
		
		if (!IsOk&&LastError != WAIT_TIMEOUT)
		{
			if (ContextObject&&This->m_IsWorking == FALSE&&ReturnLength == 0)
			{
				//当对方的套机制发生了关闭
				This->RemoteContextObject(ContextObject);
			}
			continue;
		}
		
		if (!v1)
		{
			if (v3==This->m_CurrentThreadCount)
			{
				if (v3<This->m_ThreadPoolMax)
				{
					if (ContextObject != NULL)
					{
						HANDLE ThreadHandle = (HANDLE)CreateThread(NULL,
							0,
							(LPTHREAD_START_ROUTINE)WorThreadProcedure,
							(void*)This,
							0,
							NULL);
						InterlockedIncrement(&This->m_WorkThreadCount);
						CloseHandle(ThreadHandle);

					}
				}
			}
			//销毁一个线程从线程池
			if (!IsOk && LastError==WAIT_TIMEOUT)
			{
				if (ContextObject == NULL)
				{
					if (This->m_CurrentThreadCount>This->m_ThreadPoolMin)
					{
						break;
					}
					v1 = TRUE;
				}
			}
		}
		if (!v1)
		{
			//成功 请求得到完成请求（两种  IO_RECEIVE, IO_INITIALIZE   PostQueuedCompletionStatus 客户WSARecv）
			//ReturnValue GetQueuedCompletionStatus返回值
			if (IsOk&&OverlappedEx!=NULL&&ContextObject != NULL)
			{
				try
				{
					//请求完成处理函数
					This->HandleIO(OverlappedEx->m_PackType,ContextObject, ReturnLength);
					ContextObject = NULL;
				}
				catch (...) {}
			}
		}
		if (OverlappedEx)
		{
			delete OverlappedEx;
			OverlappedEx = NULL;
		}
	}
	InterlockedDecrement(&This->m_WorkThreadCount);
	InterlockedDecrement(&This->m_CurrentThreadCount);
	InterlockedDecrement(&This->m_BusyThreadCount);
	return 0;
}

BOOL _CIOCPServer::HandleIO(PACKET_TYPE PacketType, PCONTEXT_OBJECT ContextObject, DWORD BuffferLenght)
{
	BOOL v1 = FALSE;
	if (IO_INITIALIZE == PacketType)
	{
		v1 = OnInitializing(ContextObject, BuffferLenght);//OnAccept 函数中投递的请求
	}
	if (IO_RECEIVE == PacketType)//WSAResc 被控端传送的数据
	{
		v1 = OnReceiving(ContextObject, BuffferLenght);
	}
	if (IO_SEND == PacketType)  //主控端向端传送数据
	{
		//已经将数据包发送完成后 才能请求
		//发送数据到客户端
		v1 = OnPostSending(ContextObject, BuffferLenght);
	}
	return v1;
}

BOOL _CIOCPServer::OnInitializing(PCONTEXT_OBJECT ContextObject, DWORD BuffferLenght)
{

	//MessageBox(NULL, "on", "on", 0);
	return TRUE;
}

BOOL _CIOCPServer::OnReceiving(PCONTEXT_OBJECT ContextObject, DWORD BuffferLenght)
{
	_CCriticalSection CriticalSection(m_CriticalSection);
	try
	{
		if (BuffferLenght == 0)
		{
			//MessageBox(NULL, "关闭套接字", "guanbi", 0);
			RemoteContextObject(ContextObject);
			return FALSE;
		}
		//将接到的数据拷贝到m_ReceivedCompressdBufferData
		ContextObject->m_ReceivedCompressdBufferData.
			WriteBuffer((PBYTE)ContextObject->BufferData, BuffferLenght);

		//读取数据包的头部（数据包的头部是不参与压缩的）
		while (ContextObject->m_ReceivedCompressdBufferData.GetBufferLength()>PACKET_HEADER_LENGHT)
		{
			//储存数据包头部标志
			char v1[PACKET_FLAG_LENGHT] = { 0 };
			//拷贝数据包头部标志
			CopyMemory(v1, ContextObject->m_ReceivedCompressdBufferData.GetBuffer(), PACKET_FLAG_LENGHT);
			//校验数据包的头部标志
			
			if (memcmp(m_PacketHeaderFlag, v1, PACKET_FLAG_LENGHT) != 0)
			{
				throw "Bad Buffer";
			}
			//获得数据包总大小
			ULONG PackTotalLength = 0;
			CopyMemory(&PackTotalLength, ContextObject->m_ReceivedCompressdBufferData.GetBuffer(PACKET_FLAG_LENGHT),
				sizeof(ULONG));
			int BufferLegth = ContextObject->m_ReceivedCompressdBufferData.GetBufferLength();
			 if (PackTotalLength && (ContextObject->m_ReceivedCompressdBufferData.GetBufferLength()) >= PackTotalLength)
			{
				ULONG DecompressdLenght = 0;
				ContextObject->m_ReceivedCompressdBufferData.ReadBuffer((PBYTE)v1,
					PACKET_FLAG_LENGHT);
				ContextObject->m_ReceivedCompressdBufferData.ReadBuffer((PBYTE)&PackTotalLength,
					sizeof(ULONG));
				ContextObject->m_ReceivedCompressdBufferData.ReadBuffer((PBYTE)&DecompressdLenght, 
					sizeof(ULONG));
				ULONG CompressedLenght = PackTotalLength - PACKET_HEADER_LENGHT;
				PBYTE CompressedData = new BYTE[CompressedLenght];
				PBYTE DecompressedData = new BYTE[DecompressdLenght];
				if (CompressedData ==NULL||DecompressedData==NULL)
				{
					throw "Bad Allocate";
				}
				//从数据包中获得压缩后的数据
				ContextObject->m_ReceivedCompressdBufferData.ReadBuffer(CompressedData, CompressedLenght);
				//解压缩
				int IsOK = uncompress(DecompressedData,&DecompressdLenght, CompressedData, CompressedLenght);
				int Error = GetLastError();

				if (IsOK == Z_OK)
				{
					ContextObject->m_ReceivedDecompressdBufferData.ClearBuffer();
					ContextObject->m_ReceivedCompressdBufferData.ClearBuffer();

					//拷贝真实数据
					ContextObject->m_ReceivedDecompressdBufferData.WriteBuffer(DecompressedData,
						DecompressdLenght);
					delete[] CompressedData;
					delete[] DecompressedData;

					m_WindowNotifyProcedure(ContextObject);
				}
				else
				{
					delete[] CompressedData;
					delete[] DecompressedData;
					throw "Bad Buffer";
				}
			}
			else
			{
				break;
			}
		}
		//上一次的异步请求e已经得到完成重新投递新的异步请求
		PostReceive(ContextObject);  //投递新的异步请求

	}
	catch (...)
	{
		ContextObject->m_ReceivedCompressdBufferData.ClearBuffer();
		ContextObject->m_ReceivedDecompressdBufferData.ClearBuffer();
		PostReceive(ContextObject);  //投递新的异步请求
	}
	return TRUE;
}

VOID _CIOCPServer::OnPrepareSending(CONTEXT_OBJECT* ContextObject, PBYTE BufferData, ULONG BufferLength)
{
	if (ContextObject == NULL)
	{
		return;
	}

	try
	{
		if (BufferLength > 0)
		{
			unsigned long	CompressedLength = (double)BufferLength * 1.001 + 12;
			LPBYTE			CompressedData = new BYTE[CompressedLength];
			int	IsOk = compress(CompressedData, &CompressedLength, (LPBYTE)BufferData, BufferLength);

			if (IsOk != Z_OK)
			{
				delete[] CompressedData;
				return;
			}

			//构建数据包头部
			ULONG PackTotalLength = CompressedLength + PACKET_HEADER_LENGHT;
			ContextObject->m_SendCompressdBufferData.WriteBuffer((LPBYTE)m_PacketHeaderFlag, PACKET_FLAG_LENGHT);
			ContextObject->m_SendCompressdBufferData.WriteBuffer((PBYTE)&PackTotalLength, sizeof(ULONG));
			ContextObject->m_SendCompressdBufferData.WriteBuffer((PBYTE)&BufferLength, sizeof(ULONG));
			ContextObject->m_SendCompressdBufferData.WriteBuffer(CompressedData, CompressedLength);
			//[xx][PackTotalLength][BufferLength][.....(真实数据)]

			delete[] CompressedData;
		}
		OVERLAPPEDEX* OverlappedEx = new OVERLAPPEDEX(IO_SEND);
		//将请求包发送到完成端口
		PostQueuedCompletionStatus(m_CompletionPortHandle, 0, (DWORD)ContextObject, &OverlappedEx->m_Overlapped);
	}
	catch (...) {}
}

BOOL _CIOCPServer::OnPostSending(CONTEXT_OBJECT* ContextObject, ULONG BufferLength)
{
	try
	{
		DWORD Flags = MSG_PARTIAL;
		//将完成的数据从数据结构中去除				
		ContextObject->m_SendCompressdBufferData.RemoveComletedBuffer(BufferLength);  
		//判断还有多少数据需要发送
		if (ContextObject->m_SendCompressdBufferData.GetBufferLength() == 0)
		{
			//数据已经发送完毕
			ContextObject->m_SendCompressdBufferData.ClearBuffer();
			return true;		                             //走到这里说明我们的数据真正完全发送
		}
		else
		{
			//真正发送数据//数据没有完成  我们继续投递 发送请求
			OVERLAPPEDEX * OverlappedEx = new OVERLAPPEDEX(IO_SEND);          
			//获得剩余的数据和长度   
			ContextObject->SendWsaBuffer.buf = (char*)ContextObject->m_SendCompressdBufferData.GetBuffer();
			ContextObject->SendWsaBuffer.len = ContextObject->m_SendCompressdBufferData.GetBufferLength();
			 
			int iOk = WSASend(ContextObject->ClientSocket,
				&ContextObject->SendWsaBuffer,
				1,
				&ContextObject->SendWsaBuffer.len,
				Flags,
				&OverlappedEx->m_Overlapped,
				NULL);
			if (iOk == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
			{
				int a = GetLastError();
				RemoteContextObject(ContextObject);
			}
		}
	}
	catch (...) {}
	return FALSE;
}
