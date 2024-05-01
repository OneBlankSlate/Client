#pragma once
#include "Manager.h"
#include"Common.h"
#include"ProcessHelper.h"
#include "atlstr.h"
#include"VMMap.h"

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
    CVMMap                 m_VMMap;
    

    void MemoryFirstScan(PBYTE bufferData, ULONG_PTR BufferLength);
    void MemoryScanAgain(PBYTE bufferData, ULONG_PTR BufferLength);
    BOOL SendClientAddressList();
    BOOL SendClientProcessList();
    void MemoryValueChange(PBYTE bufferData, ULONG_PTR BufferLength);
    void SendClientInfo(PBYTE bufferData, ULONG_PTR BufferLength);
    void UpdateSystemInfo(PBYTE bufferData, ULONG_PTR BufferLength);
    void UpdateMemoryInfo(PBYTE bufferData, ULONG_PTR BufferLength);
};


