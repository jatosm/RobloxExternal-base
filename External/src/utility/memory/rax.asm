.data

g_readSsn DWORD 63
g_writeSsn DWORD 58

.code

Rax_SetSsns PROC
	mov g_readSsn, ecx
	mov g_writeSsn, edx
	ret
Rax_SetSsns ENDP

Rax_ReadVirtualMemory PROC
	mov r10, rcx
	mov eax, g_readSsn
	syscall
	ret
Rax_ReadVirtualMemory ENDP

Rax_WriteVirtualMemory PROC
	mov r10, rcx
	mov eax, g_writeSsn
	syscall
	ret
Rax_WriteVirtualMemory ENDP

Rax_MemCpy PROC
	mov rax, rcx
	push rdi
	push rsi
	mov rdi, rcx
	mov rsi, rdx
	mov rcx, r8
	rep movsb
	pop rsi
	pop rdi
	ret
Rax_MemCpy ENDP

Rax_MemSet PROC
	mov r10, rcx
	push rdi
	mov rdi, rcx
	mov rax, rdx
	mov rcx, r8
	rep stosb
	pop rdi
	mov rax, r10
	ret
Rax_MemSet ENDP

END
