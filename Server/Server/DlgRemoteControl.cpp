// DlgRemoteControl.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "DlgRemoteControl.h"
#include "afxdialogex.h"

enum
{
	//IDM_MENU_DIALOG_REMOTE_CONNTROLLER_CONTROL = 0x1010,
	IDM_MENU_DIALOG_REMOTE_CONNTROLLER_SEND_CTRL_ALT_DEL,  //��ҵ��ͻ��˷��͸�����
	//IDM_MENU_DIALOG_REMOTE_CONNTROLLER_TRACE_CURSOR,	// ������ʾԶ�����
	//IDM_MENU_DIALOG_REMOTE_CONNTROLLER_BLOCK_INPUT,	// ����Զ�̼��������
	//IDM_MENU_DIALOG_REMOTE_CONNTROLLER_SAVE_DIB,		// ����ͼƬ
	//IDM_MENU_DIALOG_REMOTE_CONNTROLLER_GET_CLIPBOARD,	// ��ȡ������
	//IDM_MENU_DIALOG_REMOTE_CONNTROLLER_SET_CLIPBOARD,	// ���ü�����
};

// CDlgRemoteControl �Ի���

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
	//����λͼͷ��Ϣ
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

// CDlgRemoteControl ��Ϣ�������

BOOL CDlgRemoteControl::OnInitDialog()
{
	CDialog::OnInitDialog();
	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	CString v1;
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int ClientAddressLength = sizeof(ClientAddress);
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength); //�õ����ӵ�ip 
	v1.Format("\\\\%s - Զ�̿���", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//���öԻ������
	SetWindowText(v1);

	m_WindowDCHandle = ::GetDC(m_hWnd);
	m_WindowMemoryDCHandle = CreateCompatibleDC(m_WindowDCHandle);
	//����Ӧ�ó������ֱ��д��ġ����豸�޹ص�λͼ
	m_BitmapHandle = CreateDIBSection(m_WindowDCHandle, m_BitmapInfo,
		DIB_RGB_COLORS, &m_BitmapData, NULL, NULL); 
	//ѡ��һ����ָ�����豸�����Ļ���
	//��ʼ��������
	SelectObject(m_WindowMemoryDCHandle, m_BitmapHandle);
	//ָ������ ����Χ����Сֵ�����ֵ
	
	//���öԻ�����ȫ��??
	//SetWindowPos(NULL,0,0,m_BitmapInfo->bmiHeader.biWidth, m_BitmapInfo->bmiHeader.biHeight,0);

	SetScrollRange(SB_HORZ, 0, m_BitmapInfo->bmiHeader.biWidth/1.9); 
	SetScrollRange(SB_VERT, 0, m_BitmapInfo->bmiHeader.biHeight/1.9);
	
	//���ϵͳ�˵�
	CMenu* SystenMenu = GetSystemMenu(FALSE);
	if (SystenMenu != NULL)
	{
		//��ҵ���Դ���һ�¿�ݼ�
		SystenMenu->AppendMenu(MF_SEPARATOR);
		SystenMenu->AppendMenu(MF_STRING, IDM_MENU_DIALOG_REMOTE_CONNTROLLER_CONTROL, "������Ļ(&L)");//��ݼ�
		SystenMenu->AppendMenu(MF_STRING, IDM_MENU_DIALOG_REMOTE_CONNTROLLER_TRACE_CURSOR, "���ٱ��ض����(&Y)");
		SystenMenu->AppendMenu(MF_STRING, IDM_MENU_DIALOG_REMOTE_CONNTROLLER_BLOCK_INPUT, "�������ض����ͼ���(&R)");
		SystenMenu->AppendMenu(MF_STRING, IDM_MENU_DIALOG_REMOTE_CONNTROLLER_SAVE_DIB, "�������(&S)");
		SystenMenu->AppendMenu(MF_SEPARATOR);
		SystenMenu->AppendMenu(MF_STRING, IDM_MENU_DIALOG_REMOTE_CONNTROLLER_GET_CLIPBOARD, "��ȡ������(&P)");
		SystenMenu->AppendMenu(MF_STRING, IDM_MENU_DIALOG_REMOTE_CONNTROLLER_SET_CLIPBOARD, "���ü�����(&T)");
		SystenMenu->AppendMenu(MF_SEPARATOR);
		SystenMenu->AppendMenu(MF_STRING, IDM_MENU_DIALOG_REMOTE_CONNTROLLER_SEND_CTRL_ALT_DEL, "����Ctrl+Alt+del(&W)");
		SystenMenu->AppendMenu(MF_SEPARATOR);
	}
	//SaveDib = ::LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDM_MENU_DIALOG_REMOTE_CONNTROLLER_SAVE_DIB));
	m_AccelHandle = ::LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_REMOTE_CONTROL_ACCELERATOR));
	m_ClientCursorPositon.x = 0;
	m_ClientCursorPositon.y = 0;

	BYTE IsToken = CLIENT_GO_ON;
	m_IOCPServer->OnPrepareSending(m_ContextObject, &IsToken, sizeof(BYTE));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}

void CDlgRemoteControl::OnPaint()
{
	CPaintDC dc(this); 

	//��������Ĺ��߸�����
	BitBlt(m_WindowDCHandle, 0, 0,
		m_BitmapInfo->bmiHeader.biWidth,
		m_BitmapInfo->bmiHeader.biHeight,
		m_WindowMemoryDCHandle,
		m_HorizontalScrollPositon,
		m_VerticalScrollPositon,
		SRCCOPY);                                    //���֣�

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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
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
		//�����յ��Ŀͻ������ݷ��뵽���ǵ�ǰ���ض˵ļ��а���
		UpdataClipboardData((char*)m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1),
			m_ContextObject->m_ReceivedDecompressdBufferData.GetBufferLength() - 1);
		break;
	}

	}
}

