#pragma once


// CDlgFileMode �Ի���

class CDlgFileMode : public CDialog
{
	DECLARE_DYNAMIC(CDlgFileMode)

public:
	CDlgFileMode(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgFileMode();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_FILE_MODE_SAME };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg	void OnEndDialog(UINT Key);
	CString m_FileFullPath;
};
