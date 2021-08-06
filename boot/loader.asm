org 0100h; com文件
	jmp LABEL_START

%include "fat12hdr.inc"
%include "load.inc"
%include "pm.inc"

[SECTION .gdt] ; 将以下大代码装到gdt段中
LABEL_GDT:			Descriptor	0,			0,					0 			;定义1个空GDT段
LABEL_DESC_FLAT_C:	Descriptor 	0,			0fffffh,				DA_CR|DA_32|DA_LIMIT_4K;代码段
LABEL_DESC_FLAT_RW: Descriptor 	0,			0fffffh,				DA_DRW|DA_32|DA_LIMIT_4K;数据段
LABEL_DESC_VIDEO:	Descriptor	0B8000h,	0ffffh,				DA_DRW|DA_DPL3 		;显存段，见内存分布图

GdtLen	equ $ - LABEL_GDT ;GDT长度
GdtPtr	dw	GdtLen - 1 ;GDT界限，低16位
		dd 	BaseOfLoaderPhyAddr + LABEL_GDT ;GDT基址，高32位 (让基地址八字节对齐将起到优化速度之效果)

SelectorFlatC	equ	LABEL_DESC_FLAT_C 	- LABEL_GDT
SelectorFlatRW	equ	LABEL_DESC_FLAT_RW 	- LABEL_GDT
SelectorVideo	equ	LABEL_DESC_VIDEO 		- LABEL_GDT + SA_RPL3

BaseOfStack equ 0100h

LABEL_START:
; 小程序中 (<64K), 代码段和数据段放在一个段里
	mov ax,cs
	mov ds,ax
	mov es,ax
	mov ss, ax
	mov sp, BaseOfStack

	mov dh, 0
	call DispStrRealMode

	; 使用中断获取内存
	mov ebx, 0
	mov di, _MemChkBuf; es:di指向地址范围描述符ARDS
.MemChkLoop:
	mov eax, 0E820h ; 功能号
	mov ecx, 20 ; 每次BIOS填充的大小
	mov edx, 0534D4150h ; 标志
	int 15h
	jc .MEM_CHK_FAIL ; 判断CF位是否有错误
	add di,20; 填充下一个空间
	inc dword [_dwMCRNumber]
	cmp ebx, 0 ; ebx结束标志
	jne .MemChkLoop
	jmp .MEM_CHK_OK
.MEM_CHK_FAIL:
	mov dword [_dwMCRNumber],0 
.MEM_CHK_OK:

	; 加载kenel.bin
	; 软驱复位
	mov word [wSectorNo], SectorNoOfRootDirectory ; 从根目录第19扇区开始查找
	xor ah, ah 
	xor dl, dl
	int 13h
LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
	cmp word [wRootDirSizeForLoop], 0 ; 循环14次
	jz LABEL_NO_KERNELBIN
	dec word [wRootDirSizeForLoop]
	mov ax, BaseOfKernelFile ; 将每个扇区的内容都读到这个位置
	mov es, ax 
	mov bx, OffsetOfKernelFile
	mov ax, [wSectorNo]
	mov cl, 1 
	call ReadSector ; 起始位置ax，读取次数cl，目标位置es:bx

	mov si, KernelFileName ; ds:si -> "LOADER  BIN"
	mov di, OffsetOfKernelFile ; es:di -> BaseOfLoader:0100
	cld ; DF = 0，地址增大，每次串操作si和di增大

	mov dx, 10h ; 每个扇区512字节，每个根Entry节点32字节，512/32=16，包含16个根节点
LABEL_SEARCH_FOR_KERNELBIN:
	cmp dx, 0
	jz LABEL_GOTO_NEXT_SEARCH_IN_ROOT_DIR
	dec dx
	mov cx, 11 ; 每个根节点的首11字节是文件名
LABEL_CMP_FILENAME:
	cmp cx, 0
	jz LABEL_FILENAME_FOUND
	dec cx
	lodsb ; ds:si->al，si++
	cmp al, byte[es:di]
	jz LABEL_GO_ON
	jmp LABEL_DIFFERENT
LABEL_GO_ON:
	inc di; di++
	jmp LABEL_CMP_FILENAME

LABEL_DIFFERENT:
	and di, 0FFE0h; E0h:11100000，低5位置0，因为根节点为32位，每个根节点初始位置的低5位都是0
	add di, 20h; 20h:100000，低5位仍然是0，偏移一个根节点
	mov si, KernelFileName ; 下个根节点的文件名
	jmp LABEL_SEARCH_FOR_KERNELBIN

