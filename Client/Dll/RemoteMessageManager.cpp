#include "RemoteMessageManager.h"
#include"Common.h"
#include"resource.h"
#include <mmsystem.h>

#pragma comment(lib, "WINMM.LIB")

extern HINSTANCE __InstanceHandle;
char __BufferData[0x1000] = {0};
UINT_PTR  _TimeEvent = 0;
_CIOCPClient* __IOCPClient = NULL;

#define ID_TIMER_POP_WINDOW		1
#define ID_TIMER_DELAY_DISPLAY	2 
#define ID_TIMER_CLOSE_WINDOW	3 

#define WIN_WIDTH		120   
#define WIN_HEIGHT		120

CRemoteMessageManager::CRemoteMessageManager(_CIOCPClient * IOCPClient) :CManager(IOCPClient)
{
	//�ش����ݰ���������
	BYTE IsToken = CLIENT_REMOTE_MESSAGE_REPLY;      //����ͷ�ļ� Common.h     
	m_IOCPClient->OnSending((char*)&IsToken, 1);
	__IOCPClient = IOCPClient;

	WaittingForDialogOpen();   //�ú����Ǹ���ʵ��
}

CRemoteMessageManager::~CRemoteMessageManager()
{
	printf("~CRemoteMessageManager()��������CRemoteMessageManager����\r\n");
}

VOID CRemoteMessageManager::HandleIO(PBYTE BufferData, ULONG BufferLength)
{
	switch (BufferData[0])
	{
	case CLIENT_GO_ON:
	{
		//�ú����Ǹ���ʵ��
		NotifyDialogIsOpen();
		break;
	}
	
	default:
	{
		//���Զ����Ϣ�������ڴ�
		memcpy(__BufferData, BufferData, BufferLength);
		//����һ������
		DialogBox(__InstanceHandle, MAKEINTRESOURCE(IDD_DIALOG_REMOTE_MESSAGE),
			NULL, DialogProcedure);  //SDK   C   MFC  C++
		break;
	}
	}
}

int CALLBACK DialogProcedure(HWND DlgHwnd, unsigned int Msg,
	WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_INITDIALOG:
	{
		OnInitDialog(DlgHwnd);
		break;
	}
	case WM_TIMER:
	{
		OnTimerDialog(DlgHwnd);
		break;
	}
	}
	return 0;
}

VOID OnInitDialog(HWND DlgHwnd)
{
	MoveWindow(DlgHwnd, 0, 0, 0, 0, TRUE);
	//�ڿؼ���IDC_EDIT_DIALOG_REMOTE_MESSAGE��������
	SetDlgItemText(DlgHwnd, IDC_EDIT_DIALOG_REMOTE_MESSAGE, __BufferData);

	memset(__BufferData, 0, sizeof(__BufferData));


	_TimeEvent = ID_TIMER_POP_WINDOW;

	SetTimer(DlgHwnd, _TimeEvent, 1, NULL);  //ʱ�ӻص�   

	PlaySound(MAKEINTRESOURCE(IDR_WAVE_REMOTE_MESSAGE),
		__InstanceHandle, SND_ASYNC | SND_RESOURCE | SND_NODEFAULT);
}

VOID OnTimerDialog(HWND DlgHwnd)   //ʱ�ӻص�
{
	RECT  Rect;
	static int Height = 0;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &Rect, 0);
	int y = Rect.bottom - Rect.top;;
	//int x = Rect.right - Rect.left;
	//x = x - WIN_WIDTH;
	int x = 0;
	switch (_TimeEvent)
	{
	case ID_TIMER_CLOSE_WINDOW:
	{
		if (Height >= 0)
		{
			Height -= 5;
			MoveWindow(DlgHwnd, x, y - Height, WIN_WIDTH, Height, TRUE);
		}
		else
		{
			KillTimer(DlgHwnd, ID_TIMER_CLOSE_WINDOW);
			//֪ͨ���������Է����µ�����
			//BYTE IsToken = CLIENT_REMOTE_MESSAGE_COMPLETE;		// ����ͷ�ļ� Common.h     
			//__IOCPClient->OnSending((char*)&IsToken, 1);		// �����������·��͵�ָ��
			//�رյ�ǰ���ٶԻ���
			EndDialog(DlgHwnd, 0);
		}
		break;
	}
	case ID_TIMER_DELAY_DISPLAY:
	{
		KillTimer(DlgHwnd, ID_TIMER_DELAY_DISPLAY);
		_TimeEvent = ID_TIMER_CLOSE_WINDOW;
		SetTimer(DlgHwnd, _TimeEvent, 5, NULL);
		break;
	}
	case ID_TIMER_POP_WINDOW:
	{
		if (Height <= WIN_HEIGHT)
		{
			Height += 3;
			MoveWindow(DlgHwnd, x, y - Height, WIN_WIDTH, Height, TRUE);
		}
		else
		{
			KillTimer(DlgHwnd, ID_TIMER_POP_WINDOW);
			_TimeEvent = ID_TIMER_DELAY_DISPLAY;
			SetTimer(DlgHwnd, _TimeEvent, 4000, NULL);
		}
		break;
	}
	}
}