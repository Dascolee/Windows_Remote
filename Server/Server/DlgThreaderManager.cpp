// DlgThreaderManager.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "DlgThreaderManager.h"
#include "afxdialogex.h"

HANDLE ListThreadHandle;

vector<SuspendInfo> _sus;

IMPLEMENT_DYNAMIC(CDlgThreaderManager, CDialog)

CDlgThreaderManager::CDlgThreaderManager(CWnd* pParent, _CIOCPServer* IOCPServer, CONTEXT_OBJECT* ContextObject)
	: CDialog(IDD_DIALOG_THREADER_PROCESS_MANAGER, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;

	m_IconHwnd = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));

	m_BufferData = (char *)(m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(0));
}

CDlgThreaderManager::~CDlgThreaderManager()
{
}

void CDlgThreaderManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_THREADER_MANAGER_SHOW, m_CListCtrl_Dialog_Threader_Manager_Show);
}


BEGIN_MESSAGE_MAP(CDlgThreaderManager, CDialog)
	ON_COMMAND(ID_BUTTON_MENU_THREADER_MANAGER_REFRESH, &CDlgThreaderManager::OnButtonMenuThreaderManagerRefresh)
	ON_COMMAND(ID_BUTTON_MENU_THREADER_MANAGER_KILL_ONE, &CDlgThreaderManager::OnButtonMenuThreaderManagerKillOne)
	ON_COMMAND(ID_BUTTON_MENU_THREADER_MANAGER_SUSPEND, &CDlgThreaderManager::OnButtonMenuThreaderManagerSuspend)
	ON_COMMAND(ID_BUTTON_MENU_THREADER_MANAGER_RESUME, &CDlgThreaderManager::OnButtonMenuThreaderManagerResume)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_THREADER_MANAGER_SHOW, &CDlgThreaderManager::OnNMRClickListThreaderManagerShow)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CDlgThreaderManager ��Ϣ�������

BOOL CDlgThreaderManager::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��ÿͻ���IP SetWindowText

	CString v1;
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int ClientAddressLength = sizeof(ClientAddress);
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength); //�õ����ӵ�ip 
	v1.Format("\\%s - Զ���̹߳���", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//���öԻ������
	SetWindowText(v1);
	//���öԻ����ϵ�����
	LOGFONT Logfont;
	CFont* v2 = m_CListCtrl_Dialog_Threader_Manager_Show.GetFont();
	v2->GetLogFont(&Logfont);
	//��������
	Logfont.lfHeight = Logfont.lfHeight*1.3;
	Logfont.lfWeight = Logfont.lfWeight*1.3;
	CFont v3;
	v3.CreateFontIndirect(&Logfont);
	m_CListCtrl_Dialog_Threader_Manager_Show.SetFont(&v3);
	v3.Detach();

	m_CListCtrl_Dialog_Threader_Manager_Show.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT);
	switch (m_BufferData[0])
	{
	case CLIENT_PROCESS_MANAGER_ENUM_THREADER_REPLY:
	{																										  
		m_CListCtrl_Dialog_Threader_Manager_Show.InsertColumn(0, "BasePriority", LVCFMT_CENTER, 100);			
		m_CListCtrl_Dialog_Threader_Manager_Show.InsertColumn(1, "ContextSwitches", LVCFMT_CENTER, 100);			
		m_CListCtrl_Dialog_Threader_Manager_Show.InsertColumn(2, "Priority", LVCFMT_CENTER, 100);			  
		m_CListCtrl_Dialog_Threader_Manager_Show.InsertColumn(3, "ThreadID", LVCFMT_CENTER, 100);			  
		m_CListCtrl_Dialog_Threader_Manager_Show.InsertColumn(0, "ThreadState", LVCFMT_CENTER, 100);			  
		m_CListCtrl_Dialog_Threader_Manager_Show.InsertColumn(1, "WaitReason", LVCFMT_CENTER, 100);			  
		m_CListCtrl_Dialog_Threader_Manager_Show.InsertColumn(2, "WaitTime", LVCFMT_CENTER, 100);			  
		m_CListCtrl_Dialog_Threader_Manager_Show.InsertColumn(3, "ThreadHandle", LVCFMT_CENTER, 100);			

		ShowThreaderList();   //���ڵ�һ������������Ϣ��������Ž��̵��������԰�������ʾ���б���\0\0

		break;
	}
	default:
		break;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}

