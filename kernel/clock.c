#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

PUBLIC void clock_handler(int irq)
{
	if (++ticks >= MAX_TICKS)
		ticks = 0;

	if (p_proc_ready->ticks)
		p_proc_ready->ticks--;

	if (key_pressed)
		inform_int(TASK_TTY); // 键盘有输入，唤醒TTY去读

	if (k_reenter != 0) {
		return;
	}

	if (p_proc_ready->ticks > 0) {
		return;
	}

	schedule();
}

// 定时器
PUBLIC void milli_delay(int mill_sec) {
	int t = get_ticks();
	while ((get_ticks() - t) * 1000 / HZ < mill_sec) {

	}
}

PUBLIC void init_clock() {
	// 初始化计时器8253
	out_byte(TIMER_MODE, RATE_GENERATOR);
	out_byte(TIMER0, (u8)(TIMER_FREQ / HZ));
	out_byte(TIMER0, (u8)(TIMER_FREQ / HZ) >> 8);

	put_irq_handler(CLOCK_IRQ, clock_handler);
	enable_irq(CLOCK_IRQ);
}