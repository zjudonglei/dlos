%include "sconst.inc"

extern disp_pos

[SECTION .data]

global disp_str
global disp_color_str
global out_byte
global in_byte
global enable_irq
global disable_irq
global enable_int
global disable_int

; ========================================================================
;                  void disp_str(char * pszInfo);
; ========================================================================
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

; ========================================================================
;                  void disp_color_str(char * info, int color);
; ========================================================================
disp_color_str:
	push	ebp
	mov	ebp, esp ; 栈顶指针

	push	ebx
	push	esi
	push	edi

	mov	esi, [ebp + 8]	; call之前push了字符串的地址，然后push了ebp，共两个字节8位
	mov	edi, [disp_pos] ; 屏幕显示位置，全局可用
	mov	ah, [ebp + 12]
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

; ========================================================================
;                  void out_byte(u16 port, u8 value);
; ========================================================================
; 参数从后往前压入栈
out_byte:
	mov edx, [esp+4]; 端口
	mov al, [esp+4+4]
	out dx,al
	nop
	nop
	ret

; ========================================================================
;                  u8 in_byte(u16 port);
; ========================================================================
in_byte:
	mov edx, [esp+4]
	xor eax,eax
	in al, dx
	nop
	nop
	ret

; ========================================================================
;                  void disable_irq(int irq);
; ========================================================================
disable_irq:
        mov     ecx, [esp + 4]          ; irq
        pushf
        cli
        mov     ah, 1 ; 00000001
        rol     ah, cl ; 假如cl为3，ah=00001000，假如cl为3+8，那么ah还是00001000
        cmp     cl, 8
        jae     disable_8               ; disable irq >= 8 at the slave 8259
disable_0:
        in      al, INT_M_CTLMASK
        test    al, ah ; 执行and，但是由于ah只有1位是1，所以除非对应位已经屏蔽，不然结果是0
        jnz     dis_already             ; already disabled?
        or      al, ah
        out     INT_M_CTLMASK, al       ; set bit at master 8259
        popf
        mov     eax, 1                  ; disabled by this function
        ret
disable_8:
        in      al, INT_S_CTLMASK
        test    al, ah
        jnz     dis_already             ; already disabled?
        or      al, ah
        out     INT_S_CTLMASK, al       ; set bit at slave 8259
        popf
        mov     eax, 1                  ; disabled by this function
        ret
dis_already:
        popf
        xor     eax, eax                ; already disabled
        ret

; ========================================================================
;                  void enable_irq(int irq);
; ========================================================================
; Enable an interrupt request line by clearing an 8259 bit.
; Equivalent code:
;       if(irq < 8)
;               out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK) & ~(1 << irq));
;       else
;               out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK) & ~(1 << irq));
;
enable_irq:
        mov     ecx, [esp + 4]          ; irq
        pushf
        cli
        mov     ah, ~1
        rol     ah, cl                  ; ah = ~(1 << (irq % 8))
        cmp     cl, 8
        jae     enable_8                ; enable irq >= 8 at the slave 8259
enable_0:
        in      al, INT_M_CTLMASK
        and     al, ah
        out     INT_M_CTLMASK, al       ; clear bit at master 8259
        popf
        ret
enable_8:
        in      al, INT_S_CTLMASK
        and     al, ah
        out     INT_S_CTLMASK, al       ; clear bit at slave 8259
        popf
        ret

; ========================================================================
;		   void disable_int();
; ========================================================================
disable_int:
	cli
	ret

; ========================================================================
;		   void enable_int();
; ========================================================================
enable_int:
	sti
	ret