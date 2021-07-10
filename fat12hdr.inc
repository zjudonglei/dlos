; 按照FAT12格式化软盘，便于DOS识别
BS_OEMName DB 'XIXIHAHA' ; 启动区的名字，必须8字节
BPB_BytsPerSec DW 512 ; 每扇区字节数
BPB_SecPerClus DB 1; 每簇多少扇区
BPB_RsvdSecCnt DW 1; FAT起始位置
BPB_NumFATs DB 2; 共有多少FAT表
BPB_RootEntCnt DW 224 ; 根目录文件数最大值
BPB_TotSec16 DW 2880 ; 逻辑扇区总数，18*80*2
BPB_Media DB 0xF0 ; 磁盘种类
BPB_FATSz16 DW 9 ; 每个FAT的扇区数
BPB_SecPerTrk DW 18 ; 每圈磁道扇区数
BPB_NumHeads DW 2 ; 磁头数
BPB_HiddSec DD 0 ; 隐藏扇区数
BPB_TotSec32 DD 0; wTotalSectorCount为0时这个值记录扇区数
BS_DrvNum DB 0; 中断13的驱动器号
BS_Reserved1 DB 0; 未使用
BS_BootSig DB 29h ; 扩展引导标记
BS_VolID DD 0; 卷序列号
BS_VolLab DB 'JuanBiaoBei' ; 卷标呗，必须11字节
BS_FileSysType DB 'FAT12   ' ; 文件系统类型，必须8字节

FATSz equ 9

RootDirSectors equ 14 ; 根目录占的扇区数
SectorNoOfRootDirectory equ 19 ; 根目录起始扇区数
SectorNoOfFAT1 equ 1
DeltaSectorNo equ 17