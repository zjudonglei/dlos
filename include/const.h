#ifndef _CONST_H_
#define _CONST_H_	

#define EXTERN extern

#define PUBLIC
#define PRIVATE static

/* Boolean */
#define	TRUE	1
#define	FALSE	0

#define GDT_SIZE 128
#define IDT_SIZE 256

#define PRIVILEGE_KRNL 0
#define PRIVILEGE_TASK 1
#define PRIVILEGE_USER 3

#define RPL_KRNL SA_RPL0
#define RPL_TASK SA_RPL1
#define RPL_USER SA_RPL3

// 外部中断端口
#define INT_M_CTL 0x20
#define INT_M_CTLMASK 0x21
#define INT_S_CTL 0xA0
#define INT_S_CTLMASK 0xA1

// 8253定时器
#define TIMER0 0X40
#define TIMER_MODE 0X43
#define RATE_GENERATOR 0X34 // 00-11-010-0
#define TIMER_FREQ 1193182
#define HZ 100

// 键盘
#define KB_DATA 0x60
#define KB_CMD 0x64
#define LED_CODE 0xED
#define KB_ACK 0xFA

// 显示器
#define CRTC_ADDR_REG 0x3D4
#define CRTC_DATA_REG 0x3D5
#define START_ADDR_H 0xC
#define START_ADDR_L 0xD
#define CURSOR_H 0xE
#define CURSOR_L 0xF
#define V_MEM_BASE 0xB8000
#define V_MEM_SIZE 0x8000

// 控制台
#define NR_CONSOLES 3

// 硬件中断
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

#define NR_SYS_CALL 3

#endif // ! _CONST_H_
