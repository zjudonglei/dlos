#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "fs.h"
#include "console.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "proto.h"

PRIVATE void block(struct proc* p);
PRIVATE void unblock(struct proc* p);
PRIVATE int msg_send(struct proc* current, int dest, MESSAGE* m);
PRIVATE int msg_receive(struct proc* p, int src, MESSAGE* m);
PRIVATE int deadlock(int src, int dest);

PUBLIC void schedule()
{
	struct proc* p;
	int	 greatest_ticks = 0;

	while (!greatest_ticks) {
		for (p = &FIRST_PROC; p <= &LAST_PROC; p++) {
			if (p->p_flags == 0) { // �ȴ���Ϣ�Ľ��̻ᱻ����
				if (p->ticks > greatest_ticks) {
					greatest_ticks = p->ticks;
					p_proc_ready = p;
				}
			}
		}

		if (!greatest_ticks) {
			for (p = &FIRST_PROC; p <= &LAST_PROC; p++) {
				if (p->p_flags == 0) {
					p->ticks = p->priority;
				}
			}
		}
	}
}

// ϵͳ�жϽӿ�
PUBLIC int sys_sendrec(int function, int src_dest, MESSAGE* m, struct proc* p) {
	assert(k_reenter == 0);
	assert((src_dest >= 0 && src_dest < NR_TASKS + NR_PROCS) || 
		src_dest == ANY || src_dest == INTERRUPT);

	int ret = 0;
	int caller = proc2pid(p);
	MESSAGE* mla = (MESSAGE*)va2la(caller, m);
	mla->source = caller; // ������

	assert(mla->source != src_dest); // �����߲�����Ŀ�ĵ�

	if (function == SEND) {
		ret = msg_send(p, src_dest, m);
		if (ret != 0) {
			return ret;
		}
	}
	else if (function == RECEIVE) {
		ret = msg_receive(p, src_dest, m);
		if (ret != 0) {
			return ret;
		}
	}
	else {
		panic("{sys_sendrec} invalid function: "
			"%d (SEND:%d, RECEIVE:%d).", function, SEND, RECEIVE);
	}

	return 0;
}

// �û�ʹ�ýӿ�
PUBLIC int send_recv(int function, int src_dest, MESSAGE* msg) {
	int ret = 0;

	if (function == RECEIVE) {
		memset(msg, 0, sizeof(MESSAGE));
	}

	switch (function)
	{
	case BOTH:
		ret = sendrec(SEND, src_dest, msg);
		if (ret == 0) {
			ret = sendrec(RECEIVE, src_dest, msg);
		}
		break;
	case SEND:
	case RECEIVE:
		ret = sendrec(function, src_dest, msg);
		break;
	default:
		assert(function == BOTH || function == SEND || function == RECEIVE);
		break;
	}
	return ret;
}

// ldtת���Ե�ַ
PUBLIC int ldt_seg_linear(struct proc* p, int idx) {
	struct descriptor* d = &p->ldts[idx];

	return d->base_high << 24 | d->base_mid << 16 | d->base_low;
}

// ���������ĳ��ָ��ָ��ĵ�ַת���Ե�ַ
PUBLIC void* va2la(int pid, void* va) {
	struct proc* p = &proc_table[pid];

	u32 seg_base = ldt_seg_linear(p, INDEX_LDT_RW);
	u32 la = seg_base + (u32)va;

	if (pid < NR_TASKS + NR_NATIVE_PROCS) {
		assert(la == (u32)va);
	}

	return (void*)la;
}

// ��Ϣ����
PUBLIC void reset_msg(MESSAGE* p) {
	memset(p, 0, sizeof(MESSAGE));
}

PRIVATE void block(struct proc* p) {
	assert(p->p_flags);
	schedule();
}

PRIVATE void unblock(struct proc* p) {
	assert(p->p_flags == 0);
}

