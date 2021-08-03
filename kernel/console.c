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
#include "keyboard.h"
#include "proto.h"

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32 addr);
PRIVATE void flush(CONSOLE* p_con);

PUBLIC void init_screen(TTY* p_tty) {
	int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;
	CONSOLE* p_console = p_tty->p_console;

	int v_mem_size = V_MEM_SIZE >> 1; // 双字节

	int con_v_mem_size = v_mem_size / NR_CONSOLES;
	p_console->original_addr = nr_tty * con_v_mem_size;
	p_console->v_mem_limit = con_v_mem_size;
	p_console->current_start_addr = p_console->original_addr;
	/* 默认光标位置在最开始处 */
	p_console->cursor = p_console->original_addr;
	if (nr_tty == 0) {
		p_console->cursor = disp_pos / 2;
		disp_pos = 0;
	}
	else {
		out_char(p_console, nr_tty + '0');
		out_char(p_console, '#');
	}

	set_cursor(p_console->cursor);
}

PUBLIC int is_current_console(CONSOLE* p_con) {
	return p_con == &console_table[nr_current_console];
}

PUBLIC void out_char(CONSOLE* p_con, char ch) {
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
	int old_row = (p_con->cursor - p_con->original_addr) / SCREEN_WIDTH;
	int new_row;
	int next_row;
	switch (ch)
	{
	case '\n':
		if (p_con->cursor + SCREEN_WIDTH < p_con->original_addr + p_con->v_mem_limit) {
			p_con->cursor = p_con->original_addr + SCREEN_WIDTH * ((p_con->cursor - p_con->original_addr) / SCREEN_WIDTH + 1);
		}
		break;
	case '\b':
		if (p_con->cursor > p_con->original_addr) {
			p_con->cursor--;
			*(p_vmem - 2) = ' ';
			*(p_vmem - 1) = DEFAULT_CHAR_COLOR;
			p_vmem -= 2;
		}
		new_row = (p_con->cursor - p_con->original_addr) / SCREEN_WIDTH;
		if(new_row != old_row){ // 如果换行了跳过行末尾的空位
			while (p_con->cursor > p_con->original_addr && *(p_vmem - 2) == ' ') {
				next_row = (p_con->cursor - p_con->original_addr - 1) / SCREEN_WIDTH;
				if (next_row != new_row) { // 防止连续删除多行
					break;
				}
				p_con->cursor--;
				p_vmem -= 2;
			}
		}
		break;
	default:
		if (p_con->cursor + 1 < p_con->original_addr + p_con->v_mem_limit) {
			*p_vmem++ = ch;
			*p_vmem++ = DEFAULT_CHAR_COLOR;
			p_con->cursor++;
		}
		break;
	}

	// 屏幕写满了，自动换行
	while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE)
	{
		scroll_screen(p_con, SCR_DN);
	}

	flush(p_con);
}

PRIVATE void flush(CONSOLE* p_con) {
	if (is_current_console(p_con)) {
		set_cursor(p_con->cursor);
		set_video_start_addr(p_con->current_start_addr);
	}
}

PRIVATE void set_cursor(unsigned int position) {
	disable_int();
	out_byte(CRTC_ADDR_REG, CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
	enable_int();
}

PRIVATE void set_video_start_addr(u32 addr) {
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}

PUBLIC void select_console(int nr_console) {
	if (nr_console < 0 || nr_console >= NR_CONSOLES) {
		return;
	}
	nr_current_console = nr_console;
	flush(&console_table[nr_current_console]);
}

PUBLIC void scroll_screen(CONSOLE* p_con, int direction) {
	if (direction == SCR_UP) {
		if (p_con->current_start_addr > p_con->original_addr) {
			p_con->current_start_addr -= SCREEN_WIDTH;
		}
	}
	else if (direction == SCR_DN) {
		if (p_con->current_start_addr + SCREEN_SIZE <
			p_con->original_addr + p_con->v_mem_limit) {
			p_con->current_start_addr += SCREEN_WIDTH;
		}
	}

	flush(p_con);
}