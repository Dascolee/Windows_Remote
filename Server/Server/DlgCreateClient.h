#pragma once


// CDlgCreateClient 对话框

class CDlgCreateClient : public CDialog
{
	DECLARE_DYNAMIC(CDlgCreateClient)

public:
	CDlgCreateClient(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgCreateClient();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CREATE_CLIENT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()




	CString m_Edit_Create_Client_Link_IP;
	CString m_Edit_Create_Client_Link_Port;
	afx_msg void OnBnClickedOk();

public:
	int CDlgCreateClient::MemoryFind(const char *BufferData,
		const char* KeyValue,
		int BufferLenght,
		int KeyLenght);
};
