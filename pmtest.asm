%include "pm.inc"
; %include "lib.inc" ;崩

PageDirBase0 equ 200000h
PageTblBase0 equ 201000h
PageDirBase1 equ 210000h
PageTblBase1 equ 211000h

LinearAddrDemo equ 00401000h
ProcFoo equ 00401000h
ProcBar equ 00501000h
ProcPagingDemo equ 00301000h

org 0100h
	jmp LABEL_BEGIN
[SECTION .gdt] ; 将以下大代码装到gdt段中
;Descriptor是pm.inc中定义的宏		段地址		段界限				属性
LABEL_GDT:			Descriptor	0,			0,					0 			;定义1个空GDT段
LABEL_DESC_NORMAL:	Descriptor 	0,			0ffffh,				DA_DRW		;
; LABEL_DESC_PAGE_DIR:	Descriptor 	PageDirBase,	4095,		DA_DRW
; LABEL_DESC_PAGE_TBL:	Descriptor 	PageTblBase,	1023,		DA_DRW|DA_LIMIT_4K;此时G位置DA_LIMIT_4K，界限粒度为4K，所以1023表示0－1023的1024个4K 
; LABEL_DESC_PAGE_TBL:	Descriptor 	PageTblBase,	16*4096 - 1,		DA_DRW;16个页即可
LABEL_DESC_FLAT_C: Descriptor 	0,			0ffffh,				DA_CR|DA_32|DA_LIMIT_4K;代码段
LABEL_DESC_FLAT_RW: Descriptor 	0,			0ffffh,				DA_DRW|DA_LIMIT_4K;数据段
LABEL_DESC_CODE32:	Descriptor	0,			SegCode32Len -1,	DA_CR+DA_32	;非一致,定义1个GDT段
LABEL_DESC_CODE16:	Descriptor 	0,			0ffffh,				DA_C 		;非一致,16位代码段
LABEL_DESC_CODE_DEST: Descriptor 	0,		SegCodeDestLen - 1,		DA_C+DA_32;非一致
LABEL_DESC_CODE_RING3: Descriptor 	0,		SegCodeRing3Len - 1,	DA_C+DA_32+DA_DPL3 ; 非一致
LABEL_DESC_DATA:	Descriptor 	0,			DataLen-1,			DA_DRW
LABEL_DESC_STACK: 	Descriptor 	0,			TopOfStack,			DA_DRWA+DA_32;32位栈
LABEL_DESC_STACK3: 	Descriptor 	0,			TopOfStack3,		DA_DRWA+DA_32+DA_DPL3;32位栈
LABEL_DESC_LDT:		Descriptor 	0,			LDTLen - 1,			DA_LDT
; LABEL_DESC_TEST:	Descriptor 	0500000h,	0ffffh,				DA_DRW
LABEL_DESC_TSS:		Descriptor 	0,			TSSLen -1,			DA_386TSS
LABEL_DESC_VIDEO:	Descriptor	0B8000h,	0ffffh,				DA_DRW+DA_DPL3 		;显存段，见内存分布图

LABEL_CALL_GATE_TEST: Gate SelectorCodeDest,0,0,DA_386CGate+DA_DPL3

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
; SelectorPageDir	equ	LABEL_DESC_PAGE_DIR 	- LABEL_GDT
; SelectorPageTbl	equ	LABEL_DESC_PAGE_TBL 	- LABEL_GDT
SelectorFlatC	equ	LABEL_DESC_FLAT_C 	- LABEL_GDT
SelectorFlatRW	equ	LABEL_DESC_FLAT_RW 	- LABEL_GDT
SelectorCode32	equ	LABEL_DESC_CODE32 		- LABEL_GDT
SelectorCode16	equ	LABEL_DESC_CODE16 		- LABEL_GDT
SelectorCodeDest	equ	LABEL_DESC_CODE_DEST 		- LABEL_GDT
SelectorCodeRing3	equ	LABEL_DESC_CODE_RING3 		- LABEL_GDT + SA_RPL3
SelectorData	equ	LABEL_DESC_DATA 		- LABEL_GDT
SelectorStack	equ	LABEL_DESC_STACK 		- LABEL_GDT
SelectorStack3	equ	LABEL_DESC_STACK3 		- LABEL_GDT + SA_RPL3
SelectorLDT		equ	LABEL_DESC_LDT 			- LABEL_GDT
; SelectorTest	equ	LABEL_DESC_TEST 		- LABEL_GDT
SelectorTSS		equ	LABEL_DESC_TSS 		- LABEL_GDT
SelectorVideo	equ	LABEL_DESC_VIDEO 		- LABEL_GDT

