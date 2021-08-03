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

// �����ļ������ҽڵ�
PUBLIC int search_file(char* path) {
	int i, j;

	char filename[MAX_PATH];
	memset(filename, 0, MAX_FILENAME_LEN);
	struct inode* dir_inode; // �ڽ������һ�н���Ϊ���ڵ�
	if (strip_path(filename, path, &dir_inode) != 0)
		return 0;

	if (filename[0] == 0)
		return dir_inode->i_num; // ��Ŀ¼

	int dir_blk0_nr = dir_inode->i_start_sect;
	int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE + 1) / SECTOR_SIZE; // �Ѿ�д�����ݵ�����
	int nr_dir_entries = dir_inode->i_size / DIR_ENTRY_SIZE; // Ŀ¼�ڵ�����

	int m = 0;
	struct dir_entry* pde;

	for (i = 0; i < nr_dir_blks; i++) {

		// ����������ȡ
		RD_SECT(dir_inode->i_dev, (u64)(dir_blk0_nr + i));

		pde = (struct dir_entry*)fsbuf;
		// ����Ŀ¼�ڵ����
		for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++, pde++) {
			// �ҵ���
			if (memcmp(filename, pde->name, MAX_FILENAME_LEN) == 0) {
				return pde->inode_nr;
			}
			// �����ܽڵ����ͷ���
			if (++m > nr_dir_entries)
				break;
		}
		if (m > nr_dir_entries)
			break;
	}

	return 0;
}

// �����ļ�����ͷ��'/'�����ص�filename��ppinodeָ����ڵ㣬�ַ�����'0'��β
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