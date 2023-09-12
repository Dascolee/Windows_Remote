#include "stdafx.h"
#include "_ConfigFlie.h"


_CConfigFlie::_CConfigFlie()
{
	InitConfigFile();
}

_CConfigFlie::~_CConfigFlie()
{
}

BOOL _CConfigFlie::InitConfigFile()
{
	CHAR FileFullPath[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, FileFullPath, MAX_PATH);
	CHAR *v1 = NULL;


	v1 = strstr(FileFullPath, ".");
	if (v1 != NULL)
	{
		*v1 = '\0';
		strcat(FileFullPath, ".ini");
	}

	//创建一个INI文件

	HANDLE FileHandle = CreateFileA(FileFullPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE,
		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_HIDDEN, NULL);//
	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	m_FileFullPathData = FileFullPath;

	ULONG HighLenght = 0;
	ULONG LowLenght = GetFileSize(FileHandle, &HighLenght);//获得文件大小
	if (LowLenght>0 || HighLenght>0)//判断不是空文件
	{
		CloseHandle(FileHandle);
		return FALSE;
	}
	CloseHandle(FileHandle);
	WritePrivateProfileStringA("Settings", "ListenPort", "2356", m_FileFullPathData.c_str());
	WritePrivateProfileStringA("Settings", "MaxConnection", "10", m_FileFullPathData.c_str());
	return TRUE;
}

int _CConfigFlie::GetInt(string MainKey, string SubKey)
{
	return ::GetPrivateProfileInt(MainKey.c_str(), SubKey.c_str(), 0, m_FileFullPathData.c_str());
}

BOOL _CConfigFlie::SetInt(string MainKey, string SubKey, int BufferData)
{
	string v1;
	sprintf((char*)v1.c_str(), "%d", BufferData);
	return WritePrivateProfileStringA(MainKey.c_str(), SubKey.c_str(), v1.c_str(), m_FileFullPathData.c_str());
}

