/* C port of video/makevid.asm: renderer scratch state.

   Everything left in makevid.asm was pure data; drawline, the window builders
   and the mosaic helpers moved to video/c_makevid.c.  The asm renderers reach
   several of these by offset from a neighbour, so the layout is reproduced
   verbatim with an inline-asm block (see asmdata.h).  resb N is N bytes,
   resw N is N*2, resd N is N*4. */
#include "../asmdata.h"

/* clang-format off */

#define BSSB(sym, n) ASM_GSYM(sym) ".skip (" #n ")\n"
#define BSSW(sym, n) ASM_GSYM(sym) ".skip (" #n ")*2\n"
#define BSSD(sym, n) ASM_GSYM(sym) ".skip (" #n ")*4\n"

__asm__(
    ASM_SEC_BSS(".bss")
    ".balign 4\n"
    BSSB(bgcoloradder, 1)
    BSSB(res512switch, 1)
    ASM_SEC_END
    ASM_SEC_DATA(".data")
    ".balign 4\n"
    ASM_GSYM(MosaicYAdder)
    ".short 0,0,0,1,0,2,1,0,0,4,2,2,3,1,0,7\n"
    ASM_GSYM(cwinptr)
    ".long " ASM_SYMREF(winbgdata) "\n"
    ASM_SEC_END
    ASM_SEC_BSS(".bss")
    BSSB(pwinbgenab, 1)
    BSSD(pwinbgtype, 1)
    BSSB(winonbtype, 1)
    BSSB(dualwinbg, 1)
    BSSB(pwinspenab, 1)
    BSSD(pwinsptype, 1)
    BSSB(winonstype, 1)
    BSSB(dualwinsp, 1)
    BSSD(dwinptrproc, 1)
    ASM_SEC_END
    ASM_SEC_BSS(".bss")
    BSSB(winonsp, 1)
    ASM_SEC_END
    ASM_SEC_BSS(".bss")
    BSSB(windowdata, 16)
    BSSB(numwin, 1)
    BSSB(multiwin, 1)
    BSSB(multiclip, 1)
    BSSB(multitype, 1)
    ASM_SEC_END
    ASM_SEC_BSS(".bss")
    BSSB(curbgnum, 1)
    ASM_SEC_END
    ASM_SEC_BSS(".bss")
    BSSB(nextprimode, 1)
    BSSD(cursprloc, 1)
    BSSW(curtileptr, 1)
    BSSD(bg1vbufloc, 1)
    BSSD(bg2vbufloc, 1)
    BSSD(bg3vbufloc, 1)
    BSSD(bg4vbufloc, 1)
    BSSD(bg1tdatloc, 1)
    BSSD(bg2tdatloc, 1)
    BSSD(bg3tdatloc, 1)
    BSSD(bg4tdatloc, 1)
    BSSD(bg1tdabloc, 1)
    BSSD(bg2tdabloc, 1)
    BSSD(bg3tdabloc, 1)
    BSSD(bg4tdabloc, 1)
    BSSD(bg1cachloc, 1)
    BSSD(bg2cachloc, 1)
    BSSD(bg3cachloc, 1)
    BSSD(bg4cachloc, 1)
    BSSD(bg1yaddval, 1)
    BSSD(bg2yaddval, 1)
    BSSD(bg3yaddval, 1)
    BSSD(bg4yaddval, 1)
    BSSD(bg1xposloc, 1)
    BSSD(bg2xposloc, 1)
    BSSD(bg3xposloc, 1)
    BSSD(bg4xposloc, 1)
    BSSB(alreadydrawn, 1)
    BSSB(bg3draw, 1)
    BSSB(maxbr, 1)
    ASM_SEC_END
    ASM_SEC_BSS(".bss")
    BSSD(bg3high2, 1)
    BSSD(cwinenabm, 1)
    ASM_SEC_END
    ASM_SEC_BSS(".bss")
    BSSD(tempbuffer, 33)
    BSSD(currentobjptr, 1)
    BSSD(curmosaicsz, 1)
    BSSB(extbgdone, 1)
    ASM_SEC_END
    ASM_SEC_DATA(".data")
    ASM_GSYM(csprbit)
    ".byte 1\n"
    ASM_GSYM(csprprlft)
    ".byte 0\n"
    ASM_SEC_END
    ASM_SEC_BSS(".bss")
    BSSB(drawn, 1)
    BSSB(curbgpr, 1)
    ASM_SEC_END
    ASM_SEC_BSS(".bss")
    BSSD(winptrref, 1)
    ASM_SEC_END
    ASM_SEC_BSS(".bss")
    BSSB(hirestiledat, 256)
    ASM_SEC_END
    ASM_SEC_BSS(".bss")
    BSSD(yadder, 1)
    BSSD(yrevadder, 1)
    BSSD(tempcach, 1)
    BSSD(temptile, 1)
    BSSD(bgptr, 1)
    BSSD(bgptrc, 1)
    BSSD(bgptrd, 1)
    BSSD(bgptrx1, 1)
    BSSD(bgptrx2, 1)
    BSSD(curvidoffset, 1)
    BSSD(winon, 1)
    BSSD(bgofwptr, 1)
    BSSD(bgsubby, 1)
    ASM_SEC_END
    ASM_SEC_BSS(".bss")
    BSSB(temp, 1)
    BSSB(bshifter, 1)
    BSSB(a16x16xinc, 1)
    BSSB(a16x16yinc, 1)
    ASM_SEC_END);

/* clang-format on */
