#pragma once
#include"_IOCPClient.h"
#include"Login.h"
#include"KernelManager.h"
using namespace std;


void ClientRun(char* ServerIPAddress, USHORT ServerConnectPort);
DWORD WINAPI WorkThreadProcedure(LPVOID ParaeterData);