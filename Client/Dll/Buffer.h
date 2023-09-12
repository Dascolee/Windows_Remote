#pragma once
#include<Windows.h>
#include<iostream>
using namespace std;

class _CArray
{
public:
	_CArray();
	~_CArray();
	//�򻺳���д������
	BOOL _CArray::WriteBuffer(PBYTE BufferData, ULONG BufferLength);
	//Ϊ���������������ڴ�
	ULONG _CArray::ReAllocateBuffer(ULONG BufferLength);
	//��û������ڴ���󳤶�
	ULONG _CArray::GetBufferMaxLength();
	//���д�뻺�����ĳ���
	ULONG _CArray::GetBufferLength();
	//���ָ����������ַ
	PBYTE _CArray::GetBuffer(ULONG Pos = 0);
	//���ٻ������ڴ� ֻ����1024�ڴ�
	VOID _CArray::ClearBuffer();

	ULONG _CArray::DeAllocateBuffer(ULONG BufferLength);
	//�ӻ���������ַ������ָ����������
	ULONG _CArray::ReadBuffer(PBYTE BufferData, ULONG BufferLength);
	ULONG _CArray::RemoveComletedBuffer(ULONG BufferLength);
protected:
	PBYTE	m_BufferBase;//����������ַ
	PBYTE	m_BufferPtr;//����������ָ��
	ULONG	m_MaxLength;//��󳤶�
	CRITICAL_SECTION  m_CriticalSection;//�ٽ���
};

