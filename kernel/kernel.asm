%include "sconst.inc"

; 导入函数
extern cstart
extern kernel_main
extern exception_handler
extern spurious_irq
extern clock_handler
extern disp_str
extern delay
extern irq_table

; 导入全局变量
extern gdt_ptr
extern idt_ptr
extern p_proc_ready
extern tss
extern disp_pos
extern k_reenter
extern sys_call_table

bits 32

[SECTION .data]
clock_int_msg db '^ ',0x0

[section .bss]; 保护模式下堆栈
StackSpace resb 2 * 1024
StackTop:

[section .text]
global _start

global restart
global sys_call

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
	call save

	; 关闭当前中断
	in al, INT_M_CTLMASK
	or al, (1 << %1) 
	out INT_M_CTLMASK, al

	mov al, EOI
	out INT_M_CTL, al ; 恢复外部中断

	sti
	push %1
	call [irq_table + 4 * %1]
	pop ecx
	cli

	; 打开当前中断
	in al, INT_M_CTLMASK
	and al, ~(1 << %1) 
	out INT_M_CTLMASK, al

	ret ; call=push eip + ret，1 push了restart_reenter的地址，2 push了restart的地址
%endmacro

ALIGN 16
hwint00:
	hwint_master 0

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

save:
	; 中断自动装入
	; ss
	; esp
	; eflags
	; cs
	; eip

	; 调用者call save的时候会装入调用者的下条指令的地址
	; retaddr

	pushad ;
	push ds 
	push es 
	push fs 
	push gs ; 到这里stackframe齐了

	mov esi, edx

	mov dx, ss  
	mov ds, dx
	mov es, dx
	mov fs, dx

	mov edx, esi

	; 在每次restart的时候esp设置成了当前进程的进程表起始地址[esp + P_STACKTOP]
	mov esi, esp ; 栈顶就是s_stackframe的起始地址

	inc dword [k_reenter]
	cmp dword [k_reenter], 0
	jne .1
	mov esp, StackTop ; 内核栈
	push restart
	jmp [esi + RETADR - P_STACKBASE] ; 由于栈发生了切换，并且pop了寄存器，所以直接找到返回地址返回，不用ret返回
.1:
	push restart_reenter
	jmp [esi + RETADR - P_STACKBASE]

sys_call:
	call save
	sti

	push esi ; 下面会恢复

	push dword [p_proc_ready] ; 刚刚发起中断的进程
	push edx
	push ecx
	push ebx
	call [sys_call_table + 4 * eax] ; 返回值int存放在eax中
	add esp, 4 * 4

	pop esi

	mov [esi + EAXREG - P_STACKBASE],eax

	cli
	ret ;

restart:
	mov esp, [p_proc_ready] ; 这个是低地址，所以是栈顶
	lldt [esp + P_LDT_SEL] ; 加载ldt
	lea eax, [esp + P_STACKTOP] ; 这个是高地址，指向栈底，下一次中断发生时，将从此处开始压栈
	mov dword [tss + TSS3_S_SP0], eax ; 中断运行在内核级0级，当中断发生时，将会把ss等信息压到sp0中的堆栈
	; ，iret返回时也会将sp0指向的堆栈寄存器弹出
restart_reenter:
	dec dword[k_reenter]
	pop gs
	pop fs
	pop es 
	pop ds  
	popad

	add esp, 4 ; 跳过retaddr

	; 接下来就是cs ss这些中断时压入的数据了
	; 现在刚好中断返回
	iretd