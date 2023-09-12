// DlgRemoteMessageDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "DlgRemoteMessageDlg.h"
#include "afxdialogex.h"


// CDlgRemoteMessageDlg �Ի���

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


// CDlgRemoteMessageDlg ��Ϣ�������


BOOL CDlgRemoteMessageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	SetIcon(m_IconHwnd, FALSE);

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	BYTE IsToken = CLIENT_GO_ON;
	m_IOCPServer->OnPrepareSending(m_ContextObject, &IsToken, sizeof(BYTE));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


void CDlgRemoteMessageDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	CancelIo((HANDLE)m_ContextObject->ClientSocket);
	closesocket(m_ContextObject->ClientSocket);
	CDialog::OnClose();
}
	

BOOL CDlgRemoteMessageDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	if (pMsg->message == WM_KEYDOWN)
	{
		// ����VK_ESCAPE��VK_DELETE
		if (pMsg->wParam == VK_ESCAPE)
		{
			PostMessage(WM_CLOSE);
			return true;
		}

		//����ǿɱ༭��Ļس���
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

	int BufferLength = m_CEdit_Dialog_Remote_Message.GetWindowTextLength();   //EditBox �ϻ�����ݳ���

	if (!BufferLength)
	{
		return;
	}
	CString v1;
	//EditBox �ϻ������
	m_CEdit_Dialog_Remote_Message.GetWindowText(v1);            
	char* BufferData = new char[BufferLength];
	memset(BufferData, 0, sizeof(char)*BufferLength);
	sprintf(BufferData, "%s", v1.GetBuffer(0));
	//EditBox �ϵ��������
	m_CEdit_Dialog_Remote_Message.SetWindowText(NULL);         
	//���Լ��ڴ��е����ݷ���
	m_IOCPServer->OnPrepareSending(m_ContextObject, (LPBYTE)BufferData, strlen(BufferData)); 


}