LABEL_GOTO_NEXT_SEARCH_IN_ROOT_DIR:
	add word[wSectorNo], 1; 下一个扇区
	jmp LABEL_SEARCH_IN_ROOT_DIR_BEGIN

LABEL_NO_KERNELBIN:
	mov dh, 3 ; 找到第二个字符串
	call DispStrRealMode
	jmp $

LABEL_FILENAME_FOUND:
	mov ax, RootDirSectors
	and di, 0FFE0h; 当前根节点起始位置

	push eax
	mov eax, [es:di + 01Ch]
	mov dword [dwKernelSize], eax
	cmp	eax, KERNEL_VALID_SPACE
	ja	.1
	pop eax
	jmp	.2
.1:
	mov	dh, 4			; "Too Large"
	call	DispStrRealMode		; 显示字符串
	jmp	$			; KERNEL.BIN 太大，死循环在这里
.2:
	add di, 01Ah; sector标记开始的地方，详见根节点结构，出现了个大大的问题，我的根节点
	mov cx, word[es:di] ; es:BaseOfLoader，初始的FAT节点

	push cx
	add cx, ax 
	add cx, DeltaSectorNo; 起始扇区从2开始，cx-2+19就是数据区真实存放的地方
	mov ax, BaseOfKernelFile
	mov es, ax 
	mov bx, OffsetOfKernelFile
	mov ax, cx ; es,bx ReadSector加载的目的地
LABEL_GOON_LOADING_FILE:
	push ax
	push bx  
	mov ah, 0Eh ;每读一个扇区就在 "Booting  " 后面
	mov al, '.';打一个点, 形成这样的效果:
	mov bl, 0Fh;Booting ......
	int 10h
	pop bx
	pop ax

	mov cl, 1
	call ReadSector ; 加载loader扇区
	pop ax; 将存到栈里的FAT节点号pop出来
	call GetFATEntry
	cmp ax, 0FFFh ;是否是最后一个扇区
	jz LABEL_FILE_LOADED
	push ax 
	mov dx, RootDirSectors
	add ax, dx ; 数据区
	add ax, DeltaSectorNo
	add bx, [BPB_BytsPerSec]
	jc .1 ; 如果 bx 重新变成 0，说明内核大于 64KB
	jmp .2
.1:
	push ax 
	mov ax, es  
	add ax, 1000h
	mov es, ax ; es += 0x1000  ← es 指向下一个段
	pop ax
.2:
	jmp LABEL_GOON_LOADING_FILE
LABEL_FILE_LOADED:
	call KillMotor

	;; 将硬盘引导扇区内容读入内存 0500h 处
	xor     ax, ax
	mov     es, ax
	mov     ax, 0201h       ; AH = 02
	                        ; AL = number of sectors to read (must be nonzero) 
	mov     cx, 1           ; CH = low eight bits of cylinder number
	                        ; CL = sector number 1-63 (bits 0-5)
	                        ;      high two bits of cylinder (bits 6-7, hard disk only)
	mov     dx, 80h         ; DH = head number
	                        ; DL = drive number (bit 7 set for hard disk)
	mov     bx, 500h        ; ES:BX -> data buffer
	int     13h
	;; 硬盘操作完毕
	mov dh, 2
	call DispStrRealMode

	lgdt [GdtPtr];将GdtPtr加载gdtr寄存器

	cli ; 清理IF位，关中断，32位下中断模式有变

	in al,92h ; 打开A20地址线，开启32位模式，没找到各个端口的定义，抱歉
	or al,00000010b
	out 92h,al

	mov eax,cr0
	or eax,1
	mov cr0, eax
	; 进入保护模式，每当把一个选择子装入到某个段寄存器时，处理器自动从描述符表中取出相应的描述符，把描述符中的信息保存到对应的高速缓冲寄存器中。此后对该段访问时，处理器都使用对应高速缓冲寄存器中的描述符信息，而不用再从描述符表中取描述符。 
	jmp dword SelectorFlatC:(BaseOfLoaderPhyAddr+LABEL_PM_START)	; 执行这一句会把 SelectorCode32 装入 cs,
					; 并跳转到 Code32Selector:0  处

