// DlgRemoteControl.cpp : 实现文件
//

#include "stdafx.h"
#include "Server.h"
#include "DlgRemoteControl.h"
#include "afxdialogex.h"

enum
{
	//IDM_MENU_DIALOG_REMOTE_CONNTROLLER_CONTROL = 0x1010,
	IDM_MENU_DIALOG_REMOTE_CONNTROLLER_SEND_CTRL_ALT_DEL,  //作业向客户端发送改命令
	//IDM_MENU_DIALOG_REMOTE_CONNTROLLER_TRACE_CURSOR,	// 跟踪显示远程鼠标
	//IDM_MENU_DIALOG_REMOTE_CONNTROLLER_BLOCK_INPUT,	// 锁定远程计算机输入
	//IDM_MENU_DIALOG_REMOTE_CONNTROLLER_SAVE_DIB,		// 保存图片
	//IDM_MENU_DIALOG_REMOTE_CONNTROLLER_GET_CLIPBOARD,	// 获取剪贴板
	//IDM_MENU_DIALOG_REMOTE_CONNTROLLER_SET_CLIPBOARD,	// 设置剪贴板
};

// CDlgRemoteControl 对话框

IMPLEMENT_DYNAMIC(CDlgRemoteControl, CDialog)

CDlgRemoteControl::CDlgRemoteControl(CWnd* pParent, _CIOCPServer* IOCPServer, CONTEXT_OBJECT* ContextObject)
	: CDialog(IDD_DIALOG_REMOTE_CONTROL, pParent)
{
	m_HorizontalScrollPositon = 0;
	m_VerticalScrollPositon = 0;
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;
	m_IconHwnd = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
	//m_BufferData = (char *)(m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(0));
	if (m_ContextObject == NULL)
	{
		return;
	}
	ULONG BufferLenght = m_ContextObject->m_ReceivedDecompressdBufferData.GetBufferLength() - 1;
	m_BitmapInfo = (BITMAPINFO*) new BYTE[BufferLenght];
	if (m_BitmapInfo == NULL)
	{
		return;
	}
	//拷贝位图头信息
	memcpy(m_BitmapInfo, m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1), BufferLenght);
}

CDlgRemoteControl::~CDlgRemoteControl()
{
	if (m_BitmapInfo != NULL)
	{
		delete m_BitmapInfo;
		m_BitmapInfo = NULL;
	}
	if (m_WindowDCHandle!=NULL)
	{
		::ReleaseDC(m_hWnd, m_WindowDCHandle);  
	}
	
	if (m_WindowMemoryDCHandle!=NULL)
	{
		DeleteDC(m_WindowMemoryDCHandle);   
		DeleteObject(m_BitmapHandle);
		if (m_BitmapData != NULL)
		{
			m_BitmapData = NULL;
		}
		m_WindowMemoryDCHandle = NULL;
	}

}

void CDlgRemoteControl::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgRemoteControl, CDialog)
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()

// CDlgRemoteControl 消息处理程序

