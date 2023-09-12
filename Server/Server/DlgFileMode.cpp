// DlgFileMode.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "DlgFileMode.h"
#include "afxdialogex.h"


// CDlgFileMode 对话框

IMPLEMENT_DYNAMIC(CDlgFileMode, CDialog)

CDlgFileMode::CDlgFileMode(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG_FILE_MODE_SAME, pParent)
{

}

CDlgFileMode::~CDlgFileMode()
{
}

void CDlgFileMode::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgFileMode, CDialog)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_BUTTON_TRANSFER_MODE_JUMP, IDC_BUTTON_TRANSFER_MODE_OVERWRITE_ALL, OnEndDialog)
END_MESSAGE_MAP()


// CDlgFileMode 消息处理程序


BOOL CDlgFileMode::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString	strTips;
	strTips.Format("衰了 咋办.... \" %s \" ", m_FileFullPath);

	for (int i = 0; i < strTips.GetLength(); i += 120)
	{
		strTips.Insert(i, "\n");
		i += 1;
	}
	SetDlgItemText(IDC_TIPS, strTips);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CDlgFileMode::OnEndDialog(UINT Key)
{
	EndDialog(Key);
}
