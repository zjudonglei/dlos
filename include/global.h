#ifdef GLOBAL_VARIABLES_HERE
#undef EXTERN // 这只是一个标志
#define EXTERN
#endif // GLOBAL_VARIABLES_HERE

EXTERN int ticks;
EXTERN int disp_pos;

EXTERN u8 gdt_ptr[6];
EXTERN struct descriptor gdt[GDT_SIZE];
EXTERN u8 idt_ptr[6];
EXTERN struct gate idt[IDT_SIZE];

EXTERN u32 k_reenter;
EXTERN int nr_current_console;

EXTERN struct tss tss;
EXTERN struct proc* p_proc_ready;

extern char task_stack[];
extern struct proc proc_table[]; // 进程表，用于调度，在global.c中有定义
extern struct task task_table[]; // 任务表
extern struct task user_proc_table[];
extern irq_handler irq_table[];

extern TTY tty_table[];
extern CONSOLE console_table[];