BOOL CDlgRemoteControl::OnInitDialog()
{
	CDialog::OnInitDialog();
	// TODO:  在此添加额外的初始化
	CString v1;
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int ClientAddressLength = sizeof(ClientAddress);
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength); //得到连接的ip 
	v1.Format("\\\\%s - 远程控制", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//设置对话框标题
	SetWindowText(v1);

	m_WindowDCHandle = ::GetDC(m_hWnd);
	m_WindowMemoryDCHandle = CreateCompatibleDC(m_WindowDCHandle);
	//创建应用程序可以直接写入的、与设备无关的位图
	m_BitmapHandle = CreateDIBSection(m_WindowDCHandle, m_BitmapInfo,
		DIB_RGB_COLORS, &m_BitmapData, NULL, NULL); 
	//选择一对象到指定的设备上下文环境
	//初始化工具箱
	SelectObject(m_WindowMemoryDCHandle, m_BitmapHandle);
	//指定滚动 条范围的最小值和最大值
	
	//设置对话框用全屏??
	//SetWindowPos(NULL,0,0,m_BitmapInfo->bmiHeader.biWidth, m_BitmapInfo->bmiHeader.biHeight,0);

	SetScrollRange(SB_HORZ, 0, m_BitmapInfo->bmiHeader.biWidth/1.9); 
	SetScrollRange(SB_VERT, 0, m_BitmapInfo->bmiHeader.biHeight/1.9);
	
	//获得系统菜单
	CMenu* SystenMenu = GetSystemMenu(FALSE);
	if (SystenMenu != NULL)
	{
		//作业可以处理一下快捷键
		SystenMenu->AppendMenu(MF_SEPARATOR);
		SystenMenu->AppendMenu(MF_STRING, IDM_MENU_DIALOG_REMOTE_CONNTROLLER_CONTROL, "控制屏幕(&L)");//快捷键
		SystenMenu->AppendMenu(MF_STRING, IDM_MENU_DIALOG_REMOTE_CONNTROLLER_TRACE_CURSOR, "跟踪被控端鼠标(&Y)");
		SystenMenu->AppendMenu(MF_STRING, IDM_MENU_DIALOG_REMOTE_CONNTROLLER_BLOCK_INPUT, "锁定被控端鼠标和键盘(&R)");
		SystenMenu->AppendMenu(MF_STRING, IDM_MENU_DIALOG_REMOTE_CONNTROLLER_SAVE_DIB, "保存快照(&S)");
		SystenMenu->AppendMenu(MF_SEPARATOR);
		SystenMenu->AppendMenu(MF_STRING, IDM_MENU_DIALOG_REMOTE_CONNTROLLER_GET_CLIPBOARD, "获取剪贴板(&P)");
		SystenMenu->AppendMenu(MF_STRING, IDM_MENU_DIALOG_REMOTE_CONNTROLLER_SET_CLIPBOARD, "设置剪贴板(&T)");
		SystenMenu->AppendMenu(MF_SEPARATOR);
		SystenMenu->AppendMenu(MF_STRING, IDM_MENU_DIALOG_REMOTE_CONNTROLLER_SEND_CTRL_ALT_DEL, "发送Ctrl+Alt+del(&W)");
		SystenMenu->AppendMenu(MF_SEPARATOR);
	}
	//SaveDib = ::LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDM_MENU_DIALOG_REMOTE_CONNTROLLER_SAVE_DIB));
	m_AccelHandle = ::LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_REMOTE_CONTROL_ACCELERATOR));
	m_ClientCursorPositon.x = 0;
	m_ClientCursorPositon.y = 0;

	BYTE IsToken = CLIENT_GO_ON;
	m_IOCPServer->OnPrepareSending(m_ContextObject, &IsToken, sizeof(BYTE));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CDlgRemoteControl::OnPaint()
{
	CPaintDC dc(this); 

	//将工具箱的工具给工人
	BitBlt(m_WindowDCHandle, 0, 0,
		m_BitmapInfo->bmiHeader.biWidth,
		m_BitmapInfo->bmiHeader.biHeight,
		m_WindowMemoryDCHandle,
		m_HorizontalScrollPositon,
		m_VerticalScrollPositon,
		SRCCOPY);                                    //滚轮？

	if (m_IsTraceCursor)
	{
		DrawIconEx(
			m_WindowDCHandle,
			m_ClientCursorPositon.x - m_HorizontalScrollPositon,
			m_ClientCursorPositon.y - m_VerticalScrollPositon,
			m_IconHwnd,
			0, 0,
			0,
			NULL,
			DI_NORMAL | DI_COMPAT
		);
	}
}

void CDlgRemoteControl::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(m_ContextObject != NULL)
	{
		CancelIo((HANDLE)m_ContextObject->ClientSocket);
		closesocket(m_ContextObject->ClientSocket);
		m_ContextObject->DialogHandle = NULL;
		m_ContextObject->DialogID = 0;
	}
	CDialog::OnClose();
}

void CDlgRemoteControl::WindowHandleIO(void)
{
	if (m_ContextObject == NULL)
	{
		return;
	}

	switch (m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(0)[0])
	{
	case CLIENT_REMOTE_CONTROLLER_FIRST_SCREEN:
	{
		DrawFirstScreen();
		break;
	}
	case CLIENT_REMOTE_CONTROLLER_NEXT_SCREEN:
	{
#define ALGORITHM_DIFF 1
		if (m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer()[1] == ALGORITHM_DIFF)
		{
			DrawNextScreen();
		}
		break;
	}
	case CLIENT_REMOTE_CONTROLLER_GET_CLIPBOARD_REPLY:
	{
		//将接收到的客户端数据放入到我们当前主控端的剪切板中
		UpdataClipboardData((char*)m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1),
			m_ContextObject->m_ReceivedDecompressdBufferData.GetBufferLength() - 1);
		break;
	}

	}
}

