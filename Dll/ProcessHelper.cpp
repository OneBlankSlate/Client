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


//�жϽ���λ��
BOOL ZhIsWow64Process(HANDLE ProcessHandle, BOOL* IsWow64Process)
{
	//���ntdllģ��ĺ���
	HMODULE	Kernel32ModuleBase = NULL;
	BOOL IsOk = FALSE;

	Kernel32ModuleBase = LoadLibrary(_T("kernel32.dll"));
	if (Kernel32ModuleBase == NULL)
	{
		return FALSE;
	}
	typedef BOOL(__stdcall* LPFN_ISWOW64PROCESS)(HANDLE ProcessHandle, BOOL* IsWow64Process);
	LPFN_ISWOW64PROCESS  v1 = NULL;
	v1 = (LPFN_ISWOW64PROCESS)GetProcAddress(Kernel32ModuleBase, "IsWow64Process");   //��������ĺ��������ǵ��ִ洢���ʲ�����˫��

	if (v1 == NULL)
	{
		goto Exit;
	}
	//�ػ�
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
	PROCESSENTRY32  ProcessEntry32;   //�ٷ�
	PROCESS_INFORMATION_ITEM    v2 = { 0 }; //�Զ���
	TCHAR  ProcessFullPath[MAX_PATH] = { 0 };
	HMODULE ModuleHandle = NULL;
	DWORD ReturnLength = 0;
	ProcessEntry32.dwSize = sizeof(PROCESSENTRY32);

	//���վ��
	SnapshotHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (SnapshotHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	//�õ���һ������˳���ж�һ��ϵͳ�����Ƿ�ɹ�
	if (Process32First(SnapshotHandle, &ProcessEntry32))
	{
		do
		{
			//�򿪽��̲����ؾ��  //4 system
			ProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
				FALSE, ProcessEntry32.th32ProcessID);   //��Ŀ�����  
			//	
			if (ProcessHandle == NULL)// Ȩ��̫�� - ���ʹ�
			{
				ProcessHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,
					FALSE, ProcessEntry32.th32ProcessID);   //��Ŀ�����

				if (ProcessHandle == NULL)
				{
					memcpy(ProcessFullPath, _T("�򿪽���ʧ��"), _tcslen(_T("�򿪽���ʧ��")));

					memcpy(IsWow64Process, _T("�޷��ж�"), _tcslen(_T("�޷��ж�")));
					goto Label;

				}

			}
			//�ж�Ŀ����̵�λ��

			if (ZhIsWow64Process(ProcessHandle, &v1) == TRUE)
			{
				if (v1)
				{
					memcpy(IsWow64Process, _T("32λ"), _tcslen(_T("32λ")));
				}
				else
				{
					memcpy(IsWow64Process, _T("64λ"), _tcslen(_T("64λ")));
				}
			}
			else
			{
				memcpy(IsWow64Process, _T("�޷��ж�"), _tcslen(_T("�޷��ж�")));
			}

			//ͨ�����̾����õ�һ��ģ������Ϣ

			//��һ����չ��    #include <Psapi.h>     #pragma comment(lib,"Psapi.lib")
			ReturnLength = GetModuleFileNameEx(ProcessHandle, ModuleHandle,   //�������·������1
				ProcessFullPath,
				sizeof(ProcessFullPath));

			if (ReturnLength == 0)
			{
				//���ʧ��
				RtlZeroMemory(ProcessFullPath, MAX_PATH);

				QueryFullProcessImageName(ProcessHandle, 0, ProcessFullPath, &ReturnLength);	// ���Ƽ�ʹ���������  //�������·������2
				if (ReturnLength == 0)
				{
					memcpy(ProcessFullPath, _T("ö����Ϣʧ��"), _tcslen(_T("ö����Ϣʧ��")));
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
	ZhEnableSeDebugPrivilege(GetCurrentProcess(), TRUE, SE_DEBUG_NAME);;  //��Ȩ

	for (int i = 0; i < BufferLength; i += sizeof(HANDLE))
		//��Ϊ�����Ŀ��ܸ���ֹ��һ������
	{
		//�򿪽���
		ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, *(LPDWORD)(BufferData + i));
		//��������
		TerminateProcess(ProcessHandle, 0);
		CloseHandle(ProcessHandle);
	}
	ZhEnableSeDebugPrivilege(GetCurrentProcess(), FALSE, SE_DEBUG_NAME);;  //��ԭ��Ȩ
	// ����Sleep�£���ֹ����
	Sleep(100);

}
void ZhCreateProcess1(LPBYTE BufferData, UINT BufferLength)
{
	//�ӽ��̵�����
	TCHAR CommandLine[] = _T("notepad.exe");  //��������·��   
	STARTUPINFO StartupInfo = { 0 };
	PROCESS_INFORMATION ProcessInfo;
	BOOL IsOk = FALSE;
	//��սṹ����ڴ�
	ZeroMemory(&StartupInfo, sizeof(STARTUPINFO));
	StartupInfo.cb = sizeof(STARTUPINFO);
	ZeroMemory(&ProcessInfo, sizeof(PROCESS_INFORMATION));
	//�����ӽ���
	IsOk = CreateProcess(NULL, CommandLine, NULL, NULL, FALSE,
		CREATE_NEW_CONSOLE, NULL, NULL, &StartupInfo, &ProcessInfo);
	DWORD LastError = GetLastError();
	if (!IsOk)
	{
		_tprintf(_T("CreateProcess failed\n"));
		return;
	}
	//�ȴ�ֱ���ӽ����˳�
	//WaitForSingleObject(ProcessInfo.hProcess, INFINITE);

	// �رս��̺��߳̾��
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
	
	HINSTANCE  ReturnValue = ShellExecute(NULL, _T("open"), BufferData, NULL, NULL, SW_NORMAL);//��exe
	if (ReturnValue < (HINSTANCE)32)//����Ƿ�ָ���ɹ�
		MessageBox(NULL, _T("ERROR"), NULL, MB_OK);

	
	//ShellExecute������������EXE�ļ���Ҳ���������Ѿ��������ļ���������ҳ  �ļ��� pdf�ĵ��ȵ�

}

BOOL ZhOpenProcessByProcessID(HANDLE ProcessIdentity, HANDLE* ProcessHandle)
{
	if (ZhIsValidWritePoint(ProcessHandle) == FALSE)
	{
		return FALSE;
	}
	//��Ȩ
	if (ZhEnableSeDebugPrivilege(_T("SeDebugPrivilege"), TRUE) == FALSE)
	{
		return FALSE;
	}

	//��Ŀ����̻��Ŀ����̾��
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
	// ��Ȩ������

	HANDLE  ProcessHandle = GetCurrentProcess();   //��õ�ǰ�Լ��Ľ��̾��
	HANDLE  TokenHandle = NULL;

	//ͨ�����̾����ý������ƾ��
	if (!OpenProcessToken(ProcessHandle, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &TokenHandle))  //
	{
		return FALSE;
	}
	LUID			 v1;
	if (!LookupPrivilegeValue(NULL, PriviledgeName, &v1))		// ͨ��Ȩ�����Ʋ���uID
	{
		CloseHandle(TokenHandle);
		TokenHandle = NULL;
		return FALSE;
	}
	TOKEN_PRIVILEGES TokenPrivileges = { 0 };
	TokenPrivileges.PrivilegeCount = 1;		// Ҫ������Ȩ�޸���
	TokenPrivileges.Privileges[0].Attributes = IsEnable == TRUE ? SE_PRIVILEGE_ENABLED : 0;    // ��̬���飬�����С����Count����Ŀ
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