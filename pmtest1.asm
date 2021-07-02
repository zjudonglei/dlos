%include "pm.inc"
org 07c00h
	jmp LABEL_BEGIN
[SECTION .gdt] ; 将以下大代码装到gdt段中
;Descriptor是pm.inc中定义的宏		段地址		段界限				属性
LABEL_GDT:			Descriptor	0,			0,					0 ;定义1个GDT段
LABEL_DESC_CODE32:	Descriptor	0,			SegCode32Len -1,	DA_C+DA_32 ;定义1个GDT段
LABEL_DESC_VIDEO:	Descriptor	0B8000h,	0ffffh,				DA_DRW ;显存段，见内存分布图

GdtLen	equ $ - LABEL_GDT ;GDT长度

;全局描述符表寄存器GDTR，48位寄存器
;中断描述符表寄存器IDTR
;局部描述符表寄存器LDTR，16位寄存器
;任务寄存器TR，16位寄存器
;共6 byte, 结构与寄存器 gdtr 一致. 用于保存 GDT 表信息, 通过 lgdt 加载.
;后3-6 byte内容, 在下面的代码中会计算出并填入.
GdtPtr	dw	GdtLen - 1 ;GDT界限，低16位
		dd 	0 ;GDT基址，高32位

;选择子，偏移量
;LABEL_GDT是8位，假如1这个位置
;下个选择子的位置就是9，再下个就是17
;所以选择子相减就是8的倍数，类似xxxx:x000这样，低3位是0，被用来存储一些信息，详见pm.inc
SelectorCode32	equ	LABEL_DESC_CODE32 - LABEL_GDT
SelectorVideo	equ	LABEL_DESC_VIDEO - LABEL_GDT

[SECTION .s16] ;将下面代码装到16位的段里
[BITS 16] ;16位编译模式
LABEL_BEGIN:
	mov ax,cs
	mov ds,ax
	mov es,ax
	mov ss,ax
	mov sp,0100h ; 未知

	;此处计算出了 LABEL_SEG_CODE32 的真实物理地址,保存于 eax 中.
	xor eax,eax
	mov ax,cs
	shl eax,4 ;左移四位，相当于乘以16，实模式下计算物理地址
	add eax,LABEL_SEG_CODE32 ;段+偏移地址, 计算出其真实物理首地址

	;此处将前面计算出的真实物理地址, 按规则放入GDT相应的段基址位置.
	;注: 保护模式寻址步骤:
    ;   1). Selector 选择子 => 取得对应 GDT 的位置
    ;   2). GDT 的 段基址 => 取得 物理首地址
    ;   3). 物理首地址 + 偏移量 => 真实地址
    ; 2,3,4,7是基址位，详见描述符
	mov word [LABEL_DESC_CODE32+2],ax 
	shr eax,16
	mov byte [LABEL_DESC_CODE32+4],al 
	mov byte [LABEL_DESC_CODE32+7],ah 
	; 为加载 GDTR 作准备
	xor eax,eax
	mov ax,ds;ds中存放的是cs值
	shl eax,4
	add eax,LABEL_GDT;左移四位，相当于乘以16，实模式下计算物理地址
	mov dword [GdtPtr+2],eax;GdtPtr低2位是界限，高4位是基址，参见GdtPtr定义dd 0，界限已经有定义，对应gdtr寄存器

	lgdt [GdtPtr];将GdtPtr加载gdtr寄存器

	cli ; 关中断，32位下中断模式有变
	in al,92h ; 打开A20地址线，开启32位模式，没找到各个端口的定义，抱歉
	or al,00000010b
	out 92h,al
;CR0	PG	0000000000000000	ET	TS	EM	MP	PE
;位		31	5 ～ 30				4	3	2	1	0

;PG	PE	处理器工作方式
;0	0	实模式
;0	1	保护模式，禁用分页机制
;1	0	非法组合
;1	1	保护方式，启用分页机制
	; 准备切换到保护模式
	mov eax,cr0
	or eax,1
	mov cr0, eax
	; 进入保护模式，每当把一个选择子装入到某个段寄存器时，处理器自动从描述符表中取出相应的描述符，把描述符中的信息保存到对应的高速缓冲寄存器中。此后对该段访问时，处理器都使用对应高速缓冲寄存器中的描述符信息，而不用再从描述符表中取描述符。 
	jmp dword SelectorCode32:0	; 执行这一句会把 SelectorCode32 装入 cs,
					; 并跳转到 Code32Selector:0  处

[SECTION .s32] ;将下面代码装到32位的段里
[BITS 32] ;32位编译模式

LABEL_SEG_CODE32:
	mov ax,SelectorVideo
	mov gs,ax
	mov edi,(80*11+79)*2 ;屏幕的第11行，第79列
	mov ah,0ch
	mov al,'P'
	mov [gs:edi],ax ; 将SelectorVideo装到gs，并偏移edi
	jmp $

SegCode32Len equ $ - LABEL_SEG_CODE32