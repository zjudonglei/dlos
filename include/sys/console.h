#ifndef _CONSOLE_H_
#define _CONSOLE_H_

typedef struct s_console {
	unsigned int current_start_addr; // ��ǰ��ʾλ��
	unsigned int original_addr; // ����̨��ʾλ��
	unsigned int v_mem_limit; // ����̨�Դ��С
	unsigned int cursor; // ��ǰ���λ��
}CONSOLE;

#define SCR_UP 1
#define SCR_DN -1

#define SCREEN_SIZE (80*25)
#define SCREEN_WIDTH 80

#define DEFAULT_CHAR_COLOR (MAKE_COLOR(BLACK, WHITE)) // �ڵװ���
#define GRAY_CHAR (MAKE_COLOR(BLACK, BLACK) | BRIGHT)
#define RED_CHAR (MAKE_COLOR(BLUE, RED) | BRIGHT)

#endif // !_CONSOLE_H_