SelectorCallGateTest	equ	LABEL_CALL_GATE_TEST 		- LABEL_GDT + SA_RPL3

[SECTION .data1] ;数据段
ALIGN 32
[BITS 32]
LABEL_DATA:
_BootMessage:		db	"Hello, OS world!", 0AH, 0
_szPMMessage: db "In Protect Mode now. ^-^",0AH,0AH,0 
_szMemChkTitle:	db "BaseAddrL BaseAddrH LengthLow LengthHigh Type", 0AH, 0
_szRAMSize db "RAM size:", 0 
_szReturn			db	0Ah, 0
_wSPValueInRealMode dw 0
_dwMCRNumber: dd 0
_dwDispPos: dd (80*6 + 0) * 2
_dwMemSize: dd 0
_ARDStruct:
	_dwBaseAddrLow: dd 0
	_dwBaseAddrHigh: dd 0
	_dwLengthLow: dd 0
	_dwLengthHigh: dd 0
	_dwType: dd 0
_PageTableNumber dd 0

_MemChkBuf: times 256 db 0

;保护模式用段+偏移
BootMessage equ _BootMessage - $$
szPMMessage equ _szPMMessage - $$
szMemChkTitle equ _szMemChkTitle - $$
szRAMSize equ _szRAMSize - $$
szReturn		equ	_szReturn	- $$
wSPValueInRealMode equ _wSPValueInRealMode - $$
dwMCRNumber equ _dwMCRNumber - $$
dwDispPos equ _dwDispPos - $$
dwMemSize equ _dwMemSize - $$
ARDStruct equ _ARDStruct - $$
	dwBaseAddrLow equ _dwBaseAddrLow - $$
	dwBaseAddrHigh equ _dwBaseAddrHigh - $$
	dwLengthLow equ _dwLengthLow - $$
	dwLengthHigh equ _dwLengthHigh - $$
	dwType equ _dwType - $$
PageTableNumber equ _PageTableNumber -$$
MemChkBuf equ _MemChkBuf - $$
	
	StrTest: db "ABCDEFGHIJKLMNOPQRSTUVWXYZ",0
	OffsetStrTest equ StrTest - $$
	
DataLen equ $-LABEL_DATA

;全局堆栈段
[SECTION .gs]
ALIGN 32
[BITS 32]
LABEL_STACK:
	times 512 db 0
	TopOfStack equ $ - LABEL_STACK - 1

; 堆栈段ring3
[SECTION .s3]
ALIGN 32
[BITS 32]
LABEL_STACK3:
	times 512 db 0
	TopOfStack3 equ $ - LABEL_STACK3 - 1

[SECTION .tss]
ALIGN 32
[BITS 32]
LABEL_TSS:
	DD 0; back
	DD TopOfStack ;0级堆栈
	DD SelectorStack;
	DD 0; 1级堆栈
	DD 0;
	DD 0; 2级堆栈
	DD 0;
	DD 0; CR3
	DD 0; EIP
	DD 0; EFLAGS
	DD 0; EAX
	DD 0; ECX
	DD 0; EDX
	DD 0; EBX
	DD 0; ESP
	DD 0; EBP
	DD 0; ESI
	DD 0; EDI
	DD 0; ES
	DD 0; CS
	DD 0; SS
	DD 0; DS
	DD 0; FS
	DD 0; GS
	DD 0; LDT
	DW 0; 调试陷进标志
	DW $-LABEL_TSS+2; I/O位图基址
	DB 0ffh; I/O位图结束标志
TSSLen equ $ - LABEL_TSS


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
	mov [LABEL_GO_BACK_TO_REAL+3],ax ;把段址存在高16位
	mov [_wSPValueInRealMode],sp

	; 使用中断获取内存
	mov ebx, 0
	mov di, _MemChkBuf; es:di指向地址范围描述符ARDS
.loop:
	mov eax, 0E820h ; 功能号
	mov ecx, 20 ; 每次BIOS填充的大小
	mov edx, 0534D4150h ; 标志
	int 15h
	jc LABEL_MEM_CHK_FAIL ; 判断CF位是否有错误
	add di,20; 填充下一个空间
	inc dword [_dwMCRNumber]
	cmp ebx, 0 ; ebx结束标志
	jne .loop
	jmp LABEL_MEM_CHK_OK
LABEL_MEM_CHK_FAIL:
	mov dword [_dwMCRNumber],0 
