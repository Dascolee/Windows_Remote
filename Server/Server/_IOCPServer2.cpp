#include "stdafx.h"
#include "_IOCPServer.h"


_CIOCPServer::_CIOCPServer()
{
	WSADATA  WsaData = { 0 };

	if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)
	{
		return;
	}
	m_ListenSocket = INVALID_SOCKET;				//初始化监听套接字
	m_ListenThreadHandle = NULL;					//初始化监听线程
	m_KillEventHandle = NULL;
	m_ListenEventHandle = WSA_INVALID_EVENT;		//初始化监听事件(事件模型)
	InitializeCriticalSection(&m_CriticalSection);
	m_CompletionPortHandle = INVALID_HANDLE_VALUE;	//初始化完成端口句柄
	m_ThreadPoolMin = 0;
	m_ThreadPoolMax = 0;
	m_WorkThreadCount = 0;
	m_CurrentThreadCount = 0;
	m_BusyThreadCount = 0;
	m_IsWorking = FALSE;
}


_CIOCPServer::~_CIOCPServer()
{
	Sleep(1);
	SetEvent(m_KillEventHandle);	//触发事件，使监听线程正常退出循环
	WaitForSingleObject(m_ListenThreadHandle, INFINITE);	//等待监听线程退出

	//关闭监听套接字、监听事件句柄和完成端口句柄
	if (m_ListenSocket != INVALID_SOCKET)
	{
		closesocket(m_ListenSocket);
		m_ListenSocket = INVALID_SOCKET;
	}
	if (m_ListenEventHandle != WSA_INVALID_EVENT)
	{
		WSACloseEvent(m_ListenEventHandle);
		m_ListenEventHandle = WSA_INVALID_EVENT;
	}
	if (m_CompletionPortHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_CompletionPortHandle);
		m_CompletionPortHandle = INVALID_HANDLE_VALUE;
	}
	m_ThreadPoolMin = 0;
	m_ThreadPoolMax = 0;
	m_WorkThreadCount = 0;
	m_CurrentThreadCount = 0;
	m_BusyThreadCount = 0;
	m_IsWorking = FALSE;
	//销毁临界区
	DeleteCriticalSection(&m_CriticalSection);
	//回收套接字库
	WSACleanup();
}

BOOL _CIOCPServer::ServerRun(USHORT ListenPort)
{
	//创建监听套接字
	m_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);//使监听套接字可以处理异步重叠请求
	if (m_ListenSocket == INVALID_SOCKET)
	{
		return FALSE;
	}
	//创建监听事件
	m_ListenEventHandle = WSACreateEvent();	//创建一个需手动设置属性的且未授信的事件
	if (m_ListenEventHandle == WSA_INVALID_EVENT)
	{
		goto Error;
	}
	//创建退出监听线程的事件
	m_KillEventHandle = CreateEvent(NULL,FALSE,FALSE,NULL);	
	if (m_KillEventHandle == NULL)
	{
		goto Error;
	}
	//事件选择模型
	BOOL IsOk = WSAEventSelect(m_ListenSocket, m_ListenEventHandle, FD_ACCEPT | FD_CLOSE);	//将监听套接字与事件关联
	if (IsOk == SOCKET_ERROR)
	{
		goto Error;
	}
	//初始化Server端网卡
	SOCKADDR_IN ServerAddress;
	ServerAddress.sin_port = htons(ListenPort);	//将监听端口转换为网络字节顺序
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_addr.s_addr = INADDR_ANY;
	//绑定监听套接字
	IsOk = bind(m_ListenSocket, (sockaddr*)&ServerAddress, sizeof(ServerAddress));
	if (IsOk == SOCKET_ERROR)
	{
		goto Error;
	}
	//监听
	IsOk = listen(m_ListenSocket,SOMAXCONN);
	if (IsOk == SOCKET_ERROR)
	{
		goto Error;
	}
	//创建监听线程
	m_ListenThreadHandle = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)ListenThreadProcedure,
		this,		//向线程回调函数中传入this，方便回调函数访问类成员
		0,
		NULL);
	if (m_ListenThreadHandle == INVALID_HANDLE_VALUE)
	{
		goto Error;
	}
	//初始化IOCP(异步请求完成时)
	//创建完成端口
	//启动工作线程
	InitializeIOCP();


	return TRUE;
