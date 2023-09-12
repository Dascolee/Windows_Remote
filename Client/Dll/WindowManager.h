#pragma once
#include "Manager.h"
#include "Common.h"

class CWindowManager :
	public CManager
{
public:

	CWindowManager(_CIOCPClient* IOCPClient);
	~CWindowManager();

	VOID CWindowManager::HandleIO(PBYTE BufferData, ULONG BufferLength);


	static BOOL CALLBACK CWindowManager::EnumWindowProcedure(HWND Hwnd, LPARAM ParameterData);


	static DWORD WINAPI ReceiveProceduce(LPVOID ParameterData);
	//·¢ËÍ´°¿Ú
	void CWindowManager::SendClientWindowList();

};

