#include "pch.h"
#include"SystemHelper.h"
void ShutdownSystem()
{
	//���ntdllģ��ĺ���
	HMODULE NtdllModuleBase = LoadLibrary(_T("Ntdll.DLL"));    //���أ����ü���++
	if (NtdllModuleBase == NULL)
	{
		return;
	}
	typedef int(_stdcall* LPFN_ZWSHUTDOWNSYSTEM)(int);
	LPFN_ZWSHUTDOWNSYSTEM  ZwShutdownSystem = NULL;
	ZwShutdownSystem = (LPFN_ZWSHUTDOWNSYSTEM)GetProcAddress(NtdllModuleBase, "ZwShutdownSystem");  //����������Ϊ���֣��ʲ�Ҫ��_T

	if (ZwShutdownSystem == NULL)
	{
		goto Exit;
	}
	//�ػ�
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
		FreeLibrary(NtdllModuleBase);          //�ͷţ����ü���--��Ϊ0���ͷ�
		NtdllModuleBase = NULL;
	}
}