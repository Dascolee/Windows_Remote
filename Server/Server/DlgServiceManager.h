#pragma once
#include"_IOCPServer.h"
#include"Common.h"
#include "afxcmn.h"

// CDlgServiceManager 对话框

class CDlgServiceManager : public CDialog
{
	DECLARE_DYNAMIC(CDlgServiceManager)

public:
	CDlgServiceManager(CWnd* Parent, _CIOCPServer* IOCPServer = NULL,
		CONTEXT_OBJECT *ContextObject = NULL);   // 标准构造函数
	virtual ~CDlgServiceManager();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SERVICE_MANAGER };
#endif


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	_CIOCPServer* m_IOCPServer;
	PCONTEXT_OBJECT m_ContextObject;
	HICON m_IconHwnd;	
	CListCtrl m_CListCtrl_Dialog_Service_Manager_Show;
public:
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	void CDlgServiceManager::WindowHandleIO();
	int CDlgServiceManager::ShowClientServiceList();
	afx_msg void OnNMRClickListServiceManagerShow(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMenuServiceManagerListRinghtRefresh();
	afx_msg void OnMenuServiceManagerListRinghtStart();
	afx_msg void OnMenuServiceManagerListRinghtStop();
	void CDlgServiceManager::ConfigClientService(BYTE IsMethod);
	afx_msg void OnMenuServiceManagerListRinghtAutoRun();
	afx_msg void OnMenuServiceManagerListRinghtDemandRun();
};