LABEL_MEM_CHK_OK:

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

	;特权级3段的代码
    xor eax,eax
    mov ax,ds
    shl eax,4 
    add eax,LABEL_CODE_RING3
    mov word [LABEL_DESC_CODE_RING3+2],ax 
    shr eax,16
    mov byte [LABEL_DESC_CODE_RING3+4],al 
    mov byte [LABEL_DESC_CODE_RING3+7],ah 
	
	;测试调用们段描述符
    xor eax,eax
    mov ax,ds
    shl eax,4 
    add eax,LABEL_SEG_CODE_DEST
    mov word [LABEL_DESC_CODE_DEST+2],ax 
    shr eax,16
    mov byte [LABEL_DESC_CODE_DEST+4],al 
    mov byte [LABEL_DESC_CODE_DEST+7],ah 

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

	; 初始化堆栈段描述符ring3
	xor eax,eax
	mov ax,ds
	shl eax,4 ;左移四位，相当于乘以16，实模式下计算物理地址
	add eax,LABEL_STACK3 ;段+偏移地址, 计算出其真实物理首地址，段是cs
	mov word [LABEL_DESC_STACK3+2],ax 
	shr eax,16
	mov byte [LABEL_DESC_STACK3+4],al 
	mov byte [LABEL_DESC_STACK3+7],ah 

	; 初始化LDT在GDT中的描述符，LDT是二级段，自身要在GDT中声明，里面又可以存放很多段
	xor eax,eax
	mov ax,ds
	shl eax,4 ;左移四位，相当于乘以16，实模式下计算物理地址
	add eax,LABEL_LDT ;段+偏移地址, 计算出其真实物理首地址，段是cs
	mov word [LABEL_DESC_LDT+2],ax 
	shr eax,16
	mov byte [LABEL_DESC_LDT+4],al 
	mov byte [LABEL_DESC_LDT+7],ah 

	; 初始化LDT中的描述符
	xor eax,eax
	mov ax,ds
	shl eax,4 ;左移四位，相当于乘以16，实模式下计算物理地址
	add eax,LABEL_CODE_A ;段+偏移地址, 计算出其真实物理首地址，段是cs
	mov word [LABEL_LDT_DESC_CODEA+2],ax 
	shr eax,16
	mov byte [LABEL_LDT_DESC_CODEA+4],al 
	mov byte [LABEL_LDT_DESC_CODEA+7],ah 

	; TSS
    xor eax,eax
    mov ax,ds
    shl eax,4 
    add eax,LABEL_TSS
    mov word [LABEL_DESC_TSS+2],ax 
    shr eax,16
    mov byte [LABEL_DESC_TSS+4],al 
    mov byte [LABEL_DESC_TSS+7],ah 

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
	mov sp,[_wSPValueInRealMode]
	in al,92h
	and al,11111101b
	out 92h,al
	sti
	;DOS下结束程序并返回
	mov ax,4c00h
	int 21h


[SECTION .s32] ;将下面代码装到32位的段里
[BITS 32] ;32位编译模式
; %include "lib.inc" ;继续崩

LABEL_SEG_CODE32:
	mov ax,SelectorData  
	mov ds,ax 
	mov ax,SelectorData  
	mov es,ax 
	mov ax,SelectorVideo
	mov gs,ax
	mov ax, SelectorStack  
	mov ss,ax
	mov esp,TopOfStack

	push szPMMessage ;入栈
	call DispStr
	add esp, 4; 恢复栈

	push szMemChkTitle 
	call DispStr
	add esp, 4

	
	call DispMemSize

	call PagingDemo

	; call SetupPaging ; 先注释掉这行获取当前内存大小，然后更改LABEL_DESC_PAGE_TBL界限大小，
	; ;所以为什么不在16位的地方直接更改界限大小？

	mov ax, SelectorTSS  
	ltr ax ; 设置任务状态段寄存器 TR

	push SelectorStack3 ;ss
	push TopOfStack3 ;esp
	push SelectorCodeRing3 ;cs
	push 0 ;eip
	retf ; 通过tss回到低特权级段

    ;call SelectorCallGateTest:0

	;mov ax, SelectorLDT
	;lldt ax

	;jmp SelectorLDTCodeA:0 ;跳入局部任务

	; call TestRead
	; call TestWrite
	; call TestRead
	; jmp SelectorCode16:0 ;回到16位

