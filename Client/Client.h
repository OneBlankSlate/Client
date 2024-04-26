#pragma once
#include<iostream>
#include<Windows.h>
#include<tchar.h>
using namespace std;
typedef void (WINAPIV* LPFN_CLIENTRUN)(char* ServerAddress, USHORT ConnectPort);
struct _CONNECT_INFORMATION_
{
	DWORD    CheckFlag;           //随便定义数字
	char     SeverAddress[20];    //IPv4 四个单字
	USHORT   ConnectPort;         //王艳萍
}__ConnectInfo = { 0x87654321,"127.0.0.1",2356 };    //echo  回声      服务端口
