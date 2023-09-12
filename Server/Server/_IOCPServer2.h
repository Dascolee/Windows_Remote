#pragma once
#include <WinSock2.h>
#include <windows.h>
#include<mstcpip.h>
#include <iostream>
#include"_CriticalSection.h"
using namespace std;
#pragma comment(lib,"WS2_32.lib")

#define PACKET_LENGTH 0x2000
typedef struct _CONTEXT_OBJECT_
{
	SOCKET ClientSocket;		//服务器使用该套接字与客户端通信
	WSABUF WsaReceiveBuffer;	//用于接收用户发送的数据，数据不在该结构本身，该结构保存指向BufferData的指针
	WSABUF WsaSendBuffer;		//用于向客户发送数据
	char BufferData[PACKET_LENGTH];//存放数据
	VOID InitMember()
	{
		ClientSocket = INVALID_SOCKET;
		memset(BufferData, 0, sizeof(char)*PACKET_LENGTH);
		memset(&WsaReceiveBuffer, 0, sizeof(WSABUF));
	}
}CONTEXT_OBJECT,*PCONTEXT_OBJECT;
typedef CList<PCONTEXT_OBJECT> CONTEXT_OBJECT_LIST;

enum PACKET_TYPE
{
	IO_INITIALIZE,
	IO_RECEIVE,
	IO_SEND,
	IO_IDLE
};

class OVERLAPPEDEX
{
public:
	OVERLAPPED m_Overlapped;
	PACKET_TYPE m_PacketType;
	OVERLAPPEDEX(PACKET_TYPE PacketType)
	{
		ZeroMemory(this, sizeof(OVERLAPPEDEX));
		m_PacketType = PacketType;
	}
};
class _CIOCPServer
{
public:
	_CIOCPServer();
	~_CIOCPServer();
	BOOL ServerRun(USHORT ListenPort);
	static DWORD WINAPI ListenThreadProcedure(LPVOID ParameterData);
	BOOL InitializeIOCP();
	static DWORD WINAPI WorkThreadProcedure(LPVOID ParameterData);
	BOOL HandIO(PACKET_TYPE PacketType, PCONTEXT_OBJECT ContextObject, DWORD BufferLength);
	BOOL OnInitializing(PCONTEXT_OBJECT ContextObject, DWORD BufferLength);
	void OnAccept();
	VOID PostRecv(PCONTEXT_OBJECT ContextObject);
	
	//上下背景文
	PCONTEXT_OBJECT AllocateContextObject();
	VOID RemoveContextObject(PCONTEXT_OBJECT ContextObject);
	VOID MoveContextObjectToFreePool(PCONTEXT_OBJECT ContextObject);
private:
	SOCKET m_ListenSocket;			//监听套接字
	HANDLE m_ListenThreadHandle;	//监听线程
	HANDLE m_ListenEventHandle;		//监听事件
	HANDLE m_KillEventHandle;		//监听线程退出循环
	BOOL   m_IsWorking;				//工作线程是否在工作
	CRITICAL_SECTION m_CriticalSection;	//关键代码段(用临界区来进行线程同步)
	HANDLE m_CompletionPortHandle;	//完成端口句柄
	CONTEXT_OBJECT_LIST m_FreeContextObjectCList;	//内存池列表
	CONTEXT_OBJECT_LIST m_ConnectionContextObjectCList;	//上线用户的上下背景文列表
	LONG m_KeepAliveTime;			//保活机制

	ULONG m_ThreadPoolMin;			//线程池中的线程最小值
	ULONG m_ThreadPoolMax;			//线程池中的线程最大值
	ULONG m_WorkThreadCount;		//工作线程计数
	ULONG m_CurrentThreadCount;
	ULONG m_BusyThreadCount;
};

