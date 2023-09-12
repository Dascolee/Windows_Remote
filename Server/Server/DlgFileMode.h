#pragma once


// CDlgFileMode 对话框

class CDlgFileMode : public CDialog
{
	DECLARE_DYNAMIC(CDlgFileMode)

public:
	CDlgFileMode(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgFileMode();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_FILE_MODE_SAME };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg	void OnEndDialog(UINT Key);
	CString m_FileFullPath;
};
