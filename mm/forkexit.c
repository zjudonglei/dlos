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
#include "keyboard.h"
#include "proto.h"

//PRIVATE void cleanup(struct proc* proc);

PUBLIC int do_fork() {
	// 1.找个可用的进程表
	struct proc* p = proc_table;
	int i;
	for (i = 0; i < NR_TASKS + NR_PROCS; i++, p++) {
		if (p->p_flags == FREE_SLOT)
			break;
	}

	int child_pid = i;
	assert(p == &proc_table[child_pid]);
	assert(child_pid >= NR_TASKS + NR_NATIVE_PROCS);
	if (i == NR_TASKS + NR_PROCS)
		return -1;
	assert(i < NR_TASKS + NR_PROCS);

	// 2.复制父进程表
	int pid = mm_msg.source;
	u16 child_ldt_sel = p->ldt_sel;
	*p = proc_table[pid]; // 将父进程表赋值给当前进程
	p->ldt_sel = child_ldt_sel; // 修改当前进程的数据
	p->p_parent = pid;
	sprintf(p->name, "%s_%d", proc_table[pid].name, child_pid);

	// 3.复制进程
	struct descriptor* ppd;
	ppd = &proc_table[pid].ldts[INDEX_LDT_C];
	int caller_T_base = reassembly(ppd->base_high, 24, ppd->base_mid, 16, ppd->base_low);
	int caller_T_limit = reassembly(0, 0, ppd->limit_high_attr2 & 0xF, 16, ppd->limit_low); // 内核大小，大概240KB
	int caller_T_size = (caller_T_limit + 1) * ((ppd->limit_high_attr2 & (DA_LIMIT_4K >> 8)) ? 4096 : 1);

	ppd = &proc_table[pid].ldts[INDEX_LDT_RW];
	int caller_D_S_base = reassembly(ppd->base_high, 24, ppd->base_mid, 16, ppd->base_low);
	int caller_D_S_limit = reassembly(0, 0, ppd->limit_high_attr2 & 0xF, 16, ppd->limit_low);
	int caller_D_S_size = (caller_T_limit + 1) * ((ppd->limit_high_attr2 & (DA_LIMIT_4K >> 8)) ? 4096 : 1);

	assert((caller_T_base == caller_D_S_base) &&
		(caller_T_limit == caller_D_S_limit) &&
		(caller_T_size == caller_D_S_size));

	int child_base = alloc_mem(child_pid, caller_T_size);
	printl("{MM} 0x%x <- 0x%x (0x%x bytes)\n", child_base, caller_T_base, caller_T_size);
	// 将父进程的一切内存空间都拷贝到新地址
	phys_copy((void*)child_base, (void*)caller_T_base, caller_T_size);

	// 初始化子进程的选择子指向子进程的空间
	init_descriptor(&p->ldts[INDEX_LDT_C],
		child_base, (PROC_IMAGE_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT,
		DA_32 | DA_LIMIT_4K | DA_C | PRIVILEGE_USER << 5);
	init_descriptor(&p->ldts[INDEX_LDT_RW],
		child_base, (PROC_IMAGE_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT,
		DA_32 | DA_LIMIT_4K | DA_DRW | PRIVILEGE_USER << 5);

	// 文件共享
	MESSAGE msg2fs;
	msg2fs.type = FORK;
	msg2fs.PID = child_pid;
	send_recv(BOTH, TASK_FS, &msg2fs);

	// 调用完成后会返回给父进程
	mm_msg.PID = child_pid;

	// 唤醒子进程
	MESSAGE m;
	m.type = SYSCALL_RET;
	m.RETVAL = 0;
	m.PID = 0;
	printl("start to resume child process\n");
	send_recv(SEND, child_pid, &m);

	return 0;
}