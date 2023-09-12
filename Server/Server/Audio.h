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
	ULONG m_BufferLength;			//�������ݴ�С

	GSM610WAVEFORMAT m_GSM610WaveFormat;

	LPWAVEHDR  m_InAudioHeader[2];  //����ͷ��Ϣ
	LPBYTE	   m_InAudioData[2];    //��������  ��������������
	//�ĸ��豸�Ƿ�����ʹ��
	BOOL m_bIsWaveInUsed;
	//�̻߳ص���� 
	HANDLE m_hThreadCallBack;
	//�豸�־��
	HWAVEIN m_hWaveIn;
	DWORD m_nWaveInIndex;
	//�����¼�
	HANDLE m_hEventWaveIn;  //e1
	HANDLE m_hStartRecord; //e2
	//¼���豸���
	HANDLE m_WaveInThreadHandle;

	HWAVEOUT m_hWaveOut;
	BOOL m_bIsWaveOutUsed;
	DWORD m_nWaveOutIndex;
	LPWAVEHDR  m_OutAudioHeader[2];     //����ͷ
	LPBYTE	   m_OutAudioData[2];       //��������  ��������������
public:
	BOOL CAudio::PlayBuffer(LPBYTE szBuffer, DWORD dwBufferSize);
	BOOL CAudio::InitializeWaveOut();
	LPBYTE CAudio::GetRecordData(LPDWORD BufferLength);
	BOOL CAudio::OnInitWaveIn();
	void CAudio::ChangeStatus();
	static DWORD WINAPI WaveInProcedure(LPVOID ParameterData);
};

