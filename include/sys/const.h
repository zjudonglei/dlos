#ifndef _CONST_H_
#define _CONST_H_	

/* max() & min() */
#define	max(a,b)	((a) > (b) ? (a) : (b))
#define	min(a,b)	((a) < (b) ? (a) : (b))
// ��ɫ
#define BLACK 0x0
#define WHITE 0x7
#define RED 0x4
#define GREEN 0x2
#define BLUE 0x1
#define FLASH 0x80
#define BRIGHT 0x08
#define MAKE_COLOR(x,y) ((x<<4)|y) /* MAKE_COLOR(Background,Foreground) */

// ����������
#define GDT_SIZE 128
#define IDT_SIZE 256

// Ȩ��
#define PRIVILEGE_KRNL 0
#define PRIVILEGE_TASK 1
#define PRIVILEGE_USER 3

#define RPL_KRNL SA_RPL0
#define RPL_TASK SA_RPL1
#define RPL_USER SA_RPL3

// ������Ϣ���ͽ���״̬
#define SENDING 0x02
#define RECEIVING 0x04
#define WAITING   0x08	/* set when proc waiting for the child to terminate */
#define HANGING   0x10	/* set when proc exits without being waited by parent */
#define FREE_SLOT 0x20	/* set when proc table entry is not used
			 * (ok to allocated to a new process)
			 */

// ������
#define NR_CONSOLES 3

// �ⲿ�ж϶˿�
#define INT_M_CTL 0x20
#define INT_M_CTLMASK 0x21
#define INT_S_CTL 0xA0
#define INT_S_CTLMASK 0xA1

// 8253��ʱ��
#define TIMER0 0x40
#define TIMER_MODE 0x43
#define RATE_GENERATOR 0x34 // 00-11-010-0
#define TIMER_FREQ 1193182L
#define HZ 100

// 8042����
#define KB_DATA 0x60
#define KB_CMD 0x64
#define LED_CODE 0xED
#define KB_ACK 0xFA

// ��ʾ��VGA
#define CRTC_ADDR_REG 0x3D4
#define CRTC_DATA_REG 0x3D5
#define START_ADDR_H 0xC
#define START_ADDR_L 0xD
#define CURSOR_H 0xE
#define CURSOR_L 0xF
#define V_MEM_BASE 0xB8000
#define V_MEM_SIZE 0x8000

// Ӳ���ж�������
#define NR_IRQ 16
#define CLOCK_IRQ 0
#define KEYBOARD_IRQ 1
#define CASCADE_IRQ 2 // �����ж�
#define ETHER_IRQ 3
#define SECONDARY_IRQ 3
#define RS232_IRQ 4
#define	XT_WINI_IRQ	5	/* xt winchester */
#define FLOPPY_IRQ 6
#define PRINTER_IRQ 7
#define AT_WINI_IRQ 14 // Ӳ���ж�

// ����ͨ��
#define INVALID_DRIVER -20
#define INTERRUPT -10
#define TASK_TTY 0 // ��global.c�е�������̱��Ӧ
#define TASK_SYS 1
#define TASK_HD 2
#define TASK_FS 3
#define TASK_MM 4
#define INIT 5
#define ANY (NR_TASKS + NR_PROCS + 10) // ��Դ�����������̵���Ϣ
#define NO_TASK (NR_TASKS + NR_PROCS + 20)

#define	MAX_TICKS	0x7FFFABCD

#define NR_SYS_CALL 3

// ��Ϣ����
#define SEND 1 // ����
#define RECEIVE 2 // ����
#define BOTH 3 // ����|����

// magic char ��ӡ�����
#define MAG_CH_PANIC '\002' // ��ʱһ���ֽ�
#define MAG_CH_ASSERT '\003'

// ���ܺ�
enum msgtype {
	// Ӳ���ж�
	HARD_INT = 1,
	// sys task
	GET_TICKS, GET_PID,

	// fs�ļ�ϵͳ
	OPEN, CLOSE, READ, WRITE, LSEEK, STAT, UNLINK, DISK_LOG,

	// fs <-> tty
	SUSPEND_PROC, RESUME_PROC,

	// MM
	EXEC, WAIT,

	// FS<->MM
	FORK, EXIT, 

	// ϵͳ���÷��أ�����TTY��SYS��FS��MM�ȵ�
	SYSCALL_RET,

	// ������ fd
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
#define RETVAL u.m3.m3i1 // �μ�tpye.h
#define STATUS u.m3.m3i1

#define DIOCTL_GET_GEO 1
#define SECTOR_SIZE 512 // ÿ�����ֽ���
#define SECTOR_BITS (SECTOR_SIZE * 8) // ÿ����������
#define SECTOR_SIZE_SHIFT 9 // ��ӦSECTOR_SIZE

// ���豸�ţ���global.c�е����������±��Ӧ
#define NO_DEV 0
#define DEV_FLOPPY 1
#define DEV_CDROM 2
#define DEV_HD 3
#define DEV_CHAR_TTY 4
#define DEV_SCSI 5

#define MAJOR_SHIFT 8
#define MAKE_DEV(a, b) ((a << MAJOR_SHIFT) | b) // �豸��=���豸������8λ+���豸��
#define MAJOR(x) ((x >> MAJOR_SHIFT) & 0xFF) // ���豸��=�豸������8λ
#define MINOR(x) (x & 0xFF)

#define MINOR_hd1a 0x10 // ������1���߼�����1a�Ĵ��豸��
#define	MINOR_hd2a		(MINOR_hd1a+NR_SUB_PER_PART)

#define ROOT_DEV MAKE_DEV(DEV_HD, MINOR_BOOT) // ����ϵͳ������

#define INVALID_INODE 0
#define ROOT_INODE 1

#define MAX_DRIVES 2 // ������
#define NR_PART_PER_DRIVE 4 // ����������
#define NR_SUB_PER_PART 16 // ÿ����չ���������е��߼���������
#define NR_SUB_PER_DRIVE (NR_SUB_PER_PART * NR_PART_PER_DRIVE) // �߼���������
#define NR_PRIM_PER_DRIVE (NR_PART_PER_DRIVE + 1) // 0��Ӳ��+4��������

#define MAX_PRIM (MAX_DRIVES * NR_PRIM_PER_DRIVE - 1) // �����������ֵ��0����Ӳ�̣�1-4��Ӳ����������5��Ӳ�̣�6-9��Ӳ��������

#define MAX_SUBPARTITIONS (NR_SUB_PER_DRIVE * MAX_DRIVES) // �߼��������ֵ

#define P_PRIMARY 0 // ��������־
#define P_EXTENDED 1 //

// ��������
#define ORANGES_PART 0x99 
#define NO_PART 0x00
#define EXT_PART 0x05

#define NR_FILES 64 // ���̴򿪵��ļ������
#define NR_FILE_DESC 64
#define NR_INODE 64
#define NR_SUPER_BLOCK 8

#define I_TYPE_MASK 0170000
#define I_REGULAR 0100000 // �����ļ�
#define I_BLOCK_SPECIAL 0060000
#define I_DIRECTORY 0040000 // Ŀ¼
#define I_CHAR_SPECIAL 0020000
#define I_NAMED_PIPE 0010000

#define is_special(m) ((m & I_TYPE_MASK) == I_BLOCK_SPECIAL || (m & I_TYPE_MASK) == I_CHAR_SPECIAL)

#define NR_DEFAULT_FILE_SECTS 2048

#endif // ! _CONST_H_
