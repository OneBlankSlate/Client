#pragma once
#include<iostream>
#include<tchar.h>
#include<Windows.h>
using namespace std;


#define PAGE_READ_FLAGS \
    (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)
#define PAGE_WRITE_FLAGS \
    (PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)
BOOL ZhIsValidReadPoint(LPVOID VirtualAddress);//0x1000
BOOL ZhIsValidWritePoint(LPVOID VirtualAddress);