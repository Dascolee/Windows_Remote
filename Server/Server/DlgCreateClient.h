#pragma once


// CDlgCreateClient �Ի���

class CDlgCreateClient : public CDialog
{
	DECLARE_DYNAMIC(CDlgCreateClient)

public:
	CDlgCreateClient(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgCreateClient();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CREATE_CLIENT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

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
