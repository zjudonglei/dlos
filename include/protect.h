#ifndef _PROTECT_H_
#define _PROTECT_H_

// ����������0-63λ
typedef struct s_descriptor {
	u16 limit_low; // ���޵�
	u16 base_low; // ��ַ��
	u8 base_mid; // ��ַ��
	u8 attr1; /* P(1) DPL(2) DT(1) TYPE(4) */
	u8 limit_high_attr2; /* G(1) D(1) 0(1) AVL(1) LimitHigh(4) */
	u8 base_high; // ��ַ��
}DESCRIPTOR;

typedef struct s_gate {
	u16 offset_low; // ƫ��
	u16 selector; // ѡ����
	u8 dcount;/* ���ֶ�ֻ�ڵ���������������Ч�����������
				   �����ŵ����ӳ���ʱ������Ȩ����ת���Ͷ�ջ
				   �ĸı䣬��Ҫ������ջ�еĲ������Ƶ��ڲ�
				   ��ջ����˫�ּ����ֶξ�������˵���������
				   ����ʱ��Ҫ���Ƶ�˫�ֲ�����������*/
	u8 attr; // ����
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
	u16 iobase;/* I/Oλͼ��ַ���ڻ����TSS�ν��ޣ��ͱ�ʾû��I/O���λͼ */
}TSS;

/*GDT*/
// ����������
#define INDEX_DUMMY 0
#define INDEX_FLAT_C 1
#define INDEX_FLAT_RW 2
#define INDEX_VIDEO 3
#define INDEX_TSS 4
#define INDEX_LDT_FIRST 5

// ѡ����
#define SELECTOR_DUMMY 0
#define SELECTOR_FLAT_C 0x08
#define SELECTOR_FLAT_RW 0x10
#define SELECTOR_VIDEO (0x18+3)// RPL3
#define SELECTOR_TSS 0x20
#define SELECTOR_LDT_FIRST 0x28

#define SELECTOR_KERNEL_CS SELECTOR_FLAT_C
#define SELECTOR_KERNEL_DS SELECTOR_FLAT_RW
#define SELECTOR_KERNEL_GS SELECTOR_VIDEO

#define LDT_SIZE 2 // ÿ�������и�LDT��ÿ��LDT����2��������

// ��Ȩ��
#define SA_RPL_MASK 0xFFFC
#define SA_RPL0 0
#define SA_RPL1 1
#define SA_RPL2 2
#define SA_RPL3 3

// 
#define SA_TI_MASK 0xFFFB
#define SA_TIG 0 // ȫ��
#define SA_TIL 4 // ����

// ������˵��
#define DA_32 0x4000 // 32λ��
#define DA_LIMIT_4K 0x8000 // �ν�������4k
#define DA_DPL0 0x00 // DPL=0
#define DA_DPL1 0x20 // DPL=1
#define DA_DPL2 0x40 // DPL=2
#define DA_DPL3 0x60 // DPL=3

#define DA_DR 0x90 // data+read
#define DA_DRW 0x92 // data+read+write
#define DA_DRWA 0x93 // data+read+write+accessed
#define DA_C 0x98 // code
#define DA_CR 0x9A // code+read
#define DA_CCO 0x9C // code+һ��
#define DA_CCOR 0x9E // code+һ��+read

#define DA_LDT 0x82 // �ֲ�������
#define DA_TaskGate 0x85 // ������
#define DA_386TSS 0x89 // 386����״̬��
#define DA_386CGate 0x8C // 386������
#define DA_386IGate 0x8E // 386�ж���
#define DA_386TGate 0x8F // 386�ݽ���

// ����ģʽ�µ��жϣ�20-31�ⲿ�жϣ�32~255�û��жϣ��ж��Ǹ��ţ�ѡ����ֻ��16λ�������ж��������256

// ϵͳ����
#define INT_VECTOR_DIVIDE 0x0 // ������
#define INT_VECTOR_DEBUG 0x1 // �����쳣
#define INT_VECTOR_NMI 0x2 // �������ж�
#define INT_VECTOR_BREAKPOINT 0x3 // ���Զϵ�
#define INT_VECTOR_OVERFLOW 0x4 // ���
#define INT_VECTOR_BOUNDS 0x5 // Խ��
#define INT_VECTOR_INVAL_OP 0x6 // δ����Ĳ�����
#define INT_VECTOR_COPROC_NOT 0x7 // ����ѧЭ������
#define INT_VECTOR_DOUBLE_FAULT 0x8 // ˫�ش���
#define INT_VECTOR_COPROC_SEG 0x9 // Э��������Խ��
#define INT_VECTOR_INVAL_TSS 0xA // ��ЧTSS
#define INT_VECTOR_SEG_NOT 0xB // �β�����
#define INT_VECTOR_STACK_FAULT 0xC // ��ջ�δ���
#define INT_VECTOR_PROTECTION 0xD // ���汣������
#define INT_VECTOR_PAGE_FAULT 0xE // ҳ����
#define INT_VECTOR_COPROC_ERR 0x10 // �����
// �ⲿ�ж�
#define INT_VECTOR_IRQ0 0x20 // IRQ0��Ӧ�ж�
#define INT_VECTOR_IRQ8 0x28 // IRQ8��Ӧ�ж�
// ϵͳ����
#define INT_VECTOR_SYS_CALL 0x90

// ���Ե�ַ->�����ַ
#define vir2phys(seg_base, vir) (u32)(((u32)seg_base) + (u32)(vir))

PUBLIC void init_descriptor(DESCRIPTOR* p_desc, u32 base, u32 limit, u16 attribute);

#endif // !_PROTECT_H_
