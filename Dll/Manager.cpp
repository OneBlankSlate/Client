#include "pch.h"
#include "Manager.h"


CManager::CManager(CIocpClient* IocpClient)
{
	m_IocpClient = IocpClient;


	//将实例类对象传入到通信类中
	IocpClient->SetManagerObject(this);   //this是抽象类CManager的子类对象




	m_EventOpenDialogHandle = CreateEvent(NULL, TRUE, FALSE, NULL);   //等待服务器执行的事件
}


CManager::~CManager()
{
	CloseHandle(m_EventOpenDialogHandle);
}

VOID CManager::WaitingForDialogOpen()
{
	WaitForSingleObject(m_EventOpenDialogHandle, INFINITE);
	//必须的Sleep,因为远程窗口从InitDialog中发送COMMAND_NEXT到显示还要一段时间
	Sleep(150);
}
