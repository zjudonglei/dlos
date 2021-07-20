%include "sconst.inc"

_NR_get_ticks equ 0
_NR_write equ 1
_NR_get_info equ 2
INT_VECTOR_SYS_CALL equ 0x90

global get_ticks 
global write 
global get_info 

bits 32
[section .text]

; ====================================================================
;                              get_ticks
; ====================================================================
get_ticks:
	mov eax, _NR_get_ticks
	int INT_VECTOR_SYS_CALL
	ret

; ====================================================================================
;                          void write(char* buf, int len);
; ====================================================================================
write:
	mov eax, _NR_write ;kernel.asm中的sys_call函数中庸
	mov ebx, [esp+4] ;kernel.asm中的sys_call函数中庸
	mov ecx, [esp+8] ;kernel.asm中的sys_call函数中庸
	int INT_VECTOR_SYS_CALL
	ret

get_info:
	mov eax, _NR_get_info
	int INT_VECTOR_SYS_CALL
	ret