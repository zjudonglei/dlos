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
	//u16 selector_ldt = SELECTOR_LDT_FIRST;
	u8 privilege;
	u8 rpl;
	int eflags;
	int i, j;
	int prio;

	for (i = 0; i < NR_TASKS + NR_PROCS; i++, p_proc++, p_task++) {
		if (i >= NR_TASKS + NR_NATIVE_PROCS) {
			p_proc->p_flags = FREE_SLOT;
			continue;
		}

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
		p_proc->p_parent = NO_TASK;
		//p_proc->pid = i;

		if (strcmp(p_task->name, "INIT") != 0) {
			//p_proc->ldt_sel = selector_ldt;
			p_proc->ldts[INDEX_LDT_C] = gdt[SELECTOR_KERNEL_CS >> 3];
			p_proc->ldts[INDEX_LDT_RW] = gdt[SELECTOR_KERNEL_CS >> 3];

			//memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(struct descriptor));
			p_proc->ldts[INDEX_LDT_C].attr1 = DA_C | privilege << 5;
			//memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(struct descriptor));
			p_proc->ldts[INDEX_LDT_RW].attr1 = DA_DRW | privilege << 5;
		}
		else {
			unsigned int k_base; 
			unsigned int k_limit;
			int ret = get_kernel_map(&k_base, &k_limit);
			assert(ret == 0);
			init_descriptor(&p_proc->ldts[INDEX_LDT_C],
				0, // 内核起始地址0x900，忽略不计
				(k_base + k_limit) >> LIMIT_4K_SHIFT,
				DA_32 | DA_LIMIT_4K | DA_C | privilege << 5);
			init_descriptor(&p_proc->ldts[INDEX_LDT_RW],
				0, (k_base + k_limit) >> LIMIT_4K_SHIFT,
				DA_32 | DA_LIMIT_4K | DA_DRW | privilege << 5);
		}
		p_proc->regs.cs = INDEX_LDT_C << 3 | SA_TIL | rpl; // 指向LDT的第一个描述符，就是上面的ldts[0]
		p_proc->regs.ds = p_proc->regs.es = p_proc->regs.fs = p_proc->regs.ss =
			INDEX_LDT_RW << 3 | SA_TIL | rpl; // 指向LDT的第二个描述符，就是上面的ldts[1]
		p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;
		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack; // 指向任务的堆栈,A在最上面，栈底
		p_proc->regs.eflags = eflags; // IF=1, IOPL=1

		//p_proc->nr_tty = 0;
		p_proc->p_flags = 0;
		p_proc->p_msg = 0;
		p_proc->p_recvfrom = NO_TASK;
		p_proc->p_sendto = NO_TASK;
		p_proc->has_int_msg = 0;
		p_proc->q_sending = 0;
		p_proc->next_sending = 0;
		p_proc->ticks = p_proc->priority = prio;

		for (j = 0; j < NR_FILES; j++)
			p_proc->filp[j] = 0;

		p_task_stack -= p_task->stacksize;
		//selector_ldt += 1 << 3;
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

// 这就是所有进程的祖先了吗，真神奇
void Init() {
	int fd_stdin = open("/dev_tty0", O_RDWR); // 一个用于读
	assert(fd_stdin == 0);
	int fd_stdout = open("/dev_tty0", O_RDWR); // 一个用于写
	assert(fd_stdout == 1);

	printf("Init() is running ...\n");

	int pid = fork();
	if (pid != 0) {
		printf("parent is running, child pid:%d\n", pid);
		spin("parent");
	}
	else {
		printf("child is running, pid:%d\n", getpid());
		spin("child");
	}
}

void TestA() {
	for (;;);
	int fd;
	int i, n;
	const char filename[] = "blah";
	const char bufw[] = "Hello, hard disk!";
	const int rd_bytes = 5;
	char bufr[rd_bytes];

	assert(rd_bytes <= strlen(bufw));

	fd = open(filename, O_CREAT | O_RDWR);
	assert(fd != -1);
	printl("File created. fd: %d\n", fd);

	n = write(fd, bufw, strlen(bufw));
	assert(n == strlen(bufw));

	close(fd);

	fd = open(filename, O_RDWR);
	assert(fd != -1);
	printl("File opened. fd: %d\n", fd);

	n = read(fd, bufr, rd_bytes);
	assert(n == rd_bytes);
	bufr[n] = 0;
	printl("%d bytes read: %s\n", rd_bytes, bufr);

	close(fd);

	char* filenames[] = { "/foo", "/bar", "baz" };
	for (i = 0; i < sizeof(filenames) / sizeof(filenames[0]);i++) {
		fd = open(filenames[i], O_CREAT | O_RDWR);
		assert(fd != -1);
		printl("File created: %s (fd %d)\n", filenames[i], fd);
		close(fd);
	}

	char* rfilenames[] = { "/bar", "/foo", "baz" , "/dev_tty0" };
	for (i = 0; i < sizeof(rfilenames) / sizeof(rfilenames[0]); i++) {
		if (unlink(rfilenames[i]) == 0)
			printl("File removed: %s\n", rfilenames[i]);
		else
			printl("Failed to remove file: %s\n", rfilenames[i]);
	}

	spin("TestA");
}

void TestB() {
	for (;;);
	char tty_name[] = "/dev_tty1";
	int fd_stdin = open(tty_name, O_RDWR); // 一个用于读
	assert(fd_stdin == 0);
	int fd_stdout = open(tty_name, O_RDWR); // 一个用于写
	assert(fd_stdout == 1);

	char rdbuf[128];

	while (1)
	{
		write(fd_stdout, "$ ", 2);

		int r = read(fd_stdin, rdbuf, 70);
		//spin("TestB");
		rdbuf[r] = 0;

		if (strcmp(rdbuf, "hello") == 0) {
			write(fd_stdout, "hello world!\n", 13);
		}
		else {
			if (rdbuf[0]) {
				write(fd_stdout, "{", 1);
				write(fd_stdout, rdbuf, r);
				write(fd_stdout, "}\n", 2);
			}
		}
	}

	assert(0);
}

void TestC() {
	for (;;);
	spin("TestC");
}

PUBLIC void panic(const char* fmt, ...) {
	char buf[256];
	va_list arg = (va_list)((char*)&fmt + 4);

	vsprintf(buf, fmt, arg);

	printl("%c !!panic!! %s", MAG_CH_PANIC, buf);

	// 一般来说不会走到下面这一行，因为在pringl中就已经走到sys_printx，然后hlt停机了
	__asm__ __volatile__("ud2"); // 汇编指令ud2
}