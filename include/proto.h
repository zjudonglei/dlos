PUBLIC void out_byte(u16 port, u8 value);
PUBLIC u8 in_byte(u16 port);
PUBLIC void disp_str(char* info);
PUBLIC void disp_color_str(char* info, int color);
PUBLIC void disp_int(int input);
PUBLIC void disable_irq(int irq);
PUBLIC void enable_irq(int irq);

PUBLIC void init_prot();
PUBLIC void	init_8259A();
PUBLIC u32 seg2phys(u16 seg);

PUBLIC void delay(int time);
PUBLIC void milli_delay(int mill_sec);

void restart();// kernel.asm

void TestA();
void TestB();
void TestC();

PUBLIC void put_irq_handler(int irq, irq_handler handler);
PUBLIC void spurious_irq(int irq);
PUBLIC void clock_handler(int irq);

// 系统调用
PUBLIC int sys_get_ticks(); // 系统方法

PUBLIC void schedule(); // 进程调度

PUBLIC void sys_call(); // 系统中断
PUBLIC int get_ticks(); // 用户接口