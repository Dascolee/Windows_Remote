#pragma once

#include"_ConfigFlie.h"
#include "afxwin.h"

// CDlgServerSet 对话框

class CDlgServerSet : public CDialog
{
	DECLARE_DYNAMIC(CDlgServerSet)

public:
	CDlgServerSet(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgServerSet();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SERVER_DIALOG_SET };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	long m_CEdit_Dialog_Server_Set_Listen_Connection;
	long m_CEdit_Dialog_Server_Set_Listen_Port;
	CButton m_CButton_Dialog_Server_Set_Apply;
	afx_msg void OnBnClickedEditDialogServerSetApply();
	afx_msg void OnEnChangeEditDialogServerSetMaxConnection();
	afx_msg void OnEnChangeEditDialogServerSetListenPort();
	virtual BOOL OnInitDialog();

};
