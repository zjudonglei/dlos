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

PUBLIC int printf(const char* fmt, ...) {
	int i;
	char buf[256];
	// va_list��һ��ָ��ջ������ָ��
	va_list arg = (va_list)((char*)(&fmt) + 4); // fmt��һ������������ջ��λ��ջ����+4��ʾ"..."�����������ʼλ��
	i = vsprintf(buf, fmt, arg);
	int c = write(1, buf, i);

	assert(c == i);

	return i;
}

PUBLIC int printl(const char* fmt, ...) {
	int i;
	char buf[256];
	// va_list��һ��ָ��ջ������ָ��
	va_list arg = (va_list)((char*)(&fmt) + 4); // fmt��һ������������ջ��λ��ջ����+4��ʾ"..."�����������ʼλ��
	i = vsprintf(buf, fmt, arg);
	buf[i] = 0;
	printx(buf);

	return i;
}