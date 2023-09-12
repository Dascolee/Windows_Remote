#include "Audio.h"



CAudio::CAudio()
{
	m_bIsWaveInUsed =FALSE ;
	m_bOutWaveInUsed = FALSE;

	m_nWaveInIndex = 0;
	m_nWaveOutIndex = 0;


	//创建两个事件
	m_hEventWaveIn = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hStartRecord = CreateEvent(NULL, TRUE, FALSE, NULL);


	//音频的格式
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
	//两个磁带
	for ( i = 0; i < 2; i++)
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
		for ( i = 0; i < 2; i++)
		{
			waveInUnprepareHeader(m_hWaveIn, m_InAudioHeader[i], sizeof(WAVEHDR));
		}
		waveInClose(m_hWaveIn);
		TerminateThread(m_WaveInThreadHandle,-1); //为了安全
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
	
	if (m_WaveInThreadHandle != NULL)
	{
		CloseHandle(m_WaveInThreadHandle);
		m_WaveInThreadHandle = NULL;
	}
	if (m_bIsWaveOutUsed)
	{
		waveOutReset(m_hWaveOut);
		for (i = 0; i < 2; i++)
		{
			//解除播放设备与缓冲区的关联
			waveOutUnprepareHeader(m_hWaveOut, m_OutAudioHeader[i], sizeof(WAVEHDR));
		}
		waveOutClose(m_hWaveOut);	//关闭播放设备
	}
	for (i = 0; i < 2; i++)
	{
		delete[] m_OutAudioData[i];
		m_OutAudioData[i] = NULL;
		delete[] m_OutAudioHeader[i];
		m_OutAudioHeader[i] = NULL;
	}
}


LPBYTE CAudio::GetRecordData(LPDWORD dwBufferSize)
{
	//录音机
	if (m_bIsWaveInUsed == FALSE && InitializeWaveIn() == FALSE)
	{
		return NULL;
	}

	if (dwBufferSize == NULL)
	{
		return NULL;
	}
	SetEvent(m_hStartRecord);
	WaitForSingleObject(m_hEventWaveIn, INFINITE);
	*dwBufferSize = m_BufferLength;
	return m_InAudioData[m_nWaveInIndex];
}

BOOL CAudio::InitializeWaveIn()
{
	MMRESULT mmResult;
	DWORD ThreadID = 0;

	m_WaveInThreadHandle = CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)waveInCallBack, (LPVOID)this, 
		CREATE_SUSPENDED, &ThreadID);
	//打开录音设备COM 1 指定声音规格 2支持通过线程回调 换缓冲
	mmResult = waveInOpen(&m_hWaveIn, (WORD)WAVE_MAPPER,
		&(m_GSM610WaveFormat.wfx), (LONG)ThreadID,(LONG)0, CALLBACK_THREAD);
	if (mmResult!= MMSYSERR_NOERROR)
	{
		return FALSE;
	}

	for (int i = 0; i < 2; i++)
	{
		m_InAudioHeader[i]->lpData = (LPSTR)m_InAudioData[i];   //m_InAudioHeader  指针数组
		m_InAudioHeader[i]->dwBufferLength = m_BufferLength;  
		m_InAudioHeader[i]->dwFlags = 0;
		m_InAudioHeader[i]->dwLoops = 0;
		waveInPrepareHeader(m_hWaveIn, m_InAudioHeader[i], sizeof(WAVEHDR));
	}

	waveInAddBuffer(m_hWaveIn, m_InAudioHeader[m_nWaveInIndex], sizeof(WAVEHDR));

	ResumeThread(m_WaveInThreadHandle);
	waveInStart(m_hWaveIn);//录音

	m_bIsWaveInUsed = TRUE;
	return TRUE;
}

DWORD WINAPI CAudio::waveInCallBack(LPVOID lParam)
{
	CAudio *This = (CAudio*)lParam;

	MSG Msg;

	while (GetMessage(&Msg, NULL,0,0))
	{
		if (Msg.message == MM_WIM_DATA)
		{
			SetEvent(This->m_hEventWaveIn);
			WaitForSingleObject(This->m_hStartRecord,INFINITE);
			ResetEvent(This->m_hEventWaveIn);
			Sleep(1);
			//换磁带
			This->m_nWaveInIndex = 1 - This->m_nWaveInIndex;

			//更新缓冲
			MMRESULT mmResult = waveInAddBuffer(This->m_hWaveIn,
				This->m_InAudioHeader[This->m_nWaveInIndex], sizeof(WAVEHDR));
			if (mmResult!=MMSYSERR_NOERROR)
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


BOOL CAudio::PlayRecordData(LPBYTE BufferData, DWORD BufferLength)
{
	if (!m_bIsWaveOutUsed && !OnInitWaveOut())
	{
		return FALSE;
	}
	for (int i = 0; i < BufferLength; i += m_BufferLength)
	{
		memcpy(m_OutAudioData[m_nWaveOutIndex], BufferData, m_BufferLength);
		waveOutWrite(m_hWaveOut, m_OutAudioHeader[m_nWaveOutIndex], sizeof(WAVEHDR));
		m_nWaveOutIndex = 1 - m_nWaveOutIndex;	//切换缓冲区
	}
	return TRUE;
}

BOOL CAudio::OnInitWaveOut()
{
	if (!waveOutGetNumDevs())
	{
		return FALSE;
	}
	int i = 0;
	for (i = 0; i < 2; i++)
	{
		memset(m_OutAudioData[i], 0, m_BufferLength);
	}
	MMRESULT MMResult;
	//打开录音设备
	MMResult = waveOutOpen(&m_hWaveOut, (WORD)WAVE_MAPPER,
		&(m_GSM610WaveFormat.wfx), (LONG)0, 0, CALLBACK_NULL);
	if (MMResult != MMSYSERR_NOERROR)
	{
		return FALSE;
	}

	//播放设备与内存关联
	for (int i = 0; i < 2; i++)
	{
		m_OutAudioHeader[i]->lpData = (LPSTR)m_OutAudioData[i];	//缓冲区与头部关联
		m_OutAudioHeader[i]->dwBufferLength = m_BufferLength;
		m_OutAudioHeader[i]->dwFlags = 0;
		m_OutAudioHeader[i]->dwLoops = 0;
		waveOutPrepareHeader(m_hWaveOut, m_OutAudioHeader[i], sizeof(WAVEHDR));	//录音设备与缓冲区头部关联
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
			//解除播放设备与缓冲区的关联
			waveOutUnprepareHeader(m_hWaveOut, m_OutAudioHeader[i], sizeof(WAVEHDR));
		}
		waveOutClose(m_hWaveOut);	//关闭播放设备
		m_hWaveOut = NULL;
		m_bIsWaveOutUsed = FALSE;
	}

	if (m_bIsWaveInUsed)
	{
		//正在录音
		waveInStop(m_hWaveIn);	//停止录音
		waveInReset(m_hWaveIn);
		for (i = 0; i < 2; i++)
		{
			//解除录音设备与缓冲区的关联
			waveInUnprepareHeader(m_hWaveIn, m_InAudioHeader[i], sizeof(WAVEHDR));
		}
		waveInClose(m_hWaveIn);	//关闭录音设备
		m_hWaveIn = NULL;
		TerminateThread(m_WaveInThreadHandle, -1);	//为了安全
		CloseHandle(m_WaveInThreadHandle);
		m_WaveInThreadHandle = NULL;
		m_bIsWaveInUsed = FALSE;
	}
	return;
}
