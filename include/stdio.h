#ifndef _STDIO_H_
#define _STDIO_H_

#include "type.h"

#define ASSERT
#ifdef ASSERT
void assertion_failure(char* exp, char* file, char* base_file, int line);
// #exp 的意思是 "#"，将exp表达式包装成字符串
// __FILE__是编译器自带的宏
#define assert(exp)  if (exp) ; \
        else assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)
#else
#define assert(exp)
#endif // ASSERT

#define EXTERN extern // global.h中重新定义了这个宏

#define STR_DEFAULT_LEN 1024

#define O_CREAT 1
#define O_RDWR 2
#define O_TRUNC 4

#define SEEK_SET 1
#define SEEK_CUR 2
#define SEEK_END 3

#define MAX_PATH 128

#ifdef ENABLE_DISK_LOG
#define SYSLOG syslog
#endif // ENABLE_DISK_LOG

// lib/printf.c
PUBLIC int printf(const char* fmt, ...);
PUBLIC int printl(const char* fmt, ...);

// lib/open.c
PUBLIC int open(const char* pathname, int flags);

// lib/close.c
PUBLIC int close(int fd);

// lib/read.c
PUBLIC int read(int fd, void* buf, int count);

// lib/write.c
PUBLIC int write(int fd, const void* buf, int count);

// lib/unlink.c
PUBLIC int unlink(const char* pathname);

#endif // !_STDIO_H_
