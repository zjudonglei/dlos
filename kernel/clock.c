#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

PUBLIC void clock_handler(int irq)
{
	ticks++;
	p_proc_ready->ticks--;

	if (k_reenter != 0) {
		return;
	}

	if (p_proc_ready->ticks > 0) {
		return;
	}

	schedule();
}

PUBLIC void milli_delay(int mill_sec) {
	int t = get_ticks();
	while ((get_ticks() - t) * 1000 / HZ < mill_sec) {

	}
}