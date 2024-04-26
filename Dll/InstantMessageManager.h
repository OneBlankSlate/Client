#pragma once
#include "Manager.h"
#include "IocpClient.h"
class CInstantMessageManager :
	public CManager
{
public:
	CInstantMessageManager(CIocpClient* IocpClient);
	~CInstantMessageManager();
	void HandleIo(PBYTE BufferData, ULONG_PTR BufferLength);
};
int CALLBACK DialogProcedure(HWND DialogHwnd, unsigned int Message,
	WPARAM ParameterData1, LPARAM ParameterData2);
VOID OnInitDialog(HWND DialogHwnd);
VOID OnTimerDialog(HWND DialogHwnd);