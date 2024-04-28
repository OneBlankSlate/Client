#pragma once
#include "Manager.h"
#include"Common.h"
#include"ProcessHelper.h"
#include "atlstr.h"

class CProcessManager :
    public CManager
{
public:
    CProcessManager(CIocpClient* IocpClient);
    ~CProcessManager();
    void HandleIo(PBYTE BufferData, ULONG_PTR BufferLength);
    HANDLE                  m_CurrentProcessIdentity;
    std::vector<size_t>*    m_Address;
    int                     m_ElementCount;
    HANDLE                  m_ProcessHandle;
    BYTE                   m_ScanRelpy;
    

    void MemoryFirstScan(PBYTE bufferData, ULONG_PTR BufferLength);
    void MemoryScanAgain(PBYTE bufferData, ULONG_PTR BufferLength);
    BOOL SendClientAddressList();
    BOOL SendClientProcessList();
    void MemoryValueChange(PBYTE bufferData, ULONG_PTR BufferLength);
    void SendClientSystemInfo();
};