PagingDemo:
	mov ax, cs
	mov ds, ax 
	mov ax, SelectorFlatRW  
	mov es, ax

	; 将两个程序拷贝到对应的位置
	push LenFoo
	push OffsetFoo
	push ProcFoo
	call MemCpy
	add esp, 12

	push LenBar
	push OffsetBar
	push ProcBar
	call MemCpy
	add esp,12

	; 程序入口，通过更改LinearAddrDemo更改ProcPagingDemo指向的程序
	push LenPagingDemoAll
	push OffsetPagingDemoProc
	push ProcPagingDemo
	call MemCpy
	add esp, 12

	mov ax, SelectorData 
	mov ds,ax 
	mov es,ax

	call SetupPaging

	call SelectorFlatC:ProcPagingDemo

	call PSwitch
	call SelectorFlatC:ProcPagingDemo
	ret

DispMemSize:
	push esi 
	push edi 
	push ecx

	mov esi, MemChkBuf
	mov ecx, [dwMCRNumber]; for(int i=0;i<[MCRNumber];i++)//每次得到一个ARDS
.loop:
	mov edx, 5; for(int j=0;j<5;j++) //每次得到一个ARDS中的成员
	mov edi, ARDStruct;{//依次显示BaseAddrLow,BaseAddrHigh,LengthLow,LengthHigh,Type
.1:
	push dword [esi]
	call DispInt; 显示一个成员
	pop eax  
	stosd ;将eax的内容复制到es:edi，并将edi增加4个字节 // 为ARDS对应部位赋值
	add esi, 4 
	dec edx
	cmp edx, 0
	jnz .1
	call DispReturn
	cmp dword [dwType], 1 ;    if(BaseAddrLow + LengthLow > MemSize)，可被使用段
	jne .2
	mov eax, [dwBaseAddrLow]
	add eax, [dwLengthLow]
	cmp eax, [dwMemSize] ;    if(BaseAddrLow + LengthLow > MemSize)，获取最大可用内存处
	jb .2
	mov [dwMemSize],eax	 ;    MemSize = BaseAddrLow + LengthLow;
.2:
	loop .loop

	call DispReturn
	push szRAMSize
	call DispStr
	add esp, 4; 恢复push的堆栈

	push dword [dwMemSize]
	call DispInt
	add esp, 4

	pop ecx
	pop edi 
	pop esi 
	ret

SetupPaging:
	xor edx, edx
	mov eax, [dwMemSize]
	mov ebx, 400000h ; 400000h = 4M = 4096 * 1024, 一个页表对应的内存大小
	div ebx 
	mov ecx, eax ; 此时 ecx 为页表的个数，也即 PDE 应该的个数
	test edx, edx ; 看看是否有余数
	jz .no_remainder
	inc ecx
.no_remainder:
	; push ecx ; 页目录数
	mov [PageTableNumber], ecx

	; 首先初始化页目录
	mov ax, SelectorFlatRW
	mov es,ax 
	; xor edi,edi
	mov edi, PageDirBase0
	xor eax,eax
	mov eax, PageTblBase0 | PG_P |PG_USU | PG_RWW
.1:
	stosd ;将eax的内容复制到es:edi，并将edi增加4个字节
	add eax, 4096
	loop .1

	; 再初始化所有页表
	; mov ax, SelectorPageTbl  
	; mov es,ax 
	; pop eax; 将刚push的ecx pop到eax
	mov eax, [PageTableNumber]
	mov ebx, 1024; 每页1024个PTE
	mul ebx; PDE * PTE = PTE总数
	mov ecx,eax
	; xor edi,edi 
	mov edi, PageTblBase0
	xor eax, eax  
	mov eax, PG_P | PG_USU | PG_RWW ; 从物理地址0000:0000h开始

.2:
	stosd  
	add eax,4096
	loop .2

	mov eax, PageDirBase0
	mov cr3, eax; 将页目录首地址放到cr3，以后会自动从这里寻址
	mov eax, cr0
	or	eax, 80000000h
	mov	cr0, eax
	jmp short .3
.3:
	nop
	ret

PSwitch:
	; 首先初始化页目录
	mov ax, SelectorFlatRW
	mov es,ax 
	mov edi, PageDirBase1
	xor eax,eax
	mov eax, PageTblBase1 | PG_P |PG_USU | PG_RWW
	mov ecx, [PageTableNumber]
.1:
	stosd ;将eax的内容复制到es:edi，并将edi增加4个字节
	add eax, 4096
	loop .1

	mov eax, [PageTableNumber]
	mov ebx, 1024; 每页1024个PTE
	mul ebx; PDE * PTE = PTE总数
	mov ecx,eax
	; xor edi,edi 
	mov edi, PageTblBase1
	xor eax, eax  
	mov eax, PG_P | PG_USU | PG_RWW ; 从物理地址0000:0000h开始

.2:
	stosd  
	add eax,4096
	loop .2

	; 将LinearAddrDemo线性地址重新映射到新页表里
	mov eax, LinearAddrDemo
	shr eax, 22; 页目录，31-22位
	mov ebx, 4096; 每页偏移4096
	mul ebx
	mov ecx, eax
	mov eax, LinearAddrDemo
	shr eax, 12; 页表，21-12位
	and eax, 03FFh
	mov ebx, 4; 每页占4个字节
	mul ebx 
	add eax, ecx ; 准确的偏移地址
	add eax, PageTblBase1; 页表起始地址+偏移=找到对应页
	mov dword [es:eax], ProcBar | PG_P | PG_USU | PG_RWW; 在相同的线性地址对应的页上装入ProcBar

	mov eax, PageDirBase1
	mov cr3, eax; 将页目录首地址放到cr3，以后会自动从这里寻址
	jmp short .3
.3:
	nop
	ret

PagingDemoProc:
OffsetPagingDemoProc equ PagingDemoProc - $$
	mov eax, LinearAddrDemo ; 通过线性地址调用程序
	call eax 
	retf
LenPagingDemoAll equ $ - PagingDemoProc

foo:
OffsetFoo		equ	foo - $$
	mov	ah, 0Ch			; 0000: 黑底    1100: 红字
	mov	al, 'F'
	mov	[gs:((80 * 17 + 0) * 2)], ax	; 屏幕第 17 行, 第 0 列。
	mov	al, 'o'
	mov	[gs:((80 * 17 + 1) * 2)], ax	; 屏幕第 17 行, 第 1 列。
	mov	[gs:((80 * 17 + 2) * 2)], ax	; 屏幕第 17 行, 第 2 列。
	ret
LenFoo			equ	$ - foo

bar:
OffsetBar		equ	bar - $$
	mov	ah, 0Ch			; 0000: 黑底    1100: 红字
	mov	al, 'B'
	mov	[gs:((80 * 18 + 0) * 2)], ax	; 屏幕第 18 行, 第 0 列。
	mov	al, 'a'
	mov	[gs:((80 * 18 + 1) * 2)], ax	; 屏幕第 18 行, 第 1 列。
	mov	al, 'r'
	mov	[gs:((80 * 18 + 2) * 2)], ax	; 屏幕第 18 行, 第 2 列。
	ret
LenBar			equ	$ - bar

%include "lib.inc"
SegCode32Len equ $ - LABEL_SEG_CODE32

[SECTION .sdest]
[BITS 32]
LABEL_SEG_CODE_DEST:
    mov ax, SelectorVideo
    mov gs, ax
    
    mov edi,(80*1+0)*2
    mov ah,0ch
    mov al, 'C'
    mov [gs:edi], ax

	mov ax, SelectorLDT
	lldt ax

	jmp SelectorLDTCodeA:0 ;跳入局部任务

SegCodeDestLen equ $ - LABEL_SEG_CODE_DEST

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
	and	eax, 7FFFFFFEh		; PE=0, PG=0
	; and al,11111110b
	mov cr0,eax
LABEL_GO_BACK_TO_REAL:
	jmp 0:LABEL_REAL_ENTRY ; 回到16位代码段，段址在LABEL_BEGIN跳入32位之前预先设置

Code16Len equ $-LABEL_SEG_CODE16

[SECTION .ldt]
ALIGN 32
LABEL_LDT:
LABEL_LDT_DESC_CODEA: Descriptor 0, CodeALen -1, DA_C+DA_32
LDTLen equ $-LABEL_LDT

SelectorLDTCodeA equ LABEL_LDT_DESC_CODEA - LABEL_LDT + SA_TIL; TI 位表示局部描述符表

[SECTION .la]
ALIGN 32
[BITS 32]
LABEL_CODE_A:
	mov ax, SelectorVideo  
	mov gs, ax

	mov edi, (80 *2+0)*2
	mov ah,0ch
	mov al, 'L'
	mov [gs:edi],ax 

	jmp SelectorCode16:0
CodeALen equ $-LABEL_CODE_A

[SECTION .ring3]
ALIGN 32
[BITS 32]
LABEL_CODE_RING3:
	mov ax, SelectorVideo  
	mov gs, ax

	mov edi, (80 *4+0)*2
	mov ah,0ch
	mov al, '3'
	mov [gs:edi],ax 

	call SelectorCallGateTest:0 ; 通过调用门回到高特权级段
	jmp $
SegCodeRing3Len equ $ - LABEL_CODE_RING3