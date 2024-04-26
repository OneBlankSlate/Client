#include "pch.h"
#include"ProcessHelper.h"
#include"MemoryHelper.h"
int ZhEnableSeDebugPrivilege(HANDLE ProcessHandle, BOOL IsEnable, LPCTSTR RequireLevel)
{
	DWORD  LastError;
	HANDLE TokenHandle = 0;

	if (!OpenProcessToken(ProcessHandle, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &TokenHandle))
	{
		LastError = GetLastError();
		if (TokenHandle)
			CloseHandle(TokenHandle);
		return LastError;
	}
	TOKEN_PRIVILEGES TokenPrivileges;
	memset(&TokenPrivileges, 0, sizeof(TOKEN_PRIVILEGES));
	LUID v1;
	if (!LookupPrivilegeValue(NULL, RequireLevel, &v1))
	{
		LastError = GetLastError();
		CloseHandle(TokenHandle);
		return LastError;
	}
	TokenPrivileges.PrivilegeCount = 1;
	TokenPrivileges.Privileges[0].Luid = v1;
	if (IsEnable)
		TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		TokenPrivileges.Privileges[0].Attributes = 0;
	AdjustTokenPrivileges(TokenHandle, FALSE, &TokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	LastError = GetLastError();
	CloseHandle(TokenHandle);
	return LastError;
}


//判断进程位数
BOOL ZhIsWow64Process(HANDLE ProcessHandle, BOOL* IsWow64Process)
{
	//获得ntdll模块的函数
	HMODULE	Kernel32ModuleBase = NULL;
	BOOL IsOk = FALSE;

	Kernel32ModuleBase = LoadLibrary(_T("kernel32.dll"));
	if (Kernel32ModuleBase == NULL)
	{
		return FALSE;
	}
	typedef BOOL(__stdcall* LPFN_ISWOW64PROCESS)(HANDLE ProcessHandle, BOOL* IsWow64Process);
	LPFN_ISWOW64PROCESS  v1 = NULL;
	v1 = (LPFN_ISWOW64PROCESS)GetProcAddress(Kernel32ModuleBase, "IsWow64Process");   //导出表里的函数名都是单字存储，故不可用双字

	if (v1 == NULL)
	{
		goto Exit;
	}
	//关机
	v1(ProcessHandle, IsWow64Process);
Exit:
	if (Kernel32ModuleBase != NULL)
	{
		FreeLibrary(Kernel32ModuleBase);
		Kernel32ModuleBase = NULL;
	}

	return IsOk;
}

BOOL ZhEnumProcessByToolHelp32(vector<PROCESS_INFORMATION_ITEM>& ProcessInfo)
{

	BOOL v1 = FALSE;
	HANDLE   SnapshotHandle = NULL;
	HANDLE   ProcessHandle = NULL;
	char     IsWow64Process[20] = { 0 };
	PROCESSENTRY32  ProcessEntry32;   //官方
	PROCESS_INFORMATION_ITEM    v2 = { 0 }; //自定义
	TCHAR  ProcessFullPath[MAX_PATH] = { 0 };
	HMODULE ModuleHandle = NULL;
	DWORD ReturnLength = 0;
	ProcessEntry32.dwSize = sizeof(PROCESSENTRY32);

	//快照句柄
	SnapshotHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (SnapshotHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	//得到第一个进程顺便判断一下系统快照是否成功
	if (Process32First(SnapshotHandle, &ProcessEntry32))
	{
		do
		{
			//打开进程并返回句柄  //4 system
			ProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
				FALSE, ProcessEntry32.th32ProcessID);   //打开目标进程  
			//	
			if (ProcessHandle == NULL)// 权限太高 - 降低打开
			{
				ProcessHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,
					FALSE, ProcessEntry32.th32ProcessID);   //打开目标进程

				if (ProcessHandle == NULL)
				{
					memcpy(ProcessFullPath, _T("打开进程失败"), _tcslen(_T("打开进程失败")));

					memcpy(IsWow64Process, _T("无法判断"), _tcslen(_T("无法判断")));
					goto Label;

				}

			}
			//判断目标进程的位数

			if (ZhIsWow64Process(ProcessHandle, &v1) == TRUE)
			{
				if (v1)
				{
					memcpy(IsWow64Process, _T("32位"), _tcslen(_T("32位")));
				}
				else
				{
					memcpy(IsWow64Process, _T("64位"), _tcslen(_T("64位")));
				}
			}
			else
			{
				memcpy(IsWow64Process, _T("无法判断"), _tcslen(_T("无法判断")));
			}

			//通过进程句柄获得第一个模块句柄信息

			//加一个扩展库    #include <Psapi.h>     #pragma comment(lib,"Psapi.lib")
			ReturnLength = GetModuleFileNameEx(ProcessHandle, ModuleHandle,   //获得完整路径方法1
				ProcessFullPath,
				sizeof(ProcessFullPath));

			if (ReturnLength == 0)
			{
				//如果失败
				RtlZeroMemory(ProcessFullPath, MAX_PATH);

				QueryFullProcessImageName(ProcessHandle, 0, ProcessFullPath, &ReturnLength);	// 更推荐使用这个函数  //获得完整路径方法2
				if (ReturnLength == 0)
				{
					memcpy(ProcessFullPath, _T("枚举信息失败"), _tcslen(_T("枚举信息失败")));
				}
			}
		Label:
			ZeroMemory(&v2, sizeof(v2));

			v2.ProcessIdentity = (HANDLE)ProcessEntry32.th32ProcessID;
			memcpy(v2.ProcessImageName, ProcessEntry32.szExeFile, (_tcslen(ProcessEntry32.szExeFile) + 1) * sizeof(TCHAR));
			memcpy(v2.ProcessFullPath, ProcessFullPath, (_tcslen(ProcessFullPath) + 1) * sizeof(TCHAR));
			memcpy(v2.IsWow64Process, IsWow64Process, (_tcslen(IsWow64Process) + 1) * sizeof(TCHAR));
			ProcessInfo.push_back(v2);

			if (ProcessHandle != NULL)
			{
				CloseHandle(ProcessHandle);
				ProcessHandle = NULL;
			}

		} while (Process32Next(SnapshotHandle, &ProcessEntry32));
	}
	else
	{
		CloseHandle(SnapshotHandle);

		return FALSE;
	}

	CloseHandle(SnapshotHandle);

	return ProcessInfo.size() > 0 ? TRUE : FALSE;
}

