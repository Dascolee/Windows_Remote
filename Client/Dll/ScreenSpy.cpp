#include "ScreenSpy.h"

#define ALGORITHM_DIFF 1


CScreenSpy::CScreenSpy()
{
}

CScreenSpy::CScreenSpy(ULONG BitmapCount)
{
	m_Algorithm = ALGORITHM_DIFF;
	m_BitmapInfo = NULL;

	switch (BitmapCount)
	{
	case 16:
	case 32:
		m_BitmapCount = BitmapCount;
		break;
	default:
		m_BitmapCount = 16;
	}
	//获得屏幕的分辨率
	m_FullMetricsWidth = ::GetSystemMetrics(SM_CXSCREEN)*1.25;
	m_FullMetricsHeight = ::GetSystemMetrics(SM_CYSCREEN)*1.25;

	m_BitmapInfo = OninitBitmapInfo(m_BitmapCount,m_FullMetricsWidth, m_FullMetricsHeight);  //构建位图信息 发送(第一波)
	//获得屏幕句柄

	//获得窗口句柄
	m_DesktopHwnd = GetDesktopWindow();
	//请一个工人
	m_DesktopDCHandle = GetDC(m_DesktopHwnd);
	//给工人一个工具箱
	m_DesktopMemoryDCHandle = CreateCompatibleDC(m_DesktopDCHandle);
	//创建适合工人的工具
	m_BitmapHandle = CreateDIBSection(m_DesktopDCHandle, m_BitmapInfo,
		DIB_RGB_COLORS, &m_BitmapData, NULL, NULL);
	//把工具放入工具箱
	SelectObject(m_DesktopMemoryDCHandle, m_BitmapHandle);


	m_DiffMemoryDCHandle = CreateCompatibleDC(m_DesktopDCHandle);
	m_DiffBitmapHandle = CreateDIBSection(m_DesktopDCHandle, m_BitmapInfo,
		DIB_RGB_COLORS, &m_DiffBitmapData, NULL, NULL);
	SelectObject(m_DiffMemoryDCHandle, m_DiffBitmapHandle);
	
	m_BufferOffset = 0;
	m_BufferData = new BYTE[m_BitmapInfo->bmiHeader.biSizeImage * 2];

}

CScreenSpy::~CScreenSpy()
{
	//解雇工人
	ReleaseDC(m_DesktopHwnd, m_DesktopDCHandle);   
	//回收工具箱
	if (m_DesktopMemoryDCHandle != NULL)
	{
		DeleteDC(m_DesktopMemoryDCHandle);                 
		//Create匹配内存DC
		DeleteObject(m_BitmapHandle);
		if (m_BitmapData != NULL)
		{
			m_BitmapData = NULL;
		}
		m_DesktopMemoryDCHandle = NULL;
	}
	
	if (m_DiffMemoryDCHandle != NULL)
	{
		DeleteDC(m_DiffMemoryDCHandle);                //Create匹配内存DC

		DeleteObject(m_DiffBitmapHandle);
		if (m_DiffBitmapData != NULL)
		{
			m_DiffBitmapData = NULL;
		}
	}

	if (m_BitmapInfo != NULL)
	{
		delete[] m_BitmapInfo;
		m_BitmapInfo = NULL;
	}

	if (m_BufferData)
	{
		delete[] m_BufferData;
		m_BufferData = NULL;
	}
	m_BufferOffset = 0;
}

LPBITMAPINFO CScreenSpy::OninitBitmapInfo(ULONG BitmapCount,
	ULONG FullMetricsWidth, ULONG FullMetricsHeight)
{
	ULONG BufferLength = sizeof(BITMAPINFOHEADER);
	BITMAPINFO* BitmapInfo = (BITMAPINFO *) new BYTE[BufferLength];
	/*typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[1];
	} BITMAPINFO, FAR *LPBITMAPINFO, *PBITMAPINFO;*/
	BITMAPINFOHEADER* BitmapInforHeader = &(BitmapInfo->bmiHeader);

	BitmapInforHeader->biSize = sizeof(BITMAPINFOHEADER);
	BitmapInforHeader->biWidth = FullMetricsWidth; //1080
	BitmapInforHeader->biHeight = FullMetricsHeight; //1920
	BitmapInforHeader->biPlanes = 1;
	BitmapInforHeader->biBitCount = BitmapCount; //16
	BitmapInforHeader->biCompression = BI_RGB;
	BitmapInforHeader->biXPelsPerMeter = 0;
	BitmapInforHeader->biYPelsPerMeter = 0;
	BitmapInforHeader->biClrUsed = 0;
	BitmapInforHeader->biClrImportant = 0;
	BitmapInforHeader->biSizeImage =
		((BitmapInforHeader->biWidth * BitmapInforHeader->biBitCount + 31) / 32) * 4 * BitmapInforHeader->biHeight;

	return BitmapInfo;
}

ULONG CScreenSpy::GetBitmapInfoLength()
{
	return sizeof(BITMAPINFOHEADER);
}

LPBITMAPINFO CScreenSpy::GetBitmapInfo()	
{
	return m_BitmapInfo;
}


LPVOID CScreenSpy::GetFirstScreenData()
{
	::BitBlt(m_DesktopMemoryDCHandle, 0, 0,
		m_FullMetricsWidth, m_FullMetricsHeight, m_DesktopDCHandle, 0, 0, SRCCOPY);
	//BitBlt对指定的源设备环境区域中的像素进行位块（bit_block）转换，以传送到目标设备环境。
	return m_BitmapData;  //内存
}

