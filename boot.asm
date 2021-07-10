; %define _BOOT_DEBUG_
%ifdef _BOOT_DEBUG_
	org 0100h; com文件
%else
	org 07c00h ; 将代码加载到这个地址，见内存分布图
%endif

%ifdef _BOOT_DEBUG_
BaseOfStack equ 0100h
%else
BaseOfStack equ 07c00h
%endif

BaseOfLoader equ 09000h ; loader.bin被加载的段
OffsetOfLoader equ 0100h ; 偏移
RootDirSectors equ 14 ; 根目录占的扇区数
SectorNoOfRootDirectory equ 19 ; 根目录起始扇区数
SectorNoOfFAT1 equ 1
DeltaSectorNo equ 17

	jmp short LABEL_START
	nop ; 必不可少，.img文件开头的EB 3C 90，第3个字节，删除的话就少了90

	; 按照FAT12格式化软盘，便于DOS识别
	BS_OEMName DB 'XIXIHAHA' ; 启动区的名字，必须8字节
	BPB_BytsPerSec DW 512 ; 每扇区字节数
	BPB_SecPerClus DB 1; 每簇多少扇区
	BPB_RsvdSecCnt DW 1; FAT起始位置
	BPB_NumFATs DB 2; 共有多少FAT表
	BPB_RootEntCnt DW 224 ; 根目录文件数最大值
	BPB_TotSec16 DW 2880 ; 逻辑扇区总数，18*80*2
	BPB_Media DB 0xF0 ; 磁盘种类
	BPB_FATSz16 DW 9 ; 每个FAT的扇区数
	BPB_SecPerTrk DW 18 ; 每圈磁道扇区数
	BPB_NumHeads DW 2 ; 磁头数
	BPB_HiddSec DD 0 ; 隐藏扇区数
	BPB_TotSec32 DD 0; wTotalSectorCount为0时这个值记录扇区数
	BS_DrvNum DB 0; 中断13的驱动器号
	BS_Reserved1 DB 0; 未使用
	BS_BootSig DB 29h ; 扩展引导标记
	BS_VolID DD 0; 卷序列号
	BS_VolLab DB 'JuanBiaoBei' ; 卷标呗，必须11字节
	BS_FileSysType DB 'FAT12   ' ; 文件系统类型，必须8字节

LABEL_START
; 小程序中 (<64K), 代码段和数据段放在一个段里
	mov ax,cs
	mov ds,ax
	mov es,ax
	mov ss, ax
	mov sp, BaseOfStack

	; 清屏，详见10号中断
	mov ax, 0600h
	mov bx, 0700h
	mov cx, 0
	mov dx, 0184fh
	int 10h

	mov dh, 0
	call DispStr

	; 软驱复位
	xor ah, ah 
	xor dl, dl
	int 13

	mov word [wSectorNo], SectorNoOfRootDirectory ; 从根目录第19扇区开始查找
LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
	cmp word [wRootDirSizeForLoop], 0 ; 循环14次
	jz LABEL_NO_LOADERBIN
	dec word [wRootDirSizeForLoop]
	mov ax, BaseOfLoader ; 将每个扇区的内容都读到这个位置
	mov es, ax 
	mov bx, OffsetOfLoader
	mov ax, [wSectorNo]
	mov cl, 1 
	call ReadSector ; 起始位置ax，读取次数cl，目标位置es:bx

	mov si, LoaderFileName ; ds:si -> "LOADER  BIN"
	mov di, OffsetOfLoader ; es:di -> BaseOfLoader:0100
	cld ; DF = 0，地址增大，每次串操作si和di增大

	mov dx, 10h ; 每个扇区512字节，每个根Entry节点32字节，512/32=16，包含16个根节点
LABEL_SEARCH_FOR_LOADERBIN:
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
	mov si, LoaderFileName ; 下个根节点的文件名
	jmp LABEL_SEARCH_FOR_LOADERBIN

LABEL_GOTO_NEXT_SEARCH_IN_ROOT_DIR:
	add word[wSectorNo], 1; 下一个扇区
	jmp LABEL_SEARCH_IN_ROOT_DIR_BEGIN

LABEL_NO_LOADERBIN:
	mov dh, 2 ; 找到第二个字符串
	call DispStr
%ifdef _BOOT_DEBUG_
	mov ax, 4c00h
	int 21h; 回到dos页面
%else
	jmp $
%endif

LABEL_FILENAME_FOUND:
	mov ax, RootDirSectors
	and di, 0FFE0h; 当前根节点起始位置
	add di, 01Ah; sector标记开始的地方，详见根节点结构，出现了个大大的问题，我的根节点
	mov cx, word[es:di] ; es:BaseOfLoader，初始的FAT节点

	push cx
	add cx, ax 
	add cx, DeltaSectorNo; 起始扇区从2开始，cx-2+19就是数据区真实存放的地方
	mov ax, BaseOfLoader
	mov es, ax 
	mov bx, OffsetOfLoader
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
	jmp LABEL_GOON_LOADING_FILE
LABEL_FILE_LOADED:
	mov dh, 1
	call DispStr

	jmp BaseOfLoader:OffsetOfLoader ; 跳转到loader.bin

	; call DispStr ; call == push eip + jmp
	; jmp $ ; 跳转到当前地址，死循环，不如halt指令

wRootDirSizeForLoop dw RootDirSectors; 14

wSectorNo dw 0; 会初始化到19
bOdd db 0; 奇偶数
LoaderFileName db "LOADER  BIN", 0
MessageLength equ 9 
BootMessage:	db "Booting  " ; 9个字节
Message1	db "Ready.   "
Message2 db "No LOADER"

DispStr:
	mov ax, MessageLength
	mul dh
	add ax, BootMessage

	mov bp, ax ; AH为13时，ES:BP表示串地址
	mov ax, ds  
	mov es, ax  

	mov cx, MessageLength ; AH为13时，表示串长度

	mov ax, 01301h ; AH 中断入口
	mov bx, 000ch ; BH 页号，AL为1时BL表示属性
	; mov dx, 0102h ; DH DL 起始行列
	mov dl, 0
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
	mov ax, BaseOfLoader
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


times 510 -($-$$) db 0 ; $-$$：本行距离程序开始处的相对距离
dw 0xaa55 ;引导区结束标志，占2个字节