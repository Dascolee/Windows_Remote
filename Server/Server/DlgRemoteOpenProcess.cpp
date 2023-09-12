// DlgRemoteOpenProcess.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "DlgRemoteOpenProcess.h"
#include "afxdialogex.h"

char __ProcessName[MAX_PATH] = {0};
// CDlgRemoteOpenProcess �Ի���

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


// CDlgRemoteOpenProcess ��Ϣ�������


void CDlgRemoteOpenProcess::OnBnClickedOk()
{
	UpdateData(TRUE);
	
	int BufferLength = m_CEdit_Remote_Open_Process_Name.GetLength();
	if (!BufferLength)
	{
		return;
	}
	BufferLength += 2;	//�����־+\0
						//���Edit�ؼ��ϵ�����
	CString TextData;
	TextData= m_CEdit_Remote_Open_Process_Name;
	TextData += '\0';
	//�����ݸ�ʽ����BufferData��(IOͨ���׽���ֻ֧��char������),����ӱ�־
	char* BufferData = (char*)VirtualAlloc(NULL, BufferLength, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	memset(BufferData, 0, sizeof(char)*BufferLength);
	BYTE Token = CLIENT_REMOTE_OPEN_PROCESS;
	BufferData[0] = Token;
	sprintf(BufferData + 1, "%s", TextData.GetBuffer(0));
	//���Edit�ؼ��ϵ�����

	::SendMessage(m_hwnd, UM_OPEN_REMOTE_OPEN_PROCESS_DIALOG, 0, (LPARAM)BufferData);	//ͬ��


	if (BufferData != NULL)
	{
		VirtualFree(BufferData, BufferLength, MEM_RELEASE);
		BufferData = NULL;
	}


	CDialog::OnOK();
}
