// DlgFileManager.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "DlgFileManager.h"
#include "DlgFileNewFolder.h"
#include "DlgFileMode.h"
#include "afxdialogex.h"

// CDlgFileManager �Ի���

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
	//��ͼ��
	HIMAGELIST ImageListHwnd;   //SDK
								// ����ϵͳͼ���б�
	ImageListHwnd = (HIMAGELIST)SHGetFileInfo
	(
		NULL,
		0,
		&v1,
		sizeof(SHFILEINFO),
		SHGFI_LARGEICON | SHGFI_SYSICONINDEX
	);
	m_CImageList_Large = CImageList::FromHandle(ImageListHwnd);   //CimageList*
																  //����ϵͳͼ���б�
	ImageListHwnd = (HIMAGELIST)SHGetFileInfo
	(
		NULL,
		0,
		&v1,
		sizeof(SHFILEINFO),
		SHGFI_SMALLICON | SHGFI_SYSICONINDEX
	);
	m_CImageList_Small = CImageList::FromHandle(ImageListHwnd);
	//����ק�ļ�
	m_IsDragging = FALSE;
	//�����ļ��Ĺ��̿���ֹͣ
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
	//ת��
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgFileManager::OnBnClickedButton1)
	
	//������һ��Ŀ¼
	ON_COMMAND(IDR_TOOLBAR_DIALOG_FILE_MANAGER_MAIN_PREVIOUS, &CDlgFileManager::OnServerFilePrevious)
	ON_COMMAND(IDR_TOOLBAR_DIALOG_REMOTE_FILE_MANAGER_MAIN_PREVIOUS, &CDlgFileManager::OnClientFilePrevious)
	//ɾ���ļ�
	ON_COMMAND(IDR_TOOLBAR_DIALOG_FILE_MANAGER_MAIN_DELETE, &CDlgFileManager::OnServerFileDelete)
	ON_COMMAND(IDR_TOOLBAR_DIALOG_REMOTE_FILE_MANAGER_MAIN_DELETE, &CDlgFileManager::OnClientFileDelete)
	//�½��ļ�
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

// CDlgFileManager ��Ϣ�������

BOOL CDlgFileManager::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_IconHwnd, FALSE);
	CString v1;
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int ClientAddressLength = sizeof(ClientAddress);
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength); //�õ����ӵ�ip 
	v1.Format("\\\\%s - �ļ�����", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//���öԻ������
	SetWindowText(v1);



	//��������
	if (!m_ToolBar_Server_Dialog_File_Manager.Create(this, WS_CHILD |
		WS_VISIBLE | CBRS_ALIGN_ANY | CBRS_TOOLTIPS | CBRS_FLYBY, IDR_TOOLBAR_DIALOG_FILE_MANAGER_MAIN)
		|| !m_ToolBar_Server_Dialog_File_Manager.LoadToolBar(IDR_TOOLBAR_DIALOG_FILE_MANAGER_MAIN))
	{

		return -1;
	}
	m_ToolBar_Server_Dialog_File_Manager.LoadTrueColorToolBar
	(
		24,    //������ʹ����� 
		IDB_BITMAP_FILE,
		IDB_BITMAP_FILE,
		IDB_BITMAP_FILE    //û����
	);



	if (!m_ToolBar_Client_Dialog_File_Manager.Create(this, WS_CHILD |
		WS_VISIBLE | CBRS_ALIGN_ANY | CBRS_TOOLTIPS | CBRS_FLYBY, IDR_TOOLBAR_DIALOG_FILE_MANAGER_MAIN)
		|| !m_ToolBar_Client_Dialog_File_Manager.LoadToolBar(IDR_TOOLBAR_DIALOG_REMOTE_FILE_MANAGER_MAIN))
	{

		return -1;
	}
	m_ToolBar_Client_Dialog_File_Manager.LoadTrueColorToolBar
	(
		24,    //������ʹ����� 
		IDB_BITMAP_FILE,
		IDB_BITMAP_FILE,
		IDB_BITMAP_FILE    //û����
	);
	

	m_ToolBar_Server_Dialog_File_Manager.AddDropDownButton(this, 
		IDR_TOOLBAR_DIALOG_FILE_MANAGER_MAIN_VIEW, IDR_TOOLBAR_DIALOG_FILE_MANAGER_MAIN_VIEW);
	m_ToolBar_Server_Dialog_File_Manager.SetButtonText(0, "����");     //��λͼ����������ļ�
	m_ToolBar_Server_Dialog_File_Manager.SetButtonText(1, "�鿴");
	m_ToolBar_Server_Dialog_File_Manager.SetButtonText(2, "ɾ��");
	m_ToolBar_Server_Dialog_File_Manager.SetButtonText(3, "�½�");
	m_ToolBar_Server_Dialog_File_Manager.SetButtonText(4, "����");
	m_ToolBar_Server_Dialog_File_Manager.SetButtonText(5, "ֹͣ");
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);  //��ʾ

	m_ToolBar_Client_Dialog_File_Manager.AddDropDownButton(this,
		IDR_TOOLBAR_DIALOG_REMOTE_FILE_MANAGER_MAIN_VIEW, IDR_TOOLBAR_DIALOG_REMOTE_FILE_MANAGER_MAIN_VIEW);
	m_ToolBar_Client_Dialog_File_Manager.SetButtonText(0, "����");     //��λͼ����������ļ�
	m_ToolBar_Client_Dialog_File_Manager.SetButtonText(1, "�鿴");
	m_ToolBar_Client_Dialog_File_Manager.SetButtonText(2, "ɾ��");
	m_ToolBar_Client_Dialog_File_Manager.SetButtonText(3, "�½�");
	m_ToolBar_Client_Dialog_File_Manager.SetButtonText(4, "����");
	m_ToolBar_Client_Dialog_File_Manager.SetButtonText(5, "ֹͣ");
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);  //��ʾ

	m_CListCtrl_Dialog_File_Manager_Server.SetImageList(m_CImageList_Large, LVSIL_NORMAL);
	m_CListCtrl_Dialog_File_Manager_Server.SetImageList(m_CImageList_Small, LVSIL_SMALL);
	m_CListCtrl_Dialog_File_Manager_Client.SetImageList(m_CImageList_Large, LVSIL_NORMAL);
	m_CListCtrl_Dialog_File_Manager_Client.SetImageList(m_CImageList_Small, LVSIL_SMALL);

	RECT	RectClient;
	GetClientRect(&RectClient);           //����������ڴ�С

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
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0); //��ʾ״̬��

	m_StatusBar.MoveWindow(v3);

	m_StatusBar.GetItemRect(1, &RectClient);

	RectClient.bottom -= 1;

	m_ProgressCtrl = new CProgressCtrl;
	m_ProgressCtrl->Create(PBS_SMOOTH | WS_VISIBLE, RectClient, &m_StatusBar, 1);
	m_ProgressCtrl->SetRange(0, 100);           //���ý�������Χ
	m_ProgressCtrl->SetPos(0);

	//------------------------------------------------------------------------Server���
	RECT	RectServer;
	m_CStatic_Dialog_File_Manager_Server_Position.GetWindowRect(&RectServer);
	m_CStatic_Dialog_File_Manager_Server_Position.ShowWindow(SW_HIDE);
	//��ʾ������
	m_ToolBar_Server_Dialog_File_Manager.MoveWindow(&RectServer);

	//-----------------------------------------------------------------------Client���

	m_CStatic_Dialog_File_Manager_Client_Position.GetWindowRect(&RectClient);
	m_CStatic_Dialog_File_Manager_Client_Position.ShowWindow(SW_HIDE);
	//��ʾ������
	m_ToolBar_Client_Dialog_File_Manager.MoveWindow(&RectClient);


	FixedServerVolumeList();
	FixedClientVolumeList();
 

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}

