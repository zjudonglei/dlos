[SECTION .text]
global memcpy
global memset
global strcpy
; ------------------------------------------------------------------------
; 内存拷贝，仿 memcpy
; ------------------------------------------------------------------------
; void* MemCpy(void* es:pDest, void* ds:pSrc, int iSize);
; ------------------------------------------------------------------------
memcpy:
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

; ------------------------------------------------------------------------
; void memset(void* p_dst, char ch, int size);
; ------------------------------------------------------------------------
memset:
	push	ebp
	mov	ebp, esp

	push	esi
	push	edi
	push	ecx

	mov	edi, [ebp + 8]	; Destination
	mov	edx, [ebp + 12]	; Char to be putted
	mov	ecx, [ebp + 16]	; Counter
.1:
	cmp	ecx, 0		; 判断计数器
	jz	.2		; 计数器为零时跳出

	mov	byte [edi], dl		; ┓
	inc	edi			; ┛

	dec	ecx		; 计数器减一
	jmp	.1		; 循环
.2:

	pop	ecx
	pop	edi
	pop	esi
	mov	esp, ebp
	pop	ebp

	ret			; 函数结束，返回
; ------------------------------------------------------------------------

; ------------------------------------------------------------------------
; char* strcpy(char* p_dst, char* p_src);
; ------------------------------------------------------------------------
strcpy:
	push    ebp
	mov     ebp, esp

	mov     esi, [ebp + 12] ; Source
	mov     edi, [ebp + 8]  ; Destination

.1:
	mov     al, [esi]               ; ┓
	inc     esi                     ; ┃
					; ┣ 逐字节移动
	mov     byte [edi], al          ; ┃
	inc     edi                     ; ┛

	cmp     al, 0           ; 是否遇到 '\0'
	jnz     .1              ; 没遇到就继续循环，遇到就结束

	mov     eax, [ebp + 8]  ; 返回值

	pop     ebp
	ret                     ; 函数结束，返回
; strcpy 结束-------------------------------------------------------------