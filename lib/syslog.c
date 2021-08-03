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

PUBLIC int syslog(const char* fmt, ...) {
	int i;
	char buf[STR_DEFAULT_LEN];

	va_list arg = (va_list)((char*)(&fmt) + 4);
	i = vsprintf(buf, fmt, arg);
	assert(strlen(buf) == i);
	if (getpid() == TASK_FS) {
		return disklog(buf);
	}
	else {
		MESSAGE msg;
		msg.type = DISK_LOG;
		msg.BUF = buf;
		msg.CNT = i;
		send_recv(BOTH, TASK_FS, &msg);
		if (i != msg.CNT) {
			panic("failed to write log");
		}
		return msg.RETVAL;
	}
}