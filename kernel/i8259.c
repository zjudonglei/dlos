#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

PUBLIC void init_8259A() {
	out_byte(INT_M_CTL, 0x11); // 主板，ICW1
	out_byte(INT_S_CTL, 0x11); // 从板，ICW1

	out_byte(INT_M_CTLMASK, INT_VECTOR_IRQ0); // 主板，ICW2
	out_byte(INT_S_CTLMASK, INT_VECTOR_IRQ8); // 从板，ICW2

	out_byte(INT_M_CTLMASK, 0x4); // 主板，ICW3
	out_byte(INT_S_CTLMASK, 0x2); // 从板，ICW3

	out_byte(INT_M_CTLMASK, 0x1); // 主板，ICW4
	out_byte(INT_S_CTLMASK, 0x1); // 从板，ICW4

	out_byte(INT_M_CTLMASK, 0xFF); // 主板，OCW1，通过main.c中调用enable_irq开启
	out_byte(INT_S_CTLMASK, 0xFF); // 从板，OCW2

	int i;
	for (i = 0; i < NR_IRQ; i++) {
		irq_table[i] = spurious_irq;
	}
}

PUBLIC void spurious_irq(int irq) {
	disp_str("spurious_irq: ");
	disp_int(irq);
	disp_str("\n");
}

PUBLIC void put_irq_handler(int irq, irq_handler handler) {
	disable_irq(irq);
	irq_table[irq] = handler;
}