[SECTION .data]
disp_pos dd 0

global disp_str

;; 显示一个字符串
disp_str:
	push	ebp
	mov	ebp, esp ; 栈顶指针
	push	ebx
	push	esi
	push	edi

	mov	esi, [ebp + 8]	; call之前push了字符串的地址，然后push了ebp，共两个字节8位
	mov	edi, [disp_pos] ; 屏幕显示位置，全局可用
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
	mov	[disp_pos], edi

	pop	edi
	pop	esi
	pop	ebx
	pop	ebp
	ret
;; DispStr 结束