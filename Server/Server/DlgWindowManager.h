#pragma once
#include "afxcmn.h"
#include"_IOCPServer.h"
#include"Common.h"

// CDlgWindowManager �Ի���

class CDlgWindowManager : public CDialog
{
	DECLARE_DYNAMIC(CDlgWindowManager)

public:
	CDlgWindowManager(CWnd* pParent, _CIOCPServer* IOCPServer, CONTEXT_OBJECT* ContextObject);   // ��׼���캯��
	virtual ~CDlgWindowManager();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_WINDOW_MANAGER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:

	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	void CDlgWindowManager::WindowHandleIO(void);
	void CDlgWindowManager::ShowWindowsList(void);
	afx_msg void OnNMRClickListWindowManagerShow(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnButtonMenuWindowManagerListRefresh();
public:
	_CIOCPServer* m_IOCPServer;
	PCONTEXT_OBJECT m_ContextObject;
	HICON m_IconHwnd;
	char* m_BufferData;
	CListCtrl m_CListCtrl_Dialog_Window_Manager_Show;
	
};
