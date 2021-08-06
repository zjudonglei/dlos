#ifndef _TYPE_H_
#define _TYPE_H_

#define PUBLIC
#define PRIVATE static

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

typedef char* va_list;

typedef void (*int_handler) (); // ����ָ�룬����Ϊ�գ�����ֵvoid
typedef void (*task_f) ();
typedef void (*irq_handler) (int irq);

typedef void* system_call;

struct mess1
{
	int m1i1;
	int m1i2;
	int m1i3;
	int m1i4;
};
struct mess2
{
	void* m2p1;
	void* m2p2;
	void* m2p3;
	void* m2p4;
};
struct mess3
{
	int m3i1;
	int m3i2;
	int m3i3;
	int m3i4;
	u64 m3l1;
	u64 m3l2;
	void* m3p1;
	void* m3p2;
};

typedef struct {
	int source;
	int type;
	union { // �ڴ渲�ǵĹ�ϵ��m1i1����m2p1��m3i1ָ�����ͬһ���ڴ棬ȡ�����Ǹ��ṹ����ڴ�����
		struct mess1 m1;
		struct mess2 m2;
		struct mess3 m3;
	}u;
} MESSAGE;

struct boot_params {
	int mem_size;
	unsigned char* kernel_file;
};

#endif