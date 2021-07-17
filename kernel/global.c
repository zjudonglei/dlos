#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "proc.h"
#include "global.h"

PUBLIC PROCESS proc_table[NR_TASKS];
PUBLIC char task_stack[STACK_SIZE_TOTAL];

PUBLIC TASK task_table[NR_TASKS] = {
	{TestA, STACK_SIZE_TESTA, "TestAProcess"},
	{TestB, STACK_SIZE_TESTB, "TestBProcess"},
	{TestC, STACK_SIZE_TESTC, "TestCProcess"},
};

PUBLIC irq_handler irq_table[NR_IRQ];
PUBLIC system_call sys_call_table[NR_SYS_CALL] = {
	sys_get_ticks
};