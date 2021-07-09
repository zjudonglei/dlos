#1/bin/bash
nasm boot.asm -o boot.bin
dd if=boot.bin of=bochs-a.img bs=512 count=1 conv=notrunc
nasm loader.asm -o loader.bin
