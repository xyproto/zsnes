/* The .bss block that was left in chips/sa1proc.asm.
 *
 * zstate.c saves three bytes starting at SA1Status, so those three have to
 * stay adjacent and in this order. prevedi follows unaligned, as the assembly
 * had it - its ALIGN32 was commented out. Kept in its own file so the layout
 * test can link it on its own.
 */
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
    ASM_GSYM(prevedi)
    ".skip 4\n"
    ASM_GSYM(SA1xpc)
    ".skip 4\n"
    ASM_SEC_END);

/* clang-format on */