VOID ZhKillProcess(LPBYTE BufferData, UINT BufferLength)
{
	HANDLE ProcessHandle = NULL;
	ZhEnableSeDebugPrivilege(GetCurrentProcess(), TRUE, SE_DEBUG_NAME);;  //提权

	for (int i = 0; i < BufferLength; i += sizeof(HANDLE))
		//因为结束的可能个不止是一个进程
	{
		//打开进程
		ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, *(LPDWORD)(BufferData + i));
		//结束进程
		TerminateProcess(ProcessHandle, 0);
		CloseHandle(ProcessHandle);
	}
	ZhEnableSeDebugPrivilege(GetCurrentProcess(), FALSE, SE_DEBUG_NAME);;  //还原提权
	// 稍稍Sleep下，防止出错
	Sleep(100);

}
void ZhCreateProcess1(LPBYTE BufferData, UINT BufferLength)
{
	//子进程的名称
	TCHAR CommandLine[] = _T("notepad.exe");  //进程完整路径   
	STARTUPINFO StartupInfo = { 0 };
	PROCESS_INFORMATION ProcessInfo;
	BOOL IsOk = FALSE;
	//清空结构体的内存
	ZeroMemory(&StartupInfo, sizeof(STARTUPINFO));
	StartupInfo.cb = sizeof(STARTUPINFO);
	ZeroMemory(&ProcessInfo, sizeof(PROCESS_INFORMATION));
	//创建子进程
	IsOk = CreateProcess(NULL, CommandLine, NULL, NULL, FALSE,
		CREATE_NEW_CONSOLE, NULL, NULL, &StartupInfo, &ProcessInfo);
	DWORD LastError = GetLastError();
	if (!IsOk)
	{
		_tprintf(_T("CreateProcess failed\n"));
		return;
	}
	//等待直到子进程退出
	//WaitForSingleObject(ProcessInfo.hProcess, INFINITE);

	// 关闭进程和线程句柄
	CloseHandle(ProcessInfo.hProcess);
	CloseHandle(ProcessInfo.hThread);
}
//void ZhCreateProcess5(LPBYTE BufferData, UINT BufferLength)
//{
//
//}
//
void ZhCreateProcess6(char* BufferData)
{
	
	HINSTANCE  ReturnValue = ShellExecute(NULL, _T("open"), BufferData, NULL, NULL, SW_NORMAL);//打开exe
	if (ReturnValue < (HINSTANCE)32)//检测是否指定成功
		MessageBox(NULL, _T("ERROR"), NULL, MB_OK);

	
	//ShellExecute不仅可以运行EXE文件，也可以运行已经关联的文件。比如网页  文件夹 pdf文档等等

}

BOOL ZhOpenProcessByProcessID(HANDLE ProcessIdentity, HANDLE* ProcessHandle)
{
	if (ZhIsValidWritePoint(ProcessHandle) == FALSE)
	{
		return FALSE;
	}
	//提权
	if (ZhEnableSeDebugPrivilege(_T("SeDebugPrivilege"), TRUE) == FALSE)
	{
		return FALSE;
	}

	//打开目标进程获得目标进程句柄
	*ProcessHandle = OpenProcess(GENERIC_ALL, FALSE, (DWORD)ProcessIdentity);

	if (*ProcessHandle != INVALID_HANDLE_VALUE)
	{
		ZhEnableSeDebugPrivilege(_T("SeDebugPrivilege"), FALSE);
		return TRUE;
	}
	ZhEnableSeDebugPrivilege(_T("SeDebugPrivilege"), FALSE);
	return FALSE;
}
BOOL ZhEnableSeDebugPrivilege(IN const TCHAR* PriviledgeName, BOOL IsEnable)
{
	// 打开权限令牌

	HANDLE  ProcessHandle = GetCurrentProcess();   //获得当前自己的进程句柄
	HANDLE  TokenHandle = NULL;

	//通过进程句柄获得进程令牌句柄
	if (!OpenProcessToken(ProcessHandle, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &TokenHandle))  //
	{
		return FALSE;
	}
	LUID			 v1;
	if (!LookupPrivilegeValue(NULL, PriviledgeName, &v1))		// 通过权限名称查找uID
	{
		CloseHandle(TokenHandle);
		TokenHandle = NULL;
		return FALSE;
	}
	TOKEN_PRIVILEGES TokenPrivileges = { 0 };
	TokenPrivileges.PrivilegeCount = 1;		// 要提升的权限个数
	TokenPrivileges.Privileges[0].Attributes = IsEnable == TRUE ? SE_PRIVILEGE_ENABLED : 0;    // 动态数组，数组大小根据Count的数目
	TokenPrivileges.Privileges[0].Luid = v1;
	if (!AdjustTokenPrivileges(TokenHandle, FALSE, &TokenPrivileges,
		sizeof(TOKEN_PRIVILEGES), NULL, NULL))
	{

		CloseHandle(TokenHandle);
		TokenHandle = NULL;
		return FALSE;
	}
	CloseHandle(TokenHandle);
	TokenHandle = NULL;
	return TRUE;
}