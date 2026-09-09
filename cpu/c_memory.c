#include "c_memory.h"
#include "../chips/sa1regs.h"
#include "../endmem.h"
#include "../init.h"

eop* DPageR16;
eop* DPageR8;
eop* DPageW16;
eop* DPageW8;
eop* SA1DPageR16;
eop* SA1DPageR8;
eop* SA1DPageW16;
eop* SA1DPageW8;

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
