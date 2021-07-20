// kliba.asm
PUBLIC void disp_str(char* info);
PUBLIC void disp_color_str(char* info, int color);
PUBLIC void out_byte(u16 port, u8 value);
PUBLIC u8 in_byte(u16 port);
PUBLIC void enable_irq(int irq);
PUBLIC void disable_irq(int irq);
PUBLIC void enable_int();
PUBLIC void disable_int();

// klib.c
PUBLIC char* itoa(char* str, int num);
PUBLIC void disp_int(int input);
PUBLIC void delay(int time);

// protect.c
PUBLIC void init_prot();
PUBLIC u32 seg2phys(u16 seg);

// i8259.c
PUBLIC void	init_8259A();
PUBLIC void put_irq_handler(int irq, irq_handler handler);
PUBLIC void spurious_irq(int irq);

// clock.c
PUBLIC void init_clock();
PUBLIC void milli_delay(int mill_sec);

// keyboard.c
PUBLIC void init_keyboard();
PUBLIC void keyboard_read();

// tty.c
PUBLIC void task_tty();
PUBLIC void in_process(TTY* p_tty, u32 key);
PUBLIC int sys_write(char* buf, int len, PROCESS* p_proc);

// kernel.asm
void restart();

// main.c
void TestA();
void TestB();
void TestC();

// proc.c
PUBLIC void schedule(); // 进程调度
PUBLIC int sys_get_ticks(); // 系统方法
PUBLIC int sys_get_info();

// syscall.asm
PUBLIC int get_ticks();
PUBLIC void write(char* buf, int len);
PUBLIC int get_info();

// kernel.asm
PUBLIC void sys_call(); // 系统中断

// console.c
PUBLIC void init_screen(TTY* p_tty);
PUBLIC int is_current_console(CONSOLE* p_con);
PUBLIC void out_char(CONSOLE* p_con, char ch);
PUBLIC void select_console(int nr_console);
PUBLIC void scroll_screen(CONSOLE* p_con, int direction);

// printf.c
PUBLIC int printf(const char* fmt, ...);
// vsprintf.c
PUBLIC int vsprintf(char* buf, const char* fmt, va_list args);