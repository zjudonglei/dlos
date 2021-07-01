%include "pm.inc"
org 07c00h
	jmp LABEL_BEGIN
[SECTION .gdt] ; 将以下大代码装到gdt段中
;Descriptor是pm.inc中定义的宏		段地址		段界限				属性
LABEL_GDT:			Descriptor	0,			0,					0 ;定义1个GDT段
LABEL_DESC_CODE32:	Descriptor	0,			SegCode32Len -1,	DA_C+DA_32 ;定义1个GDT段
LABEL_DESC_VIDEO:	Descriptor	0B8000h,	0ffffh,				DA_DRW ;显存段
;0000:0000~9FFF:000F->Ram
;A0000~AFFFF: VGA显存
;B0000~B7FFF: 黑白显存
;B8000~BFFFF: 彩色显存
;C0000~C7FFF: 显卡ROM空间（后来被改造成多种用途，也可以映射显存）
;C8000~FFFFE: 留给BIOS以及其它硬件使用（比如硬盘ROM之类的）。

GdtLen	equ $ - LABEL_GDT ;GDT长度
GdtPtr	dw	GdtLen - 1 ;GDT栈顶指针
		dd 	0 ;GDT基址

SelectorCode32	equ	LABEL_DESC_CODE32 - LABEL_GDT ;选择子，大概就是偏移量offset
SelectorVideo	equ	LABEL_DESC_VIDEO - LABEL_GDT

[SECTION .s16] ;将下面代码装到16位的段里
[BITS 16] ;16位编译模式
LABEL_BEGIN:
	mov ax,cs
	mov ds,ax
	mov es,ax
	mov ss,ax
	mov sp,0100h
	xor eax,eax
	mov ax,cs
	shl eax,4
	add eax,LABEL_DESC_CODE32
	mov word [LABEL_DESC_CODE32+2],ax 
	shr eax,16
	mov byte [LABEL_DESC_CODE32+4],al 
	mov byte [LABEL_DESC_CODE32+7],ah 
	xor eax,eax
	mov ax,ds
	shl eax,4
	add eax,LABEL_GDT
	mov dword [GdtPtr+2],eax
	lgdt [GdtPtr]
	cli ; 关中断，32位下中断模式有变
	in al,92h ; 打开A20地址线
	or al,00000010b
	out 92h,al
	mov eax,cr0
	or eax,1
	mov cr0, eax
	jmp dword SelectorCode32:0

[SECTION .s32] ;将下面代码装到32位的段里
[BITS 32] ;32位编译模式

LABEL_SEG_CODE32:
	mov ax,SelectorVideo
	mov gs,ax
	mov edi,(80*11+79)*2
	mov ah,0ch
	mov al,'P'
	mov [gs:edi],ax
	jmp $

SegCode32Len equ $ - LABEL_SEG_CODE32