#include "pch.h"
#include"SystemHelper.h"
void ShutdownSystem()
{
	//获得ntdll模块的函数
	HMODULE NtdllModuleBase = LoadLibrary(_T("Ntdll.DLL"));    //加载，引用计数++
	if (NtdllModuleBase == NULL)
	{
		return;
	}
	typedef int(_stdcall* LPFN_ZWSHUTDOWNSYSTEM)(int);
	LPFN_ZWSHUTDOWNSYSTEM  ZwShutdownSystem = NULL;
	ZwShutdownSystem = (LPFN_ZWSHUTDOWNSYSTEM)GetProcAddress(NtdllModuleBase, "ZwShutdownSystem");  //函数导出表为单字，故不要加_T

	if (ZwShutdownSystem == NULL)
	{
		goto Exit;
	}
	//关机
	ZwShutdownSystem(2);     //ShutdownPowerOff

	//typedef enum _SHUTDOWN_ACTION
	//{
	//	ShutdownNoReboot,        //0
	//	ShutdownReboot,          //1
	//	ShutdownPowerOff         //2
	//} SHUTDOWN_ACTION;
Exit:
	if (NtdllModuleBase != NULL)
	{
		FreeLibrary(NtdllModuleBase);          //释放，引用计数--，为0则释放
		NtdllModuleBase = NULL;
	}
}