// DlgFileNewFolder.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "DlgFileNewFolder.h"
#include "afxdialogex.h"


// CDlgFileNewFolder �Ի���

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


// CDlgFileNewFolder ��Ϣ�������


void CDlgFileNewFolder::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������



	CDialog::OnOK();
}
