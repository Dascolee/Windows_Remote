#include "stdafx.h"
#include "_IOCPServer.h"


_CIOCPServer::_CIOCPServer()
{
	WSADATA  WsaData = { 0 };

	if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)
	{
		return;
	}
	m_ListenSocket = INVALID_SOCKET;				//��ʼ�������׽���
	m_ListenThreadHandle = NULL;					//��ʼ�������߳�
	m_KillEventHandle = NULL;
	m_ListenEventHandle = WSA_INVALID_EVENT;		//��ʼ�������¼�(�¼�ģ��)
	InitializeCriticalSection(&m_CriticalSection);
	m_CompletionPortHandle = INVALID_HANDLE_VALUE;	//��ʼ����ɶ˿ھ��
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
	SetEvent(m_KillEventHandle);	//�����¼���ʹ�����߳������˳�ѭ��
	WaitForSingleObject(m_ListenThreadHandle, INFINITE);	//�ȴ������߳��˳�

	//�رռ����׽��֡������¼��������ɶ˿ھ��
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
	//�����ٽ���
	DeleteCriticalSection(&m_CriticalSection);
	//�����׽��ֿ�
	WSACleanup();
}

BOOL _CIOCPServer::ServerRun(USHORT ListenPort)
{
	//���������׽���
	m_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);//ʹ�����׽��ֿ��Դ����첽�ص�����
	if (m_ListenSocket == INVALID_SOCKET)
	{
		return FALSE;
	}
	//���������¼�
	m_ListenEventHandle = WSACreateEvent();	//����һ�����ֶ��������Ե���δ���ŵ��¼�
	if (m_ListenEventHandle == WSA_INVALID_EVENT)
	{
		goto Error;
	}
	//�����˳������̵߳��¼�
	m_KillEventHandle = CreateEvent(NULL,FALSE,FALSE,NULL);	
	if (m_KillEventHandle == NULL)
	{
		goto Error;
	}
	//�¼�ѡ��ģ��
	BOOL IsOk = WSAEventSelect(m_ListenSocket, m_ListenEventHandle, FD_ACCEPT | FD_CLOSE);	//�������׽������¼�����
	if (IsOk == SOCKET_ERROR)
	{
		goto Error;
	}
	//��ʼ��Server������
	SOCKADDR_IN ServerAddress;
	ServerAddress.sin_port = htons(ListenPort);	//�������˿�ת��Ϊ�����ֽ�˳��
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_addr.s_addr = INADDR_ANY;
	//�󶨼����׽���
	IsOk = bind(m_ListenSocket, (sockaddr*)&ServerAddress, sizeof(ServerAddress));
	if (IsOk == SOCKET_ERROR)
	{
		goto Error;
	}
	//����
	IsOk = listen(m_ListenSocket,SOMAXCONN);
	if (IsOk == SOCKET_ERROR)
	{
		goto Error;
	}
	//���������߳�
	m_ListenThreadHandle = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)ListenThreadProcedure,
		this,		//���̻߳ص������д���this������ص������������Ա
		0,
		NULL);
	if (m_ListenThreadHandle == INVALID_HANDLE_VALUE)
	{
		goto Error;
	}
	//��ʼ��IOCP(�첽�������ʱ)
	//������ɶ˿�
	//���������߳�
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
		EventIndex = WaitForSingleObject(This->m_KillEventHandle, 100);	//�¼����ŷ���ֵΪ0
		EventIndex = EventIndex - WAIT_OBJECT_0;
		if (EventIndex == 0)
		{
			//��������������
			break;
		}
		DWORD v1;
		//�ȴ������¼�����
		v1 = WSAWaitForMultipleEvents(1,
			&This->m_ListenEventHandle,
			FALSE,	//���ж���¼���FALSEΪֻҪ��һ���¼����ţ���������	TRUEΪ���������¼�������
			100,
			FALSE);
		if (v1 == WSA_WAIT_TIMEOUT)
		{
			continue;
		}
		//�����¼����ţ�������FD_ACCEPT����FD_CLOSE�¼�
		//���¼�ת���������¼�
		v1 = WSAEnumNetworkEvents(This->m_ListenSocket, This->m_ListenEventHandle, &NetWorkEvents);
		if (v1 == SOCKET_ERROR)
		{
			break;
		}
		if (NetWorkEvents.lNetworkEvents==FD_ACCEPT)
		{
			if (NetWorkEvents.iErrorCode[FD_ACCEPT_BIT] == 0)
			{
				//����ͻ���������
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
	//������ɶ˿�
	m_CompletionPortHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_CompletionPortHandle == NULL)
	{
		return FALSE;
	}
	//�̳߳�
	SYSTEM_INFO SystemInfo;
	//���CPU�еĺ���
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
	//���������û�IP��ַ
	SOCKET ClientSocket = INVALID_SOCKET;
	SOCKADDR_IN ClientAddress = { 0 };
	int ClientAddressLength = sizeof(SOCKADDR_IN);
	//����һ��������׽���ͨ�ŵ�ͨ���׽���
	ClientSocket = accept(m_ListenSocket, (sockaddr*)&ClientAddress, &ClientAddressLength);
	if (ClientSocket == SOCKET_ERROR)
	{
		return;
	}
	//Ϊÿ��������źŴ���һ������������û����±�����
	PCONTEXT_OBJECT ContextObject = AllocateContextObject();
	if (ContextObject == NULL)
	{
		closesocket(ClientSocket);
		ClientSocket =INVALID_SOCKET;
		return;
	}

	ContextObject->ClientSocket = ClientSocket;
	//�����ڴ�
	ContextObject->WsaReceiveBuffer.buf = ContextObject->BufferData;
	ContextObject->WsaReceiveBuffer.len = sizeof(ContextObject->BufferData);
	//��ͨ���׽�������ɶ˿ھ������
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
	
	//�������

	//Ϊ�˱�����ͻ��˵����ӣ���2Сʱ���ڴ��׽ӿڵ���һ����û�����ݽ�������TCP�Զ����ͻ���һ�����ݰ�
	//�������ͻ��������Ƿ����

	//���ͻ��˷������߻��߶ϵ�ȷ������Ͽ���������������û������SO_KEEPALIVEѡ�
	//���һֱ���رո��׽��֣����뿪��������ơ�Ĭ�ϼ��ʱ��Ϊ2Сʱ

	//�����׽��ֵ�ѡ� setsockopt  ����������� SO_KEEPALIVE
	m_KeepAliveTime = 3;
	BOOL IsOk = TRUE;
	if (setsockopt(ContextObject->ClientSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&IsOk, sizeof(IsOk)) != 0)
	{

	}
	//���ó�ʱ��Ϣ
	tcp_keepalive KeepAlive;
	KeepAlive.onoff = 1;		//���ñ���
	KeepAlive.keepalivetime = m_KeepAliveTime;	//����ʱ��δ�������ݣ�����̽���
	KeepAlive.keepaliveinterval = 1000 * 10;	//��̽��û�л�Ӧ�����Լ��Ϊ10s
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

	_CCriticalSection CriticalSection(m_CriticalSection);	//�����ٽ���
	m_ConnectionContextObjectCList.AddTail(ContextObject);	//���뵽�����û��б���

	OVERLAPPEDEX *OverlappedEx = new OVERLAPPEDEX(IO_INITIALIZE);
	IsOk = FALSE;
	//����ɶ˿���Ͷ��һ����ʼ������
	//�����̻߳�ȴ���ɶ˿ڵ����״̬
	IsOk = PostQueuedCompletionStatus(m_CompletionPortHandle, 0, (ULONG_PTR)ContextObject, &OverlappedEx->m_Overlapped);
	if (!IsOk&&GetLastError() != ERROR_IO_PENDING)
	{
		//Ͷ��ʧ��
		RemoveContextObject(ContextObject);
		return;
	}
	//���û������������
	//����������Ͷ��PostRecv����
	PostRecv(ContextObject);

}
VOID _CIOCPServer::PostRecv(PCONTEXT_OBJECT ContextObject)
{
	//���û�Ͷ��һ���������ݵ��첽����
	//��������õ����(�û���������)
	//�����̻߳���Ӧ������
	OVERLAPPEDEX *OverlappedEx = new OVERLAPPEDEX(IO_RECEIVE);
	DWORD ReturnLength;
	ULONG Flag = MSG_PARTIAL;
	int IsOk = WSARecv(ContextObject->ClientSocket,
		&ContextObject->WsaReceiveBuffer,	//�������ݵ��ڴ�
		1,
		&ReturnLength,
		&Flag,
		&OverlappedEx->m_Overlapped,
		NULL);
	if (IsOk == SOCKET_ERROR&&WSAGetLastError() != WSA_IO_PENDING)
	{
		//������ʧ��
		RemoveContextObject(ContextObject);
	}
}
PCONTEXT_OBJECT _CIOCPServer::AllocateContextObject()
{
	//ʹ���ڴ��
	PCONTEXT_OBJECT ContextObject = NULL;
	_CCriticalSection CriticalSection(m_CriticalSection);	//�����ٽ���
	if (m_FreeContextObjectCList.IsEmpty() == FALSE)	//�ж��ڴ���Ƿ�Ϊ��
	{
		//�ڴ�طǿգ�����ȡ��һ���ڴ�
		ContextObject = m_FreeContextObjectCList.RemoveHead();
	}
	else
	{
		//�ڴ��Ϊ�գ�����һ���ڴ�
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
	_CCriticalSection CriticalSection(m_CriticalSection);	//�����ٽ���
	//���ڴ��в��Ҹ��û������±�����
	if (m_ConnectionContextObjectCList.Find(ContextObject))
	{
		//ȡ����ǰ�׽��ֵ��첽����δ��ɵ��첽����ȫ������ȡ��
		CancelIo((HANDLE)ContextObject->ClientSocket);
		closesocket(ContextObject->ClientSocket);
		ContextObject->ClientSocket = INVALID_SOCKET;
		while (!HasOverlappedIoCompleted((LPOVERLAPPED)ContextObject))
		{
			Sleep(1);
		}
		//���ڴ���յ��ڴ����
		MoveContextObjectToFreePool(ContextObject);
	}
}
VOID _CIOCPServer::MoveContextObjectToFreePool(PCONTEXT_OBJECT ContextObject)
{
	_CCriticalSection CriticalSection(m_CriticalSection);	//�����ٽ���
	POSITION Position = m_ConnectionContextObjectCList.Find(ContextObject);
	if (Position)
	{

		memset(ContextObject->BufferData, 0, PACKET_LENGTH);
		m_FreeContextObjectCList.AddTail(ContextObject);	//�������ڴ��
		m_ConnectionContextObjectCList.RemoveAt(Position);	//�������û��б����Ƴ�
	}
}
DWORD _CIOCPServer::WorkThreadProcedure(LPVOID ParameterData)
{
	_CIOCPServer* This = (_CIOCPServer*)ParameterData;
	HANDLE CompletionPortHandle = This->m_CompletionPortHandle;
	DWORD ReturnLength = 0;		//��ɵ������������
	PCONTEXT_OBJECT ContextObject = NULL;
	LPOVERLAPPED Overlapped = NULL;
	OVERLAPPEDEX* OverlappedEx = NULL;
	//ԭ����
	InterlockedIncrement(&This->m_CurrentThreadCount);	//++
	InterlockedIncrement(&This->m_BusyThreadCount);		//++
	ULONG BusyThread = 0;
	BOOL v1 = FALSE;
	while (This->m_IsWorking == FALSE)
	{
		InterlockedDecrement(&This->m_BusyThreadCount); //--
		
		//�ú������������������ú������أ�˵��������õ������
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
				//�Է��׽��ֹر�
				This->RemoveContextObject(ContextObject);
			}
			continue;
		}
		//�鿴�̳߳�״̬
		if (!v1)
		{
			//�жϹ����߳��Ƿ��㹻
			if (BusyThread == This->m_CurrentThreadCount)
			{
				if (BusyThread < This->m_ThreadPoolMax)
				{
					if (ContextObject != NULL)
					{
						//�����̲߳���������һ�����̵߳��̳߳�
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
			//�жϹ����߳��Ƿ����
			if (!IsOk && LastError == WAIT_TIMEOUT)
			{
				if (ContextObject == NULL)
				{
					if (This->m_CurrentThreadCount > This->m_ThreadPoolMin)
					{
						//�����̹߳��࣬���̳߳�����һ�������߳�
						break;
					}
					v1 == TRUE;
				}
			}
		}
		if (!v1)
		{
			//����õ����(IO_INITIALIZE IO_RECEIVE)
			if (IsOk && OverlappedEx != NULL && ContextObject != NULL)
			{
				try 
				{
					//��������
					This->HandIO(OverlappedEx->m_PacketType, ContextObject, ReturnLength);
					//û�������ڴ�
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
		//���ݷ�����ɺ�
		//v1 = OnPostSend(ContextObject, BufferLength);
	}
	return v1;
}
BOOL _CIOCPServer::OnInitializing(PCONTEXT_OBJECT ContextObject, DWORD BufferLength)
{
	MessageBox(NULL, "��ʼ���ɹ�", "��ʼ���ɹ�", NULL);
	return TRUE;
}