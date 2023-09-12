#pragma once
#include "afxcmn.h"
#include"_IOCPServer.h"
#include"Common.h"
#include<vector>
// CDlgThreaderManager �Ի���

typedef LONG KPRIORITY, *PKPRIORITY;
typedef struct _THREAD_ITEM_INFORMATION_
{
	LONG BasePriority;
	ULONG ContextSwitches;
	KPRIORITY Priority;
	HANDLE    ThreadID;
	ULONG ThreadState;
	ULONG WaitReason;
	ULONG WaitTime;
	PVOID   Teb;
	PVOID   ThreadStartAddress;
	HANDLE  ThreadHandle;

}THREAD_ITEM_INFORMATION, *PTHREAD_ITEM_INFORMATION;

typedef struct _SuspendInfo_
{
	int SuspendCount;
	HANDLE  ThreadHandle;
}SuspendInfo, *pSuspendInfo;


class CDlgThreaderManager : public CDialog
{
	DECLARE_DYNAMIC(CDlgThreaderManager)

public:
	CDlgThreaderManager(CWnd* pParent, _CIOCPServer* IOCPServer, CONTEXT_OBJECT* ContextObject);   // ��׼���캯��
	virtual ~CDlgThreaderManager();
// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_THREADER_PROCESS_MANAGER };
#endif

public:
	_CIOCPServer* m_IOCPServer;
	PCONTEXT_OBJECT m_ContextObject;
	HICON m_IconHwnd;
	char* m_BufferData;
	CListCtrl m_CListCtrl_Dialog_Threader_Manager_Show;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void CDlgThreaderManager::WindowHandleIO();
	void CDlgThreaderManager::ShowThreaderList();
	//ˢ��
	afx_msg void OnButtonMenuThreaderManagerRefresh();

	//ɱ���߳�
	afx_msg void OnButtonMenuThreaderManagerKillOne();
	void CDlgThreaderManager::ClientThreaderKill();

	afx_msg void OnButtonMenuThreaderManagerSuspend();
	afx_msg void OnButtonMenuThreaderManagerResume();
	afx_msg void OnNMRClickListThreaderManagerShow(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClose();
};
