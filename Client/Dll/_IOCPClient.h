#pragma once
#include<WinSock2.h>
#include<mstcpip.h>
#include<Windows.h>
#include<iostream>
#include"Buffer.h"
#include"zconf.h"
#include"zlib.h"
#include "Manager.h"

using namespace std;
#pragma comment (lib,"WS2_32.lib")

#define PACKET_LENGTH 0x2000
#define PACKET_FLAG_LENGHT 5
#define PACKET_HEADER_LENGHT 13
#define MAX_SEND_BUFFER 0x2000
#define MAX_RECV_BUFFER 0x2000

/*
enum PACKET_TYPE
{
	IO_INITIALIZE,
	IO_RECEIVE,
	IO_SEND,
	IO_IDLE
};

typedef struct _CONTEXT_OBJECT_
{
	SOCKET ClientSocket; //Serverʹ�ø��׽�����Ŀ��Client����ͨѶ
	WSABUF ReceiveWsaBuffer;  //�����û��������ݸýṹ����û���ڴ�  ��m_BufferDatag����
	char BufferData[PACKET_LENGTH];
	//_CArray m_ReceivedCompressdBufferData;  //���յ���ѹ��������m_BufferData�������ó�Ա����
	//_CArray m_ReceivedDecompressdBufferData;//���յ���ѹ�������ݽ��еĽ�ѹ�������
	//_CArray m_SendCompressdBufferData;

	VOID InitMember()
	{
		ClientSocket = INVALID_SOCKET;
		memset(BufferData, 0, sizeof(char)*PACKET_LENGTH);
		memset(&ReceiveWsaBuffer, 0, sizeof(WSABUF));
	}
}CONTEXT_OBJECT,*PCONTEXT_OBJECT;

class OVERLAPPEDEX
{
public:
	OVERLAPPED   m_Overlapped;
	PACKET_TYPE   m_PackType;

	OVERLAPPEDEX(PACKET_TYPE PackType)
	{
		ZeroMemory(this, sizeof(OVERLAPPEDEX));
		m_PackType = PackType;
	}
};
*/
class CManager;
class _CIOCPClient
{
public:
	_CIOCPClient();
	~_CIOCPClient();
public:
	SOCKET m_ClientSocket;   //��������������ӵ��׽�����
	HANDLE m_WorkThreadHandle; //�����߳̾��
	BOOL m_IsReceiving;        //�Ƿ��������;
	char m_PacketHeaderFlag[PACKET_FLAG_LENGHT];   //���ݰ�ƥ��
	HANDLE m_EventHandle;      //�¼�

	_CArray m_SendCompressdBufferData;    //ѹ����������
	_CArray m_ReceivedCompressdBufferData;  //���յ���ѹ��������m_BufferData�������ó�Ա����
	_CArray m_ReceivedDecompressdBufferData;//���յ���ѹ�������ݽ��еĽ�ѹ�������


	CManager* m_Manager;

public:
	BOOL _CIOCPClient::ConnectServer(char* ServerIPAddress, USHORT ServerConnectPort);
	static DWORD WINAPI WorkThreadProcedure(LPVOID ParaemterData);
	BOOL _CIOCPClient::IsReceiving()
	{
		return m_IsReceiving;
	};
	VOID _CIOCPClient::DisConnect();
	int _CIOCPClient::OnSending(char *BufferData, ULONG BufferLength);
	BOOL _CIOCPClient::SendWithSpilt(char *BufferData, ULONG BufferLength, ULONG SplitLenght);
	VOID _CIOCPClient::OnReceiving(char* BufferData, ULONG BufferLength);
	VOID _CIOCPClient::setManagerCallBack(class CManager* Manager)
	{
		//��������̬((��̬)������ָ��ָ��ʵ�������ַ)
		m_Manager = Manager;
	};
	VOID _CIOCPClient::WaitingForEvent()
	{
		//�����������
		WaitForSingleObject(m_EventHandle, INFINITE);
	};

};
