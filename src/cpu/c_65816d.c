#include "../chips/sa1regs.h"
#include "../init.h"
#include "c_65816d.h"
#include "s65816d.h"


static void restoredl(u4 const edx)
{
	u4 nz = 0;
	if   (edx & 0x80)  nz |= 0x00010000; // neg
	if (!(edx & 0x02)) nz |= 0x00000001; // no zero
	flagnz = nz;
	flagc  = edx & 0x01 ? 0x000000FF : 0; // carry
	flago  = edx & 0x40 ? 0x000000FF : 0; // v
}


void splitflags(u4 const edx)
{
	asm volatile("call %P0" :: "X" (Ssplitflags), "d" (SA1RegP) : "cc", "memory");
	restoredl(edx);
}
