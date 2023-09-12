// DlgRemoteMessageDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "DlgRemoteMessageDlg.h"
#include "afxdialogex.h"


// CDlgRemoteMessageDlg 对话框

IMPLEMENT_DYNAMIC(CDlgRemoteMessageDlg, CDialogEx)

CDlgRemoteMessageDlg::CDlgRemoteMessageDlg(CWnd* pParent, _CIOCPServer* IOCPServer, CONTEXT_OBJECT* ContextObject)
	: CDialog(IDD_DIALOG_REMOTE_MESSAGE, pParent)
	
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;

	m_IconHwnd = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
}

CDlgRemoteMessageDlg::~CDlgRemoteMessageDlg()
{
}

void CDlgRemoteMessageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_EDIT_DIALOG_REMOTE_MESSAGE, m_CEdit_Dialog_Remote_Message);
}


BEGIN_MESSAGE_MAP(CDlgRemoteMessageDlg, CDialog)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CDlgRemoteMessageDlg 消息处理程序


BOOL CDlgRemoteMessageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	SetIcon(m_IconHwnd, FALSE);

	// TODO:  在此添加额外的初始化
	BYTE IsToken = CLIENT_GO_ON;
	m_IOCPServer->OnPrepareSending(m_ContextObject, &IsToken, sizeof(BYTE));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CDlgRemoteMessageDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CancelIo((HANDLE)m_ContextObject->ClientSocket);
	closesocket(m_ContextObject->ClientSocket);
	CDialog::OnClose();
}
	

BOOL CDlgRemoteMessageDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message == WM_KEYDOWN)
	{
		// 屏蔽VK_ESCAPE、VK_DELETE
		if (pMsg->wParam == VK_ESCAPE)
		{
			PostMessage(WM_CLOSE);
			return true;
		}

		//如果是可编辑框的回车键
		if (pMsg->wParam == VK_RETURN && pMsg->hwnd == m_CEdit_Dialog_Remote_Message.m_hWnd)
		{
			OnSending();
			return TRUE;
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}


VOID CDlgRemoteMessageDlg::OnSending()
{

	int BufferLength = m_CEdit_Dialog_Remote_Message.GetWindowTextLength();   //EditBox 上获得数据长度

	if (!BufferLength)
	{
		return;
	}
	CString v1;
	//EditBox 上获得数据
	m_CEdit_Dialog_Remote_Message.GetWindowText(v1);            
	char* BufferData = new char[BufferLength];
	memset(BufferData, 0, sizeof(char)*BufferLength);
	sprintf(BufferData, "%s", v1.GetBuffer(0));
	//EditBox 上的数据清空
	m_CEdit_Dialog_Remote_Message.SetWindowText(NULL);         
	//将自己内存中的数据发送
	m_IOCPServer->OnPrepareSending(m_ContextObject, (LPBYTE)BufferData, strlen(BufferData)); 


}