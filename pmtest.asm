%include "pm.inc"
	org 0100h
	jmp LABEL_BEGIN
[SECTION .gdt] ; 将以下大代码装到gdt段中
;Descriptor是pm.inc中定义的宏		段地址		段界限				属性
LABEL_GDT:			Descriptor	0,			0,					0 			;定义1个空GDT段
LABEL_DESC_NORMAL:	Descriptor 	0,			0ffffh,				DA_DRW		;
LABEL_DESC_CODE32:	Descriptor	0,			SegCode32Len -1,	DA_C+DA_32	;定义1个GDT段
LABEL_DESC_CODE16:	Descriptor 	0,			0ffffh,				DA_C 		;16位代码段
LABEL_DESC_DATA:	Descriptor 	0,			DataLen-1,			DA_DRW
LABEL_DESC_STACK: 	Descriptor 	0,			TopOfStack,			DA_DRWA+DA_32;32位栈
LABEL_DESC_TEST:	Descriptor 	0500000h,	0ffffh,				DA_DRW
LABEL_DESC_VIDEO:	Descriptor	0B8000h,	0ffffh,				DA_DRW 		;显存段，见内存分布图

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
SelectorNormal	equ	LABEL_DESC_NORMAL 	- LABEL_GDT
SelectorCode32	equ	LABEL_DESC_CODE32 		- LABEL_GDT
SelectorCode16	equ	LABEL_DESC_CODE16 		- LABEL_GDT
SelectorData	equ	LABEL_DESC_DATA 		- LABEL_GDT
SelectorStack	equ	LABEL_DESC_STACK 		- LABEL_GDT
SelectorTest	equ	LABEL_DESC_TEST 		- LABEL_GDT
SelectorVideo	equ	LABEL_DESC_VIDEO 		- LABEL_GDT

[SECTION .data1] ;数据段
ALIGN 32
[BITS 32]
LABEL_DATA:
	SPValueInRealMode dw 0
	PMMessage: dd "In Protect Mode now. ^-^",0 
	OffsetPMMessage equ PMMessage - $$
	StrTest: dd "ABCDEFGHIJKLMNOPQRSTUVWXYZ",0
	OffsetStrTest equ StrTest - $$
	DataLen equ $-LABEL_DATA

[SECTION .gs]
ALIGN 32
[BITS 32]
LABEL_STACK:
	times 512 db 0
	TopOfStack equ $ - LABEL_STACK - 1

[SECTION .s16] ;将下面代码装到16位的段里
[BITS 16] ;16位编译模式
LABEL_BEGIN:
	mov ax,cs
	mov ds,ax
	mov es,ax
	mov ss,ax
	mov sp,0100h ; 未知


;段间转移直接寻址是5位

;段址高8位
;段址低8位
;偏移高8位
;偏移低8位
;操作数

;存储段址
	mov [LABEL_GO_BACK_TO_REAL+3],ax 
	mov [SPValueInRealMode],sp

	; 返回16位的代码
	mov ax,cs 
	movzx eax,ax;其余位补0
	shl eax,4
	add eax,LABEL_SEG_CODE16
	mov word [LABEL_DESC_CODE16+2],ax 
	shr eax,16
	mov byte [LABEL_DESC_CODE16+4],al 
	mov byte [LABEL_DESC_CODE16+7],ah 


	;此处计算出了 LABEL_SEG_CODE32 的真实物理地址,保存于 eax 中.
	xor eax,eax
	mov ax,cs
	shl eax,4 ;左移四位，相当于乘以16，实模式下计算物理地址
	add eax,LABEL_SEG_CODE32 ;段+偏移地址, 计算出其真实物理首地址，段是cs

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

	; 初始化数据段描述符
	xor eax,eax
	mov ax,ds
	shl eax,4 ;左移四位，相当于乘以16，实模式下计算物理地址
	add eax,LABEL_DATA ;段+偏移地址, 计算出其真实物理首地址，段是cs
	mov word [LABEL_DESC_DATA+2],ax 
	shr eax,16
	mov byte [LABEL_DESC_DATA+4],al 
	mov byte [LABEL_DESC_DATA+7],ah 

	; 初始化堆栈段描述符
	xor eax,eax
	mov ax,ds
	shl eax,4 ;左移四位，相当于乘以16，实模式下计算物理地址
	add eax,LABEL_STACK ;段+偏移地址, 计算出其真实物理首地址，段是cs
	mov word [LABEL_DESC_STACK+2],ax 
	shr eax,16
	mov byte [LABEL_DESC_STACK+4],al 
	mov byte [LABEL_DESC_STACK+7],ah 

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


