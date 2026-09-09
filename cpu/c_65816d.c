#include "c_65816d.h"
#include "../chips/sa1regs.h"
#include "../init.h"
#include "s65816d.h"

#include "flags65816.h"

u4 Sjoinflags_c(u4 edx);
void Ssplitflags_c(u4 edx);

void splitflags(u4 const edx)
{
    Ssplitflags_c(SA1RegP);
    restoredl(edx);
}

u4 joinflags(u4 edx)
{
    edx = makedl(edx);
    SA1RegP = Sjoinflags_c(SA1RegP);
    return edx;
}
