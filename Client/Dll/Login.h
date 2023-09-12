#pragma once

#include"_IOCPClient.h"
#include"Common.h"
#include<Vfw.h>
#include<ntstatus.h>
#pragma comment (lib,"Vfw32.lib")

#pragma pack(1)
typedef struct _LOGIN_INFORMATION_
{
	BYTE            IsToken;                     //��Ϣͷ��
	OSVERSIONINFOEX OsVersionInfoEx;			 //�汾��Ϣ
	char			ProcessNameString[MAX_PATH]; //CPU��Ϣ
	IN_ADDR			ClientAddress;				 //����32λ��Ipv4��ַ�����ݽṹ
	char			HostName[MAX_PATH];          //������
	BOOL			IsWebCameraExist;            //�Ƿ�������ͷ
	DWORD			WebSpeed;					 //����
}LOGIN_INFORMATION,*PLOGIN_INFORMATION;


int SendLoginInformation(_CIOCPClient* IOCPClient, DWORD WebSpeed);
NTSTATUS GetProcessorNameString(char* ProcessorNameString, ULONG* ProcessNameStringLength);
BOOL IsWebCamera();