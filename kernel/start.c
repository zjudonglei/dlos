#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

// C 程序的入口，做一些初始化工作
PUBLIC void cstart() {
	disp_str("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n-------\"cstart\" begins-------\n");
	// gdt_ptr是个数组，指针
	// gdt_ptr[2]存放的是一个地址
	// &gdt_ptr[2]获取gdt_ptr指针+2位置处的指针
	// u32*获取接下来的4位指针
	// *指针里的内容指向的地址
	// void*这个地址的指针

	// *((u16*)(&gdt_ptr[0]))同理
	memcpy(&gdt, (void*)(*((u32*)(&gdt_ptr[2]))), *((u16*)(&gdt_ptr[0])) + 1);
	u16* p_gdt_limit = (u16*)(&gdt_ptr[0]);
	u32* p_gdt_base = (u32*)(&gdt_ptr[2]);
	*p_gdt_limit = GDT_SIZE * sizeof(struct descriptor) - 1;
	*p_gdt_base = (u32)&gdt;

	u16* p_idt_limit = (u16*)(&idt_ptr[0]);
	u32* p_idt_base = (u32*)(&idt_ptr[2]);
	*p_idt_limit = IDT_SIZE * sizeof(struct gate) - 1;
	*p_idt_base = (u32)&idt;
	init_prot();
	disp_str("-----\"cstart\" finished-----\n");
}