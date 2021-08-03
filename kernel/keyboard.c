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
#include "keyboard.h"
#include "keymap.h"


PRIVATE KB_INPUT kb_in;

// 状态
PRIVATE int code_with_E0 = 0;
PRIVATE int shift_l;
PRIVATE int shift_r;
PRIVATE int alt_l;
PRIVATE int alt_r;
PRIVATE int ctrl_l;
PRIVATE int ctrl_r;
PRIVATE int caps_lock;
PRIVATE int num_lock;
PRIVATE int scroll_lock;
PRIVATE int column;

PRIVATE u8 get_byte_from_kbuf();
PRIVATE void kb_wait();
PRIVATE void kb_ack();
PRIVATE void set_leds();

// 由中断读取键盘数据到缓冲区
PUBLIC void keyboard_handler(int irq) {
	u8 scan_code = in_byte(KB_DATA);

	if (kb_in.count < KB_IN_BYTES) {
		*(kb_in.p_head) = scan_code;
		kb_in.p_head++;
		if (kb_in.p_head == kb_in.buf + KB_IN_BYTES) {
			kb_in.p_head = kb_in.buf;
		}
		kb_in.count++;
	}

	key_pressed = 1;
}

PUBLIC void init_keyboard() {
	kb_in.count = 0;
	kb_in.p_head = kb_in.p_tail = kb_in.buf;

	shift_l = shift_r = 0;
	alt_l = alt_r = 0;
	ctrl_l = ctrl_r = 0;

	caps_lock = 0;
	num_lock = 1;
	scroll_lock = 0;
	set_leds();

	put_irq_handler(KEYBOARD_IRQ, keyboard_handler);
	enable_irq(KEYBOARD_IRQ);
}

// 由任务读取键盘缓冲区
PUBLIC void keyboard_read(TTY* p_tty) {
	u8 scan_code;
	//char output[2]; // 字符串以0结尾，空即是0，'0'是ascii码0x30
	int make;
	u32 key = 0;
	u32* keyrow;

	if (kb_in.count > 0) {
		code_with_E0 = 0;
		scan_code = get_byte_from_kbuf();

		if (scan_code == 0xE1) {
			int i;
			u8 pausebrk_scode[] = { 0xE1,0x1D,0x45,0xE1,0x9D,0xC5 };
			int is_pausebreak = 1;
			for (i = 1; i < 6; i++) {
				if (get_byte_from_kbuf() != pausebrk_scode[i]) {
					is_pausebreak = 0;
					break;
				}
			}
			if (is_pausebreak) {
				key = PAUSEBREAK;
			}
		}
		else if (scan_code == 0xE0) {
			scan_code = get_byte_from_kbuf();

			// printscreen按下
			if (scan_code == 0x2A) {
				if (get_byte_from_kbuf() == 0xE0) {
					if (get_byte_from_kbuf() == 0x37) {
						key = PRINTSCREEN;
						make = 1;
					}
				}
			}
			// printscreen抬起
			if (scan_code == 0xB7) {
				if (get_byte_from_kbuf() == 0xE0) {
					if (get_byte_from_kbuf() == 0xAA) {
						key = PRINTSCREEN;
						make = 0;
					}
				}
			}
			if (key == 0) {
				code_with_E0 = 1;
			}
		}
		if(key != PAUSEBREAK && key != PRINTSCREEN) {
			make = (scan_code & FLAG_BREAK ? 0 : 1);
			keyrow = &keymap[(scan_code & 0x7F) * MAP_COLS];

			column = 0;

			int caps = shift_l || shift_r;
			if (caps_lock) {
				if (keyrow[0] >= 'a' && keyrow[0] <= 'z') {
					caps = !caps;
				}
			}
			if (caps) {
				column = 1;
			}
			if (code_with_E0) {
				column = 2;
				code_with_E0 = 0;
			}

			key = keyrow[column];
			switch (key)
			{
			case SHIFT_L:
				shift_l = make; // 设置状态
				break;
			case SHIFT_R:
				shift_r = make; // 设置状态
				break;
			case CTRL_L:
				ctrl_l = make; // 设置状态
				break;
			case CTRL_R:
				ctrl_r = make; // 设置状态
				break;
			case ALT_L:
				alt_l = make; // 设置状态
				break;
			case ALT_R:
				alt_r = make; // 设置状态
				break;
			case CAPS_LOCK:
				if (make) {
					caps_lock = !caps_lock;
					set_leds();
				}
				break;
			case NUM_LOCK:
				if (make) {
					num_lock = !num_lock;
					set_leds();
				}
				break;
			case SCROLL_LOCK:
				if (make) {
					scroll_lock = !scroll_lock;
					set_leds();
				}
				break;
			default:
				break;
			}

			if (make) {
				int pad = 0;
				if (key >= PAD_SLASH && key <= PAD_9) {
					pad = 1;
					switch (key)
					{
					case PAD_SLASH:
						key = '/';
						break;
					case PAD_STAR:
						key = '*';
						break;
					case PAD_MINUS:
						key = '-';
						break;
					case PAD_PLUS:
						key = '+';
						break;
					case PAD_ENTER:
						key = ENTER;
						break;
					default:
						if (num_lock) {
							if (key >= PAD_0 && key <= PAD_9) {
								key = key - PAD_0 + '0';
							}
							else if (key == PAD_DOT) {
								key = '.';
							}
						}
						else {
							switch (key)
							{
							case PAD_HOME:
								key = HOME;
								break;
							case PAD_END:
								key = END;
								break;
							case PAD_PAGEUP:
								key = PAGEUP;
								break;
							case PAD_PAGEDOWN:
								key = PAGEDOWN;
								break;
							case PAD_INS:
								key = INSERT;
								break;
							case PAD_UP:
								key = UP;
								break;
							case PAD_DOWN:
								key = DOWN;
								break;
							case PAD_LEFT:
								key = LEFT;
								break;
							case PAD_RIGHT:
								key = RIGHT;
								break;
							case PAD_DOT:
								key = DELETE;
								break;
							default:
								break;
							}
						}
						break;
					}
				}


				// 可能同时按下了多个按键，这些按键的Flag之间不会重叠
				key |= (shift_l ? FLAG_SHIFT_L : 0);
				key |= shift_r ? FLAG_SHIFT_R : 0;
				key |= ctrl_l ? FLAG_CTRL_L : 0;
				key |= ctrl_r ? FLAG_CTRL_R : 0;
				key |= alt_l ? FLAG_ALT_L : 0;
				key |= alt_r ? FLAG_ALT_R : 0;
				key |= pad ? FLAG_PAD : 0;

				in_process(p_tty, key);
			}
		}
	}
}

