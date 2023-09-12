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

//Ϊ���������д�д������
BOOL _CArray::WriteBuffer(PBYTE BufferData, ULONG BufferLength)
{
	EnterCriticalSection(&m_CriticalSection);

	//�ж�д����Ƿ񳬹��������ڴ��С ���������������ڴ�ʧ�ܻ᷵��-1
	if (ReAllocateBuffer(BufferLength + GetBufferLength()) == (ULONG)-1)
	{
		LeaveCriticalSection(&m_CriticalSection);
		return FALSE;
	}

	//Ϊ���������д�д������
	CopyMemory(m_BufferPtr, BufferData, BufferLength);

	m_BufferPtr += BufferLength;
	LeaveCriticalSection(&m_CriticalSection);
	return TRUE;
}

//�������뻺������С ��ԭ���������ݿ������ͷ�ԭ�������ڴ�
ULONG _CArray::ReAllocateBuffer(ULONG BufferLength)
{
	if (BufferLength < GetBufferMaxLength())
		return 0;
	//ceil��������ȡ��   11/3.0 *3
	ULONG  NewBufferMaxLength = (ULONG)ceil(BufferLength / F_PAGE_ALIGNMENT) * U_PAGE_ALIGNMENT;
	
	//
	PBYTE  NewBufferBase = (PBYTE)VirtualAlloc(NULL, NewBufferMaxLength, MEM_COMMIT, PAGE_READWRITE);
	if (NewBufferBase == NULL)
	{
		return -1;
	}

	ULONG OriginalBufferLength = GetBufferLength();   //ԭ�ȵ���Ч���ݳ���
	
	CopyMemory(NewBufferBase, m_BufferBase, OriginalBufferLength);

	if (m_BufferBase)
	{
		//�ͷ�ԭ���ڴ�
		VirtualFree(m_BufferBase, 0, MEM_RELEASE);
	}
	m_BufferBase = NewBufferBase;
	m_BufferPtr = m_BufferBase + OriginalBufferLength; //������ָ��ָ�򻺳���δʹ�ô����׵�ַ

	m_MaxLength = NewBufferMaxLength;//�»������ڴ��С

	return m_MaxLength;
}

//��������������ڴ��С
ULONG _CArray::GetBufferMaxLength()
{
	return m_MaxLength;
}

//�����Ч���ݳ���
ULONG _CArray::GetBufferLength()
{
	if (m_BufferBase == NULL)
		return 0;
	return (ULONG)m_BufferPtr - (ULONG)m_BufferBase;
}

//��û�����ָ����ַ
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

//���ٻ������ڴ� ֻ����1024�ڴ�
VOID _CArray::ClearBuffer()
{
	EnterCriticalSection(&m_CriticalSection);
	m_BufferPtr = m_BufferBase;

	DeAllocateBuffer(1024);
	LeaveCriticalSection(&m_CriticalSection);
}

//ClearBuffer�е���DeAllocateBuffer(1024); ֻ����1024�����ڴ�ȫ������
ULONG _CArray::DeAllocateBuffer(ULONG BufferLength)
{
	//���BufferLengthС�ڵ�ǰ��������Ч����
	if (BufferLength < GetBufferLength())
		return 0;
	
	//����ȡ��
	ULONG NewBufferLength = (ULONG)ceil(BufferLength / F_PAGE_ALIGNMENT) * U_PAGE_ALIGNMENT;

	//
	if (GetBufferMaxLength() <= NewBufferLength)
	{
		return 0;
	}
	//�����뻺�����ڴ�
	PBYTE NewBufferBase = (PBYTE)VirtualAlloc(NULL, NewBufferLength, MEM_COMMIT, PAGE_READWRITE);

	ULONG OriginalBufferLength = GetBufferLength();  //��ԭ���ڴ����Ч����
	
	CopyMemory(NewBufferBase, m_BufferBase, OriginalBufferLength);

	//�ͷ�ԭ�Ȼ������ڴ�
	VirtualFree(m_BufferBase, 0, MEM_RELEASE);


	m_BufferBase = NewBufferBase;

	m_BufferPtr = m_BufferBase + OriginalBufferLength;

	m_MaxLength = NewBufferLength;

	return m_MaxLength;
}

//��ȡ�ڴ��е�����
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
		//�����ݿ�����BufferData
		CopyMemory(BufferData, m_BufferBase, BufferLength);

		//������ǰ�� ���ǵ���ȡ�˵�����
		MoveMemory(m_BufferBase, m_BufferBase + BufferLength, GetBufferMaxLength() - BufferLength);
		m_BufferPtr -= BufferLength;
	}

	//����һ���ڴ��С ʹ�ڴ�Ϊ��Ч�ڴ�
	DeAllocateBuffer(GetBufferLength());

	LeaveCriticalSection(&m_CriticalSection);
	return BufferLength;
}


//ǰ������
ULONG _CArray::RemoveComletedBuffer(ULONG BufferLength)
{

	if (BufferLength > GetBufferMaxLength())   //��������ĳ��ȱ��ڴ�ĳ��Ȼ���
	{
		return 0;
	}
	if (BufferLength > GetBufferLength())  //��������ĳ��� ����Ч�����ݳ��Ȼ���
	{
		BufferLength = GetBufferLength();
	}
	if (BufferLength)
	{
		//����ǰ��
		MoveMemory(m_BufferBase, m_BufferBase + BufferLength, GetBufferMaxLength() - BufferLength);   
		m_BufferPtr -= BufferLength;
	}

	DeAllocateBuffer(GetBufferLength());

	return BufferLength;
}