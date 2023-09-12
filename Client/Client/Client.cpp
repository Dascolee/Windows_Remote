#include"client.h"
struct  _CONNECT_INFO_
{
	DWORD CheckFlag;
	char LinkIP[20];
	USHORT LinkPort;
} _ServerConnectInfo = { 0x87654321,"127.0.0.1",2356 };

void main()
{
	printf("%s\r\n", _ServerConnectInfo.LinkIP);
	//╪сть
	HMODULE ModuleHandle = (HMODULE)LoadLibrary("Dll.dll");
	if (ModuleHandle==NULL)
	{
		return;
	}
	LPFN_CLIENTRUN ClientRun =
		(LPFN_CLIENTRUN)GetProcAddress(ModuleHandle, "ClientRun");

	if (ClientRun==NULL)
	{
		FreeLibrary(ModuleHandle);
		return;
	}
	else
	{
		ClientRun(_ServerConnectInfo.LinkIP,_ServerConnectInfo.LinkPort);
	}
	printf("Input AnyKey To Exit\r\n");
	getchar();
	FreeLibrary(ModuleHandle);

}