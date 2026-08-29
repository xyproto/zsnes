/* The Mode 7 renderer's scratch block from video/mode716.asm. The layout is
   load-bearing: the renderer reads the current position, the relative position
   and the per-pixel adders 8 bytes at a time, so each is followed by an unnamed
   spacer the assembly labelled "keep this blank!". The mode7*pos/adder pairs at
   the end fold the spacer into a two-dword reservation. One inline-asm block
   pins it all. */
#include "../asmdata.h"

/* clang-format off */

__asm__(
    ASM_SEC_BSS(".bss")
    ".balign 4\n"
    ASM_GSYM(mtemp)
    ".skip 4\n"
    /* The same eight bytes under a second name: the map coordinate is
       read as a dword one byte in, which needs the spacer to be part of
       the object. */
    ASM_GSYM(mmode7xpos8)
    ASM_GSYM(mmode7xpos)        /* x position */
    ".skip 4\n"
    ASM_GSYM(mtempa2)           /* spacer */
    ".skip 4\n"
    ASM_GSYM(mmode7xrpos)       /* x position, relative */
    ".skip 4\n"
    ASM_GSYM(mtempa)            /* spacer */
    ".skip 4\n"
    /* The same eight bytes under a second name: the map coordinate is
       read as a dword one byte in, which needs the spacer to be part of
       the object. */
    ASM_GSYM(mmode7ypos8)
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
    /* The same eight bytes under a second name: the map coordinate is
       read as a dword one byte in, which needs the spacer to be part of
       the object. */
    ASM_GSYM(mode7xpos8)
    ASM_GSYM(mode7xpos)         /* dword plus its spacer, as above */
    ".skip 8\n"
    /* The same eight bytes under a second name: the map coordinate is
       read as a dword one byte in, which needs the spacer to be part of
       the object. */
    ASM_GSYM(mode7ypos8)
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

/* m7starty was the last of video/mode716.mac's own data: a lone word, in
   .data rather than .bss because the assembly declared it `dw 0`. */

__asm__(
    ASM_SEC_DATA(".data")
    ASM_GSYM(m7starty)
    ".short 0\n"
    ASM_SEC_END);

/* clang-format on */
