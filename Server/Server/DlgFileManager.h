#pragma once
#include"_IOCPServer.h"
#include"Common.h"
#include "TrueColorToolBar.h"
#include "afxcmn.h"
#include "afxwin.h"
// CDlgFileManager 对话框
typedef struct
{
	DWORD	FileSizeHigh;
	DWORD	FileSizeLow;
}FILE_SIZE;

#define MAKEINT64(low, high) ((unsigned __int64)(((DWORD)(low)) | ((unsigned __int64)((DWORD)(high))) << 32))
typedef CList<CString, CString&> ListJobTemplate;


//static __int64  m_OperatingFileLengthFromClient;  // 当前操作文件总大小 客户端到服务器



class CDlgFileManager : public CDialog
{
	DECLARE_DYNAMIC(CDlgFileManager)

public:
	CDlgFileManager(CWnd* Parent, _CIOCPServer* IOCPServer = NULL,
		CONTEXT_OBJECT *ContextObject = NULL);   // 标准构造函数
	virtual ~CDlgFileManager();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_FILE_MANAGER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	_CIOCPServer* m_IOCPServer;
	PCONTEXT_OBJECT m_ContextObject;
	HICON m_IconHwnd;
	BYTE	m_ClientData[0x1000];
	BYTE    m_ServerData[0x1000];
	
	CString m_ServerFileFullPath;
	CString m_ClientFileFullPath;

	//拖拉拽文件
	BOOL m_IsDragging;
	HCURSOR    m_CursorHwnd;
	//拷贝文件的过程可以停止
	BOOL m_IsStop;
	CListCtrl* m_DragControlList;
	CListCtrl* m_DropControlList;
	ListJobTemplate m_ServerFileToClientJob; //服务器所有传输任务
	ListJobTemplate m_ClientFileToServerJob; //客户端所有传输任务
	CString m_SourceFileFullPath;       //  游走任务
	CString m_DestinationFileFullPath;  
	__int64  m_OperatingFileLength;    //当前操作文件总大小
	ULONG   m_TransferMode;
	__int64 m_Counter;         //进度条


	//客户端文件拖到本地
	char  m_LocalFileFullPath[MAX_PATH];//客户端文件拖到本地的目标路径	



	//界面相关
	CImageList* m_CImageList_Large;
	CImageList* m_CImageList_Small;
	//两个工具栏
	CTrueColorToolBar m_ToolBar_Server_Dialog_File_Manager; 
	CTrueColorToolBar m_ToolBar_Client_Dialog_File_Manager;
	//显示文件列表
	CListCtrl m_CListCtrl_Dialog_File_Manager_Server;
	CListCtrl m_CListCtrl_Dialog_File_Manager_Client;

	CStatusBar     m_StatusBar; // 带进度条的状态栏
	CProgressCtrl* m_ProgressCtrl;//动态的进度条
	CStatic m_CStatic_Dialog_File_Manager_Server_Position;
	CStatic m_CStatic_Dialog_File_Manager_Client_Position;

	CComboBox m_CCombo_Dialog_File_Manager_Server_File;
	CComboBox m_CCombo_Dialog_File_Manager_Client_File;


public:

	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	void CDlgFileManager::WindowHandleIO();
	VOID CDlgFileManager::FixedServerVolumeList();
	int	CDlgFileManager::GetServerIconIndex(LPCTSTR Volume, DWORD FileAttributes);
	VOID CDlgFileManager::FixedClientVolumeList();
	VOID CDlgFileManager::FixedServerFileList(CString strDirectory = "");
	//返回上一层
	CString CDlgFileManager::GetParentDirectory(CString FileFullPath);
	VOID CDlgFileManager::GetClientFilesList(CString Directory = "");
	VOID CDlgFileManager::FixedClientFilesList(BYTE *BufferData, ULONG BufferLength);

	afx_msg void OnNMDblclkListDialogFileManagerServer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListDialogFileManagerClient(NMHDR *pNMHDR, LRESULT *pResult);
	
	//返回上一级菜单
	afx_msg void OnServerFilePrevious();
	afx_msg void OnClientFilePrevious();
	//删除文件
	afx_msg void OnServerFileDelete();
	afx_msg void OnClientFileDelete();
	BOOL CDlgFileManager::DeleteDirectory(LPCTSTR DirectoryFullPath);
	void CDlgFileManager::EnableControl(BOOL bEnable);
	//新建文件
	afx_msg void OnServerFileNewFolder();
	afx_msg void OnClientFileNewFolder();
	BOOL CDlgFileManager::MakeSureDirectoryPathExists(char* DirectoryFullPath);
	//

	afx_msg void OnServerFileStop();
	afx_msg void OnServerViewFileSmall();
	afx_msg void OnServerViewFileList();
	afx_msg void OnServerViewFileDetail();
	
	//小功能
	afx_msg void OnBnClickedButton1();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButton2();

	//拖拽文件
	afx_msg void OnLvnBegindragListDialogFileManagerServer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	VOID CDlgFileManager::DropItemDataOnList();
	VOID CDlgFileManager::OnCopyServerFileToClient();//从主控端到被控端
	VOID CDlgFileManager::OnCopyClientDataToServer();//从被控端到主控端
	BOOL CDlgFileManager::FixedServerFileToClientDirectory(LPCTSTR DircetoryFullPath);
	BOOL CDlgFileManager::SendServerFileInfoToClient();
	VOID CDlgFileManager::SendTransferMode();
	VOID CDlgFileManager::EndCopyServerToClient();
	void CDlgFileManager::ShowProgress();
	VOID CDlgFileManager::SendServerFileDataToClient();
	VOID CDlgFileManager::ClientToServerFileFullPath();
	
	VOID CDlgFileManager::LocalTransferMode();
	//判断 如果任务列表为空或者收到停止请求便停止    
	VOID CDlgFileManager::EndCopyClientToServer();
	//客户端拷贝到服务器显示进度条
	void CDlgFileManager::ShowProgressClientToServer();
	//创建文件 发送接收文件数据请求
	VOID CDlgFileManager::GetFileDataFromClient();
	VOID CDlgFileManager::WriteServerReceiveFile(LPBYTE BufferData, ULONG BufferLength);

	afx_msg void OnLvnBegindragListDialogFileManagerClient(NMHDR *pNMHDR, LRESULT *pResult);
};