void CDlgFileManager::OnClose()
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
	//�½��ļ���
	case CLIENT_FILE_MANAGER_NEW_FLODER_REPLY:
	{
		BOOL* IsOk = (BOOL*)m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1);
		if (*IsOk){
			GetClientFilesList(".");
		}
		else{
			MessageBox("�½�ʧ��");
		}
		break;
	}
	//ɾ���ļ�
	case CLIENT_FILE_MANAGER_DELETE_REPLY:
	{
		BOOL* IsOk = (BOOL*)m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1);
		if (*IsOk){
			GetClientFilesList(".");
		}
		else{
			MessageBox("ɾ��ʧ��");
		}
		EnableControl(TRUE);
		break;
	}
	//�ڿͻ��˷����������ļ�
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
	case CLIENT_FILE_MANAGER_SEND_COPY_DIRECTORY_FILEFULLPATH_REPLY://���մӿͻ����ļ�����ö�ٵ������ļ�·�� ѹ�������б�
	{
		CString FileFullPath;
		FileFullPath.Format("%s", m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1));
		m_ClientFileToServerJob.AddTail(FileFullPath);
		break;
	}
	case CLIENT_FILE_MANAGER_SEND_COPY_DIRECTORY_FILEFULLPATH_END://�ӿͻ���ö���ļ����µ��ļ����
	{
		//�ж������б��Ƿ�Ϊ��
		if (m_ClientFileToServerJob.IsEmpty())
		{
			::MessageBox(m_hWnd, "�ļ���Ϊ��", "����", MB_OK | MB_ICONWARNING);
			return;
		}
		//���õ�ǰ���� ���ڿؼ����
		EnableControl(FALSE);
		//ת����һ���ļ�·�� 
		ClientToServerFileFullPath();
		break;
	}

	case CLIENT_FILE_MANAGER_SEND_COPY_FILE_INFORMATION_FROM_CLIENT_REPLY:
	{
		memcpy(&m_OperatingFileLength, m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1), 8);
	}
	case CLIENT_FILE_MANAGER_COPY_FILE_DATA_TO_SERVER_REPLY:
	{
		//�յ��ͻ��˷��͵��ļ�����
		WriteServerReceiveFile(m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1),
			m_ContextObject->m_ReceivedDecompressdBufferData.GetBufferLength() - 1);
		break;
	}
	case CLIENT_FILE_MANAGER_ONE_COPY_FILE_SUCCESS:
	{
		//�ӿͻ��˿��������ļ��ɹ� �ж��Ƿ�����ӿͻ��˿����ļ���������
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
	//��ʼ���б���Ϣ
	m_CListCtrl_Dialog_File_Manager_Server.InsertColumn(0, "����", LVCFMT_LEFT, 50);
	m_CListCtrl_Dialog_File_Manager_Server.InsertColumn(1, "����", LVCFMT_RIGHT, 80);
	m_CListCtrl_Dialog_File_Manager_Server.InsertColumn(2, "�ļ�ϵͳ", LVCFMT_RIGHT, 60);
	m_CListCtrl_Dialog_File_Manager_Server.InsertColumn(3, "�ܴ�С", LVCFMT_RIGHT, 100);
	m_CListCtrl_Dialog_File_Manager_Server.InsertColumn(4, "���ÿռ�", LVCFMT_RIGHT, 100);

	m_CListCtrl_Dialog_File_Manager_Server.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	GetLogicalDriveStrings(sizeof(VolumeListData), (LPSTR)VolumeListData);  //c:\.d:\.
	Travel = VolumeListData;

	CHAR	FileSystemType[MAX_PATH];  //ntfs  fat32
	unsigned __int64	HardDiskAmount = 0;   //HardDisk
	unsigned __int64	HardDiskFreeSpace = 0;
	unsigned long		HardDiskAmountMB = 0; // �ܴ�С
	unsigned long		HardDiskFreeSpaceMB = 0;   // ʣ��ռ�

	for (int i = 0; *Travel != '\0'; i++, Travel += lstrlen(Travel) + 1)
	{
		// �õ����������Ϣ
		memset(FileSystemType, 0, sizeof(FileSystemType));
		// �õ��ļ�ϵͳ��Ϣ����С
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


		int	iItem = m_CListCtrl_Dialog_File_Manager_Server.InsertItem(i, Travel, GetServerIconIndex(Travel, GetFileAttributes(Travel)));    //˳����ϵͳ��ͼ��		

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
	m_CListCtrl_Dialog_File_Manager_Client.InsertColumn(0, "����", LVCFMT_LEFT, 50);
	m_CListCtrl_Dialog_File_Manager_Client.InsertColumn(1, "����", LVCFMT_LEFT, 80);
	m_CListCtrl_Dialog_File_Manager_Client.InsertColumn(2, "�ļ�ϵͳ", LVCFMT_LEFT, 60);
	m_CListCtrl_Dialog_File_Manager_Client.InsertColumn(3, "�ܴ�С", LVCFMT_LEFT, 100);
	m_CListCtrl_Dialog_File_Manager_Client.InsertColumn(4, "���ÿռ�", LVCFMT_LEFT, 100);

	m_CListCtrl_Dialog_File_Manager_Client.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	char	*Travel = NULL;
	Travel = (char *)m_ClientData;   //�Ѿ�ȥ������Ϣͷ��1���ֽ���

	int i = 0;
	ULONG v1 = 0;
	for (i = 0; Travel[i] != '\0';)
	{
		//�����������ж�ͼ�������
		if (Travel[i] == 'A' || Travel[i] == 'B')
		{
			v1 = 6;
		}
		else
		{
			switch (Travel[i + 1])   //�������ж��������� �鿴���ض�
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
				v1 = 9;	//Win7Ϊ10
				break;
			default:
				v1 = 0;
				break;
			}
		}
		//		02E3F844  43 03 04 58 02 00 73 D7 00 00 B1 BE B5 D8 B4 C5 C5 CC 00 4E 54 46 53 00 44  C..X..s...���ش���.NTFS.D
		//		2E3F85E  03 04 20 03 00 FC 5B 00 00 B1 BE B5 D8 B4 C5 C5 CC 00 4E 54 46 53 00
		CString	v3;
		//��ʽ���̷�
		v3.Format("%c:\\", Travel[i]);//c:
		int	iItem = m_CListCtrl_Dialog_File_Manager_Client.InsertItem(i, v3, v1);
		m_CListCtrl_Dialog_File_Manager_Client.SetItemData(iItem, 1);     //����ʾ  
		//�����ݲ���ʾ�����ص�ǰ����  1����Ŀ¼
		unsigned long		HardDiskAmountMB = 0; // �ܴ�С
		unsigned long		HardDiskFreeMB = 0;   // ʣ��ռ�
		memcpy(&HardDiskAmountMB, Travel + i + 2, 4);
		memcpy(&HardDiskFreeMB, Travel + i + 6, 4);
		CString  v5;
		v5.Format("%10.1f GB", (float)HardDiskAmountMB / 1024);
		m_CListCtrl_Dialog_File_Manager_Client.SetItemText(iItem, 3, v5);
		v5.Format("%10.1f GB", (float)HardDiskFreeMB / 1024);
		m_CListCtrl_Dialog_File_Manager_Client.SetItemText(iItem, 4, v5);

		i += 10;   //���嵽����һ��������

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

//˫��CList�е���Server
void CDlgFileManager::OnNMDblclkListDialogFileManagerServer(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
		//�������Ŀ¼
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

		// �����ѡ�еģ���Ŀ¼
		if (iItem != -1)
		{
			//��ø������������
			if (m_CListCtrl_Dialog_File_Manager_Server.GetItemData(iItem) == 1)   //������������
			{
				//��Ŀ¼
				strDirectory = m_CListCtrl_Dialog_File_Manager_Server.GetItemText(iItem, 0);
			}
		}
		// ����Ͽ���õ�·��
		else
		{
			m_CCombo_Dialog_File_Manager_Server_File.GetWindowText(m_ServerFileFullPath);
		}
	}

	if (strDirectory == "..")
	{
		m_ServerFileFullPath = GetParentDirectory(m_ServerFileFullPath);
	}
	// ˢ�µ�ǰ��
	else if (strDirectory != ".")   //��ϵͳ�е�ÿһ��Ŀ¼�ж������һ��.Ŀ¼��..Ŀ¼
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


	//�����յ��ļ�·������ؼ���
	m_CCombo_Dialog_File_Manager_Server_File.InsertString(0, m_ServerFileFullPath);
	m_CCombo_Dialog_File_Manager_Server_File.SetCurSel(0);

	//ɾ��Control�ϵ���
	m_CListCtrl_Dialog_File_Manager_Server.DeleteAllItems();
	while (m_CListCtrl_Dialog_File_Manager_Server.DeleteColumn(0) != 0);  //ɾ��
	m_CListCtrl_Dialog_File_Manager_Server.InsertColumn(0, "����", LVCFMT_LEFT, 150);
	m_CListCtrl_Dialog_File_Manager_Server.InsertColumn(1, "��С", LVCFMT_LEFT, 100);
	m_CListCtrl_Dialog_File_Manager_Server.InsertColumn(2, "����", LVCFMT_LEFT, 100);
	m_CListCtrl_Dialog_File_Manager_Server.InsertColumn(3, "�޸�����", LVCFMT_LEFT, 115);

	int	 v10 = 0;
	//�Լ���ControlList�Ŀؼ���дһ��..Ŀ¼��˫���ͷ�����һ�㣩
	m_CListCtrl_Dialog_File_Manager_Server.SetItemData(m_CListCtrl_Dialog_File_Manager_Server.InsertItem(v10++, "..",
		GetServerIconIndex(NULL, FILE_ATTRIBUTE_DIRECTORY)), 1);

	//ѭ�����δ����������ͣ�Ŀ¼���ļ���
	for (int i = 0; i < 2; i++) //0 �ļ���  1  �ļ�
	{
		CFileFind	FileFindObject;
		BOOL		IsLoop;
		IsLoop = FileFindObject.FindFile(m_ServerFileFullPath + "*.*");   //c:\*.*    ����һ��
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
				m_CListCtrl_Dialog_File_Manager_Server.SetItemText(iItem, 2, "�ļ���");
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

//ת��Ŀ¼����
void CDlgFileManager::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString v1;
	m_CCombo_Dialog_File_Manager_Server_File.GetWindowTextA(v1);
	m_ServerFileFullPath = v1;
	FixedServerFileList(NULL);
}

void CDlgFileManager::OnBnClickedButton2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	GetClientFilesList();
}

BOOL CDlgFileManager::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
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
//˫��CList�е���Client
void CDlgFileManager::OnNMDblclkListDialogFileManagerClient(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_CListCtrl_Dialog_File_Manager_Client.GetSelectedCount() == 0 ||
		m_CListCtrl_Dialog_File_Manager_Client.GetItemData(m_CListCtrl_Dialog_File_Manager_Client.GetSelectionMark()) != 1)
	{
		return;
	}
	GetClientFilesList();  //����Ϣ

	*pResult = 0;
}

