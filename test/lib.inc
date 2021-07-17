;; lib.inc

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