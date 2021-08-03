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
#include "proto.h"
#include "hd.h"

PRIVATE void init_hd();
PRIVATE void hd_open(int device);
PRIVATE void hd_close(int device);
PRIVATE void hd_rdwt(MESSAGE* p);
PRIVATE void hd_ioctl(MESSAGE* p);
PRIVATE void hd_cmd_out(struct hd_cmd* cmd);
PRIVATE void get_part_table(int drive, int sect_nr, struct part_ent* entry);
PRIVATE void partition(int device, int style);
PRIVATE void print_hdinfo(struct hd_info* hdi);
PRIVATE int waitfor(int mask, int val, int timeout);
PRIVATE void interrupt_wait();
PRIVATE void hd_identify(int drive);
PRIVATE void print_identify_info(u16* hdinfo);

PRIVATE u8 hd_status;
PRIVATE u8 hdbuf[SECTOR_SIZE * 2];
PRIVATE struct hd_info hd_info[1];

// ���ݴ��豸�ŵõ����̺�
#define DRV_OF_DEV(dev) (dev <= MAX_PRIM ? dev / NR_PRIM_PER_DRIVE : (dev - MINOR_hd1a) / NR_SUB_PER_DRIVE)

PUBLIC void task_hd() {
	MESSAGE msg;

	init_hd();

	while (1) {
		send_recv(RECEIVE, ANY, &msg);
		int src = msg.source;
		switch (msg.type)
		{
		case DEV_OPEN:
			hd_open(msg.DEVICE);
			break;
		case DEV_CLOSE:
			hd_close(msg.DEVICE);
			break;
		case DEV_READ:
		case DEV_WRITE:
			hd_rdwt(&msg);
			break;
		case DEV_IOCTL:
			hd_ioctl(&msg);
			break;
		default:
			dump_msg("HD driver::unknown msg", &msg);
			spin("FS::main_loop (invalid msg.type)");
			break;
		}
		send_recv(SEND, src, &msg);
	}
}

PRIVATE void init_hd() {
	int i;
	u8* pNrDrives = (u8*)(0x475); // ϵͳ����������BIOSָ��
	printl("NrDrives:%d.\n", *pNrDrives);
	assert(*pNrDrives);

	put_irq_handler(AT_WINI_IRQ, hd_handler);
	enable_irq(CASCADE_IRQ); // ����2�Ŷ˿ڴ����ж�
	enable_irq(AT_WINI_IRQ); // ��������Ӳ���ж�

	// ��ʼ��Ӳ����Ϣ��¼
	for (i = 0; i < (sizeof(hd_info) / sizeof(hd_info[0])); i++) {
		memset(&hd_info[i], 0, sizeof(hd_info[0]));
	}
	hd_info[0].open_cnt = 0;
}

PRIVATE void hd_open(int device) {
	int drive = DRV_OF_DEV(device);
	assert(drive == 0);

	hd_identify(drive);

	// ��һ�δ�
	if (hd_info[drive].open_cnt++ == 0) {
		partition(drive * (NR_PART_PER_DRIVE + 1), P_PRIMARY); // ��ȡ��������Ϣ
		print_hdinfo(&hd_info[drive]);
	}
}

PRIVATE void hd_close(int device) {
	int drive = DRV_OF_DEV(device);
	assert(drive == 0);

	hd_info[drive].open_cnt--;
}

PRIVATE void hd_rdwt(MESSAGE* p) {
	int device = p->DEVICE;
	int drive = DRV_OF_DEV(device);

	u64 pos = p->POSITION;
	assert(pos >> SECTOR_SIZE_SHIFT < 1 << 31);
	//printl("pos when receive: 0x%x\n", pos);
	assert((pos & 0x1FF) == 0); // ֻ�����������ʼ����ʼд

	u32 sect_nr = (u32)(pos >> SECTOR_SIZE_SHIFT); // λ��/�����ֽ���
	int logidx = (p->DEVICE - MINOR_hd1a) % NR_SUB_PER_DRIVE;
	// ��ǰ������ʼ����+ƫ������
	sect_nr += p->DEVICE < MAX_PRIM ?
		hd_info[drive].primary[p->DEVICE].base :
		hd_info[drive].logical[logidx].base;

	struct hd_cmd cmd;
	cmd.features = 0;
	cmd.count = (p->CNT + SECTOR_SIZE - 1) / SECTOR_SIZE; // ����������������֤����1��������ȡ
	cmd.lba_low = sect_nr & 0xFF;
	cmd.lba_mid = (sect_nr >> 8) & 0xFF;
	cmd.lba_high = (sect_nr >> 16) & 0xFF;
	cmd.device = MAKE_DEVICE_REG(1, drive, (sect_nr >> 24) & 0xF);
	cmd.command = (p->type == DEV_READ) ? ATA_READ : ATA_WRITE;
	hd_cmd_out(&cmd);

	int bytes_left = p->CNT;
	void* la = (void*)va2la(p->PROC_NR, p->BUF);

	while (bytes_left) {
		int bytes = min(SECTOR_SIZE, bytes_left);
		if (p->type == DEV_READ) {
			interrupt_wait();
			port_read(REG_DATA, hdbuf, SECTOR_SIZE);
			phys_copy(la, (void*)va2la(TASK_HD, hdbuf), bytes);
		}
		else {
			if (!waitfor(STATUS_DRQ, STATUS_DRQ, HD_TIMEOUT)) {
				panic("hd writing error.");
			}

			port_write(REG_DATA, la, bytes);
			interrupt_wait();
		}
		bytes_left -= SECTOR_SIZE;
		la += SECTOR_SIZE;
	}
}

