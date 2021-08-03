%include "sconst.inc"

INT_VECTOR_SYS_CALL equ 0x90
_NR_printx equ 0
_NR_sendrec equ 1

global printx 
global sendrec 

bits 32
[section .text]

; ====================================================================================
;                          void printx(char* s);
; ====================================================================================
printx:
	mov eax, _NR_printx
	mov edx, [esp + 4]
	int INT_VECTOR_SYS_CALL
	ret

; ====================================================================================
;                  sendrec(int function, int src_dest, MESSAGE* msg);
; ====================================================================================
; Never call sendrec() directly, call send_recv() instead.
sendrec:
	mov eax, _NR_sendrec ;kernel.asm中的sys_call函数中用
	mov ebx, [esp+4] ;kernel.asm中的sys_call函数中用
	mov ecx, [esp+8] ;kernel.asm中的sys_call函数中用
	mov edx, [esp+12]
	int INT_VECTOR_SYS_CALL
	ret