VOID CDlgFileManager::GetClientFilesList(CString Directory)
{
	if (Directory.GetLength() == 0)   //���̾�
	{
		int	iItem = m_CListCtrl_Dialog_File_Manager_Client.GetSelectionMark();

		// �����ѡ����
		if (iItem != -1)
		{
			if (m_CListCtrl_Dialog_File_Manager_Client.GetItemData(iItem) == 1)
			{
				//ѡ�����Ǹ�Ŀ¼
				Directory = m_CListCtrl_Dialog_File_Manager_Client.GetItemText(iItem, 0);    /* D:\ */
			}
		}
		//����Ͽ���õ�·��
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
		//�����Ŀ¼��ʵ�ʾ���Ҫˢ�¾�
		FixedClientVolumeList();
		return;
	}
	m_CCombo_Dialog_File_Manager_Client_File.InsertString(0, m_ClientFileFullPath);
	m_CCombo_Dialog_File_Manager_Client_File.SetCurSel(0);

	ULONG	BufferLength = m_ClientFileFullPath.GetLength() + 2;
	BYTE*   BufferData = (BYTE *)new BYTE[BufferLength];
	//��COMMAND_LIST_FILES  ���͵����ƶˣ�����������
	BufferData[0] = CLIENT_FILE_MANAGER_FILE_LIST;
	memcpy(BufferData + 1, m_ClientFileFullPath.GetBuffer(0), BufferLength - 1);
	m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, BufferLength);
	delete[] BufferData;
	BufferData = NULL;

	//	m_Remote_Directory_ComboBox.InsertString(0, m_Remote_Path);
	//	m_Remote_Directory_ComboBox.SetCurSel(0);
	// �õ���������ǰ������
	m_CListCtrl_Dialog_File_Manager_Client.EnableWindow(FALSE);   //����Ϲ��
	m_ProgressCtrl->SetPos(0);                 //��ʼ��������
}

