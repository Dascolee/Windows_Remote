#pragma once
#include"_IOCPServer.h"
#include"Common.h"
#include "Audio.h"

// CDlgAudioManager �Ի���

class CDlgAudioManager : public CDialog
{
	DECLARE_DYNAMIC(CDlgAudioManager)

public:
	CDlgAudioManager(CWnd* Parent, _CIOCPServer* IOCPServer = NULL,
		CONTEXT_OBJECT *ContextObject = NULL);   // ��׼���캯��
	virtual ~CDlgAudioManager();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_AUDIO_MANAGER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	_CIOCPServer* m_IOCPServer;
	PCONTEXT_OBJECT m_ContextObject;
	HICON m_IconHwnd;
	CAudio  m_AudioObject;
	HANDLE m_WorkThreadHandle;
public:
	void CDlgAudioManager::WindowHandleIO();
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	CString m_CEdit_Dialog_Audio_Manager_Status;
	afx_msg void OnBnClickedButtonAudioRecord();
	afx_msg void OnBnClickedButtonAudioSpeak();
	static DWORD ThreadProcedure(LPVOID ParameterData);
	VOID SendRecordData();
};
