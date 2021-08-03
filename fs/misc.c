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
#include "hd.h"
#include "fs.h"

// 根据文件名查找节点
PUBLIC int search_file(char* path) {
	int i, j;

	char filename[MAX_PATH];
	memset(filename, 0, MAX_FILENAME_LEN);
	struct inode* dir_inode; // 在接下里的一行将置为根节点
	if (strip_path(filename, path, &dir_inode) != 0)
		return 0;

	if (filename[0] == 0)
		return dir_inode->i_num; // 根目录

	int dir_blk0_nr = dir_inode->i_start_sect;
	int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE + 1) / SECTOR_SIZE; // 已经写过内容的扇区
	int nr_dir_entries = dir_inode->i_size / DIR_ENTRY_SIZE; // 目录节点数量

	int m = 0;
	struct dir_entry* pde;

	for (i = 0; i < nr_dir_blks; i++) {

		// 挨个扇区读取
		RD_SECT(dir_inode->i_dev, (u64)(dir_blk0_nr + i));

		pde = (struct dir_entry*)fsbuf;
		// 挨个目录节点遍历
		for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++, pde++) {
			// 找到了
			if (memcmp(filename, pde->name, MAX_FILENAME_LEN) == 0) {
				return pde->inode_nr;
			}
			// 超过总节点数就返回
			if (++m > nr_dir_entries)
				break;
		}
		if (m > nr_dir_entries)
			break;
	}

	return 0;
}

// 过滤文件名开头的'/'并返回到filename，ppinode指向根节点，字符串以'0'结尾
PUBLIC int strip_path(char* filename, const char* pathname, struct inode** ppinode) {
	const char* s = pathname;
	char* t = filename;

	if (s == 0)
		return -1;

	if (*s == '/')
		s++;

	while (*s) {
		if (*s == '/')
			return -1;
		*t++ = *s++;
		if (t - filename >= MAX_FILENAME_LEN)
			break;
	}
	*t = 0;
	*ppinode = root_inode;

	return 0;
}