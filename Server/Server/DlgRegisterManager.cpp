// DlgRegisterManager.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "DlgRegisterManager.h"
#include "afxdialogex.h"


// CDlgRegisterManager �Ի���

IMPLEMENT_DYNAMIC(CDlgRegisterManager, CDialog)

CDlgRegisterManager::CDlgRegisterManager(CWnd* pParent, _CIOCPServer* IOCPServer, CONTEXT_OBJECT* ContextObject)
	: CDialog(IDD_DIALOG_REGISTER_MANAGER, pParent)
{
	m_IsEnable = FALSE;
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;
	m_IconHwnd = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
}

CDlgRegisterManager::~CDlgRegisterManager()
{
}

void CDlgRegisterManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_REGISTER_MANAGER_SHOW, m_CTreeCtrl_Dialog_Register_Manager_Show);
	DDX_Control(pDX, IDC_LIST_REGISTER_MANAGER_SHOW, m_CListCtrl_Dialog_Register_Manager_Show);
}

BEGIN_MESSAGE_MAP(CDlgRegisterManager, CDialog)
	ON_WM_CLOSE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_REGISTER_MANAGER_SHOW, &CDlgRegisterManager::OnTvnSelchangedTreeRegisterManagerShow)
END_MESSAGE_MAP()

BOOL CDlgRegisterManager::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_IconHwnd, FALSE);

	CString v1;
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int ClientAddressLength = sizeof(ClientAddress);
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength); //�õ����ӵ�ip 
	v1.Format("\\\\%s - Զ��ע������", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//���öԻ������
	SetWindowText(v1);

	//�����οؼ���ͼ��
	m_ImageList_Tree.Create(18, 18, ILC_COLOR16, 10, 0);
	m_IconHwnd = (HICON)::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_FATHER), IMAGE_ICON, 18, 18, 0);
	m_ImageList_Tree.Add(m_IconHwnd);
	m_IconHwnd = (HICON)::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_FILE), IMAGE_ICON, 18, 18, 0);
	m_ImageList_Tree.Add(m_IconHwnd);

	m_CTreeCtrl_Dialog_Register_Manager_Show.SetImageList(&m_ImageList_Tree,TVSIL_NORMAL);
	
	//��Ŀ¼
	m_hRoot = m_CTreeCtrl_Dialog_Register_Manager_Show.InsertItem("ע������", 0, 0, 0, 0);
	//��ǰʹ����
	HKCU = m_CTreeCtrl_Dialog_Register_Manager_Show.InsertItem("HKEY_CURRENT_USER", 1, 1, m_hRoot, 0);
	//�����豸
	HKLM = m_CTreeCtrl_Dialog_Register_Manager_Show.InsertItem("HKEY_LOCAL_MACHINE", 1, 1, m_hRoot, 0);
	//ʹ����
	HKUS = m_CTreeCtrl_Dialog_Register_Manager_Show.InsertItem("HKEY_USERS", 1, 1, m_hRoot, 0);
	//��ǰ����
	HKCC = m_CTreeCtrl_Dialog_Register_Manager_Show.InsertItem("HKEY_CURRENT_CONFIG", 1, 1, m_hRoot, 0);
	//HKEY_CLASSES_ROOT
	HKCR = m_CTreeCtrl_Dialog_Register_Manager_Show.InsertItem("HKEY_CLASSES_ROOT", 1, 1, m_hRoot, 0);

	m_CTreeCtrl_Dialog_Register_Manager_Show.Expand(m_hRoot, TVE_EXPAND);

	m_CListCtrl_Dialog_Register_Manager_Show.InsertColumn(0,"����",LVCFMT_LEFT,150,-1);
	m_CListCtrl_Dialog_Register_Manager_Show.InsertColumn(1, "����", LVCFMT_LEFT, 60, -1);
	m_CListCtrl_Dialog_Register_Manager_Show.InsertColumn(2, "����", LVCFMT_LEFT, 300, -1);
	m_CListCtrl_Dialog_Register_Manager_Show.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	//���ͼ��
	m_ImageList_List.Create(16, 16, TRUE, 2, 2);
	m_ImageList_List.Add(AfxGetApp()->LoadIcon(IDI_ICON_STRING));
	m_ImageList_List.Add(AfxGetApp()->LoadIcon(IDI_ICON_DWORD));
	m_CListCtrl_Dialog_Register_Manager_Show.SetImageList(&m_ImageList_List, LVSIL_SMALL);

	m_IsEnable =  TRUE;

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