Error:
	if (m_ListenSocket != INVALID_SOCKET)
	{
		closesocket(m_ListenSocket);
		m_ListenSocket = INVALID_SOCKET;
	}
	if (m_ListenEventHandle != WSA_INVALID_EVENT)
	{
		WSACloseEvent(m_ListenEventHandle);
		m_ListenEventHandle = WSA_INVALID_EVENT;
	}

	return FALSE;
}

DWORD _CIOCPServer::ListenThreadProcedure(LPVOID ParameterData)
{
	_CIOCPServer* This = (_CIOCPServer*)ParameterData;
	int EventIndex = 0;
	WSANETWORKEVENTS NetWorkEvents;
	while (TRUE)
	{
		EventIndex = WaitForSingleObject(This->m_KillEventHandle, 100);	//事件授信返回值为0
		EventIndex = EventIndex - WAIT_OBJECT_0;
		if (EventIndex == 0)
		{
			//由析构函数触发
			break;
		}
		DWORD v1;
		//等待监听事件授信
		v1 = WSAWaitForMultipleEvents(1,
			&This->m_ListenEventHandle,
			FALSE,	//若有多个事件，FALSE为只要有一个事件授信，则函数返回	TRUE为必须所有事件均授信
			100,
			FALSE);
		if (v1 == WSA_WAIT_TIMEOUT)
		{
			continue;
		}
		//监听事件授信，发生了FD_ACCEPT或者FD_CLOSE事件
		//将事件转换成网络事件
		v1 = WSAEnumNetworkEvents(This->m_ListenSocket, This->m_ListenEventHandle, &NetWorkEvents);
		if (v1 == SOCKET_ERROR)
		{
			break;
		}
		if (NetWorkEvents.lNetworkEvents==FD_ACCEPT)
		{
			if (NetWorkEvents.iErrorCode[FD_ACCEPT_BIT] == 0)
			{
				//处理客户上线请求
				This->OnAccept();
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}


	return 0;
}
BOOL  _CIOCPServer::InitializeIOCP()
{
	//创建完成端口
	m_CompletionPortHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_CompletionPortHandle == NULL)
	{
		return FALSE;
	}
	//线程池
	SYSTEM_INFO SystemInfo;
	//获得CPU中的核数
	GetSystemInfo(&SystemInfo);
	m_ThreadPoolMin = 1;
	m_ThreadPoolMax = SystemInfo.dwNumberOfProcessors * 2;
	ULONG WorkThreadCount = 2;
	HANDLE WorkThreadHandle = NULL;
	for (int i = 0;i < WorkThreadCount;i++)
	{
		WorkThreadHandle = CreateThread(NULL,
			0,
			(LPTHREAD_START_ROUTINE)WorkThreadProcedure,
			this,
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
void _CIOCPServer::OnAccept()
{
	//保存上线用户IP地址
	SOCKET ClientSocket = INVALID_SOCKET;
	SOCKADDR_IN ClientAddress = { 0 };
	int ClientAddressLength = sizeof(SOCKADDR_IN);
	//生成一个与监听套接字通信的通信套接字
	ClientSocket = accept(m_ListenSocket, (sockaddr*)&ClientAddress, &ClientAddressLength);
	if (ClientSocket == SOCKET_ERROR)
	{
		return;
	}
	//为每个到达的信号创建一个与其关联的用户上下背景文
	PCONTEXT_OBJECT ContextObject = AllocateContextObject();
	if (ContextObject == NULL)
	{
		closesocket(ClientSocket);
		ClientSocket =INVALID_SOCKET;
		return;
	}

	ContextObject->ClientSocket = ClientSocket;
	//关联内存
	ContextObject->WsaReceiveBuffer.buf = ContextObject->BufferData;
	ContextObject->WsaReceiveBuffer.len = sizeof(ContextObject->BufferData);
	//将通信套接字与完成端口句柄关联
	HANDLE Handle = CreateIoCompletionPort((HANDLE)ClientSocket, m_CompletionPortHandle, (ULONG_PTR)ContextObject, 0);
	if (Handle != m_CompletionPortHandle)
	{
		delete ContextObject;
		ContextObject = NULL;
		if (ClientSocket!=INVALID_SOCKET)
		{
			closesocket(ClientSocket);
			ClientSocket =INVALID_SOCKET;
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
	if (setsockopt(ContextObject->ClientSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&IsOk, sizeof(IsOk)) != 0)
	{

	}
	//设置超时信息
	tcp_keepalive KeepAlive;
	KeepAlive.onoff = 1;		//启用保活
	KeepAlive.keepalivetime = m_KeepAliveTime;	//超过时长未发送数据，则发送探测包
	KeepAlive.keepaliveinterval = 1000 * 10;	//若探测没有回应，重试间隔为10s
	WSAIoctl
	(
		ContextObject->ClientSocket,
		SIO_KEEPALIVE_VALS,
		&KeepAlive,
		sizeof(KeepAlive),
		NULL,
		0,
		(DWORD*)IsOk,
		0,
		NULL
	);

	_CCriticalSection CriticalSection(m_CriticalSection);	//进入临界区
	m_ConnectionContextObjectCList.AddTail(ContextObject);	//插入到上线用户列表中

	OVERLAPPEDEX *OverlappedEx = new OVERLAPPEDEX(IO_INITIALIZE);
	IsOk = FALSE;
	//向完成端口中投递一个初始化请求
	//工作线程会等待完成端口的完成状态
	IsOk = PostQueuedCompletionStatus(m_CompletionPortHandle, 0, (ULONG_PTR)ContextObject, &OverlappedEx->m_Overlapped);
	if (!IsOk&&GetLastError() != ERROR_IO_PENDING)
	{
		//投递失败
		RemoveContextObject(ContextObject);
		return;
	}
	//该用户完成上线请求
	//服务器向其投递PostRecv请求
	PostRecv(ContextObject);

}
VOID _CIOCPServer::PostRecv(PCONTEXT_OBJECT ContextObject)
{
	//向用户投递一个接受数据的异步请求
	//若该请求得到完成(用户发送数据)
	//工作线程会响应并处理
	OVERLAPPEDEX *OverlappedEx = new OVERLAPPEDEX(IO_RECEIVE);
	DWORD ReturnLength;
	ULONG Flag = MSG_PARTIAL;
	int IsOk = WSARecv(ContextObject->ClientSocket,
		&ContextObject->WsaReceiveBuffer,	//接收数据的内存
		1,
		&ReturnLength,
		&Flag,
		&OverlappedEx->m_Overlapped,
		NULL);
	if (IsOk == SOCKET_ERROR&&WSAGetLastError() != WSA_IO_PENDING)
	{
		//请求发送失败
		RemoveContextObject(ContextObject);
	}
}
PCONTEXT_OBJECT _CIOCPServer::AllocateContextObject()
{
	//使用内存池
	PCONTEXT_OBJECT ContextObject = NULL;
	_CCriticalSection CriticalSection(m_CriticalSection);	//进入临界区
	if (m_FreeContextObjectCList.IsEmpty() == FALSE)	//判断内存池是否为空
	{
		//内存池非空，从中取出一块内存
		ContextObject = m_FreeContextObjectCList.RemoveHead();
	}
	else
	{
		//内存池为空，申请一块内存
		ContextObject = new CONTEXT_OBJECT;
	}
	if (ContextObject != NULL)
	{
		ContextObject->InitMember();
	}
	return ContextObject;
}
VOID _CIOCPServer::RemoveContextObject(PCONTEXT_OBJECT ContextObject)
{
	_CCriticalSection CriticalSection(m_CriticalSection);	//进入临界区
	//在内存中查找该用户的上下背景文
	if (m_ConnectionContextObjectCList.Find(ContextObject))
	{
		//取消当前套接字的异步请求，未完成的异步请求全部立即取消
		CancelIo((HANDLE)ContextObject->ClientSocket);
		closesocket(ContextObject->ClientSocket);
		ContextObject->ClientSocket = INVALID_SOCKET;
		while (!HasOverlappedIoCompleted((LPOVERLAPPED)ContextObject))
		{
			Sleep(1);
		}
		//将内存回收到内存池中
		MoveContextObjectToFreePool(ContextObject);
	}
}
VOID _CIOCPServer::MoveContextObjectToFreePool(PCONTEXT_OBJECT ContextObject)
{
	_CCriticalSection CriticalSection(m_CriticalSection);	//进入临界区
	POSITION Position = m_ConnectionContextObjectCList.Find(ContextObject);
	if (Position)
	{

		memset(ContextObject->BufferData, 0, PACKET_LENGTH);
		m_FreeContextObjectCList.AddTail(ContextObject);	//回收至内存池
		m_ConnectionContextObjectCList.RemoveAt(Position);	//从上线用户列表中移除
	}
}
DWORD _CIOCPServer::WorkThreadProcedure(LPVOID ParameterData)
{
	_CIOCPServer* This = (_CIOCPServer*)ParameterData;
	HANDLE CompletionPortHandle = This->m_CompletionPortHandle;
	DWORD ReturnLength = 0;		//完成的请求的数据量
	PCONTEXT_OBJECT ContextObject = NULL;
	LPOVERLAPPED Overlapped = NULL;
	OVERLAPPEDEX* OverlappedEx = NULL;
	//原子锁
	InterlockedIncrement(&This->m_CurrentThreadCount);	//++
	InterlockedIncrement(&This->m_BusyThreadCount);		//++
	ULONG BusyThread = 0;
	BOOL v1 = FALSE;
	while (This->m_IsWorking == FALSE)
	{
		InterlockedDecrement(&This->m_BusyThreadCount); //--
		
		//该函数是阻塞函数，若该函数返回，说明有请求得到了完成
		BOOL IsOk = GetQueuedCompletionStatus(CompletionPortHandle,
			&ReturnLength,
			(PULONG_PTR)&ContextObject,
			&Overlapped,
			60000);

		DWORD LastError = GetLastError();
		OverlappedEx = CONTAINING_RECORD(Overlapped, OVERLAPPEDEX, m_Overlapped);
		BusyThread = InterlockedIncrement(&This->m_BusyThreadCount);
		if (!IsOk && LastError != WAIT_TIMEOUT)
		{
			if (ContextObject && This->m_IsWorking == FALSE && ReturnLength == 0)
			{
				//对方套接字关闭
				This->RemoveContextObject(ContextObject);
			}
			continue;
		}
		//查看线程池状态
		if (!v1)
		{
			//判断工作线程是否足够
			if (BusyThread == This->m_CurrentThreadCount)
			{
				if (BusyThread < This->m_ThreadPoolMax)
				{
					if (ContextObject != NULL)
					{
						//工作线程不够，分配一个新线程到线程池
						HANDLE ThreadHandle = CreateThread(NULL,
							0,
							(LPTHREAD_START_ROUTINE)WorkThreadProcedure,
							This,
							0,
							NULL);
						InterlockedIncrement(&This->m_WorkThreadCount);
						CloseHandle(ThreadHandle);
					}
				}
			}
			//判断工作线程是否过多
			if (!IsOk && LastError == WAIT_TIMEOUT)
			{
				if (ContextObject == NULL)
				{
					if (This->m_CurrentThreadCount > This->m_ThreadPoolMin)
					{
						//工作线程过多，从线程池销毁一个工作线程
						break;
					}
					v1 == TRUE;
				}
			}
		}
		if (!v1)
		{
			//请求得到完成(IO_INITIALIZE IO_RECEIVE)
			if (IsOk && OverlappedEx != NULL && ContextObject != NULL)
			{
				try 
				{
					//请求处理函数
					This->HandIO(OverlappedEx->m_PacketType, ContextObject, ReturnLength);
					//没有销毁内存
					ContextObject = NULL;
				}
				catch(...){}
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
BOOL _CIOCPServer::HandIO(PACKET_TYPE PacketType, PCONTEXT_OBJECT ContextObject, DWORD BufferLength)
{
	BOOL v1 = FALSE;
	if (PacketType == IO_INITIALIZE)
	{
		v1 = OnInitializing(ContextObject, BufferLength);
	}
	if (PacketType == IO_RECEIVE)
	{
		//v1 = OnReceiving(ContextObject, BufferLength);
	}
	if (PacketType == IO_SEND)
	{
		//数据发送完成后
		//v1 = OnPostSend(ContextObject, BufferLength);
	}
	return v1;
}
BOOL _CIOCPServer::OnInitializing(PCONTEXT_OBJECT ContextObject, DWORD BufferLength)
{
	MessageBox(NULL, "初始化成功", "初始化成功", NULL);
	return TRUE;
}