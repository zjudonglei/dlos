#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "fs.h"
#include "tty.h"
#include "console.h"
#include "proc.h"
#include "global.h"
#include "proto.h"

PUBLIC struct proc proc_table[NR_TASKS + NR_PROCS];

PUBLIC struct task task_table[NR_TASKS] = {
	{task_tty, STACK_SIZE_TTY, "TTY"},
	{task_sys, STACK_SIZE_SYS, "SYS"},
	{task_hd, STACK_SIZE_HD, "HD"},
	{task_fs, STACK_SIZE_FS, "FS"},
	{task_mm, STACK_SIZE_MM, "MM"},
};

PUBLIC struct task user_proc_table[NR_PROCS] = {
	{Init, STACK_SIZE_INIT, "INIT"},
	{TestA, STACK_SIZE_TESTA, "TestA"},
	{TestB, STACK_SIZE_TESTB, "TestB"},
	{TestC, STACK_SIZE_TESTC, "TestC"},
};

PUBLIC char task_stack[STACK_SIZE_TOTAL];

PUBLIC TTY tty_table[NR_CONSOLES];
PUBLIC CONSOLE console_table[NR_CONSOLES];

PUBLIC irq_handler irq_table[NR_IRQ];
PUBLIC system_call sys_call_table[NR_SYS_CALL] = {
	sys_printx,
	sys_sendrec,
};

// �豸��������
struct dev_drv_map dd_map[] = {
	{INVALID_DRIVER}, // 0������
	{INVALID_DRIVER}, // 1��������������
	{INVALID_DRIVER}, // 2��������cd��
	{TASK_HD}, // 3��Ӳ��
	{TASK_TTY}, // 4��TTY
	{INVALID_DRIVER},
};

// �ļ�ϵͳ���������ݣ�6M~7M
PUBLIC	u8 *		fsbuf		= (u8*)0x600000;
PUBLIC	const int	FSBUF_SIZE	= 0x100000;
