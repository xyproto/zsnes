/* C port of the Mode 7 renderer's scratch block from video/mode716.asm.

   The layout is load-bearing, not incidental: the renderer keeps the current
   position, the relative position and the per-pixel adders as pairs read 8
   bytes at a time, so every one of them is followed by a spacer the assembly
   labelled "keep this blank!". Those spacers are never named anywhere; they
   exist only to space their neighbour. The mode7*pos/adder pairs at the end
   are the same thing with the spacer folded into a two-dword reservation.

   Emitted through one inline-asm block (see asmdata.h) to pin that layout.
   video/mode716.mac and the routines left in video/mode716.asm reach these by
   name, so they are all global here even though most were file-local before. */
#include "../asmdata.h"

/* clang-format off */

__asm__(
    ASM_SEC_BSS(".bss")
    ".balign 4\n"
    ASM_GSYM(mtemp)
    ".skip 4\n"
    ASM_GSYM(mmode7xpos)        /* x position */
    ".skip 4\n"
    ASM_GSYM(mtempa2)           /* spacer */
    ".skip 4\n"
    ASM_GSYM(mmode7xrpos)       /* x position, relative */
    ".skip 4\n"
    ASM_GSYM(mtempa)            /* spacer */
    ".skip 4\n"
    ASM_GSYM(mmode7ypos)        /* y position */
    ".skip 4\n"
    ASM_GSYM(mtempb2)           /* spacer */
    ".skip 4\n"
    ASM_GSYM(mmode7yrpos)       /* y position, relative */
    ".skip 4\n"
    ASM_GSYM(mtempb)            /* spacer */
    ".skip 4\n"
    ASM_GSYM(mmode7xadder)      /* number to add for x */
    ".skip 4\n"
    ASM_GSYM(mtempc2)           /* spacer */
    ".skip 4\n"
    ASM_GSYM(mmode7xadd2)
    ".skip 4\n"
    ASM_GSYM(mtempc)            /* spacer */
    ".skip 4\n"
    ASM_GSYM(mmode7yadder)      /* number to add for y */
    ".skip 4\n"
    ASM_GSYM(mtempd2)           /* spacer */
    ".skip 4\n"
    ASM_GSYM(mmode7yadd2)
    ".skip 4\n"
    ASM_GSYM(mtempd)            /* spacer */
    ".skip 4\n"
    ASM_GSYM(mmode7ptr)         /* pointer value */
    ".skip 4\n"
    ASM_GSYM(mmode7xinc)
    ".skip 4\n"
    ASM_GSYM(mmode7xincc)       /* range check for x */
    ".skip 4\n"
    ASM_GSYM(mmode7yinc)
    ".skip 4\n"
    ASM_GSYM(mmode7xsloc)       /* which screen x */
    ".skip 4\n"
    ASM_GSYM(mmode7ysloc)       /* which screen y */
    ".skip 4\n"
    ASM_GSYM(mmode7xsrl)        /* which relative screen x */
    ".skip 4\n"
    ASM_GSYM(mmode7ysrl)        /* which relative screen y */
    ".skip 4\n"
    ASM_GSYM(mcxloc)
    ".skip 2\n"
    ASM_GSYM(mcyloc)
    ".skip 2\n"
    ASM_GSYM(M7HROn)            /* high resolution on */
    ".skip 4\n"
    ASM_GSYM(switchtorep3)
    ".skip 4\n"
    ASM_GSYM(m7xaddof)
    ".skip 4\n"
    ASM_GSYM(m7xaddof2)
    ".skip 4\n"
    ASM_GSYM(m7yaddof)
    ".skip 4\n"
    ASM_GSYM(m7yaddof2)
    ".skip 4\n"
    ASM_GSYM(pixelsleft)
    ".skip 4\n"
    ASM_GSYM(mm7xaddof)
    ".skip 4\n"
    ASM_GSYM(mm7xaddof2)
    ".skip 4\n"
    ASM_GSYM(mm7yaddof)
    ".skip 4\n"
    ASM_GSYM(mm7yaddof2)
    ".skip 4\n"
    ASM_GSYM(ngwleft)           /* for byte move left */
    ".skip 4\n"
    ASM_GSYM(ngwleftb)
    ".skip 4\n"
    ASM_GSYM(mode7xpos)         /* dword plus its spacer, as above */
    ".skip 8\n"
    ASM_GSYM(mode7ypos)
    ".skip 8\n"
    ASM_GSYM(mode7xrpos)
    ".skip 8\n"
    ASM_GSYM(mode7yrpos)
    ".skip 8\n"
    ASM_GSYM(mode7xadder)
    ".skip 8\n"
    ASM_GSYM(mode7yadder)
    ".skip 8\n"
    ASM_SEC_END);

/* clang-format on */
