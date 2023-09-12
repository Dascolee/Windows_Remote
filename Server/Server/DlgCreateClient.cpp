// DlgCreateClient.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "DlgCreateClient.h"
#include "afxdialogex.h"

struct _CONNECT_INFO_
{
	DWORD CheckFlag;
	char ConnectIP[20];
	USHORT ConnectPort;
} _ConnectInfo = { 0x87654321,"",0 };

IMPLEMENT_DYNAMIC(CDlgCreateClient, CDialog)

CDlgCreateClient::CDlgCreateClient(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG_CREATE_CLIENT, pParent)
	, m_Edit_Create_Client_Link_IP(_T("请输入一个IP！"))
	, m_Edit_Create_Client_Link_Port(_T("请输入一个端口！"))
{

}

CDlgCreateClient::~CDlgCreateClient()
{
}

void CDlgCreateClient::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_CREATE_CLIENT_IP, m_Edit_Create_Client_Link_IP);
	DDX_Text(pDX, IDC_EDIT_CREATE_CLIENT_PORT, m_Edit_Create_Client_Link_Port);
}

BEGIN_MESSAGE_MAP(CDlgCreateClient, CDialog)
	ON_BN_CLICKED(IDOK, &CDlgCreateClient::OnBnClickedOk)
END_MESSAGE_MAP()

void CDlgCreateClient::OnBnClickedOk()
{
	CFile FileObject;
	UpdateData(TRUE);
	USHORT ConnectPort = atoi(m_Edit_Create_Client_Link_Port);
	strcpy(_ConnectInfo.ConnectIP,m_Edit_Create_Client_Link_IP);
	if (ConnectPort < 0 || ConnectPort > 65536)
	{
		_ConnectInfo.ConnectPort = 2356;
	}
	else
	{
		_ConnectInfo.ConnectPort = ConnectPort;
	}
	char v2[MAX_PATH];
	ZeroMemory(v2, MAX_PATH);

	LONGLONG BufferLenght =0;

	BYTE* BufferData = NULL;
	CString v3;
	CString FlieFullPath;
	try
	{
		//得到未经处理前的文件名称 
		GetModuleFileName(NULL, v2, MAX_PATH);
		v3 = v2;
		int Position = v3.ReverseFind('\\');

		v3 = v3.Left(Position);

		FlieFullPath = v3 + "\\Client.exe";
		FileObject.Open(FlieFullPath, CFile::modeRead | CFile::typeBinary);
		int error = GetLastError();
		BufferLenght = FileObject.GetLength();
		BufferData = new BYTE[BufferLenght];
		ZeroMemory(BufferData , BufferLenght);
		//读取文件内容
		FileObject.Read(BufferData, BufferLenght);
		//关闭文件对象
		 error = GetLastError();
		FileObject.Close();
		//写入上线的IP和端口 主要是在内存中寻找0x87654321这个标识然后写入这个位置

		//全局变量是在PE文件的Data节中;
		int offset = MemoryFind((char*)BufferData, (char*)&_ConnectInfo.CheckFlag,
			BufferLenght, sizeof(DWORD));
		//把自己设置的信息拷贝到exe的内存
		memcpy(BufferData+ offset,&_ConnectInfo, sizeof(_ConnectInfo));
	
		FileObject.Open(FlieFullPath, CFile::modeCreate ||CFile::modeWrite | CFile::typeBinary);
		FileObject.Write(BufferData, BufferLenght);
		//关闭文件对象
		FileObject.Close();
		delete[] BufferData;
		MessageBox("生成成功");
	}
	catch (CMemoryException* e)
	{
		MessageBox("内存不足！");
	}
	catch (CFileException* e)
	{
		MessageBox("文件操作错误！");
	}
	catch (CException* e)
	{
		MessageBox("未知错误！");
	}
	CDialog::OnOK();
}

int CDlgCreateClient::MemoryFind(const char *BufferData,const char* KeyValue,int BufferLenght,int KeyLenght)
{
	int i, j;
	if (KeyLenght == 0||BufferLenght == 0)
	{
		return -1;
	}
	for ( i = 0; i < BufferLenght; i++)
	{
		for (j = 0; j < KeyLenght; j++)
		{
			if (BufferData[i+j] != KeyValue[j])
			{
				break;
			}
		}
		if (j == KeyLenght)
		{
			return i;
		}
	}
	return -1;
}

