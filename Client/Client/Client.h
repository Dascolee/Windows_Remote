#pragma once
//#include<WinSock2.h>
//#include<mstcpip.h>
#include<iostream>
#include<windows.h>
#pragma comment (lib,"WS2_32.lib")

using namespace std;

typedef void(*LPFN_CLIENTRUN)(char* ServerIPAddress, USHORT ServerConnectPort);