PRIVATE void hd_ioctl(MESSAGE* p) {
	int device = p->DEVICE;
	int drive = DRV_OF_DEV(device);

	struct hd_info* hdi = &hd_info[drive];

	if (p->REQUEST == DIOCTL_GET_GEO) {
		void* dst = va2la(p->PROC_NR, p->BUF);
		void* src = va2la(TASK_HD, device < MAX_PRIM ? &hdi->primary[device] : 
			&hdi->logical[(device - MINOR_hd1a) % NR_SUB_PER_DRIVE]);
		phys_copy(dst, src, sizeof(struct part_info));
	}
	else {
		assert(0);
	}
}

// ��ȡӲ����Ϣ
PRIVATE void get_part_table(int drive, int sect_nr, struct part_ent* entry) {
	struct hd_cmd cmd;
	cmd.features = 0;
	cmd.count = 1;
	cmd.lba_low = sect_nr & 0xFF;
	cmd.lba_mid = (sect_nr >> 8) & 0xFF;
	cmd.lba_high = (sect_nr >> 16) & 0xFF;
	cmd.device = MAKE_DEVICE_REG(1, drive, (sect_nr >> 24) & 0xF); // ��7λ��1��LBA ģʽ
	cmd.command = ATA_READ;
	hd_cmd_out(&cmd);
	interrupt_wait();

	port_read(REG_DATA, hdbuf, SECTOR_SIZE);
	memcpy(entry, hdbuf + PARTITION_TABLE_OFFSET, sizeof(struct part_ent) * NR_PART_PER_DRIVE);
}

// ��ȡ������
// device:�������ľ���Ӳ�̱��0��5
PRIVATE void partition(int device, int style) {
	int i;
	int drive = DRV_OF_DEV(device); // Ŀǰֻ��һ����̣����driveΪ0
	struct hd_info* hdi = &hd_info[drive];

	struct part_ent part_tbl[NR_SUB_PER_DRIVE];
	if (style == P_PRIMARY) {
		get_part_table(drive, drive, part_tbl); // �ڶ���driveӦ����0

		int nr_prim_parts = 0;
		for (i = 0; i < NR_PART_PER_DRIVE; i++) { // 0~3
			if (part_tbl[i].sys_id == NO_PART) {
				continue;
			}
			nr_prim_parts++;
			int dev_nr = i + 1; // 1~4
			hdi->primary[dev_nr].base = part_tbl[i].start_sect;
			hdi->primary[dev_nr].size = part_tbl[i].nr_sects;

			if (part_tbl[i].sys_id == EXT_PART) {
				partition(device + dev_nr, P_EXTENDED); // ����
			}
		}
		assert(nr_prim_parts != 0);
	}
	else if(style == P_EXTENDED) {
		int j = device % NR_PRIM_PER_DRIVE; // 1~4
		int ext_start_sect = hdi->primary[j].base;
		int s = ext_start_sect;
		int nr_1st_sub = (j - 1) * NR_SUB_PER_PART; // 0/16/32/46

		for (i = 0; i < NR_SUB_PER_PART; i++) {
			int dev_nr = nr_1st_sub + i;/* 0~15/16~31/32~47/48~63 */

			get_part_table(drive, s, part_tbl);

			// ��ǰ������Ϣ
			hdi->logical[dev_nr].base = s + part_tbl[0].start_sect;
			hdi->logical[dev_nr].size = part_tbl[0].nr_sects;

			// ��չ���߼��������γ��ܲ�������
			s = ext_start_sect + part_tbl[1].start_sect;

			if (part_tbl[1].sys_id == NO_PART) {
				break;
			}
		}
	}
	else {
		assert(0);
	}
}

