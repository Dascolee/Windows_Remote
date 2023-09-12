#include "stdafx.h"
#include "_IOCPServer.h"

_CIOCPServer::_CIOCPServer()
{
	//��ʼ���׽���
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

//����������DIALOG�ر�
_CIOCPServer::~_CIOCPServer()
{

	Sleep(1);
	//�����¼�ʹ�������˳������̵߳�ѭ��
	SetEvent(m_KillEventHandle);
	//�ȴ������̵߳��˳�
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

	//�����˳��̵߳��¼�
	m_KillEventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	if (m_KillEventHandle == NULL)
	{
		return FALSE;
	}
	//����һ�������׽���
	m_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (m_ListenSocket == INVALID_SOCKET)
	{
		return FALSE;
	}
	//����һ�������¼�
	m_ListenEventHandle = WSACreateEvent();
	if (m_ListenEventHandle == WSA_INVALID_EVENT)
	{
		closesocket(m_ListenSocket);
		m_ListenSocket = INVALID_SOCKET;
		return FALSE;
	}
	//�¼�ѡ��ģ��
	BOOL IsOk = WSAEventSelect(m_ListenSocket, //�����׽������¼����й���������FD_CLOSE��FD_ACCEPT����
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
	//��ʼ��Server������
	SOCKADDR_IN ServerAddress;
	ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_port = htons(ListenPort);
	ServerAddress.sin_addr.S_un.S_addr = INADDR_ANY;
	//���׽���
	if (bind(m_ListenSocket, (sockaddr*)&ServerAddress, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		WSACloseEvent(m_ListenEventHandle);
		m_ListenEventHandle = WSA_INVALID_EVENT;
		closesocket(m_ListenSocket);
		m_ListenSocket = INVALID_SOCKET;
		return FALSE;
	}
	//����
	IsOk = listen(m_ListenSocket, SOMAXCONN);
	if (IsOk == SOCKET_ERROR)
	{
		WSACloseEvent(m_ListenEventHandle);
		m_ListenEventHandle = WSA_INVALID_EVENT;
		closesocket(m_ListenSocket);
		m_ListenSocket = INVALID_SOCKET;
		return FALSE;
	}
	//���������߳�
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
	
	//��ʼ��IOCP(���첽������ɵ�ʱ��)
	//����һ����ɶ˿�
	//���������̣߳��غ���ɶ˿��ϵȴ��첽�������ɣ�
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
		//�ȴ������¼����ţ������׽������ţ� �¼�ģ��
		v1 = WSAWaitForMultipleEvents(1, &This->m_ListenEventHandle,
			FALSE, 100, FALSE);

		if (v1==WSA_WAIT_TIMEOUT)
		{
			//���¼�û������
			continue;
		}
		//������FD_ACCEPT��FD_CLOSE�¼�
		//����¼��������� �͸ý����¼�ת����һ�������¼������ж�
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
				//���ͻ�������������д���
				//�����һ�������������Ǿͽ���     �������д���
				
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
	//���������û�
	SOCKET ClientSocket = INVALID_SOCKET;
	SOCKADDR_IN ClientAddress = { 0 };
	int  ClientAddressLength = sizeof(SOCKADDR_IN);
	//����ͨ��һ���׽���������һ�����ź�ͨѶ���׽��� ��Ϊ�ͻ�������һ���׽��֣�
	ClientSocket = accept(m_ListenSocket, (sockaddr*)&ClientAddress,
		&ClientAddressLength);
	if (ClientSocket==SOCKET_ERROR)
	{
		return;
	}
	//����������Ϊÿһ��������ź�ά����һ����֮����������ݽṹ������Ϊ�û������±�����
	
	PCONTEXT_OBJECT ContextObject = AllocateContextObject();

	if (ContextObject == NULL)
	{
		closesocket(ClientSocket);
		ClientSocket = INVALID_SOCKET;
		return;
	}

	//��Ա��ֵ
	ContextObject->ClientSocket = ClientSocket;
	//�����ڴ�
	ContextObject->ReceiveWsaBuffer.buf = (char*)(ContextObject->BufferData);
	ContextObject->ReceiveWsaBuffer.len = sizeof(ContextObject->BufferData);

	//�����ɵ��׽�������ɶ˿ھ�������
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

	//�������

	//Ϊ�˱�����ͻ��˵����ӣ���2Сʱ���ڴ��׽ӿڵ���һ����û�����ݽ�������TCP�Զ����ͻ���һ�����ݰ�
	//�������ͻ��������Ƿ����
	//���ͻ��˷������߻��߶ϵ�ȷ������Ͽ���������������û������SO_KEEPALIVEѡ�
	//���һֱ���رո��׽��֣����뿪��������ơ�Ĭ�ϼ��ʱ��Ϊ2Сʱ

	//�����׽��ֵ�ѡ� setsockopt  ����������� SO_KEEPALIVE
	m_KeepAliveTime = 3;
	BOOL IsOk = TRUE;
	if (setsockopt(ContextObject->ClientSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&IsOk, sizeof(IsOk))!= 0)
	{
		MessageBox(NULL,"0","setsockopt",NULL);
	}
	//���ó�ʱ��ϸ��Ϣ
	tcp_keepalive v1;
	v1.onoff = 1;		//���ñ���
	v1.keepalivetime = m_KeepAliveTime;	//����ʱ��δ�������ݣ�����̽���
	v1.keepaliveinterval = 1000 * 10;	//��̽��û�л�Ӧ�����Լ��Ϊ10s
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
	m_ConnectionContextObjectList.AddTail(ContextObject);  //���뵽���ǵ��ڴ��б���

	OVERLAPPEDEX *OverlappedEx = new OVERLAPPEDEX(IO_INITIALIZE);

	BOOL IsOK = FALSE;
	//����ɶ˿�Ͷ��һ������
	//�����߳�
	IsOK = PostQueuedCompletionStatus(m_CompletionPortHandle,
		0, (ULONG_PTR)ContextObject, &OverlappedEx->m_Overlapped);//�����߳� һ��֪ͨ
	
	//int Lasterror = GetLastError();
	/*
	if ((!IsOk) && (GetLastError() != ERROR_IO_PENDING))  //���Ͷ��ʧ��
	{
		RemoteContextObject(ContextObject);
		return;
	}*/


	//�������û�����������ߵ�����
	//����������û�Ͷ��PostRecv����
	PostReceive(ContextObject);

}

PCONTEXT_OBJECT _CIOCPServer::AllocateContextObject()
{
	PCONTEXT_OBJECT ContextObject = NULL;
	//�����ٽ���
	_CCriticalSection CriticalSection(m_CriticalSection);
	//EnterCriticalSection(&m_FreeContextObjectListLock);

	//�ж��ڴ���Ƿ�Ϊ��
	if (m_FreeContextObjectList.IsEmpty() == FALSE)
	{
		ContextObject = m_FreeContextObjectList.RemoveHead();
	}
	else
	{
		// �����µ��ڴ�
		ContextObject = new CONTEXT_OBJECT;
	}

	if (ContextObject != NULL)
	{
		ContextObject-> InitMember();  //��ʼ����Ա����
	}
	return ContextObject;
}

VOID _CIOCPServer::RemoteContextObject(CONTEXT_OBJECT * ContextObject)
{
	_CCriticalSection CriticalSection(m_CriticalSection);
	if (m_ConnectionContextObjectList.Find(ContextObject))
	{
		CancelIo((HANDLE)ContextObject->ClientSocket);
		//�ر��׽���
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
	//�����ǵĸ����ߵ��û���Ͷ��һ���������ݵ�����
	//����û��ĵ�һ�����ݰ�����Ҳ���Ǳ����Ƶĵ�¼���󵽴����ǵĹ����߳�(����õ����)
	//�����߳�(�غ�����ɶ˿�)����Ӧƽ����HANdleIO����
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
		//�����ʹ���
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
	//�̳߳�
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	
	m_ThreadPoolMin = 1;
	m_ThreadPoolMax = SystemInfo.dwNumberOfProcessors * 2;
	
	//ProcessorLowThreadsHold
	ULONG WorkThreadCount = 2;
	//= m_ThreadPoolMax;����
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
		//�ú�������������  ����ú�������˵��������õ������
 		BOOL IsOk = GetQueuedCompletionStatus(
			CompletionPortHandle,
			&ReturnLength,
			(PULONG_PTR)&ContextObject,
			&Overlapped,
			60000);
		
/*		OVERLAPPEDEX *OverlappedEx = new OVERLAPPEDEX(IO_INITIALIZE);
		BOOL IsOK = FALSE;
		//����ɶ˿�Ͷ��һ������
		//�����߳�
		IsOK = PostQueuedCompletionStatus(m_CompletionPortHandle,
		0, (ULONG_PTR)ContextObject, &OverlappedEx->m_Overlapped);//�����߳� һ��֪ͨ*/
		
		
		DWORD LastError = GetLastError();
		OverlappedEx = CONTAINING_RECORD(Overlapped, OVERLAPPEDEX, m_Overlapped);
		
		v3 = InterlockedIncrement(&This->m_BusyThreadCount);//3
															//1
		
		if (!IsOk&&LastError != WAIT_TIMEOUT)
		{
			if (ContextObject&&This->m_IsWorking == FALSE&&ReturnLength == 0)
			{
				//���Է����׻��Ʒ����˹ر�
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
			//����һ���̴߳��̳߳�
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
			//�ɹ� ����õ������������  IO_RECEIVE, IO_INITIALIZE   PostQueuedCompletionStatus �ͻ�WSARecv��
			//ReturnValue GetQueuedCompletionStatus����ֵ
			if (IsOk&&OverlappedEx!=NULL&&ContextObject != NULL)
			{
				try
				{
					//������ɴ�����
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
		v1 = OnInitializing(ContextObject, BuffferLenght);//OnAccept ������Ͷ�ݵ�����
	}
	if (IO_RECEIVE == PacketType)//WSAResc ���ض˴��͵�����
	{
		v1 = OnReceiving(ContextObject, BuffferLenght);
	}
	if (IO_SEND == PacketType)  //���ض���˴�������
	{
		//�Ѿ������ݰ�������ɺ� ��������
		//�������ݵ��ͻ���
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
			//MessageBox(NULL, "�ر��׽���", "guanbi", 0);
			RemoteContextObject(ContextObject);
			return FALSE;
		}
		//���ӵ������ݿ�����m_ReceivedCompressdBufferData
		ContextObject->m_ReceivedCompressdBufferData.
			WriteBuffer((PBYTE)ContextObject->BufferData, BuffferLenght);

		//��ȡ���ݰ���ͷ�������ݰ���ͷ���ǲ�����ѹ���ģ�
		while (ContextObject->m_ReceivedCompressdBufferData.GetBufferLength()>PACKET_HEADER_LENGHT)
		{
			//�������ݰ�ͷ����־
			char v1[PACKET_FLAG_LENGHT] = { 0 };
			//�������ݰ�ͷ����־
			CopyMemory(v1, ContextObject->m_ReceivedCompressdBufferData.GetBuffer(), PACKET_FLAG_LENGHT);
			//У�����ݰ���ͷ����־
			
			if (memcmp(m_PacketHeaderFlag, v1, PACKET_FLAG_LENGHT) != 0)
			{
				throw "Bad Buffer";
			}
			//������ݰ��ܴ�С
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
				//�����ݰ��л��ѹ���������
				ContextObject->m_ReceivedCompressdBufferData.ReadBuffer(CompressedData, CompressedLenght);
				//��ѹ��
				int IsOK = uncompress(DecompressedData,&DecompressdLenght, CompressedData, CompressedLenght);
				int Error = GetLastError();

				if (IsOK == Z_OK)
				{
					ContextObject->m_ReceivedDecompressdBufferData.ClearBuffer();
					ContextObject->m_ReceivedCompressdBufferData.ClearBuffer();

					//������ʵ����
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
		//��һ�ε��첽����e�Ѿ��õ��������Ͷ���µ��첽����
		PostReceive(ContextObject);  //Ͷ���µ��첽����

	}
	catch (...)
	{
		ContextObject->m_ReceivedCompressdBufferData.ClearBuffer();
		ContextObject->m_ReceivedDecompressdBufferData.ClearBuffer();
		PostReceive(ContextObject);  //Ͷ���µ��첽����
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

			//�������ݰ�ͷ��
			ULONG PackTotalLength = CompressedLength + PACKET_HEADER_LENGHT;
			ContextObject->m_SendCompressdBufferData.WriteBuffer((LPBYTE)m_PacketHeaderFlag, PACKET_FLAG_LENGHT);
			ContextObject->m_SendCompressdBufferData.WriteBuffer((PBYTE)&PackTotalLength, sizeof(ULONG));
			ContextObject->m_SendCompressdBufferData.WriteBuffer((PBYTE)&BufferLength, sizeof(ULONG));
			ContextObject->m_SendCompressdBufferData.WriteBuffer(CompressedData, CompressedLength);
			//[xx][PackTotalLength][BufferLength][.....(��ʵ����)]

			delete[] CompressedData;
		}
		OVERLAPPEDEX* OverlappedEx = new OVERLAPPEDEX(IO_SEND);
		//����������͵���ɶ˿�
		PostQueuedCompletionStatus(m_CompletionPortHandle, 0, (DWORD)ContextObject, &OverlappedEx->m_Overlapped);
	}
	catch (...) {}
}

BOOL _CIOCPServer::OnPostSending(CONTEXT_OBJECT* ContextObject, ULONG BufferLength)
{
	try
	{
		DWORD Flags = MSG_PARTIAL;
		//����ɵ����ݴ����ݽṹ��ȥ��				
		ContextObject->m_SendCompressdBufferData.RemoveComletedBuffer(BufferLength);  
		//�жϻ��ж���������Ҫ����
		if (ContextObject->m_SendCompressdBufferData.GetBufferLength() == 0)
		{
			//�����Ѿ��������
			ContextObject->m_SendCompressdBufferData.ClearBuffer();
			return true;		                             //�ߵ�����˵�����ǵ�����������ȫ����
		}
		else
		{
			//������������//����û�����  ���Ǽ���Ͷ�� ��������
			OVERLAPPEDEX * OverlappedEx = new OVERLAPPEDEX(IO_SEND);          
			//���ʣ������ݺͳ���   
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
