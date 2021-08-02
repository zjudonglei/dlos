#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

// ring0，kernel.asm中的bss堆栈
PUBLIC int kernel_main() {
	disp_str("----\"kernel_main\" begins----\n");

	struct task* p_task;
	struct proc* p_proc = proc_table;
	char* p_task_stack = task_stack + STACK_SIZE_TOTAL;
	u16 selector_ldt = SELECTOR_LDT_FIRST;
	u8 privilege;
	u8 rpl;
	int eflags;
	int i;
	int prio;

	for (i = 0; i < NR_TASKS + NR_PROCS; i++) {
		if (i < NR_TASKS) {
			p_task = task_table + i;
			privilege = PRIVILEGE_TASK;
			rpl = RPL_TASK;
			eflags = 0x1202;
			prio = 15;
		}
		else {
			p_task = user_proc_table + (i - NR_TASKS);
			privilege = PRIVILEGE_USER;
			rpl = RPL_USER;
			eflags = 0x202;
			prio = 5;
		}

		strcpy(p_proc->name, p_task->name);
		p_proc->pid = i;

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(struct descriptor));
		p_proc->ldts[0].attr1 = DA_C | privilege << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(struct descriptor));
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
		p_proc->p_flags = 0;
		p_proc->p_msg = 0;
		p_proc->p_recvfrom = NO_TASK;
		p_proc->p_sendto = NO_TASK;
		p_proc->has_int_msg = 0;
		p_proc->q_sending = 0;
		p_proc->next_sending = 0;
		p_proc->ticks = p_proc->priority = prio;

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

    proc_table[NR_TASKS + 0].nr_tty = 0;
    proc_table[NR_TASKS + 1].nr_tty = 1;
    proc_table[NR_TASKS + 2].nr_tty = 2;

	
	k_reenter = 0;
	ticks = 0;

	p_proc_ready = proc_table; // 指向第一个进程表

	init_clock();
	init_keyboard();

	restart();
	while(1){} // kernel.asm开始后，进程一直在这里执行
	// 中断发生后，iretd回到p_proc_ready设置的地方
}

PUBLIC int get_ticks() {
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = GET_TICKS;
	send_recv(BOTH, TASK_SYS, &msg);
	return msg.RETVAL;
}

void TestA() {
	int fd;
	int n;
	const char filename[] = "blah";
	const char bufw[] = "Hello, hard disk!";
	const int rd_bytes = 5;
	char bufr[rd_bytes];

	assert(rd_bytes <= strlen(bufw));

	fd = open(filename, O_CREAT | O_RDWR);
	assert(fd != -1);
	printf("File created. fd: %d\n", fd);

	n = write(fd, bufw, strlen(bufw));
	assert(n == strlen(bufw));

	close(fd);

	fd = open(filename, O_RDWR);
	assert(fd != -1);
	printf("File opened. fd: %d\n", fd);

	n = read(fd, bufr, rd_bytes);
	assert(n == rd_bytes);
	bufr[n] = 0;
	printf("%d bytes read: %s\n", rd_bytes, bufr);

	close(fd);

	spin("TestA");
}

void TestB() {
	while (1)
	{
		printf("B");
		milli_delay(200);
	}
}

void TestC() {
	while (1)
	{
		printf("C");
		milli_delay(200);
	}
}

PUBLIC void panic(const char* fmt, ...) {
	char buf[256];
	va_list arg = (va_list)((char*)&fmt + 4);

	vsprintf(buf, fmt, arg);

	printl("%c !!panic!! %s", MAG_CH_PANIC, buf);

	// 一般来说不会走到下面这一行，因为在pringl中就已经走到sys_printx，然后hlt停机了
	__asm__ __volatile__("ud2"); // 汇编指令ud2
}