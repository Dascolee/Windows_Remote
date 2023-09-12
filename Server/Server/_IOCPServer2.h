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
	SOCKET ClientSocket;		//������ʹ�ø��׽�����ͻ���ͨ��
	WSABUF WsaReceiveBuffer;	//���ڽ����û����͵����ݣ����ݲ��ڸýṹ�����ýṹ����ָ��BufferData��ָ��
	WSABUF WsaSendBuffer;		//������ͻ���������
	char BufferData[PACKET_LENGTH];//�������
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
	
	//���±�����
	PCONTEXT_OBJECT AllocateContextObject();
	VOID RemoveContextObject(PCONTEXT_OBJECT ContextObject);
	VOID MoveContextObjectToFreePool(PCONTEXT_OBJECT ContextObject);
private:
	SOCKET m_ListenSocket;			//�����׽���
	HANDLE m_ListenThreadHandle;	//�����߳�
	HANDLE m_ListenEventHandle;		//�����¼�
	HANDLE m_KillEventHandle;		//�����߳��˳�ѭ��
	BOOL   m_IsWorking;				//�����߳��Ƿ��ڹ���
	CRITICAL_SECTION m_CriticalSection;	//�ؼ������(���ٽ����������߳�ͬ��)
	HANDLE m_CompletionPortHandle;	//��ɶ˿ھ��
	CONTEXT_OBJECT_LIST m_FreeContextObjectCList;	//�ڴ���б�
	CONTEXT_OBJECT_LIST m_ConnectionContextObjectCList;	//�����û������±������б�
	LONG m_KeepAliveTime;			//�������

	ULONG m_ThreadPoolMin;			//�̳߳��е��߳���Сֵ
	ULONG m_ThreadPoolMax;			//�̳߳��е��߳����ֵ
	ULONG m_WorkThreadCount;		//�����̼߳���
	ULONG m_CurrentThreadCount;
	ULONG m_BusyThreadCount;
};