VOID CDlgRemoteControl::DrawFirstScreen()
{
	//得到Client端发来的数据，将它拷贝到HBITMAP的缓冲区中，这样第一个图像就出现了
	memcpy(m_BitmapData, m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1), 
		m_BitmapInfo->bmiHeader.biSizeImage);
	PostMessage(WM_PAINT);//触发消息
}

VOID CDlgRemoteControl::DrawNextScreen()
{
	//该函数不是直接得到一个全部的屏幕，而是更新一下变化部分的屏幕数据然后调用
	//OnPaint画上去
	//根据鼠标是否移动和屏幕是否变化判断是否重绘鼠标，防止鼠标闪烁
	BOOL	IsChanged = FALSE;
	ULONG	HeadLength = 1 + 1 + sizeof(POINT) + sizeof(BYTE); // 标识 + 算法 + 光标位置 + 光标类型索引
	
	//前一帧数据
	LPVOID	PreviousScreenData = m_BitmapData;
	//获得当前帧数据 +长度
	LPVOID	CurrentScreenData = m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(HeadLength);
	ULONG	BufferLength = m_ContextObject->m_ReceivedDecompressdBufferData.GetBufferLength() - HeadLength;

	POINT	PreviousClientCursorPositon;
	memcpy(&PreviousClientCursorPositon, &m_ClientCursorPositon, sizeof(POINT));
	//更新了光标位置
	memcpy(&m_ClientCursorPositon, m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(2), sizeof(POINT));

	// 鼠标移动了
	if (memcmp(&PreviousClientCursorPositon, &m_ClientCursorPositon, sizeof(POINT)) != 0)
	{
		IsChanged = TRUE;
	}

	// 屏幕是否变化
	if (BufferLength > 0)
	{
		IsChanged = TRUE;
	}

	//lodsd指令从ESI指向的内存位置4个字节内容放入EAX中并且下移4
	//movsb指令字节传送数据，通过SI和DI这两个寄存器控制字符串的源地址和目标地址
	CopyScreenData(PreviousScreenData, CurrentScreenData, BufferLength);
	if (IsChanged)
	{
		//如果有变化
		PostMessage(WM_PAINT);
	}
}
//调一下窗口上的滚动轴
void CDlgRemoteControl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	SCROLLINFO ScrollInfo;
	int i;
	ScrollInfo.cbSize = sizeof(SCROLLINFO);
	ScrollInfo.fMask = SIF_ALL;
	GetScrollInfo(SB_VERT, &ScrollInfo); //1920 1080

	switch (nSBCode)
	{
	case SB_LINEUP:
		i = nPos - 1;
		break;
	case SB_LINEDOWN:
		i = nPos + 1;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		i = ScrollInfo.nTrackPos;
		break;
	default:
		return;
	}

	i = max(i, ScrollInfo.nMin);//0
	//防止出边界
	i = min(i, (int)(ScrollInfo.nMax - ScrollInfo.nPage + 1));

	RECT ClientRect;
	GetClientRect(&ClientRect);

	if ((ClientRect.bottom+i)>m_BitmapInfo->bmiHeader.biHeight)
	{
		i = m_BitmapInfo->bmiHeader.biHeight - ClientRect.bottom;
	}
	InterlockedExchange((PLONG)&m_VerticalScrollPositon, i);
	SetScrollPos(SB_VERT, i);
	OnPaint();

	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CDlgRemoteControl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	SCROLLINFO ScrollInfo;
	int i;
	ScrollInfo.cbSize = sizeof(SCROLLINFO);
	ScrollInfo.fMask = SIF_ALL;
	GetScrollInfo(SB_HORZ, &ScrollInfo);     //1920 1080

	switch (nSBCode)
	{
	case SB_LINEUP:
		i = nPos - 1;
		break;
	case SB_LINEDOWN:
		i = nPos + 1;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		i = ScrollInfo.nTrackPos;
		break;

	default:
		return;
	}

	i = max(i, ScrollInfo.nMin);//0
	i = min(i, (int)(ScrollInfo.nMax - ScrollInfo.nPage + 1));

	RECT ClientRect;
	GetClientRect(&ClientRect);

	if ((ClientRect.right + i)>m_BitmapInfo->bmiHeader.biWidth)
	{
		i = m_BitmapInfo->bmiHeader.biWidth - ClientRect.right;
	}
	InterlockedExchange((PLONG)&m_HorizontalScrollPositon, i);
	SetScrollPos(SB_HORZ, m_HorizontalScrollPositon);
	OnPaint();
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

