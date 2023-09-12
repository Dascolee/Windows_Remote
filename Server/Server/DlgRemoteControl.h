#pragma once
#include "afxcmn.h"
#include"_IOCPServer.h"
#include"Common.h"


// CDlgRemoteControl �Ի���

extern "C" VOID CopyScreenData(PVOID SourceData, PVOID Destination, ULONG BufferLength);


class CDlgRemoteControl : public CDialog
{
	DECLARE_DYNAMIC(CDlgRemoteControl)

public:
	CDlgRemoteControl(CWnd* pParent, _CIOCPServer* IOCPServer, CONTEXT_OBJECT* ContextObject);  
	virtual ~CDlgRemoteControl();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_REMOTE_CONTROL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

public:
	_CIOCPServer*     m_IOCPServer;
	PCONTEXT_OBJECT   m_ContextObject;
	HICON             m_IconHwnd;
	HDC               m_WindowDCHandle;  //����
	HDC               m_WindowMemoryDCHandle;  //���˵Ĺ�����
	LPBITMAPINFO      m_BitmapInfo;//����
	HBITMAP           m_BitmapHandle;//����
	PVOID             m_BitmapData;//����
	ULONG			  m_HorizontalScrollPositon;
	ULONG             m_VerticalScrollPositon;

	CPoint			  m_ClientCursorPositon;  //�洢���λ�õ�
	BOOL              m_IsControl = FALSE;
	BOOL			  m_IsTraceCursor = FALSE;  //���ٹ��켣 
	HACCEL            m_AccelHandle;
	/*
	HDC				  m_DiffMemoryDCHandle;
	PVOID			  m_DiffBitmapData;
	HBITMAP			  m_DiffBitmapHandle;
	*/
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	void CDlgRemoteControl::WindowHandleIO(void);
	VOID CDlgRemoteControl::DrawFirstScreen();
	VOID CDlgRemoteControl::DrawNextScreen();
	

	afx_msg void OnPaint();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	VOID CDlgRemoteControl::OnPrepareSending(MSG* pMsg);
	BOOL CDlgRemoteControl::SaveSnapshot();
	VOID CDlgRemoteControl::UpdataClipboardData(char* BufferData, ULONG BufferLength);
	VOID CDlgRemoteControl::SendClipboardData();



};
