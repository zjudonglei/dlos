#ifndef _TTY_H_
#define _TTY_H_

#define TTY_IN_BYTES 256
#define TTY_OUT_BUF_LEN 2

struct s_console;

typedef struct s_tty
{
	// 各个控制台的缓冲区和键盘缓冲区是不一样的
	// 控制台缓冲区存放的是键盘缓冲区解析后的字符串
	u32 in_buf[TTY_IN_BYTES];
	u32* p_inbuf_head;
	u32* p_inbuf_tail;
	int inbuf_count;

	int tty_caller;
	int tty_procnr;
	void* tty_req_buf; // 请求者的buf
	int tty_left_cnt; // 请求的字符数量
	int tty_trans_cnt; // 已经传入的字符数量

	struct s_console* p_console;
}TTY;

#endif // !_TTY_H_