void CDlgThreaderManager::OnClose()
{
	m_ContextObject->DialogID = 0;
	m_ContextObject->DialogHandle = NULL;
	CancelIo((HANDLE)m_ContextObject->ClientSocket);
	closesocket(m_ContextObject->ClientSocket);
	CDialog::OnClose();
}

void CDlgThreaderManager::WindowHandleIO()
{
	if (m_ContextObject == NULL)
	{
		return;
	}

	switch (m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(0)[0])
	{
	case CLIENT_PROCESS_MANAGER_ENUM_THREADER_REPLY:
	{
		ShowThreaderList();
		break;
	}
	case CLIENT_PROCESS_MANAGER_KILL_ONE_REPLY:
	{
		OnButtonMenuThreaderManagerRefresh();
		break;
	}
	case CLIENT_THREADER_MANAGER_SUSPEND_REPLY:
	{
		vector<SuspendInfo>::iterator i;
		for (i = _sus.begin(); i != _sus.end(); i++)
		{
			if (i->ThreadHandle == ListThreadHandle)
			{
				i->SuspendCount++;
			}
		}
		break;
	}

	case CLIENT_THREADER_MANAGER_RESUME_REPLY:
	{
		vector<SuspendInfo>::iterator i;
		for (i = _sus.begin(); i != _sus.end(); i++)
		{
			if (i->ThreadHandle == ListThreadHandle)
			{
				i->SuspendCount--;

			}
		}
		break;
	}
	}
}

void CDlgThreaderManager::ShowThreaderList()
{
	//Խ��һ���ֽڵ����ݰ����� ָ��BasePriority
	char	*BufferData = (char *)(m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1));

	LONG BasePriority;
	ULONG ContextSwitches;
	KPRIORITY Priority;
	HANDLE    ThreadID;
	ULONG ThreadState;
	ULONG WaitReason;
	ULONG WaitTime;
	PVOID   Teb;
	PVOID   ThreadStartAddress;
	HANDLE  ThreadHandle;

	DWORD	Offset = 0;//ƫ��

					   //���List�б�
	m_CListCtrl_Dialog_Threader_Manager_Show.DeleteAllItems();

	int i;
	SuspendInfo Info = { 0 };
	for (i = 0; Offset < m_ContextObject->m_ReceivedDecompressdBufferData.GetBufferLength() - 1; i++)
	{
		LONG* BasePriority = (LONG*)(BufferData + Offset);      //����õ�BasePriority
		ULONG* ContextSwitches = (ULONG*)(BufferData + Offset + sizeof(LONG));
		KPRIORITY* Priority = (KPRIORITY*)(BufferData + Offset + sizeof(LONG) + sizeof(ULONG));
		HANDLE* ThreadID = (HANDLE*)(BufferData + Offset + sizeof(LONG) + sizeof(ULONG) + sizeof(KPRIORITY));
		ULONG* ThreadState = (ULONG*)(BufferData + Offset + sizeof(LONG) + sizeof(ULONG) + sizeof(KPRIORITY) + sizeof(HANDLE));
		ULONG* WaitReason = (ULONG*)(BufferData + Offset + sizeof(LONG) + sizeof(ULONG) + sizeof(KPRIORITY) + sizeof(HANDLE)
			+ sizeof(ULONG));
		ULONG* WaitTime = (ULONG*)(BufferData + Offset + sizeof(LONG) + sizeof(ULONG) + sizeof(KPRIORITY) + sizeof(HANDLE)
			+ sizeof(ULONG) * 2);
		HANDLE* ThreadHandle = (HANDLE*)(BufferData + Offset + sizeof(LONG) + sizeof(ULONG) + sizeof(KPRIORITY) + sizeof(HANDLE)
			+ sizeof(ULONG) * 3);

		Info.ThreadHandle = *ThreadHandle;
		_sus.push_back(Info);

		CString v1;
		v1.Format("%5u", *BasePriority);
		m_CListCtrl_Dialog_Threader_Manager_Show.InsertItem(i, v1);
		v1.Format("%5u", *ContextSwitches);
		m_CListCtrl_Dialog_Threader_Manager_Show.SetItemText(i, 1, v1);
		v1.Format("%5u", *Priority);
		m_CListCtrl_Dialog_Threader_Manager_Show.SetItemText(i, 2, v1);
		v1.Format("%5u", *ThreadID);
		m_CListCtrl_Dialog_Threader_Manager_Show.SetItemText(i, 3, v1);
		v1.Format("%5u", *ThreadState);
		m_CListCtrl_Dialog_Threader_Manager_Show.SetItemText(i, 4, v1);
		v1.Format("%5u", *WaitReason);
		m_CListCtrl_Dialog_Threader_Manager_Show.SetItemText(i, 5, v1);
		v1.Format("%5u", *WaitTime);
		m_CListCtrl_Dialog_Threader_Manager_Show.SetItemText(i, 6, v1);
		v1.Format("%5u", *ThreadHandle);
		m_CListCtrl_Dialog_Threader_Manager_Show.SetItemText(i, 7, v1);
		// ItemData Ϊ����ID
		m_CListCtrl_Dialog_Threader_Manager_Show.SetItemData(i, (DWORD_PTR)*ThreadHandle);   //�߳̾�� ����Hide
																  
		Offset += sizeof(LONG) + sizeof(ULONG) + sizeof(KPRIORITY) + sizeof(HANDLE) * 2  //����������ݽṹ ������һ��ѭ��
			+ sizeof(ULONG) * 3;
	}
}

