#include "c_65816d.h"
#include "../chips/sa1regs.h"
#include "../init.h"
#include "s65816d.h"

#include "flags65816.h"

void splitflags(u4 const edx)
{
    __asm__ volatile("call %P0" ::"X"(Ssplitflags), "d"(SA1RegP)
                 : "cc", "memory");
    restoredl(edx);
}

u4 joinflags(u4 edx)
{
    edx = makedl(edx);
    __asm__ volatile("call %P1"
                 : "+d"(SA1RegP)
                 : "X"(Sjoinflags)
                 : "cc", "memory");
    return edx;
}