PRIVATE int deadlock(int src, int dest) {
	struct proc* p = proc_table + dest;
	while (1)
	{
		// ���Ŀ���߳�Ҳ�ڷ���״̬�ȵ������ҵȵ���ǰ�̵߳���Ϣ���أ��γ�����
		if (p->p_flags & SENDING) {
			if (p->p_sendto == src) {
				// ��ӡ����·��
				p = proc_table + dest;
				printl("=_=%s", p->name);
				do {
					assert(p->p_msg);
					p = proc_table + p->p_sendto;
					printl("->%s", p->name);
				} while (p != proc_table + src);
				printl("=_=");

				return 1;
			}
			p = proc_table + p->p_sendto; // ����
		}
		else {
			break;
		}
	}
	return 0;
}

// ϵͳ�ж���
PRIVATE int msg_send(struct proc* current, int dest, MESSAGE* m) {
	struct proc* sender = current;
	struct proc* p_dest = proc_table + dest;

	assert(proc2pid(sender) != dest);

	// �������
	if (deadlock(proc2pid(sender), dest)) {
		panic(">>DEADLOCK<< %s->%s", sender->name, p_dest->name);
	}

	// ���Ŀ���߳��ڵȴ����գ����ҽ��յ�ǰ�̵߳���Ϣ���������̵߳���Ϣ
	if ((p_dest->p_flags & RECEIVING) && (p_dest->p_recvfrom == proc2pid(sender) || p_dest->p_recvfrom == ANY)) {
		assert(p_dest->p_msg);
		assert(m);

		// ���Լ����Ϲҵ���Ϣ����Ŀ���߳�
		phys_copy(va2la(dest, p_dest->p_msg), va2la(proc2pid(sender), m), sizeof(MESSAGE));
		p_dest->p_msg = 0; // ��տڴ�
		p_dest->p_flags &= ~RECEIVING; // �رս���״̬
		p_dest->p_recvfrom = NO_TASK;
		unblock(p_dest);

		assert(p_dest->p_flags == 0);
		assert(p_dest->p_msg == 0);
		assert(p_dest->p_recvfrom == NO_TASK);
		assert(p_dest->p_sendto == NO_TASK);
		assert(sender->p_flags == 0);
		assert(sender->p_msg == 0);
		assert(sender->p_recvfrom == NO_TASK);
		assert(sender->p_sendto == NO_TASK);
	}
	else {
		sender->p_flags |= SENDING; // ������״̬
		assert(sender->p_flags == SENDING);
		sender->p_sendto = dest;
		sender->p_msg = m;

		// �ӵ��ȴ��б�ĩβ
		struct proc* p;
		if (p_dest->q_sending) {
			p = p_dest->q_sending;
			while (p->next_sending) {
				p = p->next_sending;
			}
			p->next_sending = sender;
		}
		else {
			p_dest->q_sending = sender; // ��ͷ
		}
		sender->next_sending = 0;

		// ע���ˣ�����ѽ����л����ˣ����»ص�kernel.asm�е�sys_callʱ
		// �����return 0���Ǳ�mov��sender�Ľ��̱�stackframe��Ӧ��eax��λ�ã�����iretd�Ѿ��л������߳���
		// ��get_ticksΪ����send��֮��Ϳ���send_recv case Both�� if ret == 0��һ��
		// ��sys_task receive֮�󣬻��get_ticks���ѣ�ִ�е�if ret == 0��һ�У�����ʹ���ͽ�����
		// �������Ҫreceive���߳̾���������������
		// �����Ҫreceive���ͻ�����µ�block��ֱ��sys_task send��֮���ٴλ���������
		block(sender);

		assert(sender->p_flags == SENDING);
		assert(sender->p_msg != 0);
		assert(sender->p_recvfrom == NO_TASK);
		assert(sender->p_sendto == dest);
	}

	return 0; // �����Ǻܾ���ģ�һ���淵�ص�sender��һ���滹�ܰ��߳��л������߳�
}

