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
	//��Ƶ��׼
	GSM610WAVEFORMAT m_GSM610WaveFormat;
	ULONG m_BufferLength;			//�������ݴ�С

	LPWAVEHDR  m_InAudioHeader[2];  //����ͷ��Ϣ
	LPBYTE	   m_InAudioData[2];    //��������  ��������������
	
	//�ĸ��豸�Ƿ�����ʹ��
	BOOL m_bIsWaveInUsed;
	BOOL m_bOutWaveInUsed;

	//�̻߳ص���� 
	HANDLE m_WaveInThreadHandle;
	
	//�豸�־��
	HWAVEIN m_hWaveIn;
	DWORD m_nWaveInIndex;
	//�����¼�
	HANDLE m_hEventWaveIn;  //e1
	HANDLE m_hStartRecord; //e2
	


	HWAVEOUT m_hWaveOut;
	BOOL m_bIsWaveOutUsed;
	DWORD m_nWaveOutIndex;
	LPWAVEHDR  m_OutAudioHeader[2];     //����ͷ
	LPBYTE	   m_OutAudioData[2];       //��������  ��������������
public:
	LPBYTE CAudio::GetRecordData(LPDWORD dwBufferSize);
	BOOL CAudio::InitializeWaveIn();

	static DWORD WINAPI waveInCallBack(LPVOID lParam);
	BOOL CAudio::PlayRecordData(LPBYTE BufferData, DWORD BufferLength);
	BOOL CAudio::OnInitWaveOut();
	void CAudio::ChangeStatus();
};