LABEL_REAL_ENTRY:
	mov ax,cs ;CS段寄存器已经切换到实模式
	mov ds,ax 
	mov es,ax 
	mov ss,ax 
	mov sp,[SPValueInRealMode]
	in al,92h
	and al,11111101b
	out 92h,al
	sti
	;DOS下结束程序并返回
	mov ax,4c00h
	int 21h


[SECTION .s32] ;将下面代码装到32位的段里
[BITS 32] ;32位编译模式

LABEL_SEG_CODE32:
	mov ax,SelectorData  
	mov ds,ax 
	mov ax, SelectorTest  
	mov es,ax 
	mov ax,SelectorVideo
	mov gs,ax
	mov ax, SelectorStack  
	mov ss,ax
	mov esp,TopOfStack

	mov ah,0ch;0000: 黑底    1100: 红字
	xor esi,esi ;放置源
	xor edi,edi ;放置目标
	mov esi,OffsetPMMessage
	mov edi,(80*10+0)*2;10行0列，1个字符占两个字节
	cld

.1:
	lodsb ; 取SI中的字节
	test al,al ;Test对两个参数(目标，源)执行AND逻辑操作，并根据结果设置标志寄存器，结果本身不会保存。
	jz .2 ; 参见数据段字符串定义，末尾都有个0
	mov [gs:edi],ax 
	add edi,2 ; 空1个字符显示
	jmp .1
.2: ;显示完毕
	call DispReturn ; 换行
	call TestRead
	call TestWrite
	call TestRead
	jmp SelectorCode16:0 ;回到16位

;	mov edi,(80*11+79)*2 ;屏幕的第11行，第79列
;	mov ah,0ch; 0000: 黑底    1100: 红字
;	mov al,'P'
;	mov [gs:edi],ax ; 将SelectorVideo装到gs，并偏移edi
;	jmp $

TestRead:
	xor esi,esi 
	mov ecx,8  ;循环8次
.loop:
	mov al,[es:esi] ;es指向test段选择子，test段基地址在5M处
	call DispAL
	inc esi
	loop .loop ;循环8次退出
	call DispReturn
	ret

TestWrite:
	push esi
	push edi
	xor esi,esi  
	xor edi,edi  
	mov esi, OffsetStrTest
	cld ; 方向标志位DF复位，0向高地址增加，1向低地址减小
.1:
	lodsb ; 从SI中取一个byte
	test al,al 
	jz .2
	mov [es:edi],al
	inc edi
	jmp .1
.2:
	pop edi
	pop esi
	ret

; 显示 AL 中的数字 用16进制显示
; 默认地:
;	数字已经存在 AL 中
;	edi 始终指向要显示的下一个字符的位置
; 被改变的寄存器:
;	ax, edi

DispAL:
	push ecx 
	push edx
	mov ah,0ch; 0000: 黑底    1100: 红字
	mov dl,al  
	shr al,4;先高4位
	mov ecx,2; loop两次
.begin:
	and al,01111b
	cmp al,9;无符号大于则跳转
	ja .1
	add al,'0'
	jmp .2
.1:
	sub al,0AH
	add al, 'A'
.2:
	mov [gs:edi],ax;显示
	add edi,2
	mov al,dl  ;开始低4位
	loop .begin
	add edi,2
	pop edx
	pop ecx
	ret

;80*25模式下，一行80个字符，160个字节，总共25行
DispReturn:
	push eax
	push ebx
	mov eax,edi  
	mov bl,160
	div bl; al = ax / 160, ah = ax mod 160，al表示行，ah是余数表示列
	and eax,0FFH ;丢弃ah，只保留al，即保留行
	inc eax ;行增加1
	mov bl,160
	mul bl;下1行0列开始显示
	mov edi,eax
	pop ebx
	pop eax
	ret

SegCode32Len equ $ - LABEL_SEG_CODE32

;16位保护模式代码段，由保护模式下的32位代码段跳入
[SECTION .s16code]
ALIGN 32
[BITS 16]
LABEL_SEG_CODE16:
	; 让寄存器满足16位的要求
	; 将DS，ES，FS ，GS ，SS寄存器的值赋为规范段
	mov ax,SelectorNormal  
	mov ds,ax  
	mov es,ax  
	mov fs,ax  
	mov gs,ax   
	mov ss,ax  
	; 关闭PE位，只能在16位模式更改
	mov eax,cr0
	and al,11111110b
	mov cr0,eax
LABEL_GO_BACK_TO_REAL:
	jmp 0:LABEL_REAL_ENTRY ; 回到16位代码段，段址在LABEL_BEGIN跳入32位之前预先设置

Code16Len equ $-LABEL_SEG_CODE16