#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "fs.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

#define TTY_FIRST (tty_table)
#define TTY_END (tty_table + NR_CONSOLES)

PRIVATE void init_tty(TTY* p_tty);
PRIVATE void tty_dev_read(TTY* p_tty);
PRIVATE void tty_dev_write(TTY* p_tty);
PRIVATE void tty_do_read(TTY* p_tty, MESSAGE* msg);
PRIVATE void tty_do_write(TTY* p_tty, MESSAGE* msg);
PRIVATE void put_key(TTY* p_tty, u32 key);

PUBLIC void task_tty() {
	TTY* p_tty;
	MESSAGE msg;

	init_keyboard();

	for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++) {
		init_tty(p_tty);
	}
	select_console(0);

	while (1)
	{
		for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++) {
			do {
				tty_dev_read(p_tty);
				tty_dev_write(p_tty);
			} while (p_tty->inbuf_count);
		}

		send_recv(RECEIVE, ANY, &msg);
		int src = msg.source;
		assert(src != TASK_TTY);

		TTY* ptty = &tty_table[msg.DEVICE];

		switch (msg.type)
		{
		case DEV_OPEN:
			reset_msg(&msg);
			msg.type = SYSCALL_RET;
			send_recv(SEND, src, &msg);
			break;
		case DEV_READ:
			tty_do_read(ptty, &msg);
			break;
		case DEV_WRITE:
			tty_do_write(ptty, &msg);
			break;
		case HARD_INT:
			key_pressed = 0;
			continue;
		default:
			dump_msg("TTY::unknown msg", &msg);
			break;
		}
	}
}

PRIVATE void init_tty(TTY* p_tty) {
	p_tty->inbuf_count = 0;
	p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;

	init_screen(p_tty);
}

PUBLIC void in_process(TTY* p_tty, u32 key) {
	if (!(key & FLAG_EXT)) {
		put_key(p_tty, key);
	}
	else {
		int raw_code = key & MASK_RAW;
		switch (raw_code)
		{
		case ENTER:
			put_key(p_tty, '\n');
			break;
		case BACKSPACE:
			put_key(p_tty, '\b');
			break;
		case UP:
			if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
				scroll_screen(p_tty->p_console, SCR_DN);
			}
			break;
		case DOWN:
			if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
				scroll_screen(p_tty->p_console, SCR_UP);
			}
			break;
		case F1:
		case F2:
		case F3:
		case F4:
		case F5:
		case F6:
		case F7:
		case F8:
		case F9:
		case F10:
		case F11:
		case F12:
			if ((key & FLAG_ALT_L) || (key & FLAG_ALT_R)) {
				select_console(raw_code - F1);
			}
			else {
				if (raw_code == F12) {
					disable_int(); // 中断停止，程序结束了
					dump_proc(proc_table + 4);
					for (;;);
				}
			}
			break;
		default:
			break;
		}
	}
}

PRIVATE void put_key(TTY* p_tty, u32 key) {
	if (p_tty->inbuf_count < TTY_IN_BYTES) {
		*(p_tty->p_inbuf_head) = key;
		p_tty->p_inbuf_head++;
		if (p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_head = p_tty->in_buf;
		}
		p_tty->inbuf_count++;
	}
}

PRIVATE void tty_dev_read(TTY* p_tty) {
	if (is_current_console(p_tty->p_console)) {
		keyboard_read(p_tty);
	}
}

PRIVATE void tty_dev_write(TTY* p_tty) {
	if (p_tty->inbuf_count) {
		char ch = *(p_tty->p_inbuf_tail);
		p_tty->p_inbuf_tail++;
		if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_tail = p_tty->in_buf;
		}
		p_tty->inbuf_count--;

		if (p_tty->tty_left_cnt) { // 如果有请求读取的字节
			if (ch >= ' ' && ch <= '~') { // 可打印字符串
				out_char(p_tty->p_console, ch);
				void* p = p_tty->tty_req_buf + p_tty->tty_trans_cnt;
				phys_copy(p, (void*)va2la(TASK_TTY, &ch), 1); // 直接写入请求者消息的对应位置
				p_tty->tty_trans_cnt++;
				p_tty->tty_left_cnt--;
			}
			else if (ch == '\b' && p_tty->tty_trans_cnt) {
				out_char(p_tty->p_console, ch);
				p_tty->tty_trans_cnt--; // 回滚
				p_tty->tty_left_cnt++;
			}
			if (ch == '\n' || p_tty->tty_left_cnt == 0) {
				out_char(p_tty->p_console, '\n');
				MESSAGE msg;
				msg.type = RESUME_PROC; // 唤醒调用者
				printl("tty resume");
				msg.PROC_NR = p_tty->tty_procnr;
				msg.CNT = p_tty->tty_trans_cnt;
				send_recv(SEND, p_tty->tty_caller, &msg);
				p_tty->tty_left_cnt = 0;
			}
		}
	}
}

