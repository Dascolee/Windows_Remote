#pragma once
#include <Windows.h>

class CScreenSpy
{
public:
	CScreenSpy();
	CScreenSpy(ULONG BitmapCount);
	~CScreenSpy();


	LPBITMAPINFO CScreenSpy::OninitBitmapInfo(ULONG BitmapCount,
		ULONG FullMetricsWidth, ULONG FullMetricsHeight);

	ULONG CScreenSpy::GetBitmapInfoLength();
	LPBITMAPINFO CScreenSpy::GetBitmapInfo();
	//ץ��һ��ͼ
	LPVOID CScreenSpy::GetFirstScreenData();
	ULONG CScreenSpy::GetFirstScreenLength();
	LPVOID CScreenSpy::GetNextScreenData(ULONG* BufferLength);
	VOID CScreenSpy::WriteScreenData(LPBYTE BufferData, ULONG BufferLength);
	VOID CScreenSpy::ScanScreenData(HDC DestinationDCHandle, HDC SourceDCHandle, ULONG Width, ULONG Height);
	//�Ƚ���֡����
	ULONG CScreenSpy::CompareScreenData(LPBYTE NextScreenData, LPBYTE FirstScreenData,
		LPBYTE BufferData, DWORD ScreenDataLength);



public:
	BYTE         m_Algorithm;   //(ץͼ)�㷨  --ͨ�ñȽϷ�
	ULONG        m_BitmapCount;
	ULONG        m_FullMetricsWidth;
	ULONG        m_FullMetricsHeight;
	//λͼ��Ϣ
	LPBITMAPINFO m_BitmapInfo;
	HDC          m_DesktopDCHandle;  //����
	HDC          m_DesktopMemoryDCHandle;  //���˵Ĺ�����
	HBITMAP      m_BitmapHandle;
	PVOID        m_BitmapData;
	HWND         m_DesktopHwnd;
	//���͵������������ݰ�
	BYTE*        m_BufferData;
	ULONG        m_BufferOffset;
	//�µĹ���
	HDC          m_DiffMemoryDCHandle;
	PVOID        m_DiffBitmapData;
	HBITMAP      m_DiffBitmapHandle;




	//CCursorInfor m_CursorInfor;
	

};

