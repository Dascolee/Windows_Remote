#pragma once
#include"_IOCPServer.h"
#include"Common.h"
#include "afxcmn.h"


enum MYKEY
{
	MHKEY_CLASSES_ROOT,
	MHKEY_CURRENT_USER,
	MHKEY_LOCAL_MACHINE,
	MHKEY_USERS,
	MHKEY_CURRENT_CONFIG
};
enum MVALUE
{
	MREG_SZ,
	MREG_DWORD,
	MREG_BINARY,
	MREG_EXPAND_SZ
};

struct REGMSG
{
	int count;  //名字个数
	DWORD size; //名字大小
	DWORD valsize; //值大小
};
// CDlgRegisterManager 对话框

class CDlgRegisterManager : public CDialog
{
	DECLARE_DYNAMIC(CDlgRegisterManager)

public:
	CDlgRegisterManager(CWnd* Parent, _CIOCPServer* IOCPServer = NULL,
		CONTEXT_OBJECT *ContextObject = NULL); ;   // 标准构造函数
	virtual ~CDlgRegisterManager();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_REGISTER_MANAGER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	_CIOCPServer* m_IOCPServer;
	PCONTEXT_OBJECT m_ContextObject;
	HICON m_IconHwnd;
	CTreeCtrl m_CTreeCtrl_Dialog_Register_Manager_Show;
	CListCtrl m_CListCtrl_Dialog_Register_Manager_Show;
	CImageList m_ImageList_Tree;  //Tree控件上的图标
	CImageList m_ImageList_List;  //List控件上的图标

	HTREEITEM m_hRoot;
	HTREEITEM HKLM;
	HTREEITEM HKCR;
	HTREEITEM HKCU;
	HTREEITEM HKUS;
	HTREEITEM HKCC;
	HTREEITEM m_hSelectedTreeItem;
	BOOL m_IsEnable;

public:
	void CDlgRegisterManager::WindowHandleIO(void);

	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnTvnSelchangedTreeRegisterManagerShow(NMHDR *pNMHDR, LRESULT *pResult);
	char CDlgRegisterManager::GetFatherPath(CString& FullPath);
	CString CDlgRegisterManager::GetFullPath(HTREEITEM  hCurrent);
	void CDlgRegisterManager::AddPath(char* BufferData);
	void CDlgRegisterManager::AddKey(char* BufferData);
};
