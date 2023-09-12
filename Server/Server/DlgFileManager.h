#pragma once
#include"_IOCPServer.h"
#include"Common.h"
#include "TrueColorToolBar.h"
#include "afxcmn.h"
#include "afxwin.h"
// CDlgFileManager �Ի���
typedef struct
{
	DWORD	FileSizeHigh;
	DWORD	FileSizeLow;
}FILE_SIZE;

#define MAKEINT64(low, high) ((unsigned __int64)(((DWORD)(low)) | ((unsigned __int64)((DWORD)(high))) << 32))
typedef CList<CString, CString&> ListJobTemplate;


//static __int64  m_OperatingFileLengthFromClient;  // ��ǰ�����ļ��ܴ�С �ͻ��˵�������



class CDlgFileManager : public CDialog
{
	DECLARE_DYNAMIC(CDlgFileManager)

public:
	CDlgFileManager(CWnd* Parent, _CIOCPServer* IOCPServer = NULL,
		CONTEXT_OBJECT *ContextObject = NULL);   // ��׼���캯��
	virtual ~CDlgFileManager();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_FILE_MANAGER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

public:
	_CIOCPServer* m_IOCPServer;
	PCONTEXT_OBJECT m_ContextObject;
	HICON m_IconHwnd;
	BYTE	m_ClientData[0x1000];
	BYTE    m_ServerData[0x1000];
	
	CString m_ServerFileFullPath;
	CString m_ClientFileFullPath;

	//����ק�ļ�
	BOOL m_IsDragging;
	HCURSOR    m_CursorHwnd;
	//�����ļ��Ĺ��̿���ֹͣ
	BOOL m_IsStop;
	CListCtrl* m_DragControlList;
	CListCtrl* m_DropControlList;
	ListJobTemplate m_ServerFileToClientJob; //���������д�������
	ListJobTemplate m_ClientFileToServerJob; //�ͻ������д�������
	CString m_SourceFileFullPath;       //  ��������
	CString m_DestinationFileFullPath;  
	__int64  m_OperatingFileLength;    //��ǰ�����ļ��ܴ�С
	ULONG   m_TransferMode;
	__int64 m_Counter;         //������


	//�ͻ����ļ��ϵ�����
	char  m_LocalFileFullPath[MAX_PATH];//�ͻ����ļ��ϵ����ص�Ŀ��·��	



	//�������
	CImageList* m_CImageList_Large;
	CImageList* m_CImageList_Small;
	//����������
	CTrueColorToolBar m_ToolBar_Server_Dialog_File_Manager; 
	CTrueColorToolBar m_ToolBar_Client_Dialog_File_Manager;
	//��ʾ�ļ��б�
	CListCtrl m_CListCtrl_Dialog_File_Manager_Server;
	CListCtrl m_CListCtrl_Dialog_File_Manager_Client;

	CStatusBar     m_StatusBar; // ����������״̬��
	CProgressCtrl* m_ProgressCtrl;//��̬�Ľ�����
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
	//������һ��
	CString CDlgFileManager::GetParentDirectory(CString FileFullPath);
	VOID CDlgFileManager::GetClientFilesList(CString Directory = "");
	VOID CDlgFileManager::FixedClientFilesList(BYTE *BufferData, ULONG BufferLength);

	afx_msg void OnNMDblclkListDialogFileManagerServer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListDialogFileManagerClient(NMHDR *pNMHDR, LRESULT *pResult);
	
	//������һ���˵�
	afx_msg void OnServerFilePrevious();
	afx_msg void OnClientFilePrevious();
	//ɾ���ļ�
	afx_msg void OnServerFileDelete();
	afx_msg void OnClientFileDelete();
	BOOL CDlgFileManager::DeleteDirectory(LPCTSTR DirectoryFullPath);
	void CDlgFileManager::EnableControl(BOOL bEnable);
	//�½��ļ�
	afx_msg void OnServerFileNewFolder();
	afx_msg void OnClientFileNewFolder();
	BOOL CDlgFileManager::MakeSureDirectoryPathExists(char* DirectoryFullPath);
	//

	afx_msg void OnServerFileStop();
	afx_msg void OnServerViewFileSmall();
	afx_msg void OnServerViewFileList();
	afx_msg void OnServerViewFileDetail();
	
	//С����
	afx_msg void OnBnClickedButton1();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButton2();

	//��ק�ļ�
	afx_msg void OnLvnBegindragListDialogFileManagerServer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	VOID CDlgFileManager::DropItemDataOnList();
	VOID CDlgFileManager::OnCopyServerFileToClient();//�����ض˵����ض�
	VOID CDlgFileManager::OnCopyClientDataToServer();//�ӱ��ض˵����ض�
	BOOL CDlgFileManager::FixedServerFileToClientDirectory(LPCTSTR DircetoryFullPath);
	BOOL CDlgFileManager::SendServerFileInfoToClient();
	VOID CDlgFileManager::SendTransferMode();
	VOID CDlgFileManager::EndCopyServerToClient();
	void CDlgFileManager::ShowProgress();
	VOID CDlgFileManager::SendServerFileDataToClient();
	VOID CDlgFileManager::ClientToServerFileFullPath();
	
	VOID CDlgFileManager::LocalTransferMode();
	//�ж� ��������б�Ϊ�ջ����յ�ֹͣ�����ֹͣ    
	VOID CDlgFileManager::EndCopyClientToServer();
	//�ͻ��˿�������������ʾ������
	void CDlgFileManager::ShowProgressClientToServer();
	//�����ļ� ���ͽ����ļ���������
	VOID CDlgFileManager::GetFileDataFromClient();
	VOID CDlgFileManager::WriteServerReceiveFile(LPBYTE BufferData, ULONG BufferLength);

	afx_msg void OnLvnBegindragListDialogFileManagerClient(NMHDR *pNMHDR, LRESULT *pResult);
};