PRIVATE int msg_receive(struct proc* current, int src, MESSAGE* m) {
	struct proc* p_who_wanna_recv = current;
	struct proc* p_from = 0;
	struct proc* prev = 0;
	int copyok = 0;

	assert(proc2pid(p_who_wanna_recv) != src);

	if (p_who_wanna_recv->has_int_msg && (src == ANY || src == INTERRUPT)) {
		// ���⻯һ����Ϣ��������
		MESSAGE msg;
		reset_msg(&msg);
		msg.source = INTERRUPT;
		msg.type = HARD_INT;

		assert(m);
		phys_copy(va2la(proc2pid(p_who_wanna_recv), m), &msg, sizeof(MESSAGE));
		p_who_wanna_recv->has_int_msg = 0;
		assert(p_who_wanna_recv->p_flags == 0);
		assert(p_who_wanna_recv->p_msg == 0);
		assert(p_who_wanna_recv->p_sendto == NO_TASK);
		assert(p_who_wanna_recv->has_int_msg == 0);

		return 0;
	}

	if (src == ANY) {
		// ��Ϣ�б�Ϊ��
		if (p_who_wanna_recv->q_sending) {
			p_from = p_who_wanna_recv->q_sending;
			copyok = 1;

			assert(p_who_wanna_recv->p_flags == 0);
			assert(p_who_wanna_recv->p_msg == 0);
			assert(p_who_wanna_recv->p_recvfrom == NO_TASK);
			assert(p_who_wanna_recv->p_sendto == NO_TASK);
			assert(p_who_wanna_recv->q_sending != 0);
			assert(p_from->p_flags == SENDING);
			assert(p_from->p_msg != 0);
			assert(p_from->p_recvfrom == NO_TASK);
			assert(p_from->p_sendto == proc2pid(p_who_wanna_recv));
		}
	}
	// ֻ����ָ���̵߳���Ϣ
	else if(src >= 0 && src < NR_TASKS + NR_PROCS) {
		p_from = &proc_table[src];
		// ���������ǡ��Ҳ������Ϣ�����ҷ����˽�����
		if ((p_from->p_flags & SENDING) && p_from->p_sendto == proc2pid(p_who_wanna_recv)) {
			copyok = 1;
			
			struct proc* p = p_who_wanna_recv->q_sending;
			assert(p);

			while (p) {
				assert(p_from->p_flags & SENDING);
				if (proc2pid(p) == src) { // �ҵ���
					break;
				}
				prev = p; // �����Ƴ�����ڵ���
				p = p->next_sending;
			}

			assert(p_who_wanna_recv->p_flags == 0);
			assert(p_who_wanna_recv->p_msg == 0);
			assert(p_who_wanna_recv->p_recvfrom == NO_TASK);
			assert(p_who_wanna_recv->p_sendto == NO_TASK);
			assert(p_who_wanna_recv->q_sending != 0);
			assert(p_from->p_flags == SENDING);
			assert(p_from->p_msg != 0);
			assert(p_from->p_recvfrom == NO_TASK);
			assert(p_from->p_sendto == proc2pid(p_who_wanna_recv));
		}
	}
	// �������Ϣ
	if (copyok) {
		if (p_from == p_who_wanna_recv->q_sending) { // ͷ�ڵ�
			assert(prev == 0);
			p_who_wanna_recv->q_sending = p_from->next_sending;
			p_from->next_sending = 0;
		}
		else {
			assert(prev);
			prev->next_sending = p_from->next_sending;
			p_from->next_sending = 0;
		}

		assert(m);
		assert(p_from->p_msg);
		phys_copy(va2la(proc2pid(p_who_wanna_recv), m), va2la(proc2pid(p_from), p_from->p_msg), sizeof(MESSAGE));
		p_from->p_msg = 0; // �������ߵĿڴ����
		p_from->p_sendto = NO_TASK;
		p_from->p_flags &= ~SENDING;

		unblock(p_from);
	}
	else {
		p_who_wanna_recv->p_flags |= RECEIVING; // ������״̬
		p_who_wanna_recv->p_msg = m; // ����װ���ڴ���
		p_who_wanna_recv->p_recvfrom = src;
		block(p_who_wanna_recv);

		assert(p_who_wanna_recv->p_flags == RECEIVING);
		assert(p_who_wanna_recv->p_msg != 0);
		assert(p_who_wanna_recv->p_recvfrom != NO_TASK);
		assert(p_who_wanna_recv->p_sendto == NO_TASK);
		assert(p_who_wanna_recv->has_int_msg == 0);
	}

	return 0;
}