;==================================
;变量
;----------------------------------
wRootDirSizeForLoop dw RootDirSectors; 14
wSectorNo dw 0; 会初始化到19
bOdd db 0; 奇偶数
dwKernelSize dd 0; 内核大小

KernelFileName db "KERNEL  BIN", 0
MessageLength equ 9 
LoadMessage:	db "Loading  " ; 9个字节
Message1	db "         "
Message2	db "Ready.   "
Message3 	db "No Kernel"
Message4	db "Too LARGE"
;==================================

DispStrRealMode:
	mov ax, MessageLength
	mul dh
	add ax, LoadMessage

	mov bp, ax ; AH为13时，ES:BP表示串地址
	mov ax, ds  
	mov es, ax  

	mov cx, MessageLength ; AH为13时，表示串长度

	mov ax, 01301h ; AH 中断入口
	mov bx, 000ch ; BH 页号，AL为1时BL表示属性
	mov dl, 0
	add dh, 3
	int 10h
	ret

;(ax, cl, es:bx)
;参数ax：起始地址
;参数cl：加载扇区数
;参数es:bx：加载的目标地址
ReadSector:
	;每磁道扇区数上面有定义：18
	;圈：柱面号，上面有定义80个
	;磁头：2
	;
	;ax/每磁道扇区数=余数就是扇区号
	;			商就是从外圈往里数的第几圈，但是磁盘有两面，所以圈要除2
	;			磁头就是奇数还是偶数
	push bp  
	mov bp, sp ; 把sp备份一份，因为sp会变化，类似int b = a
	sub esp, 2; 存储扇区数用，因为堆栈是向低地址生长的，初始esp位于栈顶
	mov byte [bp -2], cl 

	push bx 
	mov bl, [BPB_SecPerTrk] ; 每磁道扇区数：18
	div bl ; 商al，余数ah
	inc ah ; 磁道从1开始
	mov cl, ah ; 扇区号
	mov dh, al ;
	shr al, 1; 除2
	mov ch, al; 柱面号
	and dh, 1 ; 磁头号

	pop bx ; 加载到es:bx内存
	mov dl, [BS_DrvNum] ; 驱动器号

.GoOnReading:
	mov ah, 2 ; 功能号
	mov al, byte[bp - 2] ; 加载扇区数量
	int 13h
	jc .GoOnReading

	add esp, 2 ; 恢复栈
	pop bp
	ret

GetFATEntry:
	push es 
	push bx 
	push ax 
	mov ax, BaseOfKernelFile
	sub ax, 0100h ;在 BaseOfLoader 后面留出 4K 空间用于存放 FAT
	mov es, ax ; es:bx是ReadSector的目的地，es*16+bx
	pop ax

; 因为每个FAT节点占1.5个字节，0 1,1.5 2 3
; 当偶数时，会读取到01，所以要右移4位得到0-1.5,
; 当奇数时，会读取到12，所以要把高4位补0，得到1.5-2
	mov byte [bOdd], 0
	mov bx,3
	mul bx 
	mov bx,2
	div bx; ax商，dx余数
	cmp dx, 0
	jz LABEL_EVEN
	mov byte [bOdd], 1
LABEL_EVEN:;偶数
	xor dx, dx 
	mov bx, [BPB_BytsPerSec]
	div bx ; ax 商，表示偏移了几个扇区，dx 余数，表示在标的扇区偏移了多少个字节
	push dx 
	mov bx, 0 ; bx <- 0 于是, es:bx = (BaseOfLoader - 100):00
	add ax, SectorNoOfFAT1; 因为FAT1就是从第1个扇区开始的
	mov cl,2;加载两个扇区
	call ReadSector

	pop dx 
	add bx, dx 
	mov ax, [es:bx]
	cmp byte [bOdd], 1
	jnz LABEL_EVEN_2
	shr ax, 4
LABEL_EVEN_2:
	and ax, 0FFFh

	pop bx
	pop es 
	ret

KillMotor:
	push dx 
	mov dx, 03F2h
	mov al, 0
	out dx, al  
	pop dx 
	ret

