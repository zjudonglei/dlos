
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

	u16 ldt_sel; // GDT�е�LDTѡ����
	struct descriptor ldts[LDT_SIZE]; // LDT�е�������

	int ticks;
	int priority;

	//u32 pid;
	char name[16];

	int p_flags; // ������ͣ��־

	MESSAGE* p_msg; // ��ֻ�Ǹ��ڴ�������װ���ţ��ŷ���ȥ�󣬿ڴ���Ҫ���
	int p_recvfrom; // �����ڵڼ������̵���Ϣ
	int p_sendto;

	int has_int_msg; // �Ƿ����жϷ���������Ϣ

	struct proc* q_sending; // �յ�����Ϣ����ͷ����δ����
	struct proc* next_sending; // ��һ����Ϣ������ṹ

	int nr_tty;

	int p_parent;

	struct file_desc* filp[NR_FILES];
};

struct task {
	task_f initial_eip;
	int stacksize;
	char name[32];
};

#define proc2pid(x)(x - proc_table) // ��ȡ���̺ţ���ʵ����global.c�е��±�

#define NR_TASKS 5 // ring1
#define NR_PROCS 32 // ring3
#define NR_NATIVE_PROCS 4 // TestA��B��C Init 
#define FIRST_PROC proc_table[0]
#define LAST_PROC proc_table[NR_TASKS + NR_PROCS - 1]

#define PROCS_BASE 0xA00000 // 10MB
#define PROC_IMAGE_SIZE_DEFAULT 0x100000 // 1MB
#define PROC_ORIGIN_STASK 0x400 // 1kb

#define STACK_SIZE_DEFAULT 0x4000
#define STACK_SIZE_TTY STACK_SIZE_DEFAULT
#define STACK_SIZE_SYS STACK_SIZE_DEFAULT
#define STACK_SIZE_HD STACK_SIZE_DEFAULT
#define STACK_SIZE_FS STACK_SIZE_DEFAULT
#define STACK_SIZE_MM STACK_SIZE_DEFAULT
#define STACK_SIZE_INIT STACK_SIZE_DEFAULT
#define STACK_SIZE_TESTA STACK_SIZE_DEFAULT // ����A�Ķ�ջ
#define STACK_SIZE_TESTB STACK_SIZE_DEFAULT // ����B�Ķ�ջ
#define STACK_SIZE_TESTC STACK_SIZE_DEFAULT
#define STACK_SIZE_TOTAL (  \
	STACK_SIZE_TTY + \
	STACK_SIZE_SYS + \
	STACK_SIZE_HD + \
	STACK_SIZE_FS + \
	STACK_SIZE_MM + \
	STACK_SIZE_INIT + \
	STACK_SIZE_TESTA + \
	STACK_SIZE_TESTB + \
	STACK_SIZE_TESTC) // ��������Ķ�ջ
