#ifndef _CONST_H_
#define _CONST_H_	

/* max() & min() */
#define	max(a,b)	((a) > (b) ? (a) : (b))
#define	min(a,b)	((a) < (b) ? (a) : (b))
// 颜色
#define BLACK 0x0
#define WHITE 0x7
#define RED 0x4
#define GREEN 0x2
#define BLUE 0x1
#define FLASH 0x80
#define BRIGHT 0x08
#define MAKE_COLOR(x,y) ((x<<4)|y) /* MAKE_COLOR(Background,Foreground) */

// 描述符个数
#define GDT_SIZE 128
#define IDT_SIZE 256

// 权限
#define PRIVILEGE_KRNL 0
#define PRIVILEGE_TASK 1
#define PRIVILEGE_USER 3

#define RPL_KRNL SA_RPL0
#define RPL_TASK SA_RPL1
#define RPL_USER SA_RPL3

// 进程消息发送接收状态
#define SENDING 0x02
#define RECEIVING 0x04
#define WAITING   0x08	/* set when proc waiting for the child to terminate */
#define HANGING   0x10	/* set when proc exits without being waited by parent */
#define FREE_SLOT 0x20	/* set when proc table entry is not used
			 * (ok to allocated to a new process)
			 */

// 分屏数
#define NR_CONSOLES 3

// 外部中断端口
#define INT_M_CTL 0x20
#define INT_M_CTLMASK 0x21
#define INT_S_CTL 0xA0
#define INT_S_CTLMASK 0xA1

// 8253定时器
#define TIMER0 0x40
#define TIMER_MODE 0x43
#define RATE_GENERATOR 0x34 // 00-11-010-0
#define TIMER_FREQ 1193182L
#define HZ 100

// 8042键盘
#define KB_DATA 0x60
#define KB_CMD 0x64
#define LED_CODE 0xED
#define KB_ACK 0xFA

// 显示器VGA
#define CRTC_ADDR_REG 0x3D4
#define CRTC_DATA_REG 0x3D5
#define START_ADDR_H 0xC
#define START_ADDR_L 0xD
#define CURSOR_H 0xE
#define CURSOR_L 0xF
#define V_MEM_BASE 0xB8000
#define V_MEM_SIZE 0x8000

// 硬件中断向量号
#define NR_IRQ 16
#define CLOCK_IRQ 0
#define KEYBOARD_IRQ 1
#define CASCADE_IRQ 2 // 从盘中断
#define ETHER_IRQ 3
#define SECONDARY_IRQ 3
#define RS232_IRQ 4
#define	XT_WINI_IRQ	5	/* xt winchester */
#define FLOPPY_IRQ 6
#define PRINTER_IRQ 7
#define AT_WINI_IRQ 14 // 硬盘中断

// 进程通信
#define INVALID_DRIVER -20
#define INTERRUPT -10
#define TASK_TTY 0 // 和global.c中的任务进程表对应
#define TASK_SYS 1
#define TASK_HD 2
#define TASK_FS 3
#define TASK_MM 4
#define INIT 5
#define ANY (NR_TASKS + NR_PROCS + 10) // 来源或接收任意进程的消息
#define NO_TASK (NR_TASKS + NR_PROCS + 20)

#define	MAX_TICKS	0x7FFFABCD

#define NR_SYS_CALL 3

// 消息动作
#define SEND 1 // 发送
#define RECEIVE 2 // 接收
#define BOTH 3 // 发送|接收

// magic char 打印辨别用
#define MAG_CH_PANIC '\002' // 这时一个字节
#define MAG_CH_ASSERT '\003'

// 功能号
enum msgtype {
	// 硬件中断
	HARD_INT = 1,
	// sys task
	GET_TICKS, GET_PID,

	// fs文件系统
	OPEN, CLOSE, READ, WRITE, LSEEK, STAT, UNLINK, DISK_LOG,

	// fs <-> tty
	SUSPEND_PROC, RESUME_PROC,

	// MM
	EXEC, WAIT,

	// FS<->MM
	FORK, EXIT, 

	// 系统调用返回，用于TTY，SYS，FS，MM等等
	SYSCALL_RET,

	// 驱动器 fd
	DEV_OPEN = 1001, DEV_CLOSE, DEV_READ, DEV_WRITE, DEV_IOCTL
};

// OPEN: USER_PROC -----------------------> FS
#define PATHNAME u.m3.m3p1
#define FLAGS u.m3.m3i1
#define NAME_LEN u.m3.m3i2