void CDlgRegisterManager::OnClose()
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

// CDlgRegisterManager ��Ϣ�������
void CDlgRegisterManager::WindowHandleIO(void)
{
	if (m_ContextObject == NULL)
	{
		return;
	}

	switch (m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(0)[0])
	{
	case CLIENT_REGISTER_PATH_DATA:
	{
		AddPath((char*)(m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1)));

		break;
	}
	case CLIENT_REGISTER_KEY_DATA:
	{
		AddKey((char*)(m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1)));

		break;
	}
	default:
		//���䷢���쳣
		break;
	}
}



void CDlgRegisterManager::OnTvnSelchangedTreeRegisterManagerShow(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (!m_IsEnable)
	{
		return;
	}
	m_IsEnable = FALSE;
	//��õ����Item
	TVITEM Item = pNMTreeView->itemNew;

	if (Item.hItem ==m_hRoot)
	{
		m_IsEnable = TRUE;
		return;
	}
	m_hSelectedTreeItem = Item.hItem;  //�����û��򿪵������ڵ��� //0,1,2,3,4,5
	m_CListCtrl_Dialog_Register_Manager_Show.DeleteAllItems();

	CString BufferData = GetFullPath(m_hSelectedTreeItem);
	
	char IsToken = GetFatherPath(BufferData);

	//������ѭ���Ƿ񷴸����һ��ѡ��

	while (m_CTreeCtrl_Dialog_Register_Manager_Show.GetChildItem(Item.hItem) != NULL)
	{
		m_CTreeCtrl_Dialog_Register_Manager_Show.
			DeleteItem(m_CTreeCtrl_Dialog_Register_Manager_Show.GetChildItem(Item.hItem));
	}

	//Ԥ��һ����
	int nitem = m_CListCtrl_Dialog_Register_Manager_Show.InsertItem(0, "(Ĭ��)", 0);
	m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(nitem, 1, "REG_SZ");
	m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(nitem, 2, "(����δ����ֵ)");

	BufferData.Insert(0, IsToken);   //����   �Ǹ���ֵ
	IsToken = CLIENT_REGISTER_DATA_CONTINUE;
	BufferData.Insert(0, IsToken);  //�����ѯ����
	m_IOCPServer->OnPrepareSending(m_ContextObject, (LPBYTE)(BufferData.GetBuffer(0)),
		BufferData.GetLength()+1);
	m_IsEnable = TRUE;
	*pResult = 0;
}

char CDlgRegisterManager::GetFatherPath(CString& FullPath)
{
	char Token;
	if (!FullPath.Find("HKEY_CLASSES_ROOT"))
	{
		Token = MHKEY_CLASSES_ROOT;
		FullPath.Delete(0, sizeof("HKEY_CLASSES_ROOT"));
	}
	else if (!FullPath.Find("HKEY_CURRENT_USER"))
	{
		Token = MHKEY_CURRENT_USER;
		FullPath.Delete(0, sizeof("HKEY_CURRENT_USER"));
	}
	else if (!FullPath.Find("HKEY_LOCAL_MACHINE"))
	{
		Token = MHKEY_LOCAL_MACHINE;
		FullPath.Delete(0, sizeof("HKEY_LOCAL_MACHINE"));
	}
	else if (!FullPath.Find("HKEY_USERS"))
	{
		Token = MHKEY_USERS;
		FullPath.Delete(0, sizeof("HKEY_USERS"));
	}
	else if (!FullPath.Find("HKEY_CURRENT_CONFIG"))
	{
		Token = MHKEY_CURRENT_CONFIG;
		FullPath.Delete(0, sizeof("HKEY_CURRENT_CONFIG"));
	}
	return Token;
}


