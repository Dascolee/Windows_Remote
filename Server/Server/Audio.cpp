#include "stdafx.h"
#include "Audio.h"


CAudio::CAudio()
{
	m_bIsWaveInUsed = FALSE;
	m_bIsWaveOutUsed = FALSE;

	m_nWaveInIndex = 0;
	m_nWaveOutIndex = 0;

	m_WaveInThreadHandle = NULL;
	//���������¼�
	m_hEventWaveIn = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hStartRecord = CreateEvent(NULL, TRUE, FALSE, NULL);


	//��Ƶ�ĸ�ʽ
	memset(&m_GSM610WaveFormat, 0, sizeof(GSM610WAVEFORMAT));
	m_GSM610WaveFormat.wfx.wFormatTag = WAVE_FORMAT_GSM610;
	m_GSM610WaveFormat.wfx.nChannels = 1;
	m_GSM610WaveFormat.wfx.nSamplesPerSec = 8000;
	m_GSM610WaveFormat.wfx.nAvgBytesPerSec = 1625;
	m_GSM610WaveFormat.wfx.nBlockAlign = 65;
	m_GSM610WaveFormat.wfx.wBitsPerSample = 0;
	m_GSM610WaveFormat.wfx.cbSize = 2;
	m_GSM610WaveFormat.wSamplesPerBlock = 320;

	m_BufferLength = 1000;

	int i = 0;
	//�����Ŵ�
	for (i = 0; i < 2; i++)
	{
		m_InAudioHeader[i] = new WAVEHDR;
		m_InAudioData[i] = new BYTE[m_BufferLength];

		m_OutAudioHeader[i] = new WAVEHDR;
		m_OutAudioData[i] = new BYTE[m_BufferLength];
	}
}


CAudio::~CAudio()
{
	int i = 0;
	if (m_bIsWaveInUsed)
	{
		waveInStop(m_hWaveIn);
		waveInReset(m_hWaveIn);
		for (i = 0; i < 2; i++)
		{
			waveInUnprepareHeader(m_hWaveIn, m_InAudioHeader[i], sizeof(WAVEHDR));
		}
		waveInClose(m_hWaveIn);
		TerminateThread(m_hThreadCallBack, -1); //Ϊ�˰�ȫ
	}

	for (i = 0; i < 2; i++)
	{
		delete[] m_InAudioData[i];
		m_InAudioData[i] = NULL;
		delete[] m_InAudioHeader[i];
		m_InAudioHeader[i] = NULL;
	}

	CloseHandle(m_hEventWaveIn);
	CloseHandle(m_hStartRecord);
	CloseHandle(m_hThreadCallBack);  //�߳̾��

	if (m_WaveInThreadHandle != NULL)
	{
		CloseHandle(m_WaveInThreadHandle);
		m_WaveInThreadHandle = NULL;
	}

	if (m_bIsWaveOutUsed)
	{
		waveOutReset(m_hWaveOut);
		for (int i = 0; i < 2; i++)
		{
			waveOutUnprepareHeader(m_hWaveOut, m_InAudioHeader[i], sizeof(WAVEHDR));
		}
		waveOutClose(m_hWaveOut);
	}

	for (i = 0; i < 2; i++)
	{
		delete[] m_OutAudioData[i];
		m_OutAudioData[i] = NULL;
		delete[] m_OutAudioHeader[i];
		m_OutAudioHeader[i] = NULL;
	}
}

LPBYTE CAudio::GetRecordData(LPDWORD BufferLength)
{
	if (m_bIsWaveInUsed == FALSE && OnInitWaveIn() == FALSE)
	{
		return NULL;
	}
	if (BufferLength == 0)
	{
		return NULL;
	}
	SetEvent(m_hEventWaveIn);
	WaitForSingleObject(m_hStartRecord, INFINITE);
	*BufferLength = m_BufferLength;
	return m_InAudioData[m_nWaveInIndex];
}


BOOL CAudio::OnInitWaveIn()
{
	DWORD ThreadID = 0;
	MMRESULT MMResult;
	m_WaveInThreadHandle = CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)WaveInProcedure, this, CREATE_SUSPENDED, &ThreadID);	//����¼���߳�

																						//��¼���豸
	MMResult = waveInOpen(&m_hWaveIn, (WORD)WAVE_MAPPER,
		&(m_GSM610WaveFormat.wfx), (LONG)ThreadID, 0, CALLBACK_THREAD);
	if (MMResult != MMSYSERR_NOERROR)
	{
		return FALSE;
	}

	//¼���豸���ڴ����
	for (int i = 0; i < 2; i++)
	{
		m_InAudioHeader[i]->lpData = (LPSTR)m_InAudioData[i];	//��������ͷ������
		m_InAudioHeader[i]->dwBufferLength = m_BufferLength;
		m_InAudioHeader[i]->dwFlags = 0;
		m_InAudioHeader[i]->dwLoops = 0;
		waveInPrepareHeader(m_hWaveIn, m_InAudioHeader[i], sizeof(WAVEHDR));	//¼���豸�뻺����ͷ������
	}
	waveInAddBuffer(m_hWaveIn, m_InAudioHeader[m_nWaveInIndex], sizeof(WAVEHDR));

	ResumeThread(m_WaveInThreadHandle);
	waveInStart(m_hWaveIn);	//��ʼ¼��,��¼���߳�¼��
	m_bIsWaveInUsed = TRUE;
	return TRUE;
}


