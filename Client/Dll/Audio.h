#pragma once
#include <Windows.h>


#include <MMSYSTEM.H>
#include <MMReg.H>

class CAudio
{
public:
	CAudio();
	~CAudio();
public:
	//音频标准
	GSM610WAVEFORMAT m_GSM610WaveFormat;
	ULONG m_BufferLength;			//两个数据大小

	LPWAVEHDR  m_InAudioHeader[2];  //两个头信息
	LPBYTE	   m_InAudioData[2];    //两个数据  保持声音的连续
	
	//哪个设备是否正在使用
	BOOL m_bIsWaveInUsed;
	BOOL m_bOutWaveInUsed;

	//线程回调句柄 
	HANDLE m_WaveInThreadHandle;
	
	//设备局句柄
	HWAVEIN m_hWaveIn;
	DWORD m_nWaveInIndex;
	//两个事件
	HANDLE m_hEventWaveIn;  //e1
	HANDLE m_hStartRecord; //e2
	


	HWAVEOUT m_hWaveOut;
	BOOL m_bIsWaveOutUsed;
	DWORD m_nWaveOutIndex;
	LPWAVEHDR  m_OutAudioHeader[2];     //两个头
	LPBYTE	   m_OutAudioData[2];       //两个数据  保持声音的连续
public:
	LPBYTE CAudio::GetRecordData(LPDWORD dwBufferSize);
	BOOL CAudio::InitializeWaveIn();

	static DWORD WINAPI waveInCallBack(LPVOID lParam);
	BOOL CAudio::PlayRecordData(LPBYTE BufferData, DWORD BufferLength);
	BOOL CAudio::OnInitWaveOut();
	void CAudio::ChangeStatus();
};

