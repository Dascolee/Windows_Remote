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
	SOCKET ClientSocket; //Serverʹ�ø��׽�����Ŀ��Client����ͨѶ
	WSABUF ReceiveWsaBuffer;  //�����û��������ݸýṹ����û���ڴ�  ��m_BufferDatag����
	WSABUF  SendWsaBuffer;      //��ͻ��˷���һ������  û���ڴ�ó�ԱҪ��m_SendCompressdBufferData����
	char BufferData[PACKET_LENGTH];
	_CArray m_ReceivedCompressdBufferData;  //���յ���ѹ��������m_BufferData�������ó�Ա����
	_CArray m_ReceivedDecompressdBufferData;//���յ���ѹ�������ݽ��еĽ�ѹ�������
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

//������һ��������
typedef CList<PCONTEXT_OBJECT> CONTEXT_OBJECT_LIST;

class _CIOCPServer
{
public:
	_CIOCPServer();
	~_CIOCPServer();
private:
	SOCKET m_ListenSocket;   //�����׽���
	HANDLE m_ListenThreadHandle;  
	HANDLE m_ListenEventHandle;//�����¼�
	HANDLE m_KillEventHandle;  //�����߳��˳�ѭ��
	BOOL m_IsWorking;          //�����̵߳�ѭ����־
	CRITICAL_SECTION m_CriticalSection;//�ؼ������
	HANDLE m_CompletionPortHandle;    //�ӿ���ɶ˿�
	
	CONTEXT_OBJECT_LIST m_FreeContextObjectList;//�ڴ��ģ��
	CONTEXT_OBJECT_LIST m_ConnectionContextObjectList; //�����û����±�����
	long m_KeepAliveTime;                       //�������
	ULONG m_ThreadPoolMin;						//
	ULONG m_ThreadPoolMax;
	ULONG m_WorkThreadCount;
	ULONG m_CurrentThreadCount;
	ULONG m_BusyThreadCount;
	char m_PacketHeaderFlag[PACKET_FLAG_LENGHT];   //���ݰ�ƥ��

	LPFN_WINDOWNOTIFYPROCEDURE m_WindowNotifyProcedure;//�����ɴ������д��ݹ����ĺ���ָ��
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
	//Ͷ�������ݵ��첽����
	VOID _CIOCPServer::PostReceive(CONTEXT_OBJECT * ContextObject);
	//�����õ������ݶ˵�����
	BOOL _CIOCPServer::OnReceiving(PCONTEXT_OBJECT ContextObject, DWORD BuffferLenght);
	//׼���������ݰ����ͻ���
	VOID _CIOCPServer::OnPrepareSending(CONTEXT_OBJECT* ContextObject, PBYTE BufferData, ULONG BufferLength);
	//�����ݷ��͵��ͻ���
	BOOL _CIOCPServer::OnPostSending(CONTEXT_OBJECT* ContextObject, ULONG BufferLength);
	
};

