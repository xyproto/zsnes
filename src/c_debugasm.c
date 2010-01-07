#include "c_debugasm.h"
#include "debugasm.h"
#include "debugger.h"
#include "types.h"


void breakops_wrapper(void)
{
	u4 eax;
	u4 ecx = PrevBreakPt_offset;
	u4 edx;
	u4 ebx = PrevBreakPt_page;
	u4 esi;
	u4 edi;
	asm volatile("push %%ebp;  call %P6;  pop %%ebp" : "=a" (eax), "+c" (ecx), "=d" (edx), "+b" (ebx), "=S" (esi), "=D" (edi) : "X" (breakops) : "cc", "memory");
}
