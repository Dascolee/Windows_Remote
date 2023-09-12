#pragma once

#include <MMSYSTEM.H>
#include <MMReg.H>
#pragma comment (lib,"winmm.lib");

class CAudio
{
public:
	CAudio();
	~CAudio();
public:
	ULONG m_BufferLength;			//两个数据大小

	GSM610WAVEFORMAT m_GSM610WaveFormat;

	LPWAVEHDR  m_InAudioHeader[2];  //两个头信息
	LPBYTE	   m_InAudioData[2];    //两个数据  保持声音的连续
	//哪个设备是否正在使用
	BOOL m_bIsWaveInUsed;
	//线程回调句柄 
	HANDLE m_hThreadCallBack;
	//设备局句柄
	HWAVEIN m_hWaveIn;
	DWORD m_nWaveInIndex;
	//两个事件
	HANDLE m_hEventWaveIn;  //e1
	HANDLE m_hStartRecord; //e2
	//录音设备句柄
	HANDLE m_WaveInThreadHandle;

	HWAVEOUT m_hWaveOut;
	BOOL m_bIsWaveOutUsed;
	DWORD m_nWaveOutIndex;
	LPWAVEHDR  m_OutAudioHeader[2];     //两个头
	LPBYTE	   m_OutAudioData[2];       //两个数据  保持声音的连续
public:
	BOOL CAudio::PlayBuffer(LPBYTE szBuffer, DWORD dwBufferSize);
	BOOL CAudio::InitializeWaveOut();
	LPBYTE CAudio::GetRecordData(LPDWORD BufferLength);
	BOOL CAudio::OnInitWaveIn();
	void CAudio::ChangeStatus();
	static DWORD WINAPI WaveInProcedure(LPVOID ParameterData);
};