// READ or WRITE: USER_PROC -----------------------> FS
#define FD u.m3.m3i1
#define BUF u.m3.m3p2
#define CNT u.m3.m3i2

// LSEEK: USER_PROC -----------------------> FS
//#define FD u.m3.m3i1
#define OFFSET u.m3.m3i2
#define WHENCE u.m3.m3i3

// DEV_READ or DEV_WRITE: FS -------------------------> HD
#define DEVICE u.m3.m3i4
#define POSITION u.m3.m3l1
//#define BUF u.m3.m3p2
//#define CNT u.m3.m3i2
#define PROC_NR u.m3.m3i3

// DEV_READ or DEV_WRITE: FS -------------------------> TTY
//#define DEVICE u.m3.m3i4
//#define BUF u.m3.m3p2
//#define CNT u.m3.m3i2
//#define PROC_NR u.m3.m3i3

// DEV_IOCTL: FS -------------------------> HD
//#define DEVICE u.m3.m3i4
#define REQUEST u.m3.m3i2
//#define BUF u.m3.m3p2
//#define PROC_NR u.m3.m3i3

#define BUF_LEN u.m3.m3i3

#define PID u.m3.m3i2
#define RETVAL u.m3.m3i1 // 参见tpye.h
#define STATUS u.m3.m3i1

#define DIOCTL_GET_GEO 1
#define SECTOR_SIZE 512 // 每扇区字节数
#define SECTOR_BITS (SECTOR_SIZE * 8) // 每扇区比特数
#define SECTOR_SIZE_SHIFT 9 // 对应SECTOR_SIZE

// 主设备号，跟global.c中的驱动程序下标对应
#define NO_DEV 0
#define DEV_FLOPPY 1
#define DEV_CDROM 2
#define DEV_HD 3
#define DEV_CHAR_TTY 4
#define DEV_SCSI 5

#define MAJOR_SHIFT 8
#define MAKE_DEV(a, b) ((a << MAJOR_SHIFT) | b) // 设备号=主设备号左移8位+次设备号
#define MAJOR(x) ((x >> MAJOR_SHIFT) & 0xFF) // 主设备号=设备号右移8位
#define MINOR(x) (x & 0xFF)

#define MINOR_hd1a 0x10 // 主分区1的逻辑分区1a的次设备号
#define	MINOR_hd2a		(MINOR_hd1a+NR_SUB_PER_PART)

#define ROOT_DEV MAKE_DEV(DEV_HD, MINOR_BOOT) // 操作系统启动盘

#define INVALID_INODE 0
#define ROOT_INODE 1

#define MAX_DRIVES 2 // 磁盘数
#define NR_PART_PER_DRIVE 4 // 主分区数量
#define NR_SUB_PER_PART 16 // 每个扩展分区可以有的逻辑分区数量
#define NR_SUB_PER_DRIVE (NR_SUB_PER_PART * NR_PART_PER_DRIVE) // 逻辑分区总数
#define NR_PRIM_PER_DRIVE (NR_PART_PER_DRIVE + 1) // 0号硬盘+4个主分区

#define MAX_PRIM (MAX_DRIVES * NR_PRIM_PER_DRIVE - 1) // 主分区的最大值，0号主硬盘，1-4主硬盘主分区，5从硬盘，6-9从硬盘主分区

#define MAX_SUBPARTITIONS (NR_SUB_PER_DRIVE * MAX_DRIVES) // 逻辑分区最大值

#define P_PRIMARY 0 // 主分区标志
#define P_EXTENDED 1 //

// 分区类型
#define ORANGES_PART 0x99 
#define NO_PART 0x00
#define EXT_PART 0x05

#define NR_FILES 64 // 进程打开的文件最大数
#define NR_FILE_DESC 64
#define NR_INODE 64
#define NR_SUPER_BLOCK 8

#define I_TYPE_MASK 0170000
#define I_REGULAR 0100000 // 常规文件
#define I_BLOCK_SPECIAL 0060000
#define I_DIRECTORY 0040000 // 目录
#define I_CHAR_SPECIAL 0020000
#define I_NAMED_PIPE 0010000

#define is_special(m) ((m & I_TYPE_MASK) == I_BLOCK_SPECIAL || (m & I_TYPE_MASK) == I_CHAR_SPECIAL)

#define NR_DEFAULT_FILE_SECTS 2048

#endif // ! _CONST_H_