//改消息接送包括系统菜单等待窗口处理消息
void CDlgRemoteControl::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CMenu* SysMenu = GetSystemMenu(FALSE);

	switch (nID)
	{
	case IDM_MENU_DIALOG_REMOTE_CONNTROLLER_CONTROL:    //远程控制
	{
		m_IsControl = !m_IsControl;
		SysMenu->CheckMenuItem(IDM_MENU_DIALOG_REMOTE_CONNTROLLER_CONTROL, m_IsControl ? MF_CHECKED : MF_UNCHECKED);   //菜单样式
		break;
	}

	case IDM_MENU_DIALOG_REMOTE_CONNTROLLER_TRACE_CURSOR: //跟踪被控端鼠标
	{
		m_IsTraceCursor = !m_IsTraceCursor;
		SysMenu->CheckMenuItem(IDM_MENU_DIALOG_REMOTE_CONNTROLLER_TRACE_CURSOR, m_IsTraceCursor ? MF_CHECKED : MF_UNCHECKED);   //菜单样式
		OnPaint();
		break;
	}
	case IDM_MENU_DIALOG_REMOTE_CONNTROLLER_BLOCK_INPUT:
	{
		BOOL IsChecked = SysMenu->GetMenuState(IDM_MENU_DIALOG_REMOTE_CONNTROLLER_BLOCK_INPUT, MF_BYCOMMAND)&MF_CHECKED;
		SysMenu->CheckMenuItem(IDM_MENU_DIALOG_REMOTE_CONNTROLLER_BLOCK_INPUT, IsChecked ? MF_CHECKED : MF_UNCHECKED);

		BYTE IsToken[2];
		IsToken[0] = CLIENT_REMOTE_CONTROLLER_BLOCK_INPUT;
		IsToken[1] = !IsChecked;
		m_IOCPServer->OnPrepareSending(m_ContextObject,IsToken,sizeof(IsToken));
		break;
	}
	case IDM_MENU_DIALOG_REMOTE_CONNTROLLER_SAVE_DIB:
	{
		SaveSnapshot();
		break;
	}
	//获得客户端剪切板内容数据
	case IDM_MENU_DIALOG_REMOTE_CONNTROLLER_GET_CLIPBOARD:
	{
		BYTE IsToken= CLIENT_REMOTE_CONTROLLER_GET_CLIPBOARD;
		m_IOCPServer->OnPrepareSending(m_ContextObject, &IsToken, sizeof(IsToken));
		break;
	}
	//向客户端剪切板中设置数据
	case IDM_MENU_DIALOG_REMOTE_CONNTROLLER_SET_CLIPBOARD:
	{
		//从当前服务器中的剪切板中获取数据后发送到客户端
		SendClipboardData();
		break;
	}

	case IDM_MENU_DIALOG_REMOTE_CONNTROLLER_SEND_CTRL_ALT_DEL:
	{
		BYTE IsToken = CLIENT_REMOTE_CONTROLLER_SEND_CTRL_ALT_DEL;
		m_IOCPServer->OnPrepareSending(m_ContextObject, &IsToken, sizeof(IsToken));
		break;
	}


	}

	CDialog::OnSysCommand(nID, lParam);
}

BOOL CDlgRemoteControl::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类

	if (m_AccelHandle)
	{
		if (::TranslateAccelerator(m_hWnd, m_AccelHandle, pMsg))
		{
			return(TRUE);
		}
	}
#define MAKEDWORD(h,l)    (((unsigned long)h<<16)|l)
	switch (pMsg->message)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	{
		MSG Msg;
		memcpy(&Msg, pMsg, sizeof(MSG));
		Msg.lParam = MAKEDWORD(HIWORD(pMsg->lParam) + m_VerticalScrollPositon, LOWORD(pMsg->lParam) + m_HorizontalScrollPositon);
		Msg.pt.x += m_HorizontalScrollPositon;
		Msg.pt.y += m_VerticalScrollPositon;
		OnPrepareSending(&Msg);
		break;
	}
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	{
	
		if (pMsg->wParam != VK_LWIN&&pMsg->wParam != VK_RWIN)
		{
			MSG Msg;
			memcpy(&Msg, pMsg, sizeof(MSG));
			Msg.lParam = MAKEDWORD(HIDWORD(pMsg->lParam) + m_VerticalScrollPositon, LOWORD(pMsg->lParam) + m_HorizontalScrollPositon);
			Msg.pt.x += m_HorizontalScrollPositon;
			Msg.pt.y += m_VerticalScrollPositon;
			OnPrepareSending(&Msg);
		}
		if (pMsg->wParam == VK_LWIN&&pMsg->wParam == VK_RWIN)
		{
			return TRUE;
		}
		break;
	}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

