// DlgCmdManager.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "DlgCmdManager.h"
#include "afxdialogex.h"


// CDlgCmdManager �Ի���

IMPLEMENT_DYNAMIC(CDlgCmdManager, CDialog)

CDlgCmdManager::CDlgCmdManager(CWnd * pParent, _CIOCPServer * IOCPServer, CONTEXT_OBJECT * ContextObject)
	: CDialog(IDD_DIALOG_CMD_MANAGER, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;

	m_IconHwnd = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));

	m_BufferData = (char *)(m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(0));
}

CDlgCmdManager::~CDlgCmdManager()
{
}

void CDlgCmdManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EIDT_CMD_MANAGER_MAIN, m_CEidt_Dialog_Cmd_Manager_Main);
}


BEGIN_MESSAGE_MAP(CDlgCmdManager, CDialog)
	ON_WM_CLOSE()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CDlgCmdManager ��Ϣ�������


BOOL CDlgCmdManager::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString v1;
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int ClientAddressLength = sizeof(ClientAddress);
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength); //�õ����ӵ�ip 
	v1.Format("%s - Զ��CMD����", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//���öԻ������
	SetWindowText(v1);

	BYTE IsToken = CLIENT_GO_ON;
	m_IOCPServer->OnPrepareSending(m_ContextObject,&IsToken,sizeof(BYTE));


	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


void CDlgCmdManager::OnClose()
{
	
	if (m_ContextObject!=NULL)
	{
		CancelIo((HANDLE)m_ContextObject->ClientSocket);
		closesocket(m_ContextObject->ClientSocket);
		m_ContextObject->DialogHandle = NULL;
		m_ContextObject->DialogID = 0;
	}
	CDialog::OnClose();
}

void CDlgCmdManager::WindowHandleIO()
{
	if (m_ContextObject == NULL)
	{
		return;
	}
	ShowCmdData();
	m_ShowDataLength = m_CEidt_Dialog_Cmd_Manager_Main.GetWindowTextLength();

}

void CDlgCmdManager::ShowCmdData()
{
	m_ContextObject->m_ReceivedDecompressdBufferData.WriteBuffer((LPBYTE)"", 1);           
	//�ӱ����ƶ�������������Ҫ����һ��\0
	CString v1 = (char*)m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(0);
	//������е����� ���� \0															  
	//�滻��ԭ���Ļ��з�  ����cmd �Ļ���ͬw32�µı༭�ؼ��Ļ��з���һ��   ���еĻس�����   
	v1.Replace("\n", "\r\n");
	//�õ���ǰ���ڵ��ַ�����
	int	BufferLength = m_CEidt_Dialog_Cmd_Manager_Main.GetWindowTextLength();    
	//hello//1.txt//2.txt//dir\r\n

	 //����궨λ����λ�ò�ѡ��ָ���������ַ�  Ҳ����ĩβ ��Ϊ�ӱ��ض��������� Ҫ��ʾ�� ���ǵ� ��ǰ���ݵĺ���
	m_CEidt_Dialog_Cmd_Manager_Main.SetSel(BufferLength, BufferLength);
	//�ô��ݹ����������滻����λ�õ��ַ�    //��ʾ
	m_CEidt_Dialog_Cmd_Manager_Main.ReplaceSel(v1);
	//���µõ��ַ��Ĵ�С
	m_911 = m_CEidt_Dialog_Cmd_Manager_Main.GetWindowTextLength();   //C:\>dir 
	//����ע�⵽��������ʹ��Զ���ն�ʱ �����͵�ÿһ�������� ����һ�����з�  ����һ���س�										
	//Ҫ�ҵ�����س��Ĵ������Ǿ�Ҫ��PreTranslateMessage�����Ķ���  
}

BOOL CDlgCmdManager::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		//��Backspace��delete����
		if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_DELETE)
		{
			return TRUE;
		}
		//��Edit�ϰ��س�
		if (pMsg->wParam == VK_RETURN&&pMsg->hwnd == m_CEidt_Dialog_Cmd_Manager_Main.m_hWnd)
		{
			//�õ��������ݴ�С
			int BufferLength = m_CEidt_Dialog_Cmd_Manager_Main.GetWindowTextLength();
			CString BufferData;

			//�õ����ھ�������
			m_CEidt_Dialog_Cmd_Manager_Main.GetWindowText(BufferData);
			//����س�����
			BufferData += "\r\n";

			m_IOCPServer->OnPrepareSending(m_ContextObject, 
				(LPBYTE)BufferData.GetBuffer(0) + m_911, BufferData.GetLength() - m_911);
			m_911 = m_CEidt_Dialog_Cmd_Manager_Main.GetWindowTextLength();
		}

		//Backspace����ɾ����ʼ���� ����VK_BACK
		if (pMsg->wParam == VK_BACK &&pMsg->hwnd == m_CEidt_Dialog_Cmd_Manager_Main.m_hWnd)
		{
			if (m_CEidt_Dialog_Cmd_Manager_Main.GetWindowTextLength() <= m_ShowDataLength)
			{
				return TRUE;
			}
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}


HBRUSH CDlgCmdManager::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ�����
	if ((pWnd->GetDlgCtrlID() == IDC_EIDT_CMD_MANAGER_MAIN) && (nCtlColor == CTLCOLOR_EDIT))
	{
		COLORREF clr = RGB(255, 255, 255);
		pDC->SetTextColor(clr);   //���ð�ɫ���ı�
		clr = RGB(0, 0, 0);
		pDC->SetBkColor(clr);     //���ú�ɫ�ı���
		return CreateSolidBrush(clr);  //��ΪԼ�������ر���ɫ��Ӧ��ˢ�Ӿ��
	}
	else
	{
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
	// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������
	return hbr;
}
