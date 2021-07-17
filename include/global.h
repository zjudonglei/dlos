#ifdef GLOBAL_VARIABLES_HERE
#undef EXTERN
#define EXTERN
#endif // GLOBAL_VARIABLES_HERE

EXTERN int ticks;
EXTERN int disp_pos;

EXTERN u8 gdt_ptr[6];
EXTERN DESCRIPTOR gdt[GDT_SIZE];
EXTERN u8 idt_ptr[6];
EXTERN GATE idt[IDT_SIZE];

EXTERN u32 k_reenter;

EXTERN TSS tss;
EXTERN PROCESS* p_proc_ready;

extern PROCESS proc_table[]; // 进程表，用于调度，在global.c中有定义
extern char task_stack[];
extern TASK task_table[]; // 任务表
extern irq_handler irq_table[];
