#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

PRIVATE struct inode* create_file(char* path, int flags);
PRIVATE int alloc_imap_bit(int dev);
PRIVATE int alloc_smap_bit(int dev, int nr_sects_to_alloc);
PRIVATE struct inode* new_inode(int dev, int inode_nr, int start_sect);
PRIVATE void new_dir_entry(struct inode* dir_inode, int inode_nr, char* filename);

PUBLIC int do_open() {

	int fd = -1;
	char pathname[MAX_PATH];

	int flags = fs_msg.FLAGS; // 创建，读，写等
	int name_len = fs_msg.NAME_LEN; // 用于下面跨访问级别的拷贝
	int src = fs_msg.source;
	assert(name_len < MAX_PATH);
	phys_copy((void*)va2la(TASK_FS, pathname),
		(void*)va2la(src, fs_msg.PATHNAME),
		name_len);
	pathname[name_len] = 0; // 末尾置0

	// 在进程表中找到一个可用的序号来存放文件描述节点
	int i;
	for (i = 0; i < NR_FILES; i++) {
		if (pcaller->filp[i] == 0) {
			fd = i;
			break;
		}
	}

	if (fd < 0 || fd >= NR_FILES) {
		panic("filp[] is full (PID:%d)", proc2pid(pcaller));
	}

	// 在全局的文件描述节点缓存中找到一个空的文件描述节点
	for (i = 0; i < NR_FILE_DESC; i++)
		if (f_desc_table[i].fd_inode == 0)
			break;
	if (i >= NR_FILE_DESC)
		panic("f_desc_table[] is full (PID:%d)", proc2pid(pcaller));

	// 在磁盘上查找节点
	int inode_nr = search_file(pathname);

	struct inode* pin = 0;
	if (flags & O_CREAT) {
		if (inode_nr) {
			return -1;
		}
		else {
			// 创建文件
			pin = create_file(pathname, flags);
		}
	}
	else {
		assert(flags & O_RDWR);

		char filename[MAX_PATH];
		struct inode* dir_inode;
		if (strip_path(filename, pathname, &dir_inode) != 0)
			return -1;
		// 在磁盘上加载节点
		pin = get_inode(dir_inode->i_dev, inode_nr);

	}

	if (pin) {

		// 将文件描述节点对应到进程表的对应位置
		pcaller->filp[fd] = &f_desc_table[i];
		// 将节点保存到文件描述缓存中
		f_desc_table[i].fd_inode = pin;
		f_desc_table[i].fd_mode = flags;
		f_desc_table[i].fd_pos = 0;

		int imode = pin->i_mode & I_TYPE_MASK;
		if (imode == I_CHAR_SPECIAL) {
			MESSAGE driver_msg;
			// 打开设备
			driver_msg.type = DEV_OPEN;
			int dev = pin->i_start_sect;
			driver_msg.DEVICE = MINOR(dev);
			assert(MAJOR(dev) == 4);
			assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);

			send_recv(BOTH, dd_map[MAJOR(dev)].driver_nr, &driver_msg);
		}
		else if (imode == I_DIRECTORY) {
			assert(pin->i_num == ROOT_INODE);
		}
		else {
			assert(pin->i_mode == I_REGULAR);
		}
	}
	else {
		return -1;
	}

	return fd;
}

// 关闭节点
PUBLIC int do_close() {
	int fd = fs_msg.FD;
	put_inode(pcaller->filp[fd]->fd_inode);
	pcaller->filp[fd]->fd_inode = 0;
	pcaller->filp[fd] = 0;

	return 0;
}

// 创建文件
PRIVATE struct inode* create_file(char* path, int flags) {
	char filename[MAX_PATH];
	struct inode* dir_inode;

	if (strip_path(filename, path, &dir_inode) != 0)
		return 0;

	// 在inode-map中分配1个bit标记
	int inode_nr = alloc_imap_bit(dir_inode->i_dev);

	// 在sector-map中分配NR_DEFAULT_FILE_SECTS个bit标记
	int free_sect_nr = alloc_smap_bit(dir_inode->i_dev, NR_DEFAULT_FILE_SECTS);

	// 新节点
	struct inode* newino = new_inode(dir_inode->i_dev, inode_nr, free_sect_nr);

	new_dir_entry(dir_inode, newino->i_num, filename);

	return newino;
}

