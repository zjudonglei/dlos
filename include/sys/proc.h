
struct stackframe {
	u32 gs;
	u32 fs;
	u32 es;
	u32 ds;
	u32 edi;
	u32 esi;
	u32 ebp;
	u32 kernel_esp;
	u32 ebx;
	u32 edx;
	u32 ecx;
	u32 eax;
	u32 retaddr;
	u32 eip;
	u32 cs;
	u32 eflags;
	u32 esp;
	u32 ss;
};

struct proc {
	struct stackframe regs;

	u16 ldt_sel; // GDT中的LDT选择子
	struct descriptor ldts[LDT_SIZE]; // LDT中的描述符

	int ticks;
	int priority;

	u32 pid;
	char name[16];

	int p_flags; // 进程暂停标志

	MESSAGE* p_msg; // 这只是个口袋，里面装封信，信发出去后，口袋就要清空
	int p_recvfrom; // 来自于第几个进程的消息
	int p_sendto;

	int has_int_msg; // 是否有中断发过来的消息

	struct proc* q_sending; // 收到的消息链表头，尚未处理
	struct proc* next_sending; // 下一个消息，链表结构

	int nr_tty;

	struct file_desc* filp[NR_FILES];
};

struct task {
	task_f initial_eip;
	int stacksize;
	char name[32];
};

#define proc2pid(x)(x - proc_table) // 获取进程号，其实就是global.c中的下标

#define NR_TASKS 4 // ring1
#define NR_PROCS 3 // ring3
#define FIRST_PROC proc_table[0]
#define LAST_PROC proc_table[NR_TASKS + NR_PROCS - 1]

#define STACK_SIZE_TTY 0x8000
#define STACK_SIZE_SYS 0x8000
#define STACK_SIZE_HD 0x8000
#define STACK_SIZE_FS 0x8000
#define STACK_SIZE_TESTA 0x8000 // 任务A的堆栈
#define STACK_SIZE_TESTB 0x8000 // 任务B的堆栈
#define STACK_SIZE_TESTC 0x8000
#define STACK_SIZE_TOTAL (  \
	STACK_SIZE_TTY + \
	STACK_SIZE_SYS + \
	STACK_SIZE_HD + \
	STACK_SIZE_FS + \
	STACK_SIZE_TESTA + \
	STACK_SIZE_TESTB + \
	STACK_SIZE_TESTC) // 所有任务的堆栈