VOID CDlgFileManager::FixedClientFilesList(BYTE *BufferData, ULONG BufferLength)
{
	// ��������ImageList
	//  SHFILEINFO	sfi;
	//  HIMAGELIST hImageListLarge = (HIMAGELIST)SHGetFileInfo(NULL, 0, &sfi,sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_LARGEICON);
	//	HIMAGELIST hImageListSmall = (HIMAGELIST)SHGetFileInfo(NULL, 0, &sfi,sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	//	ListView_SetImageList(m_list_remote.m_hWnd, hImageListLarge, LVSIL_NORMAL);
	//	ListView_SetImageList(m_list_remote.m_hWnd, hImageListSmall, LVSIL_SMALL);  //??

	// �ؽ�����
	m_CListCtrl_Dialog_File_Manager_Client.DeleteAllItems();
	while (m_CListCtrl_Dialog_File_Manager_Client.DeleteColumn(0) != 0);
	m_CListCtrl_Dialog_File_Manager_Client.InsertColumn(0, "����", LVCFMT_LEFT, 200);
	m_CListCtrl_Dialog_File_Manager_Client.InsertColumn(1, "��С", LVCFMT_LEFT, 100);
	m_CListCtrl_Dialog_File_Manager_Client.InsertColumn(2, "����", LVCFMT_LEFT, 100);
	m_CListCtrl_Dialog_File_Manager_Client.InsertColumn(3, "�޸�����", LVCFMT_LEFT, 115);
	int	v10 = 0;
	m_CListCtrl_Dialog_File_Manager_Client.SetItemData(m_CListCtrl_Dialog_File_Manager_Client.InsertItem(v10++,
		"..", GetServerIconIndex(NULL, FILE_ATTRIBUTE_DIRECTORY)), 1);
	if (BufferLength != 0)
	{
		// ������������������ʾ���б���
		for (int i = 0; i < 2; i++)
		{
			// ����Token   	//[Flag 1 HelloWorld\0��С ��С ʱ�� ʱ�� 0 1.txt\0 ��С ��С ʱ�� ʱ��]
			char *Travel = (char *)(BufferData + 1);
			//[1 HelloWorld\0��С ��С ʱ�� ʱ�� 0 1.txt\0 ��С ��С ʱ�� ʱ��]
			for (char *v1 = Travel; Travel - v1 < BufferLength - 1;)
			{
				char	*FileName = NULL;
				DWORD	FileSizeHigh = 0; // �ļ����ֽڴ�С
				DWORD	FileSizeLow = 0;  // �ļ����ֽڴ�С
				int		iItem = 0;
				bool	bIsInsert = false;
				FILETIME FileTime;

				int	v3 = *Travel ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
				// i Ϊ 0 ʱ����Ŀ¼��iΪ1ʱ���ļ�
				bIsInsert = !(v3 == FILE_ATTRIBUTE_DIRECTORY) == i;

				//0==1     0==0   !1  0

				////[HelloWorld\0��С ��С ʱ�� ʱ�� 0 1.txt\0 ��С ��С ʱ�� ʱ��]
				FileName = ++Travel;

				if (bIsInsert)
				{
					iItem = m_CListCtrl_Dialog_File_Manager_Client.InsertItem(v10++, FileName, GetServerIconIndex(FileName, v3));
					m_CListCtrl_Dialog_File_Manager_Client.SetItemData(iItem, v3 == FILE_ATTRIBUTE_DIRECTORY);   //��������
					SHFILEINFO	sfi;
					SHGetFileInfo(FileName, FILE_ATTRIBUTE_NORMAL | v3, &sfi, sizeof(SHFILEINFO),
						SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
					m_CListCtrl_Dialog_File_Manager_Client.SetItemText(iItem, 2, sfi.szTypeName);
				}
				// �õ��ļ���С
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
	// �ָ�����

	m_CListCtrl_Dialog_File_Manager_Client.EnableWindow(TRUE);
}

//ת����һ��Ŀ¼
void CDlgFileManager::OnServerFilePrevious()
{
	FixedServerFileList("..");
}

void CDlgFileManager::OnClientFilePrevious()
{
	GetClientFilesList("..");
}
//ɾ���ļ�
void CDlgFileManager::OnServerFileDelete()
{
	/*
	CString v1;
	if (m_CListCtrl_Dialog_File_Manager_Server.GetSelectedCount() > 1)
	{
		v1.Format("ȷ��Ҫ���� %d ��ɾ����?", m_CListCtrl_Dialog_File_Manager_Server.GetSelectedCount());
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
			v1.Format("ȷʵҪɾ���ļ��С�%s��������������ɾ����?", FileName);
		}
		else
		{
			v1.Format("ȷʵҪ�ѡ�%s��ɾ����?", FileName);
		}
	}
	if (::MessageBox(m_hWnd, v1, "ȷ��ɾ��", MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}
	//�������û��ҵ�
	EnableControl(FALSE);

	POSITION Pos = m_CListCtrl_Dialog_File_Manager_Server.GetFirstSelectedItemPosition();
	while (Pos)
	{
		int iItem = m_CListCtrl_Dialog_File_Manager_Server.GetNextSelectedItem(Pos);
		CString	FileFullPath = m_ServerFileFullPath + m_CListCtrl_Dialog_File_Manager_Server.GetItemText(iItem, 0);
		// �����Ŀ¼
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
	// �����ļ�������
	EnableControl(TRUE);
	//ˢ��Ŀ¼
	FixedServerFileList(".");*/
}

void CDlgFileManager::OnClientFileDelete()
{
	/*
	CString v1;
	if (m_CListCtrl_Dialog_File_Manager_Client.GetSelectedCount() > 1)
	{
		v1.Format("ȷ��Ҫ���� %d ��ɾ����?", m_CListCtrl_Dialog_File_Manager_Client.GetSelectedCount());
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
			v1.Format("ȷʵҪɾ���ļ��С�%s��������������ɾ����?", FileName);
		}
		else
		{
			v1.Format("ȷʵҪ�ѡ�%s��ɾ����?", FileName);
		}
	}
	if (::MessageBox(m_hWnd, v1, "ȷ��ɾ��", MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		return;
	}
	//�������û��ҵ�
	EnableControl(FALSE);

	POSITION Pos = m_CListCtrl_Dialog_File_Manager_Client.GetFirstSelectedItemPosition();
	while (Pos)
	{
		int iItem = m_CListCtrl_Dialog_File_Manager_Client.GetNextSelectedItem(Pos);
		CString	FileFullPath = m_ClientFileFullPath + m_CListCtrl_Dialog_File_Manager_Client.GetItemText(iItem, 0);
		// �����Ŀ¼
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
	// �����ļ�������
}
//�����ض��½��ļ���
void CDlgFileManager::OnServerFileNewFolder()
{
	if (m_ServerFileFullPath == "")
		return;


	CDlgFileNewFolder Dlg(this);
	if (Dlg.DoModal() == IDOK && Dlg.m_CEdit_New_Floder_Name.GetLength())
	{
		// �������Ŀ¼
		CString  v1;
		v1 = m_ServerFileFullPath + Dlg.m_CEdit_New_Floder_Name + "\\";
		MakeSureDirectoryPathExists(v1.GetBuffer());  /*c:\Hello\  */
		FixedServerFileList(".");
	}

}
//�ڿͻ����½��ļ���
void CDlgFileManager::OnClientFileNewFolder()
{
	if (m_ClientFileFullPath == "")
	{
		return;
	}
	CDlgFileNewFolder Dlg(this);
	if (Dlg.DoModal() == IDOK && Dlg.m_CEdit_New_Floder_Name.GetLength())
	{
		// �������Ŀ¼
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

//��ֹ�û��ڹ���������
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
	if (FileHandle == INVALID_HANDLE_VALUE) // ���û���ҵ������ʧ��
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

	FindClose(FileHandle); // �رղ��Ҿ��

	if (!RemoveDirectory(DirectoryFullPath))
	{
		return FALSE;
	}
	return TRUE;
}
//�������·���ļ���
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
		//���Ŀ¼
		while (*Travel)           //D:\Hello\\World\Shit\0
		{
			if (*Travel == '\\')
			{
				*Travel = '\0';
				DirectoryAttributes = GetFileAttributes(BufferData);   //�鿴�Ƿ��Ƿ�Ŀ¼  Ŀ¼������
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
//���������ͻ��˵��ļ�����
void CDlgFileManager::OnLvnBegindragListDialogFileManagerServer(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	if (m_ServerFileFullPath.IsEmpty() || m_ClientFileFullPath.IsEmpty())
	{
		return;
	}

	//	m_ulDragIndex = pNMLV->iItem;   //����Ҫ�ϵ���

	if (m_CListCtrl_Dialog_File_Manager_Server.GetSelectedCount() > 1) //�任������ʽ ���ѡ����������ק
	{
		//ѡ�������������
		m_CursorHwnd = AfxGetApp()->LoadCursor(IDC_CURSOR_MULITI_MDRAG);
	}
	else
	{
		//ѡ������������
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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (m_IsDragging)    //����ֻ����ק����Ȥ
	{
		CPoint Point(point);	 //������λ��
		ClientToScreen(&Point);  //ת��������Լ���Ļ��
	
								 //��������ô��ھ��
		CWnd* v1 = WindowFromPoint(Point);   //ֵ����λ�� ��û�пؼ�

		if (v1->IsKindOf(RUNTIME_CLASS(CListCtrl)))   //�������ǵĴ��ڷ�Χ��
		{
			//�ı������ʽ
			SetCursor(m_CursorHwnd);
			return;
		}
		else
		{
			SetCursor(LoadCursor(NULL, IDC_NO));   //�������ڻ������ʽ
		}
	}
	CDialog::OnMouseMove(nFlags, point);
}

void CDlgFileManager::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (m_IsDragging)
	{
		ReleaseCapture();  //�ͷ����Ĳ���

		m_IsDragging = FALSE;

		CPoint Point(point);    //��õ�ǰ����λ�������������Ļ��
		ClientToScreen(&Point); //ת��������ڵ�ǰ�û��Ĵ��ڵ�λ��

		CWnd* v1 = WindowFromPoint(Point);   //��õ�ǰ������·����޿ؼ�


		if (v1->IsKindOf(RUNTIME_CLASS(CListCtrl))) //�����һ��ListControl
		{
			m_DropControlList = (CListCtrl*)v1;       //���浱ǰ�Ĵ��ھ��
			DropItemDataOnList(); //������
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
	//�ӿͻ��˷����ļ���������
	if ((CWnd *)m_DropControlList == &m_CListCtrl_Dialog_File_Manager_Server)       //�ͻ��������ض�
	{
		OnCopyClientDataToServer();
	}
	//�ӷ����������ļ����ͻ���
	else if ((CWnd *)m_DropControlList == &m_CListCtrl_Dialog_File_Manager_Client)  //���ض���ͻ���
	{
		OnCopyServerFileToClient();
	}
	else
	{
		return;
	}
	
}
//�����ض˵����ض˿����ļ�
VOID CDlgFileManager::OnCopyServerFileToClient() 
{
	//����һ��ģ��  ��ģ�����ݽṹ��
	m_ServerFileToClientJob.RemoveAll();
	POSITION Position = m_CListCtrl_Dialog_File_Manager_Server.GetFirstSelectedItemPosition();
	while (Position)
	{
		int iItem = m_CListCtrl_Dialog_File_Manager_Server.GetNextSelectedItem(Position);
		CString	FileFullPath = NULL;

		FileFullPath = m_ServerFileFullPath + m_CListCtrl_Dialog_File_Manager_Server.GetItemText(iItem, 0);

		// �����Ŀ¼
		if (m_CListCtrl_Dialog_File_Manager_Server.GetItemData(iItem))
		{
			FileFullPath += '\\';
			FixedServerFileToClientDirectory(FileFullPath.GetBuffer(0));
		}
		else
		{
			// �жϴ��ļ��Ƿ�Ϸ�(��ӵ��ϴ������б���ȥ)
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
		//���ѡ���п�Ŀ¼�ǲ������
		::MessageBox(m_hWnd, "�ļ���Ϊ��", "����", MB_OK | MB_ICONWARNING);
		return;
	}
	EnableControl(FALSE);
	SendServerFileInfoToClient(); //���͵�һ������
}

//����Ŀ¼�е�����
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
	if (FileHandle == INVALID_HANDLE_VALUE) // ���û���ҵ������ʧ��
	{
		return FALSE;
	}
	do
	{
		// ����������Ŀ¼ '.'��'..'
		if (wfd.cFileName[0] == '.')
		{ 
			continue; 
		}
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			CHAR v1[MAX_PATH];
			sprintf(v1, "%s%s%s", DircetoryFullPath, Slash, wfd.cFileName);
			FixedServerFileToClientDirectory(v1); // ����ҵ�����Ŀ¼��������Ŀ¼���еݹ� 
		}
		else
		{
			CString FileFullPath;
			FileFullPath.Format("%s%s%s", DircetoryFullPath, Slash, wfd.cFileName);
			m_ServerFileToClientJob.AddTail(FileFullPath);
			// ���ļ����в��� 
		}
	} while (FindNextFile(FileHandle, &wfd));
	FindClose(FileHandle); // �رղ��Ҿ��
	return true;
}
//�����ļ����ͻ���
BOOL CDlgFileManager::SendServerFileInfoToClient() //�����ض˵����ض˵ķ�������
{
	if (m_ServerFileToClientJob.IsEmpty())
	{
		return FALSE;
	}
	CString	strDestDirectory = m_ClientFileFullPath;  //ȷ��Ŀ��·��(ȡ��Ŀ��ĸ�Ŀ¼)
	m_SourceFileFullPath = m_ServerFileToClientJob.GetHead();  //��õ�һ�����������   

	DWORD	FileLengthHigh;
	DWORD	FileLengthlow;
	// 1 �ֽ�token, 8�ֽڴ�С, �ļ�����, '\0'
	HANDLE	FileHandle;
	CString	v1 = m_SourceFileFullPath;//Զ���ļ�·��          
	
	//�õ�Ҫ���浽��Զ�̵��ļ�·��
	//�û�·��
	v1.Replace(m_ServerFileFullPath, m_ClientFileFullPath);  //D:1.txt     E:1.txt
	m_DestinationFileFullPath = v1;  //�����õ�����
	//�򿪱����ļ�
	FileHandle = CreateFile(m_SourceFileFullPath.GetBuffer(0), 
		GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);   
	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	//���Ҫ�����ļ��Ĵ�С
	FileLengthlow = GetFileSize(FileHandle, &FileLengthHigh);
	//�����ļ���С  ����֧���ļ����ͣ�
	//���ļ����ͣ���ҵ��
	m_OperatingFileLength = (FileLengthHigh * (MAXDWORD + 1)) + FileLengthlow;
	CloseHandle(FileHandle);
	// �������ݰ��������ļ�����

	ULONG BufferLength = v1.GetLength() + 10;
	BYTE*  BufferData = (BYTE*)LocalAlloc(LPTR, BufferLength);
	memset(BufferData, 0, BufferLength);

	BufferData[0] = CLIENT_FILE_MANAGER_SEND_FILE_INFORMATION;

	//[Flag 0001 0001 E:\1.txt\0 ]

	//�򱻿ض˷�����Ϣ COMMAND_FILE_SIZE ���ض˻�ִ��CreateLocalRecvFile�����Ӷ��ֳ��������һ����Ҫ�����ļ��Ѿ����ھͻ���յ� TOKEN_GET_TRANSFER_MODE 
	//��һ���Ǳ��ض˵���GetFileData�����Ӷ����յ�TOKEN_DATA_CONTINUE

	memcpy(BufferData + 1, &FileLengthHigh, sizeof(DWORD));
	memcpy(BufferData + 5, &FileLengthlow, sizeof(DWORD));

	memcpy(BufferData + 9, v1.GetBuffer(0), v1.GetLength() + 1);

	m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, BufferLength);

	LocalFree(BufferData);

	// �����������б���ɾ���Լ�
	m_ServerFileToClientJob.RemoveHead();
	return TRUE;
}
//������ͬ���ļ��Ĳ���
VOID CDlgFileManager::SendTransferMode()		//������ض˷��͵��ļ��ڱ��ض��ϴ�����ʾ��δ���  
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
//�������ˣ��ָ�����
VOID CDlgFileManager::EndCopyServerToClient()	//���������ͼ�������û�оͻָ�����	                       
{
	m_Counter = 0;
	m_OperatingFileLength = 0;
	ShowProgress();
	if (m_ServerFileToClientJob.IsEmpty() || m_IsStop)
	{
		m_ServerFileToClientJob.RemoveAll();
		m_IsStop = FALSE;
		EnableControl(TRUE); //�û����Ե���
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
//��ʾ������
void CDlgFileManager::ShowProgress()
{
	//�ָ�������
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

	ShowProgress(); //֪ͨ������
    //pFileSize->dwSizeLow == -1  �ǶԷ�ѡ��������    m_nCounter == m_nOperatingFileLength  ��ɵ�ǰ�Ĵ���
	if (m_Counter == m_OperatingFileLength || m_IsStop || v1->FileSizeLow == -1)     
	{
		EndCopyServerToClient(); //�����¸�����Ĵ����������
		return;
	}

	HANDLE	FileHandle;
	FileHandle = CreateFile(m_SourceFileFullPath.GetBuffer(0), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		return;
	}

	SetFilePointer(FileHandle, OffsetLow, &OffsetHigh, FILE_BEGIN);   //8192    4G  300  �����ļ���С

	int		v3 = 9; // 1 + 4 + 4  ���ݰ�ͷ����С��Ϊ�̶���9

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

//�ӱ��ض˵����ض˿����ļ�
VOID CDlgFileManager::OnCopyClientDataToServer()
{
	//���һ�������б�
	m_ClientFileToServerJob.RemoveAll();


	POSITION Pos = m_CListCtrl_Dialog_File_Manager_Client.GetFirstSelectedItemPosition();
	while (Pos)
	{
		int iItem = m_CListCtrl_Dialog_File_Manager_Client.GetNextSelectedItem(Pos);

		CString	FileFullPath = NULL;

		FileFullPath = m_ClientFileFullPath + m_CListCtrl_Dialog_File_Manager_Client.GetItemText(iItem, 0);

		// ������ļ���
		if (m_CListCtrl_Dialog_File_Manager_Client.GetItemData(iItem))
		{
			FileFullPath += '\\';
			//���ļ���·�����͵��ͻ��� �ÿͻ���ö���ļ�·�� ���ص�������
			char* BufferData = new char[FileFullPath.GetLength() + 1];
			BufferData[0] = CLIENT_FILE_MANAGER_SEND_COPY_DIRECTORY_FILEFULLPATH;
			memcpy(BufferData + 1, FileFullPath.GetBuffer(0), FileFullPath.GetLength());
			m_IOCPServer->OnPrepareSending(m_ContextObject, (PBYTE)BufferData, FileFullPath.GetLength() + 1);
		}
		else
		{
			//�����ļ���ӵ������б����
			m_ClientFileToServerJob.AddTail(FileFullPath);
			//���õ�ǰ���� ���ڿؼ����
			EnableControl(FALSE);
			//ת����һ���ļ�·�� 
			ClientToServerFileFullPath();
		}

	}

}

VOID CDlgFileManager::ClientToServerFileFullPath()
{
	//�ж������б��Ƿ�Ϊ��
	if (m_ClientFileToServerJob.IsEmpty())
		return;

	//��Ҫ���ļ���������·����
	// ���� E:\  ·��
	CString	strDestDirectory = m_ServerFileFullPath;

	//��õ�һ�������ļ�����·��
	m_SourceFileFullPath = m_ClientFileToServerJob.GetHead();
	//����  E:\�½��ļ���\��ҵ.txt   ·��


	//���ͻ��˵��ļ�����·�����͸��ͻ��� E:\�½��ļ���\��ҵ.txt
	PBYTE BufferData = new BYTE[m_SourceFileFullPath.GetLength() + 1];
	BufferData[0] = CLIENT_FILE_MANAGER_SEND_COPY_FILE_INFORMATION_FROM_CLIENT;
	memcpy(BufferData + 1, m_SourceFileFullPath.GetBuffer(0), m_SourceFileFullPath.GetLength());
	m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, m_SourceFileFullPath.GetLength() + 1);

	DWORD	SizeHigh;
	DWORD	SizeLow;

	HANDLE	FileHandle;
	CString	v1 = m_SourceFileFullPath;
	//���� v1 =  E:\�½��ļ���\��ҵ.txt    ·��

	//�� v1 �е� m_ServerFileFullPath�ַ��� �滻�� m_ClientFileFullPath�ַ���
	//����  E:\�½��ļ���\��ҵ.txt �е�  E:\�½��ļ���\ �滻�� E:\ 
	v1.Replace(m_ClientFileFullPath, m_ServerFileFullPath);

	//������Ŀ���ļ����յľ���·��Ϊ E:\��ҵ.txt
	m_DestinationFileFullPath = v1;

	memset(m_LocalFileFullPath, 0, MAX_PATH);
	memcpy(m_LocalFileFullPath, m_DestinationFileFullPath.GetBuffer(0), m_DestinationFileFullPath.GetLength());

	// �������Ŀ¼
	MakeSureDirectoryPathExists(m_LocalFileFullPath);

	//�����ļ�����·�� FindFirstFile�ܴ��ļ���Ŀ¼��������ļ��Ƿ����

	WIN32_FIND_DATA wfa;
	FileHandle = FindFirstFile(m_LocalFileFullPath, &wfa);

	//m_TransferMode����ȫ�����Ǻ�ȫ���滻 ���ҵ�ͬ���ļ�
	if (FileHandle != INVALID_HANDLE_VALUE
		&& m_TransferMode != TRANSFER_MODE_OVERWRITE_ALL
		&& m_TransferMode != TRANSFER_MODE_JUMP_ALL
		)
	{
		//�����·��������ͬ���Ƶ��ļ�
		LocalTransferMode();
	}
	else
	{
		//�ӿͻ��˻���ļ�����
		GetFileDataFromClient();
	}

	FindClose(FileHandle);
	// �������б���ɾ��
	m_ClientFileToServerJob.RemoveHead();
}
//������������ش���ͬ���ļ� ��ʾ��δ���  
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
	//����ļ�����
	GetFileDataFromClient();
}
//�ж� ��������б�Ϊ�ջ����յ�ֹͣ�����ֹͣ    
VOID CDlgFileManager::EndCopyClientToServer()
{
	m_Counter = 0;

	m_OperatingFileLength = 0;

	//������
	ShowProgressClientToServer();

	//�����б�Ϊ�� �����յ�ֹͣ����
	if (m_ClientFileToServerJob.IsEmpty() || m_IsStop)
	{
		m_ClientFileToServerJob.RemoveAll();
		m_IsStop = FALSE;
		EnableControl(TRUE);

		m_TransferMode = TRANSFER_MODE_NORMAL;
		//ˢ��
		FixedServerFileList(0);
	}
	else
	{
		Sleep(5);
		//��һ�� ת��·��
		ClientToServerFileFullPath();
	}
	return;
}
//�ͻ��˿�������������ʾ������
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
//�����ļ� ���ͽ����ļ���������
VOID CDlgFileManager::GetFileDataFromClient()
{
	int	iTransferMode;//�ļ�����ʽ

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

	DWORD	CreationDisposition; // �ļ��򿪷�ʽ 
	WIN32_FIND_DATA wfa;
	HANDLE FileHandle = FindFirstFile(m_LocalFileFullPath, &wfa);

	char BufferData[9] = { 0 };

	BufferData[0] = CLIENT_FILE_MANAGER_COPY_FILE_DATA_TO_SERVER;

	// �ļ��Ѿ�����
	if (FileHandle != INVALID_HANDLE_VALUE)
	{

		// ����
		if (iTransferMode == TRANSFER_MODE_OVERWRITE)
		{
			memset(BufferData + 1, 0, 8);//0000 0000	
			CreationDisposition = CREATE_ALWAYS;//����һ���µ��ļ�  
		}
		// ����
		else if (iTransferMode == TRANSFER_MODE_JUMP)
		{
			DWORD v1 = -1;
			memcpy(BufferData + 5, &v1, 4);//0000 -1
			CreationDisposition = OPEN_EXISTING;
		}
	}
	// �ļ�������
	else
	{
		memset(BufferData + 1, 0, 8);//0000 0000	
		CreationDisposition = CREATE_ALWAYS;//����һ���µ��ļ�
	}
	FindClose(FileHandle);

	FileHandle = CreateFile(m_LocalFileFullPath, GENERIC_WRITE, FILE_SHARE_WRITE,
		NULL,
		CreationDisposition, // �ļ��򿪷�ʽ 
		FILE_ATTRIBUTE_NORMAL,
		0
	);
	// ��Ҫ������
	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		m_OperatingFileLength = 0;
		return;
	}
	CloseHandle(FileHandle);

	//���ݰ�����+[]+[]
	m_IOCPServer->OnPrepareSending(m_ContextObject, (PBYTE)BufferData, 9);
}
//�����ܵ��ļ�����д���ļ��� 
VOID CDlgFileManager::WriteServerReceiveFile(LPBYTE BufferData, ULONG BufferLength)
{
	BYTE	*Travel;
	DWORD	NumberOfBytesToWrite = 0;
	DWORD	NumberOfBytesWirte = 0;
	int		v3 = 9; // 1 + 4 + 4  ���ݰ�ͷ����С��Ϊ�̶���9
	FILE_SIZE* v1;

	v1 = (FILE_SIZE *)BufferData;

	// �õ��������ļ��е�ƫ��
	LONG	OffsetHigh = v1->FileSizeHigh;
	LONG	OffsetLow = v1->FileSizeLow;

	// �õ����ݵ�ƫ��
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
	//��λ�ļ�ָ��
	SetFilePointer(FileHandle, OffsetLow, &OffsetHigh, FILE_BEGIN);

	// д���ļ�
	WriteFile
	(
		FileHandle,
		Travel,
		NumberOfBytesToWrite,
		&NumberOfBytesWirte,
		NULL
	);

	//������Ϊɶ���������ߣ�����
	m_Counter = MAKEINT64(v1->FileSizeLow, v1->FileSizeHigh);
	ShowProgressClientToServer(); //֪ͨ������


	CloseHandle(FileHandle);
	BYTE	IsToken[9];

	//�������ͽ�����������
	IsToken[0] = CLIENT_FILE_MANAGER_COPY_FILE_DATA_TO_SERVER;

	//�ļ�����ƫ�Ƽӹ�ȥ
	OffsetLow += NumberOfBytesWirte;

	memcpy(IsToken + 1, &OffsetHigh, sizeof(OffsetHigh));
	memcpy(IsToken + 5, &OffsetLow, sizeof(OffsetLow));

	m_IOCPServer->OnPrepareSending(m_ContextObject, IsToken, sizeof(IsToken));
}

void CDlgFileManager::OnLvnBegindragListDialogFileManagerClient(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	//��ק֮ǰ�����������ļ�·��
	if (m_ServerFileFullPath.IsEmpty() || m_ClientFileFullPath.IsEmpty())
	{
		return;
	}

	//�任������ʽ 
	if (m_CListCtrl_Dialog_File_Manager_Client.GetSelectedCount() > 1)
	{
		//���ѡ����������ק
		m_CursorHwnd = AfxGetApp()->LoadCursor(IDC_CURSOR_MULITI_MDRAG);
	}
	else
	{
		//���ֻ��קһ��
		m_CursorHwnd = AfxGetApp()->LoadCursor(IDC_CURSOR_SINGLE_SDRAG);
	}

	//�Ƿ���ק���TRUE
	m_IsDragging = TRUE;

	//��קԴ�б���ɿͻ���List
	m_DragControlList = &m_CListCtrl_Dialog_File_Manager_Client;
	m_DropControlList = &m_CListCtrl_Dialog_File_Manager_Client;

	//������겶��
	SetCapture();

	*pResult = 0;
}
