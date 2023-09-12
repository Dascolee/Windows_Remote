// DlgWindowManager.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "DlgWindowManager.h"
#include "afxdialogex.h"


// CDlgWindowManager �Ի���

IMPLEMENT_DYNAMIC(CDlgWindowManager, CDialog)

CDlgWindowManager::CDlgWindowManager(CWnd* pParent, _CIOCPServer* IOCPServer, CONTEXT_OBJECT* ContextObject)
	: CDialog(IDD_DIALOG_WINDOW_MANAGER, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;

	m_IconHwnd = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));

	m_BufferData = (char *)(m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(0));
}

CDlgWindowManager::~CDlgWindowManager()
{

}

void CDlgWindowManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_WINDOW_MANAGER_SHOW, m_CListCtrl_Dialog_Window_Manager_Show);
}


BEGIN_MESSAGE_MAP(CDlgWindowManager, CDialog)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST_WINDOW_MANAGER_SHOW, &CDlgWindowManager::OnNMRClickListWindowManagerShow)
	ON_COMMAND(ID_BUTTON_MENU_WINDOW_MANAGER_LIST_REFRESH, &CDlgWindowManager::OnButtonMenuWindowManagerListRefresh)
END_MESSAGE_MAP()


// CDlgWindowManager ��Ϣ�������
void CDlgWindowManager::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (m_ContextObject != NULL)
	{
		CancelIo((HANDLE)m_ContextObject->ClientSocket);
		closesocket(m_ContextObject->ClientSocket);
		m_ContextObject->DialogHandle = NULL;
		m_ContextObject->DialogID = 0;
	}
	CDialog::OnClose();
}


BOOL CDlgWindowManager::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��ÿͻ���IP SetWindowText

	CString v1;
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int ClientAddressLength = sizeof(ClientAddress);
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength); //�õ����ӵ�ip 
	v1.Format("\\\\%s - Զ�̴��ڹ���", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//���öԻ������
	SetWindowText(v1);
	//���öԻ����ϵ�����
	/*LOGFONT Logfont;
	CFont* v2 = m_CListCtrl_Dialog_Window_Manager_Show.GetFont();
	v2->GetLogFont(&Logfont);
	//��������
	Logfont.lfHeight = Logfont.lfHeight*1.3;
	Logfont.lfWeight = Logfont.lfWeight*1.3;
	CFont v3;
	v3.CreateFontIndirect(&Logfont);
	m_CListCtrl_Dialog_Window_Manager_Show.SetFont(&v3);
	v3.Detach();*/

	m_CListCtrl_Dialog_Window_Manager_Show.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);

	switch (m_BufferData[0])
	{
	case CLIENT_WINDOWS_MANAGER_REPLY:
	{
		m_CListCtrl_Dialog_Window_Manager_Show.InsertColumn(0, "���ھ��", LVCFMT_LEFT, 60);
		m_CListCtrl_Dialog_Window_Manager_Show.InsertColumn(1, "��������", LVCFMT_LEFT, 300);
		m_CListCtrl_Dialog_Window_Manager_Show.InsertColumn(2, "���ڴ���", LVCFMT_LEFT, 120);
		ShowWindowsList();   //���ڵ�һ������������Ϣ��������Ž��̵��������԰�������ʾ���б���\0\0


		break;
	}
	default:
		break;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}

void CDlgWindowManager::WindowHandleIO(void)
{
	if (m_ContextObject == NULL)
	{
		return;
	}

	switch (m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(0)[0])
	{
	case CLIENT_WINDOWS_MANAGER_REPLY:
	{
		ShowWindowsList();
		break;
	}
	
	}
}

void CDlgWindowManager::ShowWindowsList(void)
{
	char *BufferData = (char *)(m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1));
	DWORD	Offset = 0;
	char	*WindowTitleName = NULL;
	bool bDel = false;
	/*do
	{
	isDel=false;
	for (int j=0;j<m_list_process.GetItemCount();j++)  //
	{
	CString temp=m_list_process.GetItemText(j,2);
	CString restr="����";                                //show  show  show  show show
	//if (temp!=restr)                                     //
	{
	m_list_process.DeleteItem(j);
	isDel=true;
	break;
	}
	}
	} while (isDel);*/

	m_CListCtrl_Dialog_Window_Manager_Show.DeleteAllItems();
	CString	v1;
	int i;
	for (i = 0; Offset <m_ContextObject->m_ReceivedDecompressdBufferData.GetBufferLength() - 1; i++)
	{
		LPDWORD	Hwnd = LPDWORD(BufferData + Offset);   //���ھ��
		WindowTitleName = (char *)BufferData + Offset + sizeof(DWORD);   //���ڱ���    
		v1.Format("%5u", *Hwnd);
		m_CListCtrl_Dialog_Window_Manager_Show.InsertItem(i, v1);
		m_CListCtrl_Dialog_Window_Manager_Show.SetItemText(i, 1, WindowTitleName);
		m_CListCtrl_Dialog_Window_Manager_Show.SetItemText(i, 2, "��ʾ"); //(d) ������״̬��ʾΪ "��ʾ"
														// ItemData Ϊ���ھ��
		m_CListCtrl_Dialog_Window_Manager_Show.SetItemData(i, *Hwnd);  //(d)   
		Offset += sizeof(DWORD) + lstrlen(WindowTitleName) + 1;
	}
	v1.Format("���ڸ���[%d]", i);   //�޸�CtrlList 
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT;
	lvc.pszText = v1.GetBuffer(0);
	lvc.cchTextMax = v1.GetLength();
	m_CListCtrl_Dialog_Window_Manager_Show.SetColumn(1, &lvc);
}



void CDlgWindowManager::OnNMRClickListWindowManagerShow(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CMenu Menu;
	Menu.LoadMenu(IDR_MENU_WINDOW_MANAGER_LIST_RINGHT);
	CMenu* SubMenu = Menu.GetSubMenu(0);
	//������λ��
	CPoint Point;
	GetCursorPos(&Point);
	SetForegroundWindow();
	SubMenu->TrackPopupMenu(TPM_LEFTBUTTON | TPM_RIGHTBUTTON, Point.x, Point.y, this, NULL);

	*pResult = 0;
}


void CDlgWindowManager::OnButtonMenuWindowManagerListRefresh()
{
	// TODO: �ڴ���������������
	BYTE IsToken = CLIENT_WINDOWS_MANAGER_REFRESH;
	m_IOCPServer->OnPrepareSending(m_ContextObject, &IsToken, 1);
}