DWORD WINAPI CAudio::WaveInProcedure(LPVOID ParameterData)
{
	CAudio* This = (CAudio*)ParameterData;

	MSG Msg;

	while (GetMessage(&Msg, NULL, 0, 0))
	{
		if (Msg.message == MM_WIM_DATA)
		{
			//¼�����
			SetEvent(This->m_hStartRecord);
			WaitForSingleObject(This->m_hEventWaveIn, INFINITE);
			ResetEvent(This->m_hStartRecord);
			Sleep(1);
			//����������
			This->m_nWaveInIndex = 1 - This->m_nWaveInIndex;
			MMRESULT MMResult = waveInAddBuffer(This->m_hWaveIn,
				This->m_InAudioHeader[This->m_nWaveInIndex], sizeof(WAVEHDR));
			if (MMResult != MMSYSERR_NOERROR)
			{
				return -1;
			}
		}
		if (Msg.message == MM_WIM_CLOSE)
		{
			break;
		}
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return 0;
}


BOOL CAudio::PlayBuffer(LPBYTE BufferData, DWORD BufferLength)
{
	if (!m_bIsWaveOutUsed&&!InitializeWaveOut())
	{
		return FALSE;
	}
	for (int i = 0; i < BufferLength; i++)
	{
		memcpy(m_OutAudioData[m_nWaveOutIndex], BufferData, m_BufferLength);
		waveOutWrite(m_hWaveOut, m_OutAudioHeader[m_nWaveOutIndex], sizeof(WAVEHDR));
		m_nWaveOutIndex = 1 - m_nWaveOutIndex;
	}

	return TRUE;
}

BOOL CAudio::InitializeWaveOut()
{
	if (!waveOutGetNumDevs())
	{
		return FALSE;
	}
	int i;
	for ( i = 0; i < 2; i++)
	{
		memset(m_OutAudioData[i], 0, m_BufferLength);//��������
	}
	MMRESULT mmResult;

	//��¼���豸COM 1 ָ��������� 2֧��ͨ���̻߳ص� ������
	mmResult = waveOutOpen(&m_hWaveOut, (WORD)WAVE_MAPPER,
		&(m_GSM610WaveFormat.wfx), (LONG)0, (LONG)0, CALLBACK_NULL);
	if (mmResult != MMSYSERR_NOERROR)
	{
		return FALSE;
	}
	//�����豸���ڴ����
	for (i = 0; i < 2; i++)
	{
		m_OutAudioHeader[i]->lpData = (LPSTR)m_OutAudioData[i];   //m_OutAudioHeader  ָ������
		m_OutAudioHeader[i]->dwBufferLength = m_BufferLength;
		m_OutAudioHeader[i]->dwFlags = 0;
		m_OutAudioHeader[i]->dwLoops = 0;
		waveOutPrepareHeader(m_hWaveOut, m_OutAudioHeader[i], sizeof(WAVEHDR));
	}
	m_bIsWaveOutUsed = TRUE;
	return TRUE;

}

void CAudio::ChangeStatus()
{
	int i = 0;
	if (m_bIsWaveOutUsed)
	{
		waveOutReset(m_hWaveOut);
		for (i = 0; i < 2; i++)
		{
			//��������豸�뻺�����Ĺ���
			waveOutUnprepareHeader(m_hWaveOut, m_OutAudioHeader[i], sizeof(WAVEHDR));
		}
		waveOutClose(m_hWaveOut);	//�رղ����豸
		m_hWaveOut = NULL;
		m_bIsWaveOutUsed = FALSE;
	}

	if (m_bIsWaveInUsed)
	{
		//����¼��
		waveInStop(m_hWaveIn);	//ֹͣ¼��
		waveInReset(m_hWaveIn);
		for (i = 0; i < 2; i++)
		{
			//���¼���豸�뻺�����Ĺ���
			waveInUnprepareHeader(m_hWaveIn, m_InAudioHeader[i], sizeof(WAVEHDR));
		}
		waveInClose(m_hWaveIn);	//�ر�¼���豸
		m_hWaveIn = NULL;
		TerminateThread(m_WaveInThreadHandle, -1);	//Ϊ�˰�ȫ
		CloseHandle(m_WaveInThreadHandle);
		m_WaveInThreadHandle = NULL;
		m_bIsWaveInUsed = FALSE;
	}
	return;
}