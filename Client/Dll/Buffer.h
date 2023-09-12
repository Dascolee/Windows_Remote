#pragma once
#include<Windows.h>
#include<iostream>
using namespace std;

class _CArray
{
public:
	_CArray();
	~_CArray();
	//向缓冲区写入数据
	BOOL _CArray::WriteBuffer(PBYTE BufferData, ULONG BufferLength);
	//为缓冲区重新申请内存
	ULONG _CArray::ReAllocateBuffer(ULONG BufferLength);
	//获得缓冲区内存最大长度
	ULONG _CArray::GetBufferMaxLength();
	//获得写入缓冲区的长度
	ULONG _CArray::GetBufferLength();
	//获得指定缓冲区地址
	PBYTE _CArray::GetBuffer(ULONG Pos = 0);
	//销毁缓冲区内存 只保留1024内存
	VOID _CArray::ClearBuffer();

	ULONG _CArray::DeAllocateBuffer(ULONG BufferLength);
	//从缓冲区基地址处读出指定长度数据
	ULONG _CArray::ReadBuffer(PBYTE BufferData, ULONG BufferLength);
	ULONG _CArray::RemoveComletedBuffer(ULONG BufferLength);
protected:
	PBYTE	m_BufferBase;//缓冲区基地址
	PBYTE	m_BufferPtr;//缓冲区游走指针
	ULONG	m_MaxLength;//最大长度
	CRITICAL_SECTION  m_CriticalSection;//临界区
};