PUBLIC void dump_proc(struct proc* p) {
	char info[STR_DEFAULT_LEN];
	int i;
	int text_color = MAKE_COLOR(GREEN, RED);
	int dump_len = sizeof(struct proc);

	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, 0);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, 0);

	sprintf(info, "byte dump of proc_table[%d]:\n", p - proc_table); // �ַ����滻
	disp_color_str(info, text_color);
	for (i = 0; i < dump_len; i++) {
		sprintf(info, "%x.", ((unsigned char*) p)[i]); // �ַ����滻
		disp_color_str(info, text_color);
	}

	disp_color_str("\n\n", text_color);
	sprintf(info, "ANY: 0x%x.\n", ANY);
	disp_color_str(info, text_color);
	sprintf(info, "NO_TASK: 0x%x.\n", NO_TASK);
	disp_color_str(info, text_color);
	disp_color_str("\n", text_color);

	sprintf(info, "ldt_sel: 0x%x.  ", p->ldt_sel); disp_color_str(info, text_color);
	sprintf(info, "ticks: 0x%x.  ", p->ticks); disp_color_str(info, text_color);
	sprintf(info, "priority: 0x%x.  ", p->priority); disp_color_str(info, text_color);
	//sprintf(info, "pid: 0x%x.  ", p->pid); disp_color_str(info, text_color);
	sprintf(info, "name: %s.  ", p->name); disp_color_str(info, text_color);
	disp_color_str("\n", text_color);
	sprintf(info, "p_flags: 0x%x.  ", p->p_flags); disp_color_str(info, text_color);
	sprintf(info, "p_recvfrom: 0x%x.  ", p->p_recvfrom); disp_color_str(info, text_color);
	sprintf(info, "p_sendto: 0x%x.  ", p->p_sendto); disp_color_str(info, text_color);
	//sprintf(info, "nr_tty: 0x%x.  ", p->nr_tty); disp_color_str(info, text_color);
	disp_color_str("\n", text_color);
	sprintf(info, "has_int_msg: 0x%x.  ", p->has_int_msg); disp_color_str(info, text_color);
}

PUBLIC void dump_msg(const char* title, MESSAGE* m) {
	int packed = 0;
	printl("{%s}<0x%x>{%ssrc:%s(%d),%stype:%d,%s(0x%x,0x%x,0x%x,0x%x,0x%x,0x%x)%s}%s",
		title,
		(int)m,
		packed ? "" : "\n        ",
		proc_table[m->source].name,
		m->source,
		packed ? " " : "\n        ", 
		m->type,
		packed ? " " : "\n        ", 
		m->u.m3.m3i1,
		m->u.m3.m3i2, 
		m->u.m3.m3i3, 
		m->u.m3.m3i4,
		(int)m->u.m3.m3p1,
		(int)m->u.m3.m3p2,
		packed ? "" : "\n", 
		packed ? "" : "\n"
	);
}

// ���жϰ�װ��message
PUBLIC void inform_int(int task_nr) {
	struct proc* p = proc_table + task_nr;

	if ( (p->p_flags & RECEIVING) && (p->p_recvfrom == INTERRUPT || p->p_recvfrom == ANY)) {
		p->p_msg->source = INTERRUPT;
		p->p_msg->type = HARD_INT;
		p->p_msg = 0;
		p->has_int_msg = 0;
		p->p_flags &= ~RECEIVING;
		p->p_recvfrom = NO_TASK;
		assert(p->p_flags == 0);
		unblock(p);

		assert(p->p_flags == 0);
		assert(p->p_msg == 0);
		assert(p->p_recvfrom == NO_TASK);
		assert(p->p_sendto == NO_TASK);
	}
	else { // ��msg_recv�д���
		p->has_int_msg = 1;
	}
}