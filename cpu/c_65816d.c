#include "c_65816d.h"
#include "../chips/sa1regs.h"
#include "../init.h"
#include "s65816d.h"

static void restoredl(u4 const edx)
{
    u4 nz = 0;
    if (edx & 0x80)
        nz |= 0x00010000; // neg
    if (!(edx & 0x02))
        nz |= 0x00000001; // no zero
    flagnz = nz;
    flagc = edx & 0x01 ? 0x000000FF : 0; // carry
    flago = edx & 0x40 ? 0x000000FF : 0; // v
}

void splitflags(u4 const edx)
{
    asm volatile("call %P0" ::"X"(Ssplitflags), "d"(SA1RegP)
                 : "cc", "memory");
    restoredl(edx);
}

static u4 makedl(u4 edx)
{
    edx &= 0xFFFFFF3C;
    if ((flagnz & 0x00018000) != 0)
        edx |= 0x80; // neg
    if ((flagnz & 0x0000FFFF) == 0)
        edx |= 0x02; // zero
    if ((flagc & 0x000000FF) != 0)
        edx |= 0x01; // carry
    if ((flago & 0x000000FF) != 0)
        edx |= 0x40; // v
    return edx;
}

u4 joinflags(u4 edx)
{
    edx = makedl(edx);
    asm volatile("call %P1"
                 : "+d"(SA1RegP)
                 : "X"(Sjoinflags)
                 : "cc", "memory");
    return edx;
}
