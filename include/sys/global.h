#ifdef GLOBAL_VARIABLES_HERE
#undef EXTERN // 这只是一个标志
#define	EXTERN
#else
#define	EXTERN	extern
#endif

EXTERN int ticks; // 嘀嗒号
EXTERN int disp_pos; // 屏幕显示位置

EXTERN u8 gdt_ptr[6];
EXTERN struct descriptor gdt[GDT_SIZE];
EXTERN u8 idt_ptr[6];
EXTERN struct gate idt[IDT_SIZE];

EXTERN u32 k_reenter; // 中断重入次数
EXTERN int nr_current_console;
EXTERN int key_pressed;

EXTERN struct tss tss;
EXTERN struct proc* p_proc_ready; // 待执行进程

extern char task_stack[]; // 进程堆栈
extern struct proc proc_table[]; // 进程表ring1，用于调度，在global.c中有定义
extern struct task task_table[]; // 任务表
extern struct task user_proc_table[]; // 用户进程表ring4
extern irq_handler irq_table[]; // 外部中断表

extern TTY tty_table[]; // 分屏表
extern CONSOLE console_table[]; // 分屏对应的控制台表

// MM
EXTERN MESSAGE mm_msg;
extern u8* mmbuf;
extern const int MMBUF_SIZE;
EXTERN int memory_size;

// FS
EXTERN struct file_desc f_desc_table[NR_FILE_DESC]; // 文件handler点缓存
EXTERN struct inode inode_table[NR_INODE];
EXTERN struct super_block super_block[NR_SUPER_BLOCK];
extern u8* fsbuf; // FS和HD通信的缓存
extern const int FSBUF_SIZE;
EXTERN MESSAGE fs_msg;
EXTERN struct proc* pcaller;
EXTERN struct inode* root_inode;
extern struct dev_drv_map dd_map[]; // 主设备号表
