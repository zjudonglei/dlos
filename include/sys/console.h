#ifndef _CONSOLE_H_
#define _CONSOLE_H_

typedef struct s_console {
	unsigned int current_start_addr; // 当前显示位置
	unsigned int original_addr; // 控制台显示位置
	unsigned int v_mem_limit; // 控制台显存大小
	unsigned int cursor; // 当前光标位置
}CONSOLE;

#define SCR_UP 1
#define SCR_DN -1

#define SCREEN_SIZE (80*25)
#define SCREEN_WIDTH 80

#define DEFAULT_CHAR_COLOR (MAKE_COLOR(BLACK, WHITE)) // 黑底白字
#define GRAY_CHAR (MAKE_COLOR(BLACK, BLACK) | BRIGHT)
#define RED_CHAR (MAKE_COLOR(BLUE, RED) | BRIGHT)

#endif // !_CONSOLE_H_
