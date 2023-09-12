#pragma once
#include"_IOCPServer.h"
#include"Common.h"
#include "afxwin.h"

// CDlgRemoteMessageDlg �Ի���

class CDlgRemoteMessageDlg : public CDialog
{
	DECLARE_DYNAMIC(CDlgRemoteMessageDlg)

public:
	CDlgRemoteMessageDlg(CWnd* Parent, _CIOCPServer* IOCPServer = NULL,
		CONTEXT_OBJECT *ContextObject = NULL);   // ��׼���캯��
	virtual ~CDlgRemoteMessageDlg();
	virtual BOOL OnInitDialog();
// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_REMOTE_MESSAGE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	DECLARE_MESSAGE_MAP()
public:
	_CIOCPServer* m_IOCPServer;
	PCONTEXT_OBJECT m_ContextObject;
	CEdit m_CEdit_Dialog_Remote_Message;
	HICON m_IconHwnd;

public:
	afx_msg void OnClose();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	VOID CDlgRemoteMessageDlg::OnSending();
};
