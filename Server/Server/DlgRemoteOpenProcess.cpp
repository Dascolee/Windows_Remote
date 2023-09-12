// DlgRemoteOpenProcess.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "DlgRemoteOpenProcess.h"
#include "afxdialogex.h"

char __ProcessName[MAX_PATH] = {0};
// CDlgRemoteOpenProcess 对话框

IMPLEMENT_DYNAMIC(CDlgRemoteOpenProcess, CDialog)

CDlgRemoteOpenProcess::CDlgRemoteOpenProcess(CWnd * pParent, _CIOCPServer * IOCPServer)
	: CDialog(IDD_DIALOG_WRITE_REMOTE_PROCESS_NAME, pParent)
	, m_CEdit_Remote_Open_Process_Name(_T(""))
{
	m_hwnd =(HWND)pParent;
}

CDlgRemoteOpenProcess::~CDlgRemoteOpenProcess()
{
}

void CDlgRemoteOpenProcess::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_REMOTO_OPEN_PROCESS, m_CEdit_Remote_Open_Process_Name);
}


BEGIN_MESSAGE_MAP(CDlgRemoteOpenProcess, CDialog)
	ON_BN_CLICKED(IDOK, &CDlgRemoteOpenProcess::OnBnClickedOk)

END_MESSAGE_MAP()


// CDlgRemoteOpenProcess 消息处理程序


void CDlgRemoteOpenProcess::OnBnClickedOk()
{
	UpdateData(TRUE);
	
	int BufferLength = m_CEdit_Remote_Open_Process_Name.GetLength();
	if (!BufferLength)
	{
		return;
	}
	BufferLength += 2;	//请求标志+\0
						//获得Edit控件上的数据
	CString TextData;
	TextData= m_CEdit_Remote_Open_Process_Name;
	TextData += '\0';
	//将数据格式化到BufferData中(IO通信套接字只支持char型数据),并添加标志
	char* BufferData = (char*)VirtualAlloc(NULL, BufferLength, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	memset(BufferData, 0, sizeof(char)*BufferLength);
	BYTE Token = CLIENT_REMOTE_OPEN_PROCESS;
	BufferData[0] = Token;
	sprintf(BufferData + 1, "%s", TextData.GetBuffer(0));
	//清空Edit控件上的数据

	::SendMessage(m_hwnd, UM_OPEN_REMOTE_OPEN_PROCESS_DIALOG, 0, (LPARAM)BufferData);	//同步


	if (BufferData != NULL)
	{
		VirtualFree(BufferData, BufferLength, MEM_RELEASE);
		BufferData = NULL;
	}


	CDialog::OnOK();
}
