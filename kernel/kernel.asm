SELECTOR_KERNEL_CS equ 8 

extern cstart

extern gdt_ptr

[section .bss]; 保护模式下堆栈
StackSpace resb 2 * 1024
StackTop:

[section .text]
global _start

_start:
	mov esp,StackTop; 转移堆栈
	sgdt [gdt_ptr]
	call cstart
	lgdt [gdt_ptr]

	jmp SELECTOR_KERNEL_CS:csinit

csinit:
	push 0
	popfd 
	hlt