PRIVATE void print_hdinfo(struct hd_info* hdi) {
	int i;
	for (i = 0; i < NR_PART_PER_DRIVE + 1; i++) {
		printl("%sPART_%d: base %d(0x%x), size %d(0x%x) (in sector)\n",
			i == 0 ? " " : "    ",
			i,
			hdi->primary[i].base,
			hdi->primary[i].base, 
			hdi->primary[i].size, 
			hdi->primary[i].size);
	}
	for (i = 0; i < NR_SUB_PER_DRIVE; i++) {
		if (hdi->logical[i].size == 0)
			continue;
		printl("        %d: base %d(0x%x), size %d(0x%x) (in sector)\n",
			i,
			hdi->logical[i].base,
			hdi->logical[i].base,
			hdi->logical[i].size,
			hdi->logical[i].size);
	}
}

PRIVATE void hd_identify(int drive) {
	struct hd_cmd cmd;

	cmd.device = MAKE_DEVICE_REG(0, drive, 0); // �����ע��
	cmd.command = ATA_IDENTIFY;
	hd_cmd_out(&cmd);
	interrupt_wait(); // �ȴ�Ӳ���жϷ�����Ϣ
	port_read(REG_DATA, hdbuf, SECTOR_SIZE);

	print_identify_info((u16*)hdbuf);

	u16* hdinfo = (u16*)hdbuf;
	hd_info[drive].primary[0].base = 0;
	hd_info[drive].primary[0].size = ((int)hdinfo[61] << 16) + hdinfo[60];
}

PRIVATE void print_identify_info(u16* hdinfo) {
	int i, k;
	char s[64];

	struct iden_info_ascii
	{
		int idx;
		int len;
		char* desc;
	}iinfo[] = { {10, 20, "HD SN"}, // 10-19���к�
	{27, 40, "HD Model"} }; // 27-46�ͺ�

	for (k = 0; k < sizeof(iinfo) / sizeof(iinfo[0]); k++) {
		char* p = (char*)&hdinfo[iinfo[k].idx];
		for (i = 0; i < iinfo[k].len / 2; i++) {
			s[i * 2 + 1] = *p++;
			s[i * 2] = *p++;
		}
		s[i * 2] = 0;
		printl("%s: %s\n", iinfo[k].desc, s);
	}

	int capabilities = hdinfo[49]; // ���ܣ�bit9Ϊ1֧��lba
	printl("LBA supported: %s\n", (capabilities & 0x0200) ? "Yes" : "No");

	int cmd_set_supported = hdinfo[83]; // ֧�ֵ�ָ���bit10Ϊ1��ʾ֧��48λѰַ
	printl("LBA48 supported: %s\n", (cmd_set_supported & 0x0400) ? "Yes" : "No");

	int sectors = ((int)hdinfo[61] << 16) + hdinfo[60]; // �û�������������
	printl("HD size: %dMB\n", sectors * 512 / 1000000);
}

PRIVATE void hd_cmd_out(struct hd_cmd* cmd) {
	// �ȴ�״̬�Ĵ����ղ�������
	if (!waitfor(STATUS_BSY, 0, HD_TIMEOUT)) {
		panic("hd error.");
	}

	// ��IDE�����Ĵ���д��ֵ
	// https://www.cnblogs.com/weiweishuo/archive/2013/05/26/3100254.html
	out_byte(REG_DEV_CTRL, 0); // ���ж�

	out_byte(REG_FEATURES, cmd->features);
	out_byte(REG_NSECTOR, cmd->count);
	out_byte(REG_LBA_LOW, cmd->lba_low);
	out_byte(REG_LBA_MID, cmd->lba_mid);
	out_byte(REG_LBA_HIGH, cmd->lba_high);
	out_byte(REG_DEVICE, cmd->device);
	out_byte(REG_CMD, cmd->command); // ָ������
}

PRIVATE void interrupt_wait() {
	MESSAGE msg;
	send_recv(RECEIVE, INTERRUPT, &msg);
}

PRIVATE int waitfor(int mask, int val, int timeout) {
	int t = get_ticks();

	while ((get_ticks() - t) * 1000 / HZ < timeout)
	{
		if ((in_byte(REG_STATUS) & mask) == val) {
			return 1;
		}
	}
	return 0;
}

PUBLIC void hd_handler(int irq) {
	hd_status = in_byte(REG_STATUS); // ��ȡ״̬λ���ָ��ж���Ӧ
	inform_int(TASK_HD);
}