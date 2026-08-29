/* The .data and .bss blocks from cpu/regsw.inc. The six sprite tables were
   file-local labels inside reg2101w, indexed `.objsize1+ebx`; they are global
   here (reg2101w_*) and that indexing still relies on their order and
   adjacency. Data-only so the layout test can link it alone. */
#include "../asmdata.h"

/* clang-format off */

__asm__(
    ASM_SEC_DATA(".data")
    ASM_GSYM(reg2101w_objsize1)
    ".byte 1, 1, 1, 4, 4, 16, 8, 8\n"
    ASM_GSYM(reg2101w_objsize2)
    ".byte 4, 16, 64, 16, 64, 64, 32, 16\n"
    ASM_GSYM(reg2101w_objmovs1)
    ".byte 2, 2, 2, 2, 2, 4, 2, 2\n"
    ASM_GSYM(reg2101w_objmovs2)
    ".byte 2, 4, 8, 4, 8, 8, 4, 4\n"
    ASM_GSYM(reg2101w_objadds1)
    ".short 14, 14, 14, 14, 14, 12, 14, 14\n"
    ASM_GSYM(reg2101w_objadds2)
    ".short 14, 12, 8, 12, 8, 8, 12, 12\n"
    ASM_GSYM(bgscrolPrev)
    ".byte 0\n"
    ASM_GSYM(bg1scrolx_m7)
    ".short 0\n"
    ASM_GSYM(bg1scroly_m7)
    ".short 0\n"
    ASM_GSYM(multchange)
    ".byte 1\n"
    ASM_GSYM(m7byte)
    ".byte 0\n"
    ASM_SEC_END

    ASM_SEC_BSS(".bss")
    ASM_GSYM(prevoamptr)
    ".skip 1\n"
    ASM_GSYM(oamlow)
    ".skip 1\n"
    ASM_GSYM(MultiTapStat)
    ".skip 1\n"
    ASM_SEC_END);

/* clang-format on */