VOID CDlgRemoteControl::DrawFirstScreen()
{
	//�õ�Client�˷��������ݣ�����������HBITMAP�Ļ������У�������һ��ͼ��ͳ�����
	memcpy(m_BitmapData, m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1), 
		m_BitmapInfo->bmiHeader.biSizeImage);
	PostMessage(WM_PAINT);//������Ϣ
}

VOID CDlgRemoteControl::DrawNextScreen()
{
	//�ú�������ֱ�ӵõ�һ��ȫ������Ļ�����Ǹ���һ�±仯���ֵ���Ļ����Ȼ�����
	//OnPaint����ȥ
	//��������Ƿ��ƶ�����Ļ�Ƿ�仯�ж��Ƿ��ػ���꣬��ֹ�����˸
	BOOL	IsChanged = FALSE;
	ULONG	HeadLength = 1 + 1 + sizeof(POINT) + sizeof(BYTE); // ��ʶ + �㷨 + ���λ�� + �����������
	
	//ǰһ֡����
	LPVOID	PreviousScreenData = m_BitmapData;
	//��õ�ǰ֡���� +����
	LPVOID	CurrentScreenData = m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(HeadLength);
	ULONG	BufferLength = m_ContextObject->m_ReceivedDecompressdBufferData.GetBufferLength() - HeadLength;

	POINT	PreviousClientCursorPositon;
	memcpy(&PreviousClientCursorPositon, &m_ClientCursorPositon, sizeof(POINT));
	//�����˹��λ��
	memcpy(&m_ClientCursorPositon, m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(2), sizeof(POINT));

	// ����ƶ���
	if (memcmp(&PreviousClientCursorPositon, &m_ClientCursorPositon, sizeof(POINT)) != 0)
	{
		IsChanged = TRUE;
	}

	// ��Ļ�Ƿ�仯
	if (BufferLength > 0)
	{
		IsChanged = TRUE;
	}

	//lodsdָ���ESIָ����ڴ�λ��4���ֽ����ݷ���EAX�в�������4
	//movsbָ���ֽڴ������ݣ�ͨ��SI��DI�������Ĵ��������ַ�����Դ��ַ��Ŀ���ַ
	CopyScreenData(PreviousScreenData, CurrentScreenData, BufferLength);
	if (IsChanged)
	{
		//����б仯
		PostMessage(WM_PAINT);
	}
}
//��һ�´����ϵĹ�����
void CDlgRemoteControl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
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
	//��ֹ���߽�
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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
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

//����Ϣ���Ͱ���ϵͳ�˵��ȴ����ڴ�����Ϣ
void CDlgRemoteControl::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	CMenu* SysMenu = GetSystemMenu(FALSE);

	switch (nID)
	{
	case IDM_MENU_DIALOG_REMOTE_CONNTROLLER_CONTROL:    //Զ�̿���
	{
		m_IsControl = !m_IsControl;
		SysMenu->CheckMenuItem(IDM_MENU_DIALOG_REMOTE_CONNTROLLER_CONTROL, m_IsControl ? MF_CHECKED : MF_UNCHECKED);   //�˵���ʽ
		break;
	}

	case IDM_MENU_DIALOG_REMOTE_CONNTROLLER_TRACE_CURSOR: //���ٱ��ض����
	{
		m_IsTraceCursor = !m_IsTraceCursor;
		SysMenu->CheckMenuItem(IDM_MENU_DIALOG_REMOTE_CONNTROLLER_TRACE_CURSOR, m_IsTraceCursor ? MF_CHECKED : MF_UNCHECKED);   //�˵���ʽ
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
	//��ÿͻ��˼��а���������
	case IDM_MENU_DIALOG_REMOTE_CONNTROLLER_GET_CLIPBOARD:
	{
		BYTE IsToken= CLIENT_REMOTE_CONTROLLER_GET_CLIPBOARD;
		m_IOCPServer->OnPrepareSending(m_ContextObject, &IsToken, sizeof(IsToken));
		break;
	}
	//��ͻ��˼��а�����������
	case IDM_MENU_DIALOG_REMOTE_CONNTROLLER_SET_CLIPBOARD:
	{
		//�ӵ�ǰ�������еļ��а��л�ȡ���ݺ��͵��ͻ���
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
	// TODO: �ڴ����ר�ô����/����û���

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
	//����λͼ
	CString FileFullPath = CTime::GetCurrentTime().Format("%Y-%m-%d_%H-%M-%S.bmp");
	CFileDialog Dlg(FALSE, "bmp", FileFullPath, OFN_OVERWRITEPROMPT, "λͼ�ļ�(*.bmp)|*.bmp|", this);
	if (Dlg.DoModal() != IDOK)
	{
		return FALSE;
	}
	BITMAPFILEHEADER BitmapFileHeader;
	LPBITMAPINFO     BitmapInfor = m_BitmapInfo;
	CFile            File;
	//����һ���ļ�
	if (!File.Open(Dlg.GetPathName(), CFile::modeWrite | CFile::modeCreate))
	{
		return FALSE;
	}
	// BITMAPINFO��С
	int	BitmapInforLength = sizeof(BITMAPINFO);

	//Э��  TCP  У��ֵ
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
		//�����ڴ�
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