void CDlgThreaderManager::OnNMRClickListThreaderManagerShow(NMHDR *pNMHDR, LRESULT *pResult)
{
	CString strS;
	CString strR;
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int Index = m_CListCtrl_Dialog_Threader_Manager_Show.GetSelectionMark();
	if (Index == -1)
	{
		return;
	}
	else
	{

		CMenu Menu;
		Menu.LoadMenu(IDR_MENU_THREADER_MANAGER_LIST_RINGHT);

		ListThreadHandle = (HANDLE)m_CListCtrl_Dialog_Threader_Manager_Show.GetItemData(Index);
		vector<SuspendInfo>::iterator i;
		for (i = _sus.begin(); i != _sus.end(); i++)
		{
			if (i->ThreadHandle == ListThreadHandle)
			{
				strS.Format("�����߳�(%d)", i->SuspendCount);
				strR.Format("�ָ��߳�(%d)", i->SuspendCount);
				//���Ĳ˵���
				Menu.ModifyMenu(ID_BUTTON_MENU_THREADER_MANAGER_SUSPEND, MF_BYCOMMAND | 
					MF_STRING, ID_BUTTON_MENU_THREADER_MANAGER_SUSPEND, strS);
				Menu.ModifyMenu(ID_BUTTON_MENU_THREADER_MANAGER_RESUME, MF_BYCOMMAND | 
					MF_STRING, ID_BUTTON_MENU_THREADER_MANAGER_RESUME, strR);
			}
		}
		CPoint Point;
		GetCursorPos(&Point);
		SetForegroundWindow();
		Menu.GetSubMenu(0)->TrackPopupMenu(
			TPM_LEFTBUTTON | TPM_RIGHTBUTTON, Point.x, Point.y, this, NULL);
	}
	*pResult = 0;
}

void CDlgThreaderManager::OnButtonMenuThreaderManagerRefresh()
{
	// TODO: �ڴ���������������
	BYTE IsToken = CLIENT_THREADER_MANAGER_REFRESH;
	m_IOCPServer->OnPrepareSending(m_ContextObject, &IsToken, 1);

}

void CDlgThreaderManager::OnButtonMenuThreaderManagerKillOne()
{
	// TODO: �ڴ���������������
	ClientThreaderKill();
}

void CDlgThreaderManager::ClientThreaderKill()
{
	CListCtrl	*v1 = NULL;
	if (m_CListCtrl_Dialog_Threader_Manager_Show.IsWindowVisible())
	{
		v1 = &m_CListCtrl_Dialog_Threader_Manager_Show;
	}
	else
	{
		return;
	}
	//[��Ϣ][ID][ID][ID][ID]
	//���仺����
	LPBYTE BufferData = (LPBYTE)LocalAlloc(LPTR, 1 + (v1->GetSelectedCount() * 4));//1.exe  4  ID   Handle
																				   //����������̵�����ͷ
	BufferData[0] = CLIENT_THREADER_MANAGER_KILL_ONE;

	//��ʾ������Ϣ
	char *WARNING = "����: ��ֹ�߳̿��ܻᵼ�½��̱���\n";

	CString v2;
	if (v1->GetSelectedCount() > 1)
	{
		v2.Format("%sȷʵ\n����ֹ��%d���߳���?", WARNING, v1->GetSelectedCount());
	}
	else
	{
		v2.Format("%sȷʵ\n����ֹ�����߳���?", WARNING);
	}
	if (::MessageBox(m_hWnd, v2, "�߳̽�������", MB_YESNO | MB_ICONQUESTION) == IDNO)
		return;


	DWORD	Offset = 1;
	//��ȡ��һ��ѡ�����λ���б���ͼ�ؼ���
	POSITION Pos = v1->GetFirstSelectedItemPosition();
	//�õ�Ҫ�����ĸ�����
	//����һ�½����������
	while (Pos)
	{
		int	iItem = v1->GetNextSelectedItem(Pos);
		DWORD ThreadHandle = v1->GetItemData(iItem);
		memcpy(BufferData + Offset, &ThreadHandle, sizeof(HANDLE));
		Offset += sizeof(HANDLE);
	}
	//�������ݵ����ض��ڱ��ض��в���CLIENT_PROCESS_MANAGER_KILL_ONE�������ͷ
	m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, LocalSize(BufferData));
	LocalFree(BufferData);
}

