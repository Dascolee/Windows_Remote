#pragma once
#include"_IOCPServer.h"
#include"Common.h"
#include "afxcmn.h"

// CDlgProcessManager �Ի���

class CDlgProcessManager : public CDialog
{
	DECLARE_DYNAMIC(CDlgProcessManager)

public:
	CDlgProcessManager(CWnd* Parent, _CIOCPServer* IOCPServer = NULL,
		CONTEXT_OBJECT *ContextObject = NULL);   // ��׼���캯��
	virtual ~CDlgProcessManager();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PROCESS_MANAGER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

public:
	_CIOCPServer* m_IOCPServer;
	PCONTEXT_OBJECT m_ContextObject;
	HICON m_IconHwnd;
	CListCtrl m_CListCtrl_Dialog_Process_Manager_Show;
	char* m_BufferData;
	
public:
	void CDlgProcessManager::WindowHandleIO(void);
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	void CDlgProcessManager::ShowProcessList();
	afx_msg void OnNMCustomdrawListProcessManagerShow(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnButtonMenuProcessManagerListRefresh();
	afx_msg void OnNMRClickListProcessManagerShow(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnButtonMenuProcessManagerKillOne();

	//ɱ������
	void CDlgProcessManager::ClientProcessKill();
	afx_msg void OnButtonMenuProcessManagerEnumThreader();
	afx_msg void OnButtonMenuProcessManagerEnumHandle();
	afx_msg void OnButtonMenuProcessManagerEnumModule();
	afx_msg void OnButtonMenuProcessManagerOpenProcess();


	LRESULT CDlgProcessManager::OnRemoteOpenProcessDialog(WPARAM ParamterData1, LPARAM ParamterData2);
};
