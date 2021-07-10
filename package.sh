#1/bin/bash
nasm boot.asm -o boot.bin
dd if=boot.bin of=bochs-a.img bs=512 count=1 conv=notrunc
nasm loader.asm -o loader.bin
nasm -f elf -o kernel.o kernel.asm
ld -m elf_i386 -s -Ttext 0x30400 -o kernel.bin kernel.o
# gcc -m32 -c bar.c -o bar.o