[SECTION .s32] ;将下面代码装到32位的段里
ALIGN 32
[BITS 32] ;32位编译模式
LABEL_PM_START:
	mov ax,SelectorVideo
	mov gs,ax
	mov ax,SelectorFlatRW
	mov ds,ax 
	mov es,ax 
	mov fs,ax
	mov ss,ax



	mov esp,TopOfStack

	
	call DispMemInfo
	call SetupPaging

	; mov ah, 0Fh 
	; mov al, 'P'
	; mov [gs:((80*0+39)*2)],ax 
	
	call InitKernel

	mov	dword [BOOT_PARAM_ADDR], BOOT_PARAM_MAGIC	; BootParam[0] = 魔数;
	mov	eax, [dwMemSize]				;
	mov	[BOOT_PARAM_ADDR + 4], eax			; BootParam[1] = 内存大小;
	mov	eax, BaseOfKernelFile
	shl	eax, 4
	add	eax, OffsetOfKernelFile
	mov	[BOOT_PARAM_ADDR + 8], eax			; BootParam[2] = kernel.bin开始的地方;

	;***************************************************************
	jmp	SelectorFlatC:KRNL_ENT_PT_PHY_ADDR	; 正式进入内核 *
	;***************************************************************
	; 内存看上去是这样的：
	;              ┃                                    ┃
	;              ┃                 .                  ┃
	;              ┃                 .                  ┃
	;              ┃                 .                  ┃
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃■■■■■■■■■■■■■■■■■■┃
	;              ┃■■■■■■Page  Tables■■■■■■┃
	;              ┃■■■■■(大小由LOADER决定)■■■■┃
	;    00101000h ┃■■■■■■■■■■■■■■■■■■┃ PAGE_TBL_BASE
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃■■■■■■■■■■■■■■■■■■┃
	;    00100000h ┃■■■■Page Directory Table■■■■┃ PAGE_DIR_BASE  <- 1M
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃□□□□□□□□□□□□□□□□□□┃
	;       F0000h ┃□□□□□□□System ROM□□□□□□┃
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃□□□□□□□□□□□□□□□□□□┃
	;       E0000h ┃□□□□Expansion of system ROM □□┃
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃□□□□□□□□□□□□□□□□□□┃
	;       C0000h ┃□□□Reserved for ROM expansion□□┃
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃□□□□□□□□□□□□□□□□□□┃ B8000h ← gs
	;       A0000h ┃□□□Display adapter reserved□□□┃
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃□□□□□□□□□□□□□□□□□□┃
	;       9FC00h ┃□□extended BIOS data area (EBDA)□┃
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃■■■■■■■■■■■■■■■■■■┃
	;       90000h ┃■■■■■■■LOADER.BIN■■■■■■┃ somewhere in LOADER ← esp
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃■■■■■■■■■■■■■■■■■■┃
	;              ┃■■■■■■■■■■■■■■■■■■┃
	;       70000h ┃■■■■■■■KERNEL.BIN■■■■■■┃
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃■■■■■■■■■■■■■■■■■■┃
	;              ┃■■■■■■■■■■■■■■■■■■┃
	;              ┃■■■■■■■■■■■■■■■■■■┃
	;              ┃■■■■■■■■■■■■■■■■■■┃
	;              ┃■■■■■■■■■■■■■■■■■■┃
	;              ┃■■■■■■■■■■■■■■■■■■┃
	;              ┃■■■■■■■■■■■■■■■■■■┃ 7C00h~7DFFh : BOOT SECTOR, overwritten by the kernel
	;              ┃■■■■■■■■■■■■■■■■■■┃
	;              ┃■■■■■■■■■■■■■■■■■■┃
	;              ┃■■■■■■■■■■■■■■■■■■┃
	;        1000h ┃■■■■■■■■KERNEL■■■■■■■┃ 1000h ← KERNEL 入口 (KRNL_ENT_PT_PHY_ADDR)
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃                                    ┃
	;         500h ┃              F  R  E  E            ┃
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃□□□□□□□□□□□□□□□□□□┃
	;         400h ┃□□□□ROM BIOS parameter area □□┃
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇┃
	;           0h ┃◇◇◇◇◇◇Int  Vectors◇◇◇◇◇◇┃
	;              ┗━━━━━━━━━━━━━━━━━━┛ ← cs, ds, es, fs, ss
	;
	;
	;		┏━━━┓		┏━━━┓
	;		┃■■■┃ 我们使用 	┃□□□┃ 不能使用的内存
	;		┗━━━┛		┗━━━┛
	;		┏━━━┓		┏━━━┓
	;		┃      ┃ 未使用空间	┃◇◇◇┃ 可以覆盖的内存
	;		┗━━━┛		┗━━━┛
	;
	; 注：KERNEL 的位置实际上是很灵活的，可以通过同时改变 LOAD.INC 中的 KRNL_ENT_PT_PHY_ADDR 和 MAKEFILE 中参数 -Ttext 的值来改变。
	;     比如，如果把 KRNL_ENT_PT_PHY_ADDR 和 -Ttext 的值都改为 0x400400，则 KERNEL 就会被加载到内存 0x400000(4M) 处，入口在 0x400400。
	;




