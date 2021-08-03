#ifndef _FS_H_
#define _FS_H_

// ���豸��
struct dev_drv_map {
	int driver_nr;
};

#define MAGIC_V1 0x111

struct super_block
{
	u32 magic;
	u32 nr_inodes; // i-node����
	u32 nr_sects; // �ļ�ϵͳ�ܹ�������
	u32 nr_imap_sects; // inode-mapռ�õ�����������¼inodeʹ�����
	u32 nr_smap_sects; // sector-mapռ�õ�����������¼����ʹ�����
	u32 n_1st_sect; // ��һ������������������

	u32 nr_inode_sects; // inode_arrayռ�õ���������������inode�ڵ�������ط��洢

	u32 root_inode; // ��Ŀ¼����i-node


	u32 inode_size;
	u32 inode_isize_off;
	u32 inode_start_off;
	u32 dir_ent_size;
	u32 dir_ent_inode_off;
	u32 dir_ent_fname_off;

	int sb_dev; // ��������豸��Դ
};

#define SUPER_BLOCK_SIZE 56

struct inode {
	u32 i_mode; // �ļ�����
	u32 i_size; // �ļ���С
	u32 i_start_sect; // ��ʼ����
	u32 i_nr_sects; // �������������õ������������Ԥ����
	u8 _unused[16];

	int i_dev; // ����
	int i_cnt; // ������
	int i_num; // ���
};

#define INODE_SIZE 32

#define MAX_FILENAME_LEN 12

struct dir_entry {
	int inode_nr; // i-node
	char name[MAX_FILENAME_LEN]; // �ļ���
};

#define DIR_ENTRY_SIZE sizeof(struct dir_entry)

struct file_desc {
	int fd_mode; // �������ͣ���д��
	int fd_pos; // ��д����λ��
	struct inode* fd_inode; // ָ��Ľڵ�
};

#define RD_SECT(dev, sect_nr) rw_sector(DEV_READ, dev, sect_nr * SECTOR_SIZE, SECTOR_SIZE, TASK_FS, fsbuf);

#define WR_SECT(dev, sect_nr) rw_sector(DEV_WRITE, dev, sect_nr * SECTOR_SIZE, SECTOR_SIZE, TASK_FS, fsbuf);

#endif // !_FS_H_