CString CDlgRegisterManager::GetFullPath(HTREEITEM hCurrent)//SelectItem
{
	CString StrTemp;
	CString StrReturn = "";
	while (1)
	{
		if (hCurrent == m_hRoot)
		{
			return  StrReturn;
		}
		StrTemp = m_CTreeCtrl_Dialog_Register_Manager_Show.GetItemText(hCurrent);
		if (StrTemp.Right(1)!="\\")
		{
			StrTemp += "\\";
		}
		StrReturn = StrTemp + StrReturn;
		hCurrent = m_CTreeCtrl_Dialog_Register_Manager_Show.GetParentItem(hCurrent);
	}
	return StrReturn;
}

void CDlgRegisterManager::AddPath(char* BufferData)
{
	
	if (BufferData ==NULL)
	{
		return;
	}
	int msgsize = sizeof(REGMSG);
	REGMSG msg;
	memcpy((void*)&msg, BufferData, msgsize);
	DWORD size = msg.size;
	int count = msg.count;

	if (size>0&&count>0)
	{
		for (int i=0;i<count;i++)
		{
			CString v2 = GetFullPath(m_hSelectedTreeItem);
			char* szKeyName = BufferData + size*i + msgsize;
			m_CTreeCtrl_Dialog_Register_Manager_Show.InsertItem(szKeyName, 1, 1, m_hSelectedTreeItem,0);
			m_CTreeCtrl_Dialog_Register_Manager_Show.Expand(m_hSelectedTreeItem, TVE_EXPAND);
		}
	}
}

void CDlgRegisterManager::AddKey(char* BufferData)
{
	
	m_CListCtrl_Dialog_Register_Manager_Show.DeleteAllItems();
	int Item = m_CListCtrl_Dialog_Register_Manager_Show.InsertItem(0, "(Data)", 0);
	m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(Item, 1, "REG_SZ");
	m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(Item, 2, "(NULL)");

	if (BufferData == NULL)
	{
		return;
	}
	REGMSG msg;
	memcpy((void*)&msg, BufferData, sizeof(msg));
	char *szTemp = BufferData + sizeof(msg);
	for (int i = 0; i < msg.count; i++)
	{
		BYTE Type = szTemp[0];   //����
		szTemp += sizeof(BYTE);
		char* szValueName = szTemp; //ȡ������
		szTemp += msg.size;
		BYTE *szValueData = (BYTE *)szTemp;
		szTemp += msg.valsize;
		if (Type == MREG_SZ)
		{
			int Item = m_CListCtrl_Dialog_Register_Manager_Show.InsertItem(0, szValueName, 0);
			m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(Item, 1, "REG_SZ");
			m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(Item, 2, (char*)szValueData);
		}
		if (Type == MREG_DWORD)
		{
			char ValueData[256];
			DWORD d = (DWORD)szValueData;
			memcpy((void*)&d, szValueData, sizeof(DWORD));
			CString strValue;
			strValue.Format("0x%x", d);
			//sprintf(ValueData, "(%d)", d);
			//strValue += " ";
			//strValue += ValueData;

			int Item = m_CListCtrl_Dialog_Register_Manager_Show.InsertItem(0, szValueName, 1);
			m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(Item, 1, "MREG_DWORD");
			m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(Item, 2, strValue);

		}
		if (Type == MREG_BINARY)
		{
			char ValueData[256];
			sprintf(ValueData, "%d", szValueData);

			int Item = m_CListCtrl_Dialog_Register_Manager_Show.InsertItem(0, szValueName, 0);
			m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(Item, 1, "MREG_BINARY");
			m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(Item, 2, ValueData);

		}
		if (Type == MREG_EXPAND_SZ)
		{
			int Item = m_CListCtrl_Dialog_Register_Manager_Show.InsertItem(0, szValueName, 0);
			m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(Item, 1, "MREG_EXPAND_SZ");
			m_CListCtrl_Dialog_Register_Manager_Show.SetItemText(Item, 2, (char*)szValueData);
		}
	}
}