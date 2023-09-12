#pragma once
#include <Windows.h>
#include"Common.h"

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

class CRegisterOperation
{
public:
	CRegisterOperation(char IsToken);
	virtual ~CRegisterOperation();
	void CRegisterOperation::SetPath(char *FullPath);
	char* CRegisterOperation::FindPath();
	char* CRegisterOperation::FindKey();
public:
	HKEY MKEY; //KeyHandle
	char KeyPath[MAX_PATH];
};

