#ifndef _CONST_H_
#define _CONST_H_	

#define ASSERT
#ifdef ASSERT
void assertion_failure(char* exp, char* file, char* base_file, int line);
// #exp 的意思是 "#"，将exp表达式包装成字符串
// __FILE__是编译器自带的宏
#define assert(exp)  if (exp) ; \
        else assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)
#else
#define assert(exp)
#endif // ASSERT


#define EXTERN extern // global.h中重新定义了这个宏

#define PUBLIC
#define PRIVATE static

#define STR_DEFAULT_LEN 1024

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
#define CASCADE_IRQ 2
#define ETHER_IRQ 3
#define SECONDARY_IRQ 3
#define RS232_IRQ 4
#define XT_WINT_IRQ 5
#define FLOPPY_IRQ 6
#define PRINTER_IRQ 7
#define AT_WINT_IRQ 14

// 进程通信
#define INVALID_DRIVER -20
#define INTERRUPT -10
#define TASK_TTY 0 // 和global.c中的任务进程表对应
#define TASK_SYS 1
#define ANY (NR_TASKS + NR_PROCS + 10) // 来源或接收任意进程的消息
#define NO_TASK (NR_TASKS + NR_PROCS + 20)

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
	HARD_INT = 1,
	GET_TICKS, 
};

#define RETVAL u.m3.m3i1 // 参见tpye.h

#endif // ! _CONST_H_
