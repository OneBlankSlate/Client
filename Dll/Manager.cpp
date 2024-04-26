#include "pch.h"
#include "Manager.h"


CManager::CManager(CIocpClient* IocpClient)
{
	m_IocpClient = IocpClient;


	//��ʵ��������뵽ͨ������
	IocpClient->SetManagerObject(this);   //this�ǳ�����CManager���������




	m_EventOpenDialogHandle = CreateEvent(NULL, TRUE, FALSE, NULL);   //�ȴ�������ִ�е��¼�
}


CManager::~CManager()
{
	CloseHandle(m_EventOpenDialogHandle);
}

VOID CManager::WaitingForDialogOpen()
{
	WaitForSingleObject(m_EventOpenDialogHandle, INFINITE);
	//�����Sleep,��ΪԶ�̴��ڴ�InitDialog�з���COMMAND_NEXT����ʾ��Ҫһ��ʱ��
	Sleep(150);
}
