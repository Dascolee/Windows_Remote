#pragma once
#include"Manager.h"

class CRemoteMessageManager : public CManager
{
public:
	CRemoteMessageManager(_CIOCPClient* IOCPClient);
	~CRemoteMessageManager();
	VOID CRemoteMessageManager::HandleIO(PBYTE BufferData, ULONG BufferLength);


};

int CALLBACK DialogProcedure(HWND DlgHwnd, unsigned int Msg,WPARAM wParam, LPARAM lParam);
VOID OnInitDialog(HWND DlgHwnd);
VOID OnTimerDialog(HWND DlgHwnd);  //Ê±ÖÓ»Øµ÷
