#pragma once
#include "Common.h"
#include"_IOCPServer.h"

// CDlgRemoteOpenProcess �Ի���

class CDlgRemoteOpenProcess : public CDialog
{
	DECLARE_DYNAMIC(CDlgRemoteOpenProcess)

public:
	CDlgRemoteOpenProcess(CWnd * pParent, _CIOCPServer * IOCPServer);   // ��׼���캯��
	virtual ~CDlgRemoteOpenProcess();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_WRITE_REMOTE_PROCESS_NAME };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CString m_CEdit_Remote_Open_Process_Name;
	afx_msg void OnBnClickedOk();		

	HWND m_hwnd;
	
};
