#pragma once
#include <iostream>
#include <Windows.h>
#include <tchar.h>
#include "IocpClient.h"
using namespace std;

//抽象类  -- --- --- --- ---

class CIocpClient;
class CManager
{
public:
	CManager(CIocpClient* IocpClient);
	~CManager();
	VOID WaitingForDialogOpen();
	virtual void HandleIo(PBYTE BufferData, ULONG_PTR BufferLength)
	{

	}
	VOID NotifyDialogIsOpen()
	{
		SetEvent(m_EventOpenDialogHandle);
	}
public:
	CIocpClient* m_IocpClient;   //通信类指针
	HANDLE m_EventOpenDialogHandle;
};



