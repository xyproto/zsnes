/* The .bss blocks that were spread through video/makev16t.asm.
 *
 * Kept as one pinned block in the order and at the offsets the assembly gave
 * them: the transparency buffer is indexed with signed displacements off its
 * middle, so what sits after it is part of the shape. coadder16 lands on an
 * odd address, exactly as before - the assembly asked for no alignment and
 * x86 does not need any. Data-only file so the layout test can link it alone.
 */
#include "../asmdata.h"

/* clang-format off */

__asm__(
    ASM_SEC_BSS(".bss")
    ASM_GSYM(transpbuf)         /* 576 + 16 + 288*2 */
    ".skip 1168\n"
    ASM_GSYM(prevrgbcol)
    ".skip 4\n"
    ASM_GSYM(prevrgbpal)
    ".skip 4\n"
    ASM_GSYM(DoTransp)
    ".skip 1\n"
    ASM_GSYM(coadder16)
    ".skip 4\n"
    ASM_GSYM(yadd)
    ".skip 2\n"
    ASM_GSYM(yflipadd)
    ".skip 2\n"
    ASM_SEC_END);

/* clang-format on */
