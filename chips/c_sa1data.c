/* The .bss block left in chips/sa1proc.asm. zstate.c saves three bytes from
   SA1Status, so those stay adjacent and in this order; prevedi holds a host
   pointer, so it is pointer-wide and outside the saved run. Its own file so
   the layout test can link it alone. */
#include "../asmdata.h"

/* clang-format off */

__asm__(
    ASM_SEC_BSS(".bss")
    /* 3 bytes from here are saved as one run; see zstate.c. */
    ASM_GSYM(SA1Status_run)
    ASM_GSYM(SA1Status)
    ".skip 1\n"
    ASM_GSYM(CurrentExecSA1)
    ".skip 1\n"
    ASM_GSYM(CurrentCPU)
    ".skip 1\n"
    /* The 65816 program counter across an SA-1 switch: a host pointer. */
    ".balign " ASM_STR(__SIZEOF_POINTER__) "\n"
    ASM_GSYM(prevedi)
    ".skip " ASM_STR(__SIZEOF_POINTER__) "\n"
    ASM_GSYM(SA1xpc)
    ".skip 4\n"
    ASM_SEC_END);

/* clang-format on */
