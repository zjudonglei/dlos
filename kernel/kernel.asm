%include "sconst.inc"

; 导入函数
extern cstart
extern kernel_main
extern exception_handler
extern spurious_irq
extern clock_handler
extern disp_str
extern delay

; 导入全局变量
extern gdt_ptr
extern idt_ptr
extern p_proc_ready
extern tss
extern disp_pos
extern k_reenter

bits 32

[SECTION .data]
clock_int_msg db "^ ",0
clock_int_msg2 db " dec0 ",0
clock_int_msg3 db " dec1 ",0

[section .bss]; 保护模式下堆栈
StackSpace resb 2 * 1024
StackTop:

[section .text]
global _start

global restart

global divide_error;
global single_step_exception;
global nmi;
global breakpoint_exception;
global overflow;
global bounds_check;
global inval_opcode;
global copr_not_available;
global double_fault;
global copr_seg_overrun;
global inval_tss;
global segment_not_present;
global stack_exception;
global general_protection;
global page_fault;
global copr_error;
global hwint00;
global hwint01;
global hwint02;
global hwint03;
global hwint04;
global hwint05;
global hwint06;
global hwint07;
global hwint08;
global hwint09;
global hwint10;
global hwint11;
global hwint12;
global hwint13;
global hwint14;
global hwint15;

_start:
	mov esp,StackTop; 转移堆栈

	mov dword [disp_pos],0

	sgdt [gdt_ptr]
	call cstart
	lgdt [gdt_ptr]

	lidt [idt_ptr]

	jmp SELECTOR_KERNEL_CS:csinit ; 这个跳转指令强制使用刚刚初始化的结构

csinit:
	; ud2
	; jmp 0x40:0
	; sti
	; hlt

	xor eax, eax 
	mov ax, SELECTOR_TSS
	ltr ax

	jmp kernel_main

%macro hwint_master 1 
	push %1
	call spurious_irq
	add esp, 4
	hlt
%endmacro

ALIGN 16
hwint00:
; 对应proc.h
	; eflags
	; cs
	; eip
	sub esp, 4 ; retaddr

	pushad 
	push ds 
	push es 
	push fs 
	push gs 
	mov dx, ss  
	mov ds, dx
	mov es, dx

	inc byte [gs:0]
	mov al, EOI
	out INT_M_CTL, al ; 恢复外部中断

	inc dword [k_reenter]
	cmp dword [k_reenter], 0
	jne .re_enter 

	; 从这里开始后面的代码每次调度会切换一个任务，并执行
	mov esp, StackTop ; 内核栈

	; push 100 ; 假如这里有延迟，任务调度会完成
	; call delay
	; add esp,4

	sti ; 允许中断，但是新的中断在cmp k_reenter那里就会跳走了

	; push clock_int_msg
	; call disp_str
	; add esp, 4

	; push 500 ; 假如这里有延迟，任务调度会完成
	; call delay
	; add esp,4

	push 0
	call clock_handler
	add esp,4

	cli ; 关闭了中断，执行到这里就不会再有中断进来，保证调度一定会完成

	; push 1 ; 假如这里有延迟，新任务不会执行，立刻进入新的中断
	; call delay
	; add esp,4

	mov esp, [p_proc_ready] ; 任务表起点

	lea eax, [esp + P_STACKTOP]
	mov dword [tss+TSS3_S_SP0], eax

.re_enter
	dec dword [k_reenter]
	pop gs 
	pop fs
	pop es 
	pop ds
	popad
	add esp, 4

	iretd

ALIGN 16
hwint01:
	hwint_master 1

ALIGN 16
hwint02:
	hwint_master 2

ALIGN 16
hwint03:
	hwint_master 3

ALIGN 16
hwint04:
	hwint_master 4

ALIGN 16
hwint05:
	hwint_master 5

ALIGN 16
hwint06:
	hwint_master 6

ALIGN 16
hwint07:
	hwint_master 7

%macro hwint_slave 1 
	push %1
	call spurious_irq
	add esp, 4
	hlt
%endmacro

ALIGN 16
hwint08:
	hwint_slave 8

ALIGN 16
hwint09:
	hwint_slave 9

ALIGN 16
hwint10:
	hwint_slave 10

ALIGN 16
hwint11:
	hwint_slave 11

ALIGN 16
hwint12:
	hwint_slave 12

ALIGN 16
hwint13:
	hwint_slave 13

ALIGN 16
hwint14:
	hwint_slave 14

ALIGN 16
hwint15:
	hwint_slave 15

divide_error:
	push 0xFFFFFFFF;
	push 0
	jmp exception
single_step_exception:
	push 0xFFFFFFFF;
	push 1
	jmp exception
nmi:
	push 0xFFFFFFFF;
	push 2
	jmp exception
breakpoint_exception:
	push 0xFFFFFFFF;
	push 3
	jmp exception
overflow:
	push 0xFFFFFFFF;
	push 4
	jmp exception
bounds_check:
	push 0xFFFFFFFF;
	push 5
	jmp exception
inval_opcode:
	push 0xFFFFFFFF;
	push 6
	jmp exception
copr_not_available:
	push 0xFFFFFFFF;
	push 7
	jmp exception
double_fault: ; 自带出错码，见保护模式中断和异常表
	push 8
	jmp exception
copr_seg_overrun:
	push 0xFFFFFFFF;
	push 9
	jmp exception
inval_tss:
	push 10
	jmp exception
segment_not_present:
	push 11
	jmp exception
stack_exception:
	push 12
	jmp exception
general_protection:
	push 13
	jmp exception
page_fault:
	push 14
	jmp exception
copr_error:
	push 0xFFFFFFFF;
	push 16
	jmp exception

exception:
	call exception_handler
	add esp, 4*2 ;堆栈中从顶向下依次是：EIP、CS、EFLAGS
	hlt


restart:
	mov esp, [p_proc_ready] ; 这个是低地址，所以是栈顶
	lldt [esp + P_LDT_SEL] ; 加载ldt
	lea eax, [esp + P_STACKTOP] ; 这个是高地址，指向栈底，下一次中断发生时，将从此处开始压栈
	mov dword [tss + TSS3_S_SP0], eax ; 设置tss 级别0堆栈

	pop gs
	push gs
	pop gs
	pop fs
	pop es 
	pop ds  
	popad

	add esp, 4

	iretd