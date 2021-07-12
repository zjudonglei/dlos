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
	// gdt_ptr�Ǹ����飬ָ��
	// gdt_ptr[2]��ŵ���һ����ַ
	// &gdt_ptr[2]��ȡgdt_ptrָ��+2λ�ô���ָ��
	// u32*��ȡ��������4λָ��
	// *ָ���������ָ��ĵ�ַ
	// void*�����ַ��ָ��

	// *((u16*)(&gdt_ptr[0]))ͬ��
	memcpy(&gdt, (void*)(*((u32*)(&gdt_ptr[2]))), *((u16*)(&gdt_ptr[0])) + 1);
	u16* p_gdt_limit = (u16*)(&gdt_ptr[0]);
	u32* p_gdt_base = (u32*)(&gdt_ptr[2]);
	*p_gdt_limit = GDT_SIZE * sizeof(DESCRIPTOR) - 1;
	*p_gdt_base = (u32)&gdt;
	disp_str("-------\"cstart\" end-------\n");
}