#pragma once
#include<iostream>
#include<tchar.h>
#include<Windows.h>
using namespace std;
#include <vector>
#include "shellapi.h"
#include <TlHelp32.h>
#include <Psapi.h>
#pragma comment(lib,"Psapi.lib")

#define PAGE_SIZE 0x1000

#ifdef _WIN64
#define  RING3_LIMITED 0x00007FFFFFFEFFFF   
#else
#define  RING3_LIMITED 0x7FFEFFFF
#endif // _WIN64

#pragma pack(1)
typedef struct PROCESS_INFORMATION_ITEM
{
	HANDLE  ProcessIdentity;
	TCHAR   ProcessImageName[MAX_PATH];
	TCHAR   ProcessFullPath[MAX_PATH];
	TCHAR   IsWow64Process[20];
}PROCESS_INFORMATION_ITEM, *PPROCESS_INFORMATION_ITEM;


#define PAGE_READ_FLAGS \
    (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)
#define PAGE_WRITE_FLAGS \
    (PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)
int ZhEnableSeDebugPrivilege(HANDLE ProcessHandle, BOOL IsEnable, LPCTSTR RequireLevel);
BOOL ZhIsWow64Process(HANDLE ProcessHandle, BOOL* IsWow64Process);
BOOL ZhEnumProcessByToolHelp32(vector<PROCESS_INFORMATION_ITEM>& ProcessInfo);
VOID ZhKillProcess(LPBYTE BufferData, UINT BufferLength);
void ZhCreateProcess6(char* BufferData);
BOOL ZhGetProcessIDByProcessImageName(TCHAR* ProcessImageName, HANDLE* ProcessIdentity);
BOOL ZhOpenProcessByProcessID(HANDLE ProcessIdentity, HANDLE* ProcessHandle);

BOOL ZhEnableSeDebugPrivilege(IN const TCHAR* PriviledgeName, BOOL IsEnable);


