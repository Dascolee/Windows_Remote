#pragma once
#include"_IOCPServer.h"
#include"Common.h"
#include "afxcmn.h"

// DlgHandleManager 对话框

class CDlgHandleManager : public CDialog
{
	DECLARE_DYNAMIC(CDlgHandleManager)

public:
	CDlgHandleManager(CWnd* Parent, _CIOCPServer* IOCPServer = NULL,
		CONTEXT_OBJECT *ContextObject = NULL);   // 标准构造函数
	virtual ~CDlgHandleManager();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_HADNLE_PROCESS_MANAGER };
#endif
public:
	_CIOCPServer* m_IOCPServer;
	PCONTEXT_OBJECT m_ContextObject;
	HICON m_IconHwnd;
	char* m_BufferData;
	CListCtrl m_CListCtrl_Dialog_Handle_Manager_Show;



protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	void CDlgHandleManager::ShowHandleList();
	
};
