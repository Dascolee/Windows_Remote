#pragma once


// CDlgFileNewFolder �Ի���

class CDlgFileNewFolder : public CDialog
{
	DECLARE_DYNAMIC(CDlgFileNewFolder)

public:
	CDlgFileNewFolder(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgFileNewFolder();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_FILE_MANAGER_NEW_FOLDER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	// //Ҫ�����ļ����ļ���
	CString m_CEdit_New_Floder_Name;
};
