
kernel/systask.o:     file format elf32-i386


Disassembly of section .group:

00000000 <.group>:
   0:	01 00                	add    %eax,(%eax)
   2:	00 00                	add    %al,(%eax)
   4:	07                   	pop    %es
   5:	00 00                	add    %al,(%eax)
	...

Disassembly of section .text:

00000000 <task_sys>:
   0:	f3 0f 1e fb          	endbr32 
   4:	55                   	push   %ebp
   5:	89 e5                	mov    %esp,%ebp
   7:	53                   	push   %ebx
   8:	83 ec 44             	sub    $0x44,%esp
   b:	e8 fc ff ff ff       	call   c <task_sys+0xc>
  10:	81 c3 02 00 00 00    	add    $0x2,%ebx
  16:	83 ec 04             	sub    $0x4,%esp
  19:	8d 45 c4             	lea    -0x3c(%ebp),%eax
  1c:	50                   	push   %eax
  1d:	6a 11                	push   $0x11
  1f:	6a 02                	push   $0x2
  21:	e8 fc ff ff ff       	call   22 <task_sys+0x22>
  26:	83 c4 10             	add    $0x10,%esp
  29:	8b 45 c4             	mov    -0x3c(%ebp),%eax
  2c:	89 45 f4             	mov    %eax,-0xc(%ebp)
  2f:	8b 45 c8             	mov    -0x38(%ebp),%eax
  32:	83 ec 08             	sub    $0x8,%esp
  35:	50                   	push   %eax
  36:	8d 83 00 00 00 00    	lea    0x0(%ebx),%eax
  3c:	50                   	push   %eax
  3d:	e8 fc ff ff ff       	call   3e <task_sys+0x3e>
  42:	83 c4 10             	add    $0x10,%esp
  45:	eb cf                	jmp    16 <task_sys+0x16>

Disassembly of section .rodata:

00000000 <.rodata>:
   0:	6d                   	insl   (%dx),%es:(%edi)
   1:	73 67                	jae    6a <task_sys+0x6a>
   3:	20 74 79 70          	and    %dh,0x70(%ecx,%edi,2)
   7:	65 3a 20             	cmp    %gs:(%eax),%ah
   a:	25                   	.byte 0x25
   b:	64 0a 00             	or     %fs:(%eax),%al

Disassembly of section .text.__x86.get_pc_thunk.bx:

00000000 <__x86.get_pc_thunk.bx>:
   0:	8b 1c 24             	mov    (%esp),%ebx
   3:	c3                   	ret    

Disassembly of section .comment:

00000000 <.comment>:
   0:	00 47 43             	add    %al,0x43(%edi)
   3:	43                   	inc    %ebx
   4:	3a 20                	cmp    (%eax),%ah
   6:	28 55 62             	sub    %dl,0x62(%ebp)
   9:	75 6e                	jne    79 <task_sys+0x79>
   b:	74 75                	je     82 <task_sys+0x82>
   d:	20 39                	and    %bh,(%ecx)
   f:	2e 33 2e             	xor    %cs:(%esi),%ebp
  12:	30 2d 31 37 75 62    	xor    %ch,0x62753731
  18:	75 6e                	jne    88 <task_sys+0x88>
  1a:	74 75                	je     91 <task_sys+0x91>
  1c:	31 7e 32             	xor    %edi,0x32(%esi)
  1f:	30 2e                	xor    %ch,(%esi)
  21:	30 34 29             	xor    %dh,(%ecx,%ebp,1)
  24:	20 39                	and    %bh,(%ecx)
  26:	2e 33 2e             	xor    %cs:(%esi),%ebp
  29:	30 00                	xor    %al,(%eax)

Disassembly of section .note.gnu.property:

00000000 <.note.gnu.property>:
   0:	04 00                	add    $0x0,%al
   2:	00 00                	add    %al,(%eax)
   4:	0c 00                	or     $0x0,%al
   6:	00 00                	add    %al,(%eax)
   8:	05 00 00 00 47       	add    $0x47000000,%eax
   d:	4e                   	dec    %esi
   e:	55                   	push   %ebp
   f:	00 02                	add    %al,(%edx)
  11:	00 00                	add    %al,(%eax)
  13:	c0 04 00 00          	rolb   $0x0,(%eax,%eax,1)
  17:	00 03                	add    %al,(%ebx)
  19:	00 00                	add    %al,(%eax)
	...

Disassembly of section .eh_frame:

00000000 <.eh_frame>:
   0:	14 00                	adc    $0x0,%al
   2:	00 00                	add    %al,(%eax)
   4:	00 00                	add    %al,(%eax)
   6:	00 00                	add    %al,(%eax)
   8:	01 7a 52             	add    %edi,0x52(%edx)
   b:	00 01                	add    %al,(%ecx)
   d:	7c 08                	jl     17 <.eh_frame+0x17>
   f:	01 1b                	add    %ebx,(%ebx)
  11:	0c 04                	or     $0x4,%al
  13:	04 88                	add    $0x88,%al
  15:	01 00                	add    %eax,(%eax)
  17:	00 18                	add    %bl,(%eax)
  19:	00 00                	add    %al,(%eax)
  1b:	00 1c 00             	add    %bl,(%eax,%eax,1)
  1e:	00 00                	add    %al,(%eax)
  20:	00 00                	add    %al,(%eax)
  22:	00 00                	add    %al,(%eax)
  24:	47                   	inc    %edi
  25:	00 00                	add    %al,(%eax)
  27:	00 00                	add    %al,(%eax)
  29:	45                   	inc    %ebp
  2a:	0e                   	push   %cs
  2b:	08 85 02 42 0d 05    	or     %al,0x50d4202(%ebp)
  31:	44                   	inc    %esp
  32:	83 03 10             	addl   $0x10,(%ebx)
  35:	00 00                	add    %al,(%eax)
  37:	00 38                	add    %bh,(%eax)
  39:	00 00                	add    %al,(%eax)
  3b:	00 00                	add    %al,(%eax)
  3d:	00 00                	add    %al,(%eax)
  3f:	00 04 00             	add    %al,(%eax,%eax,1)
  42:	00 00                	add    %al,(%eax)
  44:	00 00                	add    %al,(%eax)
	...
