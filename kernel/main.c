#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "proc.h"
#include "proto.h"
#include "string.h"
#include "global.h"

// ring0，kernel.asm中的bss堆栈
PUBLIC int kernel_main() {
	disp_str("----\"kernel_main\" begins----\n");
	TASK* p_task;
	PROCESS* p_proc = proc_table;
	char* p_task_stack = task_stack + STACK_SIZE_TOTAL;
	u16 selector_ldt = SELECTOR_LDT_FIRST;
	int i;
	u8 privilege;
	u8 rpl;
	int eflags;

	for (i = 0; i < NR_TASKS + NR_PROCS; i++) {
		if (i < NR_TASKS) {
			p_task = task_table + i;
			privilege = PRIVILEGE_TASK;
			rpl = RPL_TASK;
			eflags = 0x1202;
		}
		else {
			p_task = user_proc_table + (i - NR_TASKS);
			privilege = PRIVILEGE_USER;
			rpl = RPL_USER;
			eflags = 0x202;
		}


		strcpy(p_proc->p_name, p_task->name);
		p_proc->pid = i;

		p_proc->ldt_sel = selector_ldt;

		//init_descriptor(&p_proc->ldts[0], (u32)p_task->initial_eip, 0xffff, DA_C | DA_32 | DA_LIMIT_4K | PRIVILEGE_TASK << 5);
		//init_descriptor(&p_proc->ldts[1], 0xeffff, 0xffff, DA_DRW | DA_32 | DA_LIMIT_4K | PRIVILEGE_TASK << 5);

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | privilege << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;
		p_proc->regs.cs = (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl; // 指向LDT的第一个描述符，就是上面的ldts[0]
		p_proc->regs.ds = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl; // 指向LDT的第二个描述符，就是上面的ldts[1]
		p_proc->regs.es = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.fs = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ss = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;
		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack; // 指向任务的堆栈,A在最上面，栈底
		p_proc->regs.eflags = eflags; // IF=1, IOPL=1

		p_proc->nr_tty = 0;

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

	proc_table[0].ticks = proc_table[0].priority = 15;
	proc_table[1].ticks = proc_table[1].priority = 5;
	proc_table[2].ticks = proc_table[2].priority = 5;
	proc_table[3].ticks = proc_table[3].priority = 5;

	proc_table[1].nr_tty = 0;
	proc_table[2].nr_tty = 1;
	proc_table[3].nr_tty = 0;
	
	k_reenter = 0;
	ticks = 0;

	p_proc_ready = proc_table; // 指向第一个进程表

	init_clock();
	init_keyboard();

	restart();
	while(1){} // kernel.asm开始后，进程一直在这里执行
	// 中断发生后，iretd回到p_proc_ready设置的地方
}

void TestA() {

	int i = 0;
	while (1)
	{
		//disp_str("A");
		//disp_int(get_ticks());
		//disp_str(".");
		//printf("<Ticks:%x>", get_ticks());
		milli_delay(200);
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
		//disp_str("B");
		//disp_int(i++);
		//disp_str(".");
		//printf("B");
		milli_delay(200);
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
		//disp_str("C");
		//disp_int(i++);
		//disp_str(".");
		//int test = get_info();
		//printf("<test int:%x>", test);
		//printf("C");
		milli_delay(200);
		//i++;
		//if (i % 5000 == 0) {
		//	disp_str("C");
		//}
	}
}