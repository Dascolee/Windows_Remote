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
	//�����Ļ�ķֱ���
	m_FullMetricsWidth = ::GetSystemMetrics(SM_CXSCREEN)*1.25;
	m_FullMetricsHeight = ::GetSystemMetrics(SM_CYSCREEN)*1.25;

	m_BitmapInfo = OninitBitmapInfo(m_BitmapCount,m_FullMetricsWidth, m_FullMetricsHeight);  //����λͼ��Ϣ ����(��һ��)
	//�����Ļ���

	//��ô��ھ��
	m_DesktopHwnd = GetDesktopWindow();
	//��һ������
	m_DesktopDCHandle = GetDC(m_DesktopHwnd);
	//������һ��������
	m_DesktopMemoryDCHandle = CreateCompatibleDC(m_DesktopDCHandle);
	//�����ʺϹ��˵Ĺ���
	m_BitmapHandle = CreateDIBSection(m_DesktopDCHandle, m_BitmapInfo,
		DIB_RGB_COLORS, &m_BitmapData, NULL, NULL);
	//�ѹ��߷��빤����
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
	//��͹���
	ReleaseDC(m_DesktopHwnd, m_DesktopDCHandle);   
	//���չ�����
	if (m_DesktopMemoryDCHandle != NULL)
	{
		DeleteDC(m_DesktopMemoryDCHandle);                 
		//Createƥ���ڴ�DC
		DeleteObject(m_BitmapHandle);
		if (m_BitmapData != NULL)
		{
			m_BitmapData = NULL;
		}
		m_DesktopMemoryDCHandle = NULL;
	}
	
	if (m_DiffMemoryDCHandle != NULL)
	{
		DeleteDC(m_DiffMemoryDCHandle);                //Createƥ���ڴ�DC

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
	//BitBlt��ָ����Դ�豸���������е����ؽ���λ�飨bit_block��ת�����Դ��͵�Ŀ���豸������
	return m_BitmapData;  //�ڴ�
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


	// д��ʹ���������㷨
	WriteScreenData((LPBYTE)&m_Algorithm, sizeof(m_Algorithm));
	//m_BufferData =[m_Algorithm]
	//��ÿͻ������λ��
	// д����λ��
	POINT	CursorPositon;
	GetCursorPos(&CursorPositon);
	WriteScreenData((LPBYTE)&CursorPositon, sizeof(POINT));
	//m_BufferData =[m_Algorithm][POINT]
	//��ÿͻ��˹������
	// д�뵱ǰ�������
	//BYTE	CursorTypeIndex = m_CursorInfor.GetCurrentCursorIndex();
	BYTE	CursorTypeIndex = -1;//���дһ��
	WriteScreenData(&CursorTypeIndex, sizeof(BYTE));
	//m_BufferData =[m_Algorithm][POINT][CursorTypeIndex]

	// ����Ƚ��㷨
	if (m_Algorithm == ALGORITHM_DIFF)
	{
		// �ֶ�ɨ��ȫ��Ļ  ���µ�λͼ���뵽m_DiffMemDCHandle��
		ScanScreenData(m_DiffMemoryDCHandle, m_DesktopDCHandle, m_BitmapInfo->bmiHeader.biWidth,
			m_BitmapInfo->bmiHeader.biHeight);

		//����Bitmap���бȽ������һ��   
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
	//�ֶο���λͼ���ݰ�
	ULONG JumpLine = 50;
	ULONG JumpSleep = JumpLine / 10;

	for (int i = 0, ToJump = 0; i < Height; i += ToJump)
	{
		ULONG v1 = Height - i;
		if (v1 > JumpLine)
		{
			//ÿ�ΰ�50����λ����ɨ��
			ToJump = JumpLine; //���һ��
		}
		else
		{
			//���һ��
			ToJump = v1;
		}
		BitBlt(DestinationDCHandle, 0, i, Width, ToJump, SourceDCHandle, 0, i, SRCCOPY);
		Sleep(JumpSleep);
	}
}

ULONG CScreenSpy::CompareScreenData(LPBYTE NextScreenData, LPBYTE FirstScreenData, 
	LPBYTE BufferData, DWORD ScreenDataLength)
{
	// Windows�涨һ��ɨ������ռ���ֽ���������4�ı���, ������DWORD�Ƚ�
	LPDWORD	v1, v2;
	v1 = (LPDWORD)FirstScreenData;
	v2 = (LPDWORD)NextScreenData;

	//m_NextScreenBufferData [0004 0003 000A 000A 000A 0007 0000 ]  m_rectBufferOffset = 10

	// ƫ�Ƶ�ƫ�ƣ���ͬ���ȵ�ƫ��
	ULONG Offset = 0, v11 = 0, v22 = 0;
	ULONG Count = 0; 
	// ���ݼ�����

	for (int i = 0; i < ScreenDataLength; i += 4, v1++, v2++)   //HellAAArld0   HellAAArld1
	{
		if (*v1 == *v2)
		{
			//��֡���������һ��
			continue;
		}

		// �������ݲ�һ��-->д��ƫ�Ƶ�ַ
		*(LPDWORD)(BufferData + Offset) = i;
		// ��¼���ݴ�С�Ĵ��λ��
		v11 = Offset + sizeof(int);  //4
		v22 = v11 + sizeof(int);     //8
		Count = 0; 
		// ���ݼ���������
		
		// ����ǰһ֡����
		*v1 = *v2;
		*(LPDWORD)(BufferData + v22 + Count) = *v2;

		Count += 4;
		i += 4, v1++, v2++;
		for (int j = i; j < ScreenDataLength; j += 4, i += 4, v1++, v2++)
		{
			if (*v1 == *v2)
				break;

			// ����Dest�е�����
			*v1 = *v2;
			*(LPDWORD)(BufferData + v22 + Count) = *v2;
			Count += 4;
		}
		// д�����ݳ���
		*(LPDWORD)(BufferData + v11) = Count;
		Offset = v22 + Count;
	}
	return Offset;
}