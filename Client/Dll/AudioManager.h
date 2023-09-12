#pragma once
#include "Common.h"
#include "Manager.h"
#include "Audio.h"
class CAudioManager :
	public CManager
{
public:
	CAudioManager(_CIOCPClient* IOCPClient);
	~CAudioManager();
	BOOL CAudioManager::Initialize();
	static DWORD WorkThreadProdurce(LPVOID lParam);
	VOID CAudioManager::HandleIO(PBYTE BufferData, ULONG BufferLength);
	int CAudioManager::SendRecordBuffer();
public:
	BOOL m_bIsWorking;
	CAudio* m_AudioObject;
	HANDLE m_WorkThreadHandle = NULL;
};

