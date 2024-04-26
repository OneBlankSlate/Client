#include"Client.h"
void _tmain()
{
	_tprintf(_T("ServerAddress:%s\r\n"), __ConnectInfo.SeverAddress);
	_tprintf(_T("ConnectPort:%d\r\n"), __ConnectInfo.ConnectPort);
	//加载Dll.dll模块
	HMODULE  ModuleHandle = (HMODULE)LoadLibrary(_T("Dll.dll"));   //加载自己写的模块
	if (ModuleHandle == NULL)
	{
		return;
	}
	//获取一个Dll模块中的一个导出函数
	LPFN_CLIENTRUN ClientRun =
		(LPFN_CLIENTRUN)GetProcAddress(ModuleHandle, "ClientRun");

	//没有获取到Dll.dll中的导出函数
	if (ClientRun == NULL)
	{
		FreeLibrary(ModuleHandle);   //释放模块 退出
		return;
	}

	else
	{
		ClientRun(__ConnectInfo.SeverAddress, __ConnectInfo.ConnectPort);
	}



	_tprintf(_T("Input AnyKey To Exit\r\n"));
	_gettchar();
	FreeLibrary(ModuleHandle);   //释放模块

}