PRIVATE u8 get_byte_from_kbuf() {
	u8 scan_code;
	while (kb_in.count <= 0)
	{

	}
	disable_int(); // 
	scan_code = *(kb_in.p_tail);
	kb_in.p_tail++;
	if (kb_in.p_tail == kb_in.buf + KB_IN_BYTES) {
		kb_in.p_tail = kb_in.buf;
	}
	kb_in.count--;
	enable_int();
	return scan_code;
}

//Bit7: PARITY-EVEN(P_E): 从键盘获得的数据奇偶校验错误
//Bit6: RCV - TMOUT(R_T) : 接收超时，置1
//Bit5 : TRANS_TMOUT(T_T) : 发送超时，置1
//Bit4 : KYBD_INH(K_I) : 为1，键盘没有被禁止。为0，键盘被禁止。
//Bit3 : CMD_DATA(C_D) : 为1，输入缓冲器中的内容为命令，为0，输入缓冲器中的内容为数据。
//Bit2 : SYS_FLAG(S_F) : 系统标志，加电启动置0，自检通过后置1
//Bit1 : INPUT_BUF_FULL(I_B_F) : 输入缓冲器满置1，i8042 取走后置0
//BitO : OUT_BUF_FULL(O_B_F) : 输出缓冲器满置1，CPU读取后置0
PRIVATE void kb_wait() {
	u8 kb_stat;
	do {
		kb_stat = in_byte(KB_CMD);
	} while (kb_stat & 0x02);
}

PRIVATE void kb_ack() {
	u8 kb_read;

	do
	{
		kb_read = in_byte(KB_DATA);
	} while (kb_read != KB_ACK);
}

PRIVATE void set_leds() {
	u8 leds = (caps_lock << 2) | (num_lock << 1) | scroll_lock;

	kb_wait();
	out_byte(KB_DATA, LED_CODE);
	kb_ack();

	kb_wait();
	out_byte(KB_DATA, leds);
	kb_ack();
}