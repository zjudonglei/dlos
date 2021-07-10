BaseOfLoader equ 09000h ; loader.bin被加载的段
OffsetOfLoader equ 0100h ; 偏移

BaseOfLoaderPhyAddr equ BaseOfLoader*10h; 段*16+偏移地址

BaseOfKernelFile equ 08000h ; kernel.bin被加载的段
OffsetOfKernelFile equ 0 ; 偏移

BaseOfKernelFilePhyAddr equ BaseOfKernelFile * 10h
KernelEntryPointPhyAddr equ 030400h

PageDirBase equ 100000h
PageTblBase equ 101000h