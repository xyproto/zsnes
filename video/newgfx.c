/* C port of video/newgfx.asm: new-graphics-engine scratch state.

   Everything left in newgfx.asm was pure data; the code moved to
   video/c_makev16b.c.  The asm renderers reach several of these by offset from
   a neighbour, so the layout is reproduced verbatim with an inline-asm block
   (see asmdata.h).  resb N is N bytes, resd N is N*4; the asm ALIGN32 macro
   pads with nop (0x90) bytes, not zeroes. */
#include "../asmdata.h"

/* clang-format off */

#define BSSB(sym, n) ASM_GSYM(sym) ".skip (" #n ")\n"
#define BSSD(sym, n) ASM_GSYM(sym) ".skip (" #n ")*4\n"

__asm__(
    ASM_SEC_BSS(".bss")
    ".balign 4\n"
    BSSD(bgcmsung, 1)
    BSSD(modeused, 2)
    BSSD(reslbyl, 1)
    BSSD(csprival, 1)
    BSSD(cfieldad, 1)
    BSSD(ofsmcptr, 1)
    BSSD(ofsmtptr, 1)
    BSSD(ofsmmptr, 1)
    BSSD(ofsmcyps, 1)
    BSSD(ofsmady, 1)
    BSSD(ofsmadx, 1)
    ASM_SEC_END
    ASM_SEC_DATA(".data")
    ".balign 32, 0x90\n"
    ASM_GSYM(ngwintable)
    ".fill 32,4,0xEE00\n"
    ASM_GSYM(ngwintablec)
    ".fill 32,4,0xEE00\n"
    ASM_GSYM(ngcwinptr)
    ".long ngwintable\n"
    ASM_SEC_END
    ASM_SEC_BSS(".bss")
    BSSD(ngwinen, 1)
    BSSD(ngcwinmode, 1)
    BSSD(ngcpixleft, 1)
    BSSD(Mode7BackA, 1)
    BSSD(Mode7BackC, 1)
    BSSD(Mode7BackX0, 1)
    BSSD(Mode7BackSet, 1)
    BSSD(ngextbg, 1)
    BSSD(ofsmval, 1)
    BSSD(ofsmvalh, 1)
    ASM_SEC_END
    ASM_SEC_DATA(".data")
    ASM_GSYM(pwinen)
    ".long 0xFFFF\n"
    ASM_GSYM(pngwinen)
    ".long 0xFFFF\n"
    ASM_SEC_END
    ASM_SEC_BSS(".bss")
    BSSD(pwinbound, 1)
    BSSD(WinPtrAPos, 1)
    BSSD(WinPtrBPos, 1)
    ASM_SEC_END
    ASM_SEC_DATA(".data")
    ASM_GSYM(OrLogicTable)
    ".byte 0,1,1,0\n"
    ASM_GSYM(AndLogicTable)
    ".byte 0,0,1,0\n"
    ASM_GSYM(XorLogicTable)
    ".byte 0,1,0,0\n"
    ASM_GSYM(XNorLogicTable)
    ".byte 1,0,1,0\n"
    ASM_SEC_END
    ASM_SEC_BSS(".bss")
    BSSD(nglogicval, 1)
    BSSD(mosjmptab, 15)
    BSSB(Mode7HiRes, 1)
    BSSD(pesimpng, 1)
    BSSD(bgtxadd2, 1)
    ASM_SEC_END
    ASM_SEC_BSS(".bss")
    BSSD(bgtxadd, 1)
    BSSD(tleftn, 1)
    BSSD(tleftnb, 1)
    BSSD(bg1totng, 1)
    BSSD(bg2totng, 1)
    BSSD(bg3totng, 1)
    BSSD(bg4totng, 1)
    BSSD(bg1drwng, 1)
    BSSD(bg2drwng, 1)
    BSSD(bg3drwng, 1)
    BSSD(bg4drwng, 1)
    BSSD(scfbl, 1)
    BSSD(mode0ads, 1)
    BSSD(mode0add, 1)
    BSSD(taddnfy16x16, 1)
    BSSD(taddfy16x16, 1)
    BSSD(switch16x16, 1)
    BSSD(yposng, 1)
    BSSD(flipyposng, 1)
    BSSD(yposngom, 1)
    BSSD(flipyposngom, 1)
    ASM_SEC_END
    ASM_SEC_BSS(".bss")
    BSSB(NGNumSpr, 1)
    ASM_SEC_END
    ASM_SEC_BSS(".bss")
    BSSD(sprclprio, 1)
    BSSD(sprsingle, 1)
    ASM_SEC_END);

/* clang-format on */
