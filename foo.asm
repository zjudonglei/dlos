extern choose

[section .data]
num1st dd 3
num2nd dd 4

[section .text]
global _start ; 导出这个方法，_start是入口
global myprint ; 导出

_start:
	push dword [num2nd] ; 后面的参数先入栈
	push dword [num1st] ; 这样pop出来就是正序
	call choose

	add esp, 8
	mov ebx, 0
	mov eax, 1
	int 0x80

; void myprint(char* msg, int len)
; 同样遵循后面的参数先入栈
myprint:
	mov edx, [esp + 8] ; len
	mov ecx, [esp + 4] ; msg
	mov ebx, 1
	mov eax, 4
	int 0x80
	ret