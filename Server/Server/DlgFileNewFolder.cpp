// DlgFileNewFolder.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "DlgFileNewFolder.h"
#include "afxdialogex.h"


// CDlgFileNewFolder 对话框

IMPLEMENT_DYNAMIC(CDlgFileNewFolder, CDialog)

CDlgFileNewFolder::CDlgFileNewFolder(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG_FILE_MANAGER_NEW_FOLDER, pParent)
	, m_CEdit_New_Floder_Name(_T(""))
{

}

CDlgFileNewFolder::~CDlgFileNewFolder()
{
}

void CDlgFileNewFolder::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FILE_NEW_FLOFER_NAME, m_CEdit_New_Floder_Name);
}


BEGIN_MESSAGE_MAP(CDlgFileNewFolder, CDialog)
	ON_BN_CLICKED(IDOK, &CDlgFileNewFolder::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgFileNewFolder 消息处理程序


void CDlgFileNewFolder::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码



	CDialog::OnOK();
}
