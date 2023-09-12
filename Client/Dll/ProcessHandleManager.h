#pragma once
#include "Manager.h"
#include "Common.h"
#include <vector>
#include "SeEnumThreadInfoByNtQuerySystemInformation.h"

typedef struct _OBJECT_NAME_INFORMATION
{
	UNICODE_STRING Name;
} OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;

typedef struct _HANDLE_ITEM_INFORMATION_
{
	ULONG_PTR HandleValue;
	PVOID  Object;
	string HandleName;
	string HandleType;
}HANDLE_ITEM_INFORMATION, *PHANDLE_ITEM_INFORMATION;

typedef struct _SYSTEM_HANDLE_INFORMATION_
{
	USHORT UniqueProcessID;
	USHORT CreatorBackTraceIndex;
	UCHAR  ObjectTypeIndex;
	UCHAR  HandleAttributes;
	USHORT HandleValue;
	PVOID  Object;
	ULONG  GrantedAccess;
}SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

typedef struct _SYSTEM_HANDLES_INFORMATION_
{
	ULONG NumberOfHandles;
	SYSTEM_HANDLE_INFORMATION SystemHandleInfo[1];
}SYSTEM_HANDLES_INFORMATION, *PSYSTEM_HANDLES_INFORMATION;


typedef struct _OBJECT_TYPE_INFORMATION
{
	UNICODE_STRING TypeName;
	ULONG TotalNumberOfObjects;
	ULONG TotalNumberOfHandles;
	ULONG TotalPagedPoolUsage;
	ULONG TotalNonPagedPoolUsage;
	ULONG TotalNamePoolUsage;
	ULONG TotalHandleTableUsage;
	ULONG HighWaterNumberOfObjects;
	ULONG HighWaterNumberOfHandles;
	ULONG HighWaterPagedPoolUsage;
	ULONG HighWaterNonPagedPoolUsage;
	ULONG HighWaterNamePoolUsage;
	ULONG HighWaterHandleTableUsage;
	ULONG InvalidAttributes;
	GENERIC_MAPPING GenericMapping;
	ULONG ValidAccessMask;
	BOOLEAN SecurityRequired;
	BOOLEAN MaintainHandleCount;
	UCHAR TypeIndex; // Since Windows 8.1
	CHAR ReservedByte;
	ULONG PoolType;
	ULONG DefaultPagedPoolCharge;
	ULONG DefaultNonPagedPoolCharge;
} OBJECT_TYPE_INFORMATION, *POBJECT_TYPE_INFORMATION;

/*
typedef
NTSTATUS(NTAPI *LPFN_NTQUERYSYSTEMINFORMATION)(
	IN SYSTEM_INFORMATION_CLASS  SystemInformationClass,
	OUT PVOID SystemInformation,
	IN ULONG   SystemInformationLength,
	OUT PULONG ReturnLength);
*/

typedef enum OBJECT_INFORMATION_CLASS
{
	ObjectBasicInformation,
	ObjectNameInformation,
	ObjectTypeInformation,
	ObjectTypesInformation,
	ObjectHandleFlagInformation,
	ObjectSessionInformation,
	MaxObjectInfoClass
}OBJECT_INFORMATION_CLASS;



typedef
NTSTATUS
(NTAPI* LPFN_NTQUERYOBJECT)(
	IN  HANDLE HandleValue,
	IN  OBJECT_INFORMATION_CLASS ObjectInformationClass,
	OUT PVOID  ObjectInformation,
	IN  ULONG  ObjectInformationLength,
	OUT PULONG ReturnLength OPTIONAL);


class CProcessHandleManager 
	:public CManager
{
public:
	CProcessHandleManager(_CIOCPClient * IOCPClient);

	~CProcessHandleManager();

	BOOL CProcessHandleManager::SendClientProcessHandleList();

	BOOL CProcessHandleManager::EnumHandleInfoByProcessID(HANDLE ProcessID);

	vector<HANDLE_ITEM_INFORMATION> m_HandleItemInfoVector;


};

BOOL SeEnumHandleInfoByNtQuerySystemInformation(HANDLE ProcessID,
	vector<HANDLE_ITEM_INFORMATION>& HandleItemInfoVector);

void* SeGetHandleNameByNtQueryObject(HANDLE ProcessID, HANDLE HandleValue, BOOL IsTranslateName);

void* SeGetHandleNameByNtQueryObjectW(HANDLE ProcessID, HANDLE HandleValue, BOOL IsTranslateName);

void* SeGetHandleTypeByNtQueryObject(HANDLE ProcessID, HANDLE HandleValue);


BOOL HanEnableSeDebugPrivilege(HANDLE ProcessHandle, BOOL IsEnable);
HANDLE HanOpenProcess(DWORD DesiredAccess, BOOL IsInheritHandle, HANDLE ProcessID);
BOOL  HanCloseHandle(HANDLE HandleValue);
void* HanDosPathToNtPathW(wchar_t* StringName);
