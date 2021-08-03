#ifndef _FS_H_
#define _FS_H_

// 主设备号
struct dev_drv_map {
	int driver_nr;
};

#define MAGIC_V1 0x111

struct super_block
{
	u32 magic;
	u32 nr_inodes; // i-node数量
	u32 nr_sects; // 文件系统总共扇区数
	u32 nr_imap_sects; // inode-map占用的扇区数，记录inode使用情况
	u32 nr_smap_sects; // sector-map占用的扇区数，记录扇区使用情况
	u32 n_1st_sect; // 第一个数据扇区的扇区号

	u32 nr_inode_sects; // inode_array占用的扇区数，真正的inode节点在这个地方存储

	u32 root_inode; // 根目录区的i-node


	u32 inode_size;
	u32 inode_isize_off;
	u32 inode_start_off;
	u32 dir_ent_size;
	u32 dir_ent_inode_off;
	u32 dir_ent_fname_off;

	int sb_dev; // 超级块的设备来源
};

#define SUPER_BLOCK_SIZE 56

struct inode {
	u32 i_mode; // 文件类型
	u32 i_size; // 文件大小
	u32 i_start_sect; // 起始扇区
	u32 i_nr_sects; // 总扇区数，可用的最大扇区数，预留好
	u8 _unused[16];

	int i_dev; // 分区
	int i_cnt; // 打开数量
	int i_num; // 编号
};

#define INODE_SIZE 32

#define MAX_FILENAME_LEN 12

struct dir_entry {
	int inode_nr; // i-node
	char name[MAX_FILENAME_LEN]; // 文件名
};

#define DIR_ENTRY_SIZE sizeof(struct dir_entry)

struct file_desc {
	int fd_mode; // 操作类型，读写等
	int fd_pos; // 读写到的位置
	struct inode* fd_inode; // 指向的节点
};

#define RD_SECT(dev, sect_nr) rw_sector(DEV_READ, dev, sect_nr * SECTOR_SIZE, SECTOR_SIZE, TASK_FS, fsbuf);

#define WR_SECT(dev, sect_nr) rw_sector(DEV_WRITE, dev, sect_nr * SECTOR_SIZE, SECTOR_SIZE, TASK_FS, fsbuf);

#endif // !_FS_H_