// 用户进程通过read或write发消息给FS，FS转发消息到TTY
PRIVATE void tty_do_read(TTY* tty, MESSAGE* msg) {
	tty->tty_caller = msg->source; // FS
	tty->tty_procnr = msg->PROC_NR; // 用户进程
	tty->tty_req_buf = va2la(tty->tty_procnr, msg->BUF);
	tty->tty_left_cnt = msg->CNT;
	tty->tty_trans_cnt = 0;

	msg->type = SUSPEND_PROC; // 挂起用户进程
	msg->CNT = tty->tty_left_cnt;
	send_recv(SEND, tty->tty_caller, msg);
}

// 直接写字符串
PRIVATE void tty_do_write(TTY* tty, MESSAGE* msg) {
	char buf[TTY_OUT_BUF_LEN];
	char* p = (char*)va2la(msg->PROC_NR, msg->BUF);
	int i = msg->CNT;
	int j;
	while (i)
	{
		int bytes = min(TTY_OUT_BUF_LEN, i);
		phys_copy(va2la(TASK_TTY, buf), (void*)p, bytes);
		for (j = 0; j < bytes; j++)
			out_char(tty->p_console, buf[j]);
		i -= bytes;
		p += bytes;
	}

	msg->type = SYSCALL_RET;
	send_recv(SEND, msg->source, msg);
}

PUBLIC void tty_write(TTY* p_tty, char* buf, int len) {
	char* p = buf;
	int i = len;
	while (i)
	{
		out_char(p_tty->p_console, *p++);
		i--;
	}
}

PUBLIC int sys_printx(int _unused1, int _unused2, char* s, struct proc* p_proc) {
	const char* p;
	char ch;
	
	char reenter_err[] = "? k_reenter is incorrect for unknown reason";
	reenter_err[0] = MAG_CH_PANIC;

	// 在ring1-3运行时，进入中断后k_reenter == 0
	// 这时处在ring0级别，如果再发生中断，k_reenter > 0
	// 然后从最新的这个中断返回，k_reenter == 0
	// 然后返回到ring1-3，k_reenter == -1
	if (k_reenter == 0) {
		p = va2la(proc2pid(p_proc), s);
	}
	else if (k_reenter > 0) {
		p = s; // ring 0下地址可以直接用
	}
	else {
		p = reenter_err;
	}

	if (*p == MAG_CH_PANIC || (*p == MAG_CH_ASSERT && p_proc_ready < &proc_table[NR_TASKS])) {
		disable_int(); // 如果panic或者是task级别下的assert，直接结束了
		char* v = (char*)V_MEM_BASE;
		const char* q = p + 1; // 跳过magic char

		while (v < (char*)(V_MEM_BASE + V_MEM_SIZE))
		{
			*v++ = *q++;
			*v++ = RED_CHAR;
			if (!*q) {
				// 16 = 8 * 2，每8行打印一次
				while (((int)v - V_MEM_BASE) % (SCREEN_WIDTH * 16))
				{
					v++;
					*v++ = GRAY_CHAR;
				}
				q = p + 1;
			}
		}

		__asm__ __volatile__("hlt"); // 停机了
	}

	while ((ch = *p++) != 0) {
		if (ch == MAG_CH_PANIC || ch == MAG_CH_ASSERT) {
			continue;
		}
		out_char(tty_table[p_proc->nr_tty].p_console, ch);
	}
	return 0;
}