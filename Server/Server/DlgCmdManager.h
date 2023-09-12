#pragma once
#include"_IOCPServer.h"
#include"Common.h"
#include "afxwin.h"


// CDlgCmdManager 对话框

class CDlgCmdManager : public CDialog
{
	DECLARE_DYNAMIC(CDlgCmdManager)

public:
	CDlgCmdManager(CWnd * pParent, _CIOCPServer * IOCPServer, CONTEXT_OBJECT * ContextObject);   // 标准构造函数
	virtual ~CDlgCmdManager();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CMD_MANAGER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	_CIOCPServer* m_IOCPServer;
	PCONTEXT_OBJECT m_ContextObject;
	HICON m_IconHwnd;
	char* m_BufferData;
	CEdit m_CEidt_Dialog_Cmd_Manager_Main;
private:
	UINT            m_911;
	UINT            m_ShowDataLength;
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	void CDlgCmdManager::WindowHandleIO();
	void CDlgCmdManager::ShowCmdData();
	
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
