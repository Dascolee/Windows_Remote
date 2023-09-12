#pragma once


// CDlgFileNewFolder 对话框

class CDlgFileNewFolder : public CDialog
{
	DECLARE_DYNAMIC(CDlgFileNewFolder)

public:
	CDlgFileNewFolder(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgFileNewFolder();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_FILE_MANAGER_NEW_FOLDER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	// //要创建文件的文件名
	CString m_CEdit_New_Floder_Name;
};
