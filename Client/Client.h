#pragma once
#include<iostream>
#include<Windows.h>
#include<tchar.h>
using namespace std;
typedef void (WINAPIV* LPFN_CLIENTRUN)(char* ServerAddress, USHORT ConnectPort);
struct _CONNECT_INFORMATION_
{
	DWORD    CheckFlag;           //��㶨������
	char     SeverAddress[20];    //IPv4 �ĸ�����
	USHORT   ConnectPort;         //����Ƽ
}__ConnectInfo = { 0x87654321,"127.0.0.1",2356 };    //echo  ����      ����˿�
