// DlgServerSet.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "DlgServerSet.h"
#include "afxdialogex.h"


// CDlgServerSet 对话框
extern _CConfigFlie __ConfigFlie;  


IMPLEMENT_DYNAMIC(CDlgServerSet, CDialog)

CDlgServerSet::CDlgServerSet(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_SERVER_DIALOG_SET, pParent)
	, m_CEdit_Dialog_Server_Set_Listen_Connection(0)
	, m_CEdit_Dialog_Server_Set_Listen_Port(0)
{

}

CDlgServerSet::~CDlgServerSet()
{
}

void CDlgServerSet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_DIALOG_SERVER_SET_MAX_CONNECTION, m_CEdit_Dialog_Server_Set_Listen_Connection);
	DDX_Text(pDX, IDC_EDIT_DIALOG_SERVER_SET_LISTEN_PORT, m_CEdit_Dialog_Server_Set_Listen_Port);
	DDV_MinMaxLong(pDX, m_CEdit_Dialog_Server_Set_Listen_Connection, 1, 100);
	DDV_MinMaxLong(pDX, m_CEdit_Dialog_Server_Set_Listen_Port, 2048, 65536);
	DDX_Control(pDX, IDC_EDIT_DIALOG_SERVER_SET_APPLY, m_CButton_Dialog_Server_Set_Apply);
}


BEGIN_MESSAGE_MAP(CDlgServerSet, CDialog)
	ON_BN_CLICKED(IDC_EDIT_DIALOG_SERVER_SET_APPLY, &CDlgServerSet::OnBnClickedEditDialogServerSetApply)

	ON_EN_CHANGE(IDC_EDIT_DIALOG_SERVER_SET_MAX_CONNECTION, &CDlgServerSet::OnEnChangeEditDialogServerSetMaxConnection)
	ON_EN_CHANGE(IDC_EDIT_DIALOG_SERVER_SET_LISTEN_PORT, &CDlgServerSet::OnEnChangeEditDialogServerSetListenPort)
END_MESSAGE_MAP()


// CDlgServerSet 消息处理程序


void CDlgServerSet::OnBnClickedEditDialogServerSetApply()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);

	__ConfigFlie.SetInt("Settings", "ListenPort",
		m_CEdit_Dialog_Server_Set_Listen_Port);
	__ConfigFlie.SetInt("Settings", "MaxConnection",
		m_CEdit_Dialog_Server_Set_Listen_Connection);

	m_CButton_Dialog_Server_Set_Apply.EnableWindow(FALSE);
	m_CButton_Dialog_Server_Set_Apply.ShowWindow(SW_HIDE);
	//SendMessage(WM_CLOSE);
}

void CDlgServerSet::OnEnChangeEditDialogServerSetMaxConnection()
{
	m_CButton_Dialog_Server_Set_Apply.ShowWindow(SW_NORMAL);
	m_CButton_Dialog_Server_Set_Apply.EnableWindow(TRUE);
}

void CDlgServerSet::OnEnChangeEditDialogServerSetListenPort()
{
	m_CButton_Dialog_Server_Set_Apply.ShowWindow(SW_NORMAL);
	m_CButton_Dialog_Server_Set_Apply.EnableWindow(TRUE);
}


BOOL CDlgServerSet::OnInitDialog()
{
	CDialog::OnInitDialog();
	// TODO:  在此添加额外的初始化
	int ListenPort = 0;
	int MaxConnection = 0;

	ListenPort    = __ConfigFlie.GetInt("Settings", "ListenPort");
	MaxConnection = __ConfigFlie.GetInt("Settings", "MaxConnection");

	m_CEdit_Dialog_Server_Set_Listen_Connection = MaxConnection;
	m_CEdit_Dialog_Server_Set_Listen_Port = ListenPort;
	
	
	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
