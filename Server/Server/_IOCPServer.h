#pragma once
#include<WinSock2.h>
#include<mstcpip.h>
#include<Windows.h>
#include<iostream>
#include "_CriticalSection.h"
#include"zlib.h"
#include"zconf.h"
using namespace std;
#pragma comment (lib,"WS2_32.lib")
#include"Buffer.h"


#define MAX_RECV_BUFFER  0x2000
#define MAX_SEND_BUFFER  0x2000

#define PACKET_LENGTH 0x2000
#define PACKET_FLAG_LENGHT 5
#define PACKET_HEADER_LENGHT 13

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
	WSABUF  SendWsaBuffer;      //向客户端发送一个数据  没有内存该成员要与m_SendCompressdBufferData关联
	char BufferData[PACKET_LENGTH];
	_CArray m_ReceivedCompressdBufferData;  //接收到的压缩的数据m_BufferData拷贝到该成员当中
	_CArray m_ReceivedDecompressdBufferData;//接收到的压缩的数据进行的解压后的数据
	_CArray m_SendCompressdBufferData;
	int DialogID;
	HANDLE DialogHandle;
	VOID InitMember()
	{
		ClientSocket = INVALID_SOCKET;
		memset(BufferData, 0, sizeof(char)*PACKET_LENGTH);
		memset(&ReceiveWsaBuffer, 0, sizeof(WSABUF));
		memset(&SendWsaBuffer, 0, sizeof(WSABUF));
		DialogID = 0;
		DialogHandle = NULL;
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
typedef void(CALLBACK *LPFN_WINDOWNOTIFYPROCEDURE)(PCONTEXT_OBJECT ContextObject);

//定义了一个数据型
typedef CList<PCONTEXT_OBJECT> CONTEXT_OBJECT_LIST;

class _CIOCPServer
{
public:
	_CIOCPServer();
	~_CIOCPServer();
private:
	SOCKET m_ListenSocket;   //监听套接字
	HANDLE m_ListenThreadHandle;  
	HANDLE m_ListenEventHandle;//监听事件
	HANDLE m_KillEventHandle;  //监听线程退出循环
	BOOL m_IsWorking;          //工作线程的循环标志
	CRITICAL_SECTION m_CriticalSection;//关键代码段
	HANDLE m_CompletionPortHandle;    //加快完成端口
	
	CONTEXT_OBJECT_LIST m_FreeContextObjectList;//内存池模板
	CONTEXT_OBJECT_LIST m_ConnectionContextObjectList; //上下用户上下背景文
	long m_KeepAliveTime;                       //保活机制
	ULONG m_ThreadPoolMin;						//
	ULONG m_ThreadPoolMax;
	ULONG m_WorkThreadCount;
	ULONG m_CurrentThreadCount;
	ULONG m_BusyThreadCount;
	char m_PacketHeaderFlag[PACKET_FLAG_LENGHT];   //数据包匹配

	LPFN_WINDOWNOTIFYPROCEDURE m_WindowNotifyProcedure;//保存由窗口类中传递过来的函数指针
public:
	BOOL _CIOCPServer::ServerRun(USHORT ListenPort,LPFN_WINDOWNOTIFYPROCEDURE WindowNotifyProcedure);
	static DWORD WINAPI ListenThreadProcedure(LPVOID ParameterData);
	void  _CIOCPServer::OnAccept();
	PCONTEXT_OBJECT _CIOCPServer::AllocateContextObject();
	VOID _CIOCPServer::RemoteContextObject(CONTEXT_OBJECT* ContextObject);
	VOID _CIOCPServer::MoveContextObjectToFreePool(CONTEXT_OBJECT * ContextObject);
	BOOL _CIOCPServer::InitializeIOCP(VOID);
	static DWORD WINAPI WorThreadProcedure(LPVOID ParameterData);
	BOOL _CIOCPServer::HandleIO(PACKET_TYPE PacketType, PCONTEXT_OBJECT ContextObject, DWORD BuffferLenght);
	BOOL _CIOCPServer::OnInitializing(PCONTEXT_OBJECT ContextObject, DWORD BuffferLenght);
	//投递了数据的异步请求
	VOID _CIOCPServer::PostReceive(CONTEXT_OBJECT * ContextObject);
	//真正得到了数据端的数据
	BOOL _CIOCPServer::OnReceiving(PCONTEXT_OBJECT ContextObject, DWORD BuffferLenght);
	//准备发送数据包到客户端
	VOID _CIOCPServer::OnPrepareSending(CONTEXT_OBJECT* ContextObject, PBYTE BufferData, ULONG BufferLength);
	//将数据发送到客户端
	BOOL _CIOCPServer::OnPostSending(CONTEXT_OBJECT* ContextObject, ULONG BufferLength);
	
};