; 显示 AL 中的数字 用16进制显示
; 默认地:
;	数字已经存在 AL 中
;	edi 始终指向要显示的下一个字符的位置
; 被改变的寄存器:
;	ax, edi
DispAL:
	push ecx 
	push edx
	push edi

	mov edi, [dwDispPos]

	mov ah,0Fh; 0000: 黑底    1100: 红字 1111:白字
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
		; jmp SelectorCode16:0
	add edi,2
	mov al,dl  ;开始低4位
	loop .begin
	; add edi,2

	mov [dwDispPos], edi; 将最新存储到的位置刷新进去

	pop edi
	pop edx
	pop ecx
	ret
;; DispAL 结束


;; 显示一个整型数
; call之前要把显示的数字push进来
DispInt:
	mov	eax, [esp + 4] ; 取刚push进来的数字
	shr	eax, 24
	call	DispAL

	mov	eax, [esp + 4]
	shr	eax, 16
	call	DispAL

	mov	eax, [esp + 4]
	shr	eax, 8
	call	DispAL

	mov	eax, [esp + 4]
	call	DispAL

	mov	ah, 07h			; 0000b: 黑底    0111b: 灰字
	mov	al, 'h'
	push	edi
	mov	edi, [dwDispPos]
	mov	[gs:edi], ax
	add	edi, 4
	mov	[dwDispPos], edi
	pop	edi

	ret
;; DispInt 结束

;; 显示一个字符串
DispStr:
	push	ebp
	mov	ebp, esp ; 栈顶指针
	push	ebx
	push	esi
	push	edi

	mov	esi, [ebp + 8]	; call之前push了字符串的地址，然后push了ebp，共两个字节8位
	mov	edi, [dwDispPos] ; 屏幕显示位置，全局可用
	mov	ah, 0Fh
.1:
	lodsb ; 取SI中的字节
	test	al, al ;Test对两个参数(目标，源)执行AND逻辑操作，并根据结果设置标志寄存器，结果本身不会保存。
	jz	.2 ; 参见数据段字符串定义，末尾都有个0
	cmp	al, 0Ah	; 是回车吗?
	jnz	.3
	push	eax ; 
	mov	eax, edi
	mov	bl, 160;80*25模式下，一行80个字符，160个字节，总共25行
	div	bl; al = ax / 160, ah = ax mod 160，al表示行，ah是余数表示列
	and	eax, 0FFh ;丢弃ah，只保留al，即保留行
	inc	eax ;行增加1
	mov	bl, 160
	mul	bl ;下1行0列开始显示
	mov	edi, eax
	pop	eax
	jmp	.1
.3:
	mov	[gs:edi], ax
	add	edi, 2; 1个字符占用2个字节
	jmp	.1

.2: ;显示完毕
	mov	[dwDispPos], edi

	pop	edi
	pop	esi
	pop	ebx
	pop	ebp
	ret
;; DispStr 结束

;; 换行
DispReturn:
	push	szReturn; 将换行符入栈
	call	DispStr			;printf("\n");
	add	esp, 4; 恢复栈

	ret
;; DispReturn 结束


; ------------------------------------------------------------------------
; 内存拷贝，仿 memcpy
; ------------------------------------------------------------------------
; void* MemCpy(void* es:pDest, void* ds:pSrc, int iSize);
; ------------------------------------------------------------------------
MemCpy:
	push	ebp
	mov	ebp, esp

	push	esi
	push	edi
	push	ecx

	mov	edi, [ebp + 8]	; Destination
	mov	esi, [ebp + 12]	; Source
	mov	ecx, [ebp + 16]	; Counter
.1:
	cmp	ecx, 0		; 判断计数器
	jz	.2		; 计数器为零时跳出

	mov	al, [ds:esi]		; ┓
	inc	esi			; ┃
					; ┣ 逐字节移动
	mov	byte [es:edi], al	; ┃
	inc	edi			; ┛

	dec	ecx		; 计数器减一
	jmp	.1		; 循环
