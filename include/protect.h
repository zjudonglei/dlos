#ifndef _PROTECT_H_
#define _PROTECT_H_

// 描述符，从0-63位
typedef struct s_descriptor {
	u16 limit_low; // 界限低
	u16 base_low; // 基址低
	u8 base_mid; // 基址中
	u8 attr1; /* P(1) DPL(2) DT(1) TYPE(4) */
	u8 limit_high_attr2; /* G(1) D(1) 0(1) AVL(1) LimitHigh(4) */
	u8 base_high; // 基址高
}DESCRIPTOR;

typedef struct s_gate {
	u16 offset_low; // 偏移
	u16 selector; // 选择子
	u8 dcount;/* 该字段只在调用门描述符中有效。如果在利用
				   调用门调用子程序时引起特权级的转换和堆栈
				   的改变，需要将外层堆栈中的参数复制到内层
				   堆栈。该双字计数字段就是用于说明这种情况
				   发生时，要复制的双字参数的数量。*/
	u8 attr; // 属性
	u16 offset_high;
}GATE;

typedef struct s_tss {
	u32 backlink;
	u32 esp0;
	u32 ss0;
	u32 esp1;
	u32 ss1;
	u32 esp2;
	u32 ss2;
	u32 cr3;
	u32 eip;
	u32 eflags;
	u32 eax;
	u32 ecx;
	u32 edx;
	u32 ebx;
	u32 esp;
	u32 ebp;
	u32 esi;
	u32 edi;
	u32 es;
	u32 cs;
	u32 ss;
	u32 ds;
	u32 fs;
	u32 gs;
	u32 ldt;
	u16 trap;
	u16 iobase;/* I/O位图基址大于或等于TSS段界限，就表示没有I/O许可位图 */
}TSS;

/*GDT*/
// 描述符索引
#define INDEX_DUMMY 0
#define INDEX_FLAT_C 1
#define INDEX_FLAT_RW 2
#define INDEX_VIDEO 3
#define INDEX_TSS 4
#define INDEX_LDT_FIRST 5

// 选择子
#define SELECTOR_DUMMY 0
#define SELECTOR_FLAT_C 0x08
#define SELECTOR_FLAT_RW 0x10
#define SELECTOR_VIDEO (0x18+3)// RPL3
#define SELECTOR_TSS 0x20
#define SELECTOR_LDT_FIRST 0x28

#define SELECTOR_KERNEL_CS SELECTOR_FLAT_C
#define SELECTOR_KERNEL_DS SELECTOR_FLAT_RW
#define SELECTOR_KERNEL_GS SELECTOR_VIDEO

#define LDT_SIZE 2 // 每个任务都有个LDT，每个LDT里有2个描述符

// 特权级
#define SA_RPL_MASK 0xFFFC
#define SA_RPL0 0
#define SA_RPL1 1
#define SA_RPL2 2
#define SA_RPL3 3

// 
#define SA_TI_MASK 0xFFFB
#define SA_TIG 0 // 全局
#define SA_TIL 4 // 本地

// 描述符说明
#define DA_32 0x4000 // 32位段
#define DA_LIMIT_4K 0x8000 // 段界限粒度4k
#define DA_DPL0 0x00 // DPL=0
#define DA_DPL1 0x20 // DPL=1
#define DA_DPL2 0x40 // DPL=2
#define DA_DPL3 0x60 // DPL=3

#define DA_DR 0x90 // data+read
#define DA_DRW 0x92 // data+read+write
#define DA_DRWA 0x93 // data+read+write+accessed
#define DA_C 0x98 // code
#define DA_CR 0x9A // code+read
#define DA_CCO 0x9C // code+一致
#define DA_CCOR 0x9E // code+一致+read

#define DA_LDT 0x82 // 局部描述符
#define DA_TaskGate 0x85 // 任务门
#define DA_386TSS 0x89 // 386任务状态段
#define DA_386CGate 0x8C // 386调用门
#define DA_386IGate 0x8E // 386中断门
#define DA_386TGate 0x8F // 386陷进门

// 保护模式下的中断，20-31外部中断，32~255用户中断，中断是个门，选择子只有16位，所以中断数量最大256

// 系统定义
#define INT_VECTOR_DIVIDE 0x0 // 除法错
#define INT_VECTOR_DEBUG 0x1 // 调试异常
#define INT_VECTOR_NMI 0x2 // 非屏蔽中断
#define INT_VECTOR_BREAKPOINT 0x3 // 调试断点
#define INT_VECTOR_OVERFLOW 0x4 // 溢出
#define INT_VECTOR_BOUNDS 0x5 // 越界
#define INT_VECTOR_INVAL_OP 0x6 // 未定义的操作码
#define INT_VECTOR_COPROC_NOT 0x7 // 无数学协处理器
#define INT_VECTOR_DOUBLE_FAULT 0x8 // 双重错误
#define INT_VECTOR_COPROC_SEG 0x9 // 协处理器段越界
#define INT_VECTOR_INVAL_TSS 0xA // 无效TSS
#define INT_VECTOR_SEG_NOT 0xB // 段不存在
#define INT_VECTOR_STACK_FAULT 0xC // 堆栈段错误
#define INT_VECTOR_PROTECTION 0xD // 常规保护错误
#define INT_VECTOR_PAGE_FAULT 0xE // 页错误
#define INT_VECTOR_COPROC_ERR 0x10 // 浮点错
// 外部中断
#define INT_VECTOR_IRQ0 0x20 // IRQ0对应中断
#define INT_VECTOR_IRQ8 0x28 // IRQ8对应中断
// 系统调用
#define INT_VECTOR_SYS_CALL 0x90

// 线性地址->物理地址
#define vir2phys(seg_base, vir) (u32)(((u32)seg_base) + (u32)(vir))

PUBLIC void init_descriptor(DESCRIPTOR* p_desc, u32 base, u32 limit, u16 attribute);

#endif // !_PROTECT_H_
