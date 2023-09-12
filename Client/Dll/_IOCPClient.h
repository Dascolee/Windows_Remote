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
	SOCKET ClientSocket; //Server使用该套接字与目标Client进行通讯
	WSABUF ReceiveWsaBuffer;  //接受用户到达数据该结构本体没有内存  与m_BufferDatag关联
	char BufferData[PACKET_LENGTH];
	//_CArray m_ReceivedCompressdBufferData;  //接收到的压缩的数据m_BufferData拷贝到该成员当中
	//_CArray m_ReceivedDecompressdBufferData;//接收到的压缩的数据进行的解压后的数据
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
	SOCKET m_ClientSocket;   //与服务器进程链接的套接字字
	HANDLE m_WorkThreadHandle; //工作线程句柄
	BOOL m_IsReceiving;        //是否接受数据;
	char m_PacketHeaderFlag[PACKET_FLAG_LENGHT];   //数据包匹配
	HANDLE m_EventHandle;      //事件

	_CArray m_SendCompressdBufferData;    //压缩后发送数据
	_CArray m_ReceivedCompressdBufferData;  //接收到的压缩的数据m_BufferData拷贝到该成员当中
	_CArray m_ReceivedDecompressdBufferData;//接收到的压缩的数据进行的解压后的数据


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
		//面向对象多态((多态)抽象类指针指向实例对象地址)
		m_Manager = Manager;
	};
	VOID _CIOCPClient::WaitingForEvent()
	{
		//避免对象销毁
		WaitForSingleObject(m_EventHandle, INFINITE);
	};

};
