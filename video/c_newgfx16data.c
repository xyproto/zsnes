/* The .data blocks spread through video/newgfx16.asm, pinned as one block in
   the assembly's order. The gaps are nop (0x90), not zero, because NASM's
   ALIGN pads that way in a data section too; and mosstart, moscountdown and
   clinemainsub were file-local labels, global here because the block moved out
   of the file that used them. Data-only so the layout test can link it
   alone. */
#include "../asmdata.h"

/* clang-format off */

__asm__(
    ASM_SEC_DATA(".data")
    ASM_GSYM(prevbrightdc)
    ".byte 16\n"
    ASM_GSYM(mosstart)          /* was file-local */
    ".long 0, 0, 0, 0\n"
    ASM_GSYM(moscountdown)      /* was file-local */
    ".byte 0\n"
    ASM_GSYM(BackAreaAdd)       /* deliberately unaligned, as the asm had it */
    ".long 0\n"
    ASM_GSYM(BackAreaUnFillCol)
    ".long 0\n"
    ASM_GSYM(BackAreaFillCol)
    ".long 0\n"
    ASM_GSYM(clinemainsub)      /* was file-local */
    ".long 0\n"

    ".balign 32, 0x90\n"
    ASM_GSYM(cpalptrng)
    ".long 0\n"
    ASM_GSYM(ngmsdraw)
    ".long 0\n"
    /* These hold the address of a window table, so they are pointer-sized,
       not a dword. */
    ASM_GSYM(CMainWinScr)
    ".zero " ASM_STR(__SIZEOF_POINTER__) "\n"
    ASM_GSYM(CSubWinScr)
    ".zero " ASM_STR(__SIZEOF_POINTER__) "\n"
    ASM_GSYM(Prevcoladdr)
    ".long 0\n"
    ASM_GSYM(ColResult)
    ".long 0\n"
    /* A palette address, so pointer-sized. */
    ASM_GSYM(CPalPtrng)
    ".zero " ASM_STR(__SIZEOF_POINTER__) "\n"
    ASM_GSYM(WindowRedraw)
    ".long 0\n"
    ASM_GSYM(mostranspval)
    ".long 0\n"
    ASM_GSYM(mosclineval)
    ".long 0\n"
    ASM_GSYM(startlinet)
    ".long 0\n"
    ASM_GSYM(endlinet)
    ".long 0\n"
    ASM_GSYM(palchanged)
    ".long 0\n"
    ASM_GSYM(ng16bbgval)        /* bg number */
    ".long 0\n"
    ASM_GSYM(ng16bprval)        /* 0 = pr0, 2000h = pr1 */
    ".long 0\n"
    ASM_GSYM(mosjmptab16b)
    ".fill 15, 4, 0\n"
    ASM_GSYM(mosjmptab16bt)
    ".fill 15, 4, 0\n"
    ASM_GSYM(mosjmptab16btms)
    ".fill 15, 4, 0\n"
    ASM_GSYM(mosjmptab16bntms)
    ".fill 15, 4, 0\n"

    ".balign 32, 0x90\n"
    ASM_GSYM(UnusedBit)
    ".long 0x00200020, 0x00200020\n"
    ASM_GSYM(HalfTrans)
    ".long 0xF7DEF7DE, 0xF7DEF7DE, 0, 0\n"
    ASM_GSYM(UnusedBitXor)
    ".long 0xFFDFFFDF, 0xFFDFFFDF\n"
    ASM_GSYM(ngrposng)
    ".long 11, 0\n"
    ASM_GSYM(nggposng)
    ".long 6, 0\n"
    ASM_GSYM(ngbposng)
    ".long 0, 0\n"
    ASM_GSYM(HiResDone)
    ".long 0, 0\n"
    ASM_GSYM(FullBitAnd)
    ".long 0xF800F800, 0xF800F800\n"
    ASM_GSYM(HalfTransB)
    ".long 0x08410841, 0x08410841\n"
    ASM_GSYM(HalfTransC)
    ".long 0xF79EF79E, 0xF79EF79E\n"
    ASM_GSYM(NGNoTransp)
    ".long 0\n"
    ASM_SEC_END);

/* clang-format on */
