#ifndef _CONST_H_
#define _CONST_H_	

#define ASSERT
#ifdef ASSERT
void assertion_failure(char* exp, char* file, char* base_file, int line);
// #exp ����˼�� "#"����exp���ʽ��װ���ַ���
// __FILE__�Ǳ������Դ��ĺ�
#define assert(exp)  if (exp) ; \
        else assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)
#else
#define assert(exp)
#endif // ASSERT


#define EXTERN extern // global.h�����¶����������

#define PUBLIC
#define PRIVATE static

#define STR_DEFAULT_LEN 1024

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
#define CASCADE_IRQ 2
#define ETHER_IRQ 3
#define SECONDARY_IRQ 3
#define RS232_IRQ 4
#define XT_WINT_IRQ 5
#define FLOPPY_IRQ 6
#define PRINTER_IRQ 7
#define AT_WINT_IRQ 14

// ����ͨ��
#define INVALID_DRIVER -20
#define INTERRUPT -10
#define TASK_TTY 0 // ��global.c�е�������̱��Ӧ
#define TASK_SYS 1
#define ANY (NR_TASKS + NR_PROCS + 10) // ��Դ�����������̵���Ϣ
#define NO_TASK (NR_TASKS + NR_PROCS + 20)

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
	HARD_INT = 1,
	GET_TICKS, 
};

#define RETVAL u.m3.m3i1 // �μ�tpye.h

#endif // ! _CONST_H_
