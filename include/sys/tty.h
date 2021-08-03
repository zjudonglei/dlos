#ifndef _TTY_H_
#define _TTY_H_

#define TTY_IN_BYTES 256
#define TTY_OUT_BUF_LEN 2

struct s_console;

typedef struct s_tty
{
	// ��������̨�Ļ������ͼ��̻������ǲ�һ����
	// ����̨��������ŵ��Ǽ��̻�������������ַ���
	u32 in_buf[TTY_IN_BYTES];
	u32* p_inbuf_head;
	u32* p_inbuf_tail;
	int inbuf_count;

	int tty_caller;
	int tty_procnr;
	void* tty_req_buf; // �����ߵ�buf
	int tty_left_cnt; // ������ַ�����
	int tty_trans_cnt; // �Ѿ�������ַ�����

	struct s_console* p_console;
}TTY;

#endif // !_TTY_H_
