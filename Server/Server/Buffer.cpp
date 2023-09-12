#include"stdafx.h"
#include "Buffer.h"


#define U_PAGE_ALIGNMENT   3
#define F_PAGE_ALIGNMENT 3.0

_CArray::_CArray()
{
	m_MaxLength = 0;

	m_BufferPtr = m_BufferBase = NULL;

	InitializeCriticalSection(&m_CriticalSection);
}


_CArray::~_CArray()
{
	if (m_BufferBase)
	{
		VirtualFree(m_BufferBase, 0, MEM_RELEASE);
		m_BufferBase = NULL;
	}

	DeleteCriticalSection(&m_CriticalSection);

	m_BufferBase = m_BufferPtr = NULL;
	m_MaxLength = 0;
}

//为缓冲区空闲处写入数据
BOOL _CArray::WriteBuffer(PBYTE BufferData, ULONG BufferLength)
{
	EnterCriticalSection(&m_CriticalSection);

	//判断写入后是否超过缓冲区内存大小 超过并重新申请内存失败会返回-1
	if (ReAllocateBuffer(BufferLength + GetBufferLength()) == (ULONG)-1)
	{
		LeaveCriticalSection(&m_CriticalSection);
		return FALSE;
	}

	//为缓冲区空闲处写入数据
	CopyMemory(m_BufferPtr, BufferData, BufferLength);

	m_BufferPtr += BufferLength;
	LeaveCriticalSection(&m_CriticalSection);
	return TRUE;
}

//重新申请缓冲区大小 将原缓冲区数据拷贝并释放原缓冲区内存
ULONG _CArray::ReAllocateBuffer(ULONG BufferLength)
{
	if (BufferLength < GetBufferMaxLength())
		return 0;
	//ceil函数向上取整   11/3.0 *3
	ULONG  NewBufferMaxLength = (ULONG)ceil(BufferLength / F_PAGE_ALIGNMENT) * U_PAGE_ALIGNMENT;
	
	//
	PBYTE  NewBufferBase = (PBYTE)VirtualAlloc(NULL, NewBufferMaxLength, MEM_COMMIT, PAGE_READWRITE);
	if (NewBufferBase == NULL)
	{
		return -1;
	}

	ULONG OriginalBufferLength = GetBufferLength();   //原先的有效数据长度
	
	CopyMemory(NewBufferBase, m_BufferBase, OriginalBufferLength);

	if (m_BufferBase)
	{
		//释放原先内存
		VirtualFree(m_BufferBase, 0, MEM_RELEASE);
	}
	m_BufferBase = NewBufferBase;
	m_BufferPtr = m_BufferBase + OriginalBufferLength; //将游走指针指向缓冲区未使用处的首地址

	m_MaxLength = NewBufferMaxLength;//新缓冲区内存大小

	return m_MaxLength;
}

//获得整个缓冲区内存大小
ULONG _CArray::GetBufferMaxLength()
{
	return m_MaxLength;
}

//获得有效数据长度
ULONG _CArray::GetBufferLength()
{
	if (m_BufferBase == NULL)
		return 0;
	return (ULONG)m_BufferPtr - (ULONG)m_BufferBase;
}

//获得缓冲区指定地址
PBYTE _CArray::GetBuffer(ULONG Pos)
{

	if (m_BufferBase == NULL)
	{
		return NULL;
	}
	if (Pos >= GetBufferLength())
	{
		return NULL;
	}
	return m_BufferBase + Pos;
}

//销毁缓冲区内存 只保留1024内存
VOID _CArray::ClearBuffer()
{
	EnterCriticalSection(&m_CriticalSection);
	m_BufferPtr = m_BufferBase;

	DeAllocateBuffer(1024);
	LeaveCriticalSection(&m_CriticalSection);
}

//ClearBuffer中调用DeAllocateBuffer(1024); 只保留1024其他内存全部销毁
ULONG _CArray::DeAllocateBuffer(ULONG BufferLength)
{
	//如果BufferLength小于当前缓冲区有效长度
	if (BufferLength < GetBufferLength())
		return 0;
	
	//向上取整
	ULONG NewBufferLength = (ULONG)ceil(BufferLength / F_PAGE_ALIGNMENT) * U_PAGE_ALIGNMENT;

	//
	if (GetBufferMaxLength() <= NewBufferLength)
	{
		return 0;
	}
	//新申请缓冲区内存
	PBYTE NewBufferBase = (PBYTE)VirtualAlloc(NULL, NewBufferLength, MEM_COMMIT, PAGE_READWRITE);

	ULONG OriginalBufferLength = GetBufferLength();  //算原先内存的有效长度
	
	CopyMemory(NewBufferBase, m_BufferBase, OriginalBufferLength);

	//释放原先缓冲区内存
	VirtualFree(m_BufferBase, 0, MEM_RELEASE);


	m_BufferBase = NewBufferBase;

	m_BufferPtr = m_BufferBase + OriginalBufferLength;

	m_MaxLength = NewBufferLength;

	return m_MaxLength;
}

//读取内存中的数据
ULONG _CArray::ReadBuffer(PBYTE BufferData, ULONG BufferLength)
{
	EnterCriticalSection(&m_CriticalSection);

	if (BufferLength > GetBufferMaxLength())
	{
		LeaveCriticalSection(&m_CriticalSection);
		return 0;
	}
	if (BufferLength > GetBufferLength())
	{
		BufferLength = GetBufferLength();
	}

	if (BufferLength)
	{
		//将数据拷贝到BufferData
		CopyMemory(BufferData, m_BufferBase, BufferLength);

		//将数据前移 覆盖掉读取了得数据
		MoveMemory(m_BufferBase, m_BufferBase + BufferLength, GetBufferMaxLength() - BufferLength);
		m_BufferPtr -= BufferLength;
	}

	//更新一下内存大小 使内存为有效内存
	DeAllocateBuffer(GetBufferLength());

	LeaveCriticalSection(&m_CriticalSection);
	return BufferLength;
}


//前移数据
ULONG _CArray::RemoveComletedBuffer(ULONG BufferLength)
{

	if (BufferLength > GetBufferMaxLength())   //如果传进的长度比内存的长度还大
	{
		return 0;
	}
	if (BufferLength > GetBufferLength())  //如果传进的长度 比有效的数据长度还大
	{
		BufferLength = GetBufferLength();
	}
	if (BufferLength)
	{
		//数据前移
		MoveMemory(m_BufferBase, m_BufferBase + BufferLength, GetBufferMaxLength() - BufferLength);   
		m_BufferPtr -= BufferLength;
	}

	DeAllocateBuffer(GetBufferLength());

	return BufferLength;
}