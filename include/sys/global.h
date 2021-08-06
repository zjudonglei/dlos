#ifdef GLOBAL_VARIABLES_HERE
#undef EXTERN // ��ֻ��һ����־
#define	EXTERN
#else
#define	EXTERN	extern
#endif

EXTERN int ticks; // ��઺�
EXTERN int disp_pos; // ��Ļ��ʾλ��

EXTERN u8 gdt_ptr[6];
EXTERN struct descriptor gdt[GDT_SIZE];
EXTERN u8 idt_ptr[6];
EXTERN struct gate idt[IDT_SIZE];

EXTERN u32 k_reenter; // �ж��������
EXTERN int nr_current_console;
EXTERN int key_pressed;

EXTERN struct tss tss;
EXTERN struct proc* p_proc_ready; // ��ִ�н���

extern char task_stack[]; // ���̶�ջ
extern struct proc proc_table[]; // ���̱�ring1�����ڵ��ȣ���global.c���ж���
extern struct task task_table[]; // �����
extern struct task user_proc_table[]; // �û����̱�ring4
extern irq_handler irq_table[]; // �ⲿ�жϱ�

extern TTY tty_table[]; // ������
extern CONSOLE console_table[]; // ������Ӧ�Ŀ���̨��

// MM
EXTERN MESSAGE mm_msg;
extern u8* mmbuf;
extern const int MMBUF_SIZE;
EXTERN int memory_size;

// FS
EXTERN struct file_desc f_desc_table[NR_FILE_DESC]; // �ļ�handler�㻺��
EXTERN struct inode inode_table[NR_INODE];
EXTERN struct super_block super_block[NR_SUPER_BLOCK];
extern u8* fsbuf; // FS��HDͨ�ŵĻ���
extern const int FSBUF_SIZE;
EXTERN MESSAGE fs_msg;
EXTERN struct proc* pcaller;
EXTERN struct inode* root_inode;
extern struct dev_drv_map dd_map[]; // ���豸�ű�
