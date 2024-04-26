#pragma once

#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include<tchar.h>
#include"Array1.h"
#include"zconf.h"
#include"zlib.h"
#include"Manager.h"
//属性 链接器 输入 附加依赖项
#pragma comment(lib, "zlib.lib")
#define PACKET_LENGTH 0x2000
#define PACKET_HEADER_LENGTH 13        //Shine[数据包总长(4)][原始数据总长(4)]
using namespace std;
#define PACKET_LENGTH 0x2000
#define PACKET_FLAG_LENGTH 5     //Shine
#define MAX_SEND_BUFFER 0x2000
#pragma comment(lib, "WS2_32.lib")
class CIocpClient
{
public:
	CIocpClient();
	~CIocpClient();
	BOOL ConnectServer(char* ServerAddress, unsigned short ConnectPort);
	static DWORD WINAPI WorkThreadProcedure(LPVOID ParameterData);
	BOOL IsReceiving()
	{
		return m_IsReceiving;
	}

	VOID SetManagerObject(class CManager* Manager)
	{
		//面向对象(多态(抽象类指针指向实例对象地址)) 
		m_Manager = Manager;
	}
	VOID WaitingForEvent()
	{
		//避免对象销毁
		WaitForSingleObject(m_EventHandle, INFINITE);
	}
	VOID Disconnect();
	VOID OnReceiving(char* BufferData, ULONG BufferLength);
	int OnSending(char* BufferData, ULONG BufferLength);
	BOOL SendWithSplit(char* BufferData, ULONG BufferLength, ULONG SplitLength);
public:
	SOCKET m_ClientSocket;
	HANDLE m_WorkThreadHandle;
	HANDLE m_EventHandle;
private:
	BOOL m_IsReceiving=TRUE;
	CArray1 m_SendCompressedBufferData;
	char m_PacketHeaderFlag[PACKET_FLAG_LENGTH];      //数据包匹配
	CArray1 m_ReceivedCompressedBufferData;
	CArray1 m_ReceivedDecompressedBufferData;

	//多态性
	CManager* m_Manager;
};