.2:
	mov	eax, [ebp + 8]	; 返回值

	pop	ecx
	pop	edi
	pop	esi
	mov	esp, ebp
	pop	ebp

	ret			; 函数结束，返回
; MemCpy 结束-------------------------------------------------------------

DispMemInfo:
	push esi 
	push edi 
	push ecx

	push	szMemChkTitle
	call	DispStr
	add	esp, 4
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
	push ecx ; 页目录数

	; 首先初始化页目录
	mov ax, SelectorFlatRW
	mov es,ax 
	mov edi, PageDirBase
	xor eax,eax
	mov eax, PageTblBase | PG_P |PG_USU | PG_RWW
.1:
	stosd ;将eax的内容复制到es:edi，并将edi增加4个字节
	add eax, 4096
	loop .1

	; 再初始化所有页表
	pop eax; 将刚push的ecx pop到eax
	mov ebx, 1024; 每页1024个PTE
	mul ebx; PDE * PTE = PTE总数
	mov ecx,eax
	mov edi, PageTblBase
	xor eax, eax  
	mov eax, PG_P | PG_USU | PG_RWW ; 从物理地址0000:0000h开始

.2:
	stosd  
	add eax,4096
	loop .2

	mov eax, PageDirBase
	mov cr3, eax; 将页目录首地址放到cr3，以后会自动从这里寻址
	mov eax, cr0
	or	eax, 80000000h
	mov	cr0, eax
	jmp short .3
.3:
	nop
	ret

InitKernel:
	xor esi,esi 
	mov cx, word[BaseOfKernelFilePhyAddr+2Ch]; ELFheader里程序数量
	movzx ecx, cx 

	mov esi, [BaseOfKernelFilePhyAddr+1ch] ; ELF header里header长度，即可一个程序header节点的位置
	add esi, BaseOfKernelFilePhyAddr
.Begin:
	mov eax, [esi+0]
	cmp eax, 0 ; 比较程序头节点首4字节即类型是否是0
	jz .NoAction
	push dword[esi+010h]; 程序长度
	mov eax, [esi + 04h]; 源地点偏移
	add eax, BaseOfKernelFilePhyAddr
	push eax 
	push dword[esi + 08h] ; 目的地
	call MemCpy
	add esp, 12
.NoAction:
	add esi, 020h
	dec ecx 
	jnz .Begin
	ret


[SECTION .data1] ;数据段
ALIGN 32
LABEL_DATA:
_szMemChkTitle:	db "BaseAddrL BaseAddrH LengthLow LengthHigh Type", 0AH, 0
_szRAMSize db "RAM size:", 0 
_szReturn			db	0Ah, 0

_dwMCRNumber: dd 0
_dwDispPos: dd (80*7 + 0) * 2
_dwMemSize: dd 0
_ARDStruct:
	_dwBaseAddrLow: dd 0
	_dwBaseAddrHigh: dd 0
	_dwLengthLow: dd 0
	_dwLengthHigh: dd 0
	_dwType: dd 0

_MemChkBuf: times 256 db 0

;保护模式用段+偏移
szMemChkTitle equ _szMemChkTitle + BaseOfLoaderPhyAddr
szRAMSize equ _szRAMSize + BaseOfLoaderPhyAddr
szReturn		equ	_szReturn	+ BaseOfLoaderPhyAddr

dwMCRNumber equ _dwMCRNumber + BaseOfLoaderPhyAddr
dwDispPos equ _dwDispPos + BaseOfLoaderPhyAddr
dwMemSize equ _dwMemSize + BaseOfLoaderPhyAddr
ARDStruct equ _ARDStruct + BaseOfLoaderPhyAddr
	dwBaseAddrLow equ _dwBaseAddrLow + BaseOfLoaderPhyAddr
	dwBaseAddrHigh equ _dwBaseAddrHigh + BaseOfLoaderPhyAddr
	dwLengthLow equ _dwLengthLow + BaseOfLoaderPhyAddr
	dwLengthHigh equ _dwLengthHigh + BaseOfLoaderPhyAddr
	dwType equ _dwType + BaseOfLoaderPhyAddr
MemChkBuf equ _MemChkBuf + BaseOfLoaderPhyAddr
	
;全局堆栈段
[SECTION .gs]
[BITS 32]
StackSpace:	times 1000h db 0
TopOfStack equ BaseOfLoaderPhyAddr + $