VOID CDlgRemoteControl::OnPrepareSending(MSG* Msg)
{
	if (!m_IsControl)
	{
		return;
	}

	LPBYTE BufferData = new BYTE[sizeof(MSG) + 1];
	BufferData[0] = CLIENT_REMOTE_CONTROLLER_CONTROL;
	memcpy(BufferData + 1, Msg, sizeof(MSG)); 
	m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, sizeof(MSG) + 1);

	delete[] BufferData;
}

BOOL CDlgRemoteControl::SaveSnapshot()
{
	//保存位图
	CString FileFullPath = CTime::GetCurrentTime().Format("%Y-%m-%d_%H-%M-%S.bmp");
	CFileDialog Dlg(FALSE, "bmp", FileFullPath, OFN_OVERWRITEPROMPT, "位图文件(*.bmp)|*.bmp|", this);
	if (Dlg.DoModal() != IDOK)
	{
		return FALSE;
	}
	BITMAPFILEHEADER BitmapFileHeader;
	LPBITMAPINFO     BitmapInfor = m_BitmapInfo;
	CFile            File;
	//创建一个文件
	if (!File.Open(Dlg.GetPathName(), CFile::modeWrite | CFile::modeCreate))
	{
		return FALSE;
	}
	// BITMAPINFO大小
	int	BitmapInforLength = sizeof(BITMAPINFO);

	//协议  TCP  校验值
	BitmapFileHeader.bfType = ((WORD)('M' << 8) | 'B');
	BitmapFileHeader.bfSize = BitmapInfor->bmiHeader.biSizeImage + sizeof(BITMAPFILEHEADER);  
	BitmapFileHeader.bfReserved1 = 0;                         
	BitmapFileHeader.bfReserved2 = 0;
	BitmapFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + BitmapInforLength;

	File.Write(&BitmapFileHeader, sizeof(BITMAPFILEHEADER));
	File.Write(BitmapInfor, BitmapInforLength);

	File.Write(m_BitmapData, BitmapInfor->bmiHeader.biSizeImage);
	File.Close();

	return TRUE;
}

void CDlgRemoteControl::UpdataClipboardData(char* ClipboardData, ULONG ClipboardDataLength)
{
	if (OpenClipboard())
	{
		HGLOBAL GlobalHandle;
		char* BufferData;
		int BufferLength = 0;

		EmptyClipboard();
		//申请内存
		GlobalHandle = GlobalAlloc(GMEM_DDESHARE, 256);
		if (GlobalHandle != NULL)
		{
			BufferData = (char*)GlobalLock(GlobalHandle);

			memcpy(BufferData, ClipboardData, ClipboardDataLength);

			GlobalUnlock(GlobalHandle);
			SetClipboardData(CF_OEMTEXT, GlobalHandle);
			GlobalFree(GlobalHandle);
		}
		CloseClipboard();
	}
}

void CDlgRemoteControl::SendClipboardData()
{
	if (!OpenClipboard())
	{
		return;
	}

	HGLOBAL GlobalHandle = GetClipboardData(CF_TEXT);
	if (GlobalHandle == NULL)
	{
		CloseClipboard();
		return;
	}

	int BufferLength = GlobalSize(GlobalHandle) + 1;
	char* Text = (LPSTR)GlobalLock(GlobalHandle);
	LPBYTE BufferData = new BYTE[BufferLength];

	BufferData[0] = CLIENT_REMOTE_CONTROLLER_SET_CLIPBOARD_REPLY;
	memcpy(BufferData + 1, Text, BufferLength - 1);
	GlobalUnlock(GlobalHandle);
	CloseClipboard();

	m_IOCPServer->OnPrepareSending(m_ContextObject, (PBYTE)BufferData, BufferLength);
	delete[] BufferData;
}