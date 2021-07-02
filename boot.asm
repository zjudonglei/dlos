	org 07c00h ; 将代码加载到这个地址，见内存分布图
;	org 0100h com文件
; 小程序中 (<64K), 代码段和数据段放在一个段里
	mov ax,cs
	mov ds,ax
	mov es,ax
	call DispStr ; call == push eip + jmp
	jmp $ ; 跳转到当前地址，死循环，不如halt指令
DispStr:
	mov ax, BootMessage
	mov bp, ax ; AH为13时，ES:BP表示串地址
	mov cx, 16 ; AH为13时，表示串长度
	mov ax, 01301h ; AH 中断入口
	mov bx, 000ch ; BH 页号，AL为1时BL表示属性
	mov dx, 0102h ; DH DL 起始行列
	int 10h
	ret
BootMessage:	db "Hello, OS world!" ; 等价 BootMessage db "Hello, OS world!"，定义了一个字符串
times 510 -($-$$) db 0 ; $-$$：本行距离程序开始处的相对距离
dw 0xaa55 ;引导区结束标志，占2个字节