#include "c_memory.h"
#include "../chips/sa1regs.h"
#include "../endmem.h"
#include "../init.h"

mr8*  DPageR8;
mr16* DPageR16;
mw8*  DPageW8;
mw16* DPageW16;
mr8*  SA1DPageR8;
mr16* SA1DPageR16;
mw8*  SA1DPageW8;
mw16* SA1DPageW16;

void UpdateDPage(void)
{
    u1 const d = xd >> 8;
    DPageR8 = Bank0datr8[d];
    DPageR16 = Bank0datr16[d];
    DPageW8 = Bank0datw8[d];
    DPageW16 = Bank0datw16[d];
}

void SA1UpdateDPage(void)
{
    u1 const d = SA1xd >> 8;
    SA1DPageR8 = Bank0datr8[d];
    SA1DPageR16 = Bank0datr16[d];
    SA1DPageW8 = Bank0datw8[d];
    SA1DPageW16 = Bank0datw16[d];
}