// inode-map中分配bit
PRIVATE int alloc_imap_bit(int dev) {
	int inode_nr = 0;
	int i, j, k;

	int imap_blk0_nr = 1 + 1;
	struct super_block* sb = get_super_block(dev);

	// 遍历inode-map所在扇区
	for (i = 0; i < sb->nr_imap_sects; i++) {
		RD_SECT(dev, (u32)(imap_blk0_nr + i));

		for (j = 0; j < SECTOR_SIZE; j++) {
			if (fsbuf[j] == 0xFF) // 8个bit全部用光
				continue;

			for (k = 0; ((fsbuf[j] >> k) & 1) != 0; k++) { // 8个bit中找到一个空bit

			}

			inode_nr = (i * SECTOR_SIZE + j) * 8 + k; // inode-map序号
			fsbuf[j] |= (1 << k); // 对应bit置1

			WR_SECT(dev, (u32)(imap_blk0_nr + i)); // 写磁盘
			break;
		}

		return inode_nr;
	}

	panic("inode-map is probably full.\n");

	return 0;
}

// 原理同上，但是这里是寻找一大片可用扇区
PRIVATE int alloc_smap_bit(int dev, int nr_sects_to_alloc) {
	int i;
	int j;
	int k;

	struct super_block* sb = get_super_block(dev);

	int smap_blk0_nr = 1 + 1 + sb->nr_imap_sects;
	int free_sect_nr = 0;

	for (i = 0; i < sb->nr_smap_sects; i++) {
		RD_SECT(dev, (u32)(smap_blk0_nr + i));

		for (j = 0; j < SECTOR_SIZE && nr_sects_to_alloc > 0; j++) {
			k = 0;
			if (!free_sect_nr) {
				if (fsbuf[j] == 0xFF) 
					continue;
				for(;((fsbuf[j] >> k) & 1) != 0; k++){}

				// 起始扇区
				free_sect_nr = (i * SECTOR_SIZE + j) * 8 + k - 1 + sb->n_1st_sect;
			}

			for (; k < 8; k++) {
				assert(((fsbuf[j] >> k) & 1) == 0); // 获取大片扇区失败
				fsbuf[j] |= 1 << k;
				if (--nr_sects_to_alloc == 0)
					break;
			}
		}

		if (free_sect_nr)
			WR_SECT(dev, (u32)(smap_blk0_nr + i));

		if (nr_sects_to_alloc == 0)
			break;
	}

	assert(nr_sects_to_alloc == 0);

	return free_sect_nr;
}

PRIVATE struct inode* new_inode(int dev, int inode_nr, int start_sect) {
	struct inode* new_inode = get_inode(dev, inode_nr);

	new_inode->i_mode = I_REGULAR;
	new_inode->i_size = 0;
	new_inode->i_start_sect = start_sect;
	new_inode->i_nr_sects = NR_DEFAULT_FILE_SECTS;

	new_inode->i_dev = dev;
	new_inode->i_cnt = 1;
	new_inode->i_num = inode_nr; // 节点标志位

	sync_inode(new_inode);

	return new_inode;
}

// 在根节点中添加一个dir_entry
PRIVATE void new_dir_entry(struct inode* dir_inode, int inode_nr, char* filename) {
	int dir_blk0_nr = dir_inode->i_start_sect;
	int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE) / SECTOR_SIZE;
	int nr_dir_entries = dir_inode->i_size / DIR_ENTRY_SIZE;
	int m = 0;
	struct dir_entry* pde;
	struct dir_entry* new_de = 0;

	int i, j;
	for (i = 0; i < nr_dir_blks; i++) {
		RD_SECT(dir_inode->i_dev, (u32)(dir_blk0_nr + i));
		pde = (struct dir_entry*)fsbuf;

		for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++, pde++) {
			if (++m > nr_dir_entries) {
				break;
			}

			if (pde->inode_nr == 0) { // 找到一个空的dir_entry
				new_de = pde;
				break;
			}
		}

		if (m > nr_dir_entries || new_de)
			break;
	}
	if (!new_de) {
		new_de = pde;
		dir_inode->i_size += DIR_ENTRY_SIZE; // 增加长度
	}
	// 新节点初始化
	new_de->inode_nr = inode_nr;
	strcpy(new_de->name, filename);

	WR_SECT(dir_inode->i_dev, (u32)(dir_blk0_nr + i)); // 向数据库更新根节点变化的部分

	sync_inode(dir_inode); // 更新根节点
}