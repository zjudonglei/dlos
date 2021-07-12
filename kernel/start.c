#include "type.h"
#include "const.h"
#include "protect.h"

PUBLIC void* memcpy(void* pDst, void* pSrc, int iSize);

PUBLIC void disp_str(char* pszInfo);

PUBLIC u8 gdt_ptr[6];

PUBLIC DESCRIPTOR gdt[GDT_SIZE];

PUBLIC void cstart() {
	disp_str("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
	"-------\"cstart\" begins-------\n");
	// gdt_ptr是个数组，指针
	// gdt_ptr[2]存放的是一个地址
	// &gdt_ptr[2]获取gdt_ptr指针+2位置处的指针
	// u32*获取接下来的4位指针
	// *指针里的内容指向的地址
	// void*这个地址的指针

	// *((u16*)(&gdt_ptr[0]))同理
	memcpy(&gdt, (void*)(*((u32*)(&gdt_ptr[2]))), *((u16*)(&gdt_ptr[0])) + 1);
	u16* p_gdt_limit = (u16*)(&gdt_ptr[0]);
	u32* p_gdt_base = (u32*)(&gdt_ptr[2]);
	*p_gdt_limit = GDT_SIZE * sizeof(DESCRIPTOR) - 1;
	*p_gdt_base = (u32)&gdt;
	disp_str("-------\"cstart\" end-------\n");
}