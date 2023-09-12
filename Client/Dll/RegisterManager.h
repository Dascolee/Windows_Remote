#pragma once
#include "Manager.h"
#include "Common.h"
#include "Manager.h"
#include "RegisterOperation.h"


class CRegisterManager :
	public CManager
{
public:
	CRegisterManager(_CIOCPClient* IOCPClient);
	~CRegisterManager();
	VOID CRegisterManager::HandleIO(PBYTE BufferData, ULONG BufferLength);
	VOID CRegisterManager::Find(char IsToken, char *szPath);

};

