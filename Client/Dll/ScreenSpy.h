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
	//抓第一张图
	LPVOID CScreenSpy::GetFirstScreenData();
	ULONG CScreenSpy::GetFirstScreenLength();
	LPVOID CScreenSpy::GetNextScreenData(ULONG* BufferLength);
	VOID CScreenSpy::WriteScreenData(LPBYTE BufferData, ULONG BufferLength);
	VOID CScreenSpy::ScanScreenData(HDC DestinationDCHandle, HDC SourceDCHandle, ULONG Width, ULONG Height);
	//比较两帧数据
	ULONG CScreenSpy::CompareScreenData(LPBYTE NextScreenData, LPBYTE FirstScreenData,
		LPBYTE BufferData, DWORD ScreenDataLength);



public:
	BYTE         m_Algorithm;   //(抓图)算法  --通用比较法
	ULONG        m_BitmapCount;
	ULONG        m_FullMetricsWidth;
	ULONG        m_FullMetricsHeight;
	//位图信息
	LPBITMAPINFO m_BitmapInfo;
	HDC          m_DesktopDCHandle;  //工人
	HDC          m_DesktopMemoryDCHandle;  //工人的工具箱
	HBITMAP      m_BitmapHandle;
	PVOID        m_BitmapData;
	HWND         m_DesktopHwnd;
	//发送到服务器的数据包
	BYTE*        m_BufferData;
	ULONG        m_BufferOffset;
	//新的工人
	HDC          m_DiffMemoryDCHandle;
	PVOID        m_DiffBitmapData;
	HBITMAP      m_DiffBitmapHandle;




	//CCursorInfor m_CursorInfor;
	

};

