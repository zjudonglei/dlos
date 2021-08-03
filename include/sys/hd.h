#ifndef _HD_H_
#define _HD_H_

// 格式化硬盘后的硬盘信息节点
struct part_ent
{
	u8 boot_ind;
	u8 start_head;
	u8 start_sector;
	u8 start_cyl;
	u8 sys_id; // 分区类型
	u8 end_head;
	u8 end_sector;
	u8 end_cyl;
	u32 start_sect;
	u32 nr_sects;
}PARTITION_ENTRY;

// 硬盘控制器里的寄存器
#define REG_DATA 0x1F0
#define REG_FEATURES 0x1F1
#define REG_ERROR REG_FEATURES
#define REG_NSECTOR 0x1F2
#define REG_LBA_LOW 0x1F3
#define REG_LBA_MID 0x1F4
#define REG_LBA_HIGH 0x1F5
#define REG_DEVICE 0x1F6
#define REG_STATUS 0x1F7
// 状态寄存器每位对应的值
#define STATUS_BSY 0x80
#define STATUS_DRDY 0x40
#define STATUS_DFSE 0x20
#define STATUS_DSC 0x10
#define STATUS_DRQ 0x08
#define STATUS_CORR 0x04
#define STATUS_IDX 0x02
#define STATUS_ERR 0x01

#define REG_CMD REG_STATUS
#define REG_DEV_CTRL 0x3F6 // 硬盘IDE接口引脚
#define REG_ALT_STATUS REG_DEV_CTRL
#define REG_DRV_ADDR 0x3F7

#define MAX_IO_BYTES 256

struct hd_cmd {
	u8 features;
	u8 count;
	u8 lba_low;
	u8 lba_mid;
	u8 lba_high;
	u8 device;
	u8 command;
};

// 分区信息
struct part_info {
	u32 base; // 起始地址
	u32 size; // 大小
};

// 硬盘信息
struct hd_info {
	int open_cnt; // 打开数量
	struct part_info primary[NR_PRIM_PER_DRIVE]; // 主分区
	struct part_info logical[NR_SUB_PER_DRIVE]; // 逻辑分区
};

#define HD_TIMEOUT 10000
#define PARTITION_TABLE_OFFSET 0x1BE // 第一个分区信息开始的地方
// 指令
#define ATA_IDENTIFY 0xEC
#define ATA_READ 0x20
#define ATA_WRITE 0x30

// 1 L 1 DRV HS3 HS2 HS1 HS0
// L0:低4位表示CHS模式，柱面/磁头/扇区号 定位扇区
// L1:低4位表示LBA的24-27位，LBA磁盘外圈容量比内圈大
// DRV:磁盘驱动器号，可能有多个磁盘
#define MAKE_DEVICE_REG(lba, drv, lba_highest) (lba << 6) | (drv << 4) | (lba_highest & 0xF) | 0xA0

#endif // !_HD_H_
