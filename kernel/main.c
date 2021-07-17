#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

// ring0，kernel.asm中的bss堆栈
PUBLIC int kernel_main() {
	disp_str("----\"kernel_main\" begins----\n");
	TASK* p_task = task_table;
	PROCESS* p_proc = proc_table;
	char* p_task_stack = task_stack + STACK_SIZE_TOTAL;
	u16 selector_ldt = SELECTOR_LDT_FIRST;
	int i;

	for (i = 0; i < NR_TASKS; i++) {
		strcpy(p_proc->p_name, p_task->name);
		p_proc->pid = i;

		p_proc->ldt_sel = selector_ldt;

		//init_descriptor(&p_proc->ldts[0], (u32)p_task->initial_eip, 0xffff, DA_C | DA_32 | DA_LIMIT_4K | PRIVILEGE_TASK << 5);
		//init_descriptor(&p_proc->ldts[1], 0xeffff, 0xffff, DA_DRW | DA_32 | DA_LIMIT_4K | PRIVILEGE_TASK << 5);

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
		p_proc->regs.cs = (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK; // 指向LDT的第一个描述符，就是上面的ldts[0]
		p_proc->regs.ds = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK; // 指向LDT的第二个描述符，就是上面的ldts[1]
		p_proc->regs.es = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.fs = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.ss = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | RPL_TASK;
		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack; // 指向任务的堆栈,A在最上面，栈底
		p_proc->regs.eflags = 0x1202; // IF=1, IOPL=1

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

	proc_table[0].ticks = proc_table[0].priority = 15;
	proc_table[1].ticks = proc_table[1].priority = 5;
	proc_table[2].ticks = proc_table[2].priority = 3;
	
	k_reenter = 0;
	ticks = 0;

	put_irq_handler(CLOCK_IRQ, clock_handler);
	enable_irq(CLOCK_IRQ);

	p_proc_ready = proc_table; // 指向第一个进程表

	// 初始化计时器8253
	out_byte(TIMER_MODE, RATE_GENERATOR);
	out_byte(TIMER0, (u8)(TIMER_FREQ /HZ));
	out_byte(TIMER0, (u8)(TIMER_FREQ / HZ) >> 8);

	restart();
	while(1){} // kernel.asm开始后，进程一直在这里执行
	// 中断发生后，iretd回到p_proc_ready设置的地方
}

void TestA() {

	int i = 0;
	while (1)
	{
		disp_str("A");
		//disp_int(get_ticks());
		disp_str(".");
		milli_delay(10);
		//i++;
		//if (i % 5000 == 0) {
		//	disp_str("A");
		//	disp_str("" + NR_TASKS);
		//}
	}
}

void TestB() {

	int i = 0;
	while (1)
	{
		disp_str("B");
		//disp_int(i++);
		disp_str(".");
		milli_delay(10);
		//i++;
		//if (i % 5000 == 0) {
		//	disp_str("B");
		//}
	}
}

void TestC() {

	int i = 0;
	while (1)
	{
		disp_str("C");
		//disp_int(i++);
		disp_str(".");
		milli_delay(10);
		//i++;
		//if (i % 5000 == 0) {
		//	disp_str("C");
		//}
	}
}