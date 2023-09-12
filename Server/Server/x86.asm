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
	lodsd					; 把DestinationData的第一个双字节，放到eax中,就是DIB中改变区域的偏移
	add edi, eax			; SourceData偏移eax	
	lodsd					; 把DestinationData的下一个双字节，放到eax中, 就是改变区域的大小
	mov ecx, eax
	sub ebx, 8				; ebx 减去 两个dword
	sub ebx, ecx			; ebx 减去DIB数据的大小
	rep movsb
CopyEnd:
	cmp ebx, 0				; 是否写入完 
	jnz CopyNextBlock

	add esp, 20h
	pop ebp
	ret
CopyScreenData ENDP

END
