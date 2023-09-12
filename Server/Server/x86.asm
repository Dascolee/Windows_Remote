.386
.MODEL FLAT, C
.CODE

CopyScreenData PROC      ; SourceData, DestinationData, BufferLength
	push ebp 
	mov  ebp, esp
	sub  esp, 20h

	mov  ebx, dword ptr [ebp + 10h]    ; BufferLength
	mov  esi, dword ptr [ebp + 0Ch]    ;DestinationData
	jmp	CopyEnd
CopyNextBlock:
	mov edi, [ebp + 8h]     ; SourceData
	lodsd					; ��DestinationData�ĵ�һ��˫�ֽڣ��ŵ�eax��,����DIB�иı������ƫ��
	add edi, eax			; SourceDataƫ��eax	
	lodsd					; ��DestinationData����һ��˫�ֽڣ��ŵ�eax��, ���Ǹı�����Ĵ�С
	mov ecx, eax
	sub ebx, 8				; ebx ��ȥ ����dword
	sub ebx, ecx			; ebx ��ȥDIB���ݵĴ�С
	rep movsb
CopyEnd:
	cmp ebx, 0				; �Ƿ�д���� 
	jnz CopyNextBlock

	add esp, 20h
	pop ebp
	ret
CopyScreenData ENDP

END
