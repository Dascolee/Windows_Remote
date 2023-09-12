#pragma once

#include<Windows.h>
#include<iostream>
using namespace std;

class _CConfigFlie
{
public:
	_CConfigFlie();
	~_CConfigFlie();
	BOOL InitConfigFile();
	int GetInt(string MainKey, string SubKey);
	BOOL SetInt(string MainKey, string SubKey, int BufferData);
private:
	string m_FileFullPathData;//配置文件的绝对路径
};