ULONG CScreenSpy::GetFirstScreenLength()
{
	return m_BitmapInfo->bmiHeader.biSizeImage;
}

LPVOID CScreenSpy::GetNextScreenData(ULONG* BufferLength)
{

	if (BufferLength == NULL || m_BufferData == NULL)
	{
		return NULL;
	}

	m_BufferOffset = 0;


	// 写入使用了哪种算法
	WriteScreenData((LPBYTE)&m_Algorithm, sizeof(m_Algorithm));
	//m_BufferData =[m_Algorithm]
	//获得客户端鼠标位置
	// 写入光标位置
	POINT	CursorPositon;
	GetCursorPos(&CursorPositon);
	WriteScreenData((LPBYTE)&CursorPositon, sizeof(POINT));
	//m_BufferData =[m_Algorithm][POINT]
	//获得客户端光标类型
	// 写入当前光标类型
	//BYTE	CursorTypeIndex = m_CursorInfor.GetCurrentCursorIndex();
	BYTE	CursorTypeIndex = -1;//随便写一个
	WriteScreenData(&CursorTypeIndex, sizeof(BYTE));
	//m_BufferData =[m_Algorithm][POINT][CursorTypeIndex]

	// 差异比较算法
	if (m_Algorithm == ALGORITHM_DIFF)
	{
		// 分段扫描全屏幕  将新的位图放入到m_DiffMemDCHandle中
		ScanScreenData(m_DiffMemoryDCHandle, m_DesktopDCHandle, m_BitmapInfo->bmiHeader.biWidth,
			m_BitmapInfo->bmiHeader.biHeight);

		//两个Bitmap进行比较如果不一样   
		*BufferLength = m_BufferOffset +
			CompareScreenData((LPBYTE)m_DiffBitmapData, (LPBYTE)m_BitmapData,
				m_BufferData + m_BufferOffset,
				m_BitmapInfo->bmiHeader.biSizeImage);
		return m_BufferData;
	}
	//													  Offset	*v11     *v22
	//m_BufferData =[m_Algorithm][POINT][CursorTypeIndex][(DWORD)][(DWORD)][(DWORD)]
	//m_NextScreenBufferData [ALGORITHM_DIFF 4X 4Y BYTE 0002 0002 000A 000C]
	return NULL;
}

VOID CScreenSpy::WriteScreenData(LPBYTE BufferData, ULONG BufferLength)
{
	memcpy(m_BufferData + m_BufferOffset, BufferData, BufferLength);
	m_BufferOffset += BufferLength;
}

VOID CScreenSpy::ScanScreenData(HDC DestinationDCHandle, HDC SourceDCHandle, ULONG Width, ULONG Height)
{
	//分段拷贝位图数据包
	ULONG JumpLine = 50;
	ULONG JumpSleep = JumpLine / 10;

	for (int i = 0, ToJump = 0; i < Height; i += ToJump)
	{
		ULONG v1 = Height - i;
		if (v1 > JumpLine)
		{
			//每次按50个单位进行扫描
			ToJump = JumpLine; //最后一次
		}
		else
		{
			//最后一次
			ToJump = v1;
		}
		BitBlt(DestinationDCHandle, 0, i, Width, ToJump, SourceDCHandle, 0, i, SRCCOPY);
		Sleep(JumpSleep);
	}
}

ULONG CScreenSpy::CompareScreenData(LPBYTE NextScreenData, LPBYTE FirstScreenData, 
	LPBYTE BufferData, DWORD ScreenDataLength)
{
	// Windows规定一个扫描行所占的字节数必须是4的倍数, 所以用DWORD比较
	LPDWORD	v1, v2;
	v1 = (LPDWORD)FirstScreenData;
	v2 = (LPDWORD)NextScreenData;

	//m_NextScreenBufferData [0004 0003 000A 000A 000A 0007 0000 ]  m_rectBufferOffset = 10

	// 偏移的偏移，不同长度的偏移
	ULONG Offset = 0, v11 = 0, v22 = 0;
	ULONG Count = 0; 
	// 数据计数器

	for (int i = 0; i < ScreenDataLength; i += 4, v1++, v2++)   //HellAAArld0   HellAAArld1
	{
		if (*v1 == *v2)
		{
			//两帧数据如果都一致
			continue;
		}

		// 发现数据不一样-->写入偏移地址
		*(LPDWORD)(BufferData + Offset) = i;
		// 记录数据大小的存放位置
		v11 = Offset + sizeof(int);  //4
		v22 = v11 + sizeof(int);     //8
		Count = 0; 
		// 数据计数器归零
		
		// 更新前一帧数据
		*v1 = *v2;
		*(LPDWORD)(BufferData + v22 + Count) = *v2;

		Count += 4;
		i += 4, v1++, v2++;
		for (int j = i; j < ScreenDataLength; j += 4, i += 4, v1++, v2++)
		{
			if (*v1 == *v2)
				break;

			// 更新Dest中的数据
			*v1 = *v2;
			*(LPDWORD)(BufferData + v22 + Count) = *v2;
			Count += 4;
		}
		// 写入数据长度
		*(LPDWORD)(BufferData + v11) = Count;
		Offset = v22 + Count;
	}
	return Offset;
}