void CDlgThreaderManager::OnButtonMenuThreaderManagerSuspend()
{
	// TODO: �ڴ���������������
	CListCtrl	*v1 = NULL;
	if (m_CListCtrl_Dialog_Threader_Manager_Show.IsWindowVisible())
	{
		v1 = &m_CListCtrl_Dialog_Threader_Manager_Show;
	}
	else
	{
		return;
	}

	//[��Ϣ][ID][ID][ID][ID]
	//���仺����
	LPBYTE BufferData = (LPBYTE)LocalAlloc(LPTR, 1 + (v1->GetSelectedCount() * 4));//1.exe  4  ID   Handle
																				   //����������̵�����ͷ
	BufferData[0] = CLIENT_THREADER_MANAGER_SUSPEND;

	//��ʾ������Ϣ
	char *WARNING = "����: �����߳̿��ܻᵼ���߳���ֹ\n�������̱���\n No Zhuo No Die!";

	CString v2;
	if (v1->GetSelectedCount() > 1)
	{
		v2.Format("%sȷʵ\n�������%d���߳���?", WARNING, v1->GetSelectedCount());
	}
	else
	{
		v2.Format("%sȷʵ\n���������߳���?", WARNING);
	}
	if (::MessageBox(m_hWnd, v2, "�̹߳��𾯸�", MB_YESNO | MB_ICONQUESTION) == IDNO)
		return;


	DWORD	Offset = 1;
	//��ȡ��һ��ѡ�����λ���б���ͼ�ؼ���
	POSITION Pos = v1->GetFirstSelectedItemPosition();
	//�õ�Ҫ�����ĸ�����
	//����һ�½����������
	while (Pos)
	{
		int	iItem = v1->GetNextSelectedItem(Pos);
		DWORD ThreadHandle = v1->GetItemData(iItem);
		memcpy(BufferData + Offset, &ThreadHandle, sizeof(HANDLE));
		Offset += sizeof(HANDLE);
	}
	//�������ݵ����ض��ڱ��ض��в���CLIENT_PROCESS_MANAGER_KILL_ONE�������ͷ
	m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, LocalSize(BufferData));
	LocalFree(BufferData);
}

void CDlgThreaderManager::OnButtonMenuThreaderManagerResume()
{
	// TODO: �ڴ���������������
	CListCtrl	*v1 = NULL;
	if (m_CListCtrl_Dialog_Threader_Manager_Show.IsWindowVisible())
	{
		v1 = &m_CListCtrl_Dialog_Threader_Manager_Show;
	}
	else
	{
		return;
	}
	//[��Ϣ][ID][ID][ID][ID]
	//���仺����
	LPBYTE BufferData = (LPBYTE)LocalAlloc(LPTR, 1 + (v1->GetSelectedCount() * 4));//1.exe  4  ID   Handle
																				   //����������̵�����ͷ
	BufferData[0] = CLIENT_THREADER_MANAGER_RESUME;


	DWORD	Offset = 1;
	//��ȡ��һ��ѡ�����λ���б���ͼ�ؼ���
	POSITION Pos = v1->GetFirstSelectedItemPosition();
	//�õ�Ҫ�����ĸ�����
	//����һ�½����������
	while (Pos)
	{
		int	iItem = v1->GetNextSelectedItem(Pos);
		DWORD ThreadHandle = v1->GetItemData(iItem);
		memcpy(BufferData + Offset, &ThreadHandle, sizeof(HANDLE));
		Offset += sizeof(HANDLE);
	}
	//�������ݵ����ض��ڱ��ض��в���CLIENT_PROCESS_MANAGER_KILL_ONE�������ͷ
	m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, LocalSize(BufferData));
	LocalFree(BufferData);
}


