#pragma once
#include "afxcmn.h"
#include"_IOCPServer.h"
#include"Common.h"

// CDlgModuleManager �Ի���

class CDlgModuleManager : public CDialog
{
	DECLARE_DYNAMIC(CDlgModuleManager)

public:
	CDlgModuleManager(CWnd* Parent, _CIOCPServer* IOCPServer = NULL,
		CONTEXT_OBJECT *ContextObject = NULL);   // ��׼���캯��
	virtual ~CDlgModuleManager();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DI ALOG_MODULE_PROCESS_MANAGER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	_CIOCPServer* m_IOCPServer;
	PCONTEXT_OBJECT m_ContextObject;
	HICON m_IconHwnd;
	char* m_BufferData;
	CListCtrl m_CListCtrl_Dialog_Module_Manager_Show;
	
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	void CDlgModuleManager::ShowModuleList();
};
