/* The .data and .bss blocks that were in cpu/regs.inc.
 *
 * This is the CPU/PPU register file. zstate.c saves PHnum2writeppureg bytes
 * starting at sndrot, and that length is assembled from this block's own
 * layout, so every distance here is part of the save-state format - do not
 * insert, reorder or re-align anything without changing the format.
 *
 * Generated from the assembly and byte-compared against it; the gaps are nop
 * (0x90) fill because NASM's ALIGN pads that way in a data section too.
 */
#include "../asmdata.h"

/* clang-format off */

__asm__(
    ASM_SEC_DATA(".data")
    ASM_GSYM(invreg)
    ".word 0\n"
    ASM_GSYM(sndrot)
    ".byte 0\n"
    ASM_GSYM(sndrot2)
    ".byte 0\n"
    ASM_GSYM(INTEnab)
    ".byte 0\n"
    ASM_GSYM(NMIEnab)
    ".byte 1\n"
    ASM_GSYM(VIRQLoc)
    ".word 0\n"
    ASM_GSYM(vidbright)
    ".byte 0\n"
    ASM_GSYM(previdbr)
    ".byte 0\n"
    ASM_GSYM(forceblnk)
    ".byte 0x80\n"
    ASM_GSYM(objptr)
    ".long 0\n"
    ASM_GSYM(objptrn)
    ".long 0\n"
    ASM_GSYM(objsize1)
    ".byte 1\n"
    ASM_GSYM(objsize2)
    ".byte 4\n"
    ASM_GSYM(objmovs1)
    ".byte 2\n"
    ASM_GSYM(objadds1)
    ".word 14\n"
    ASM_GSYM(objmovs2)
    ".byte 2\n"
    ASM_GSYM(objadds2)
    ".word 14\n"
    ASM_GSYM(oamaddrt)
    ".word 0\n"
    ASM_GSYM(oamaddrs)
    ".word 0\n"
    ASM_GSYM(objhipr)
    ".byte 0\n"
    ASM_GSYM(bgmode)
    ".byte 0\n"
    ASM_GSYM(bg3highst)
    ".byte 0\n"
    ASM_GSYM(bgtilesz)
    ".byte 0\n"
    ASM_GSYM(mosaicon)
    ".byte 0\n"
    ASM_GSYM(mosaicsz)
    ".byte 0\n"
    ASM_GSYM(bg1ptr)
    ".word 0\n"
    ASM_GSYM(bg2ptr)
    ".word 0\n"
    ASM_GSYM(bg3ptr)
    ".word 0\n"
    ASM_GSYM(bg4ptr)
    ".word 0\n"
    ASM_GSYM(bg1ptrb)
    ".word 0\n"
    ASM_GSYM(bg2ptrb)
    ".word 0\n"
    ASM_GSYM(bg3ptrb)
    ".word 0\n"
    ASM_GSYM(bg4ptrb)
    ".word 0\n"
    ASM_GSYM(bg1ptrc)
    ".word 0\n"
    ASM_GSYM(bg2ptrc)
    ".word 0\n"
    ASM_GSYM(bg3ptrc)
    ".word 0\n"
    ASM_GSYM(bg4ptrc)
    ".word 0\n"
    ASM_GSYM(bg1ptrd)
    ".word 0\n"
    ASM_GSYM(bg2ptrd)
    ".word 0\n"
    ASM_GSYM(bg3ptrd)
    ".word 0\n"
    ASM_GSYM(bg4ptrd)
    ".word 0\n"
    ASM_GSYM(bg1scsize)
    ".byte 0\n"
    ASM_GSYM(bg2scsize)
    ".byte 0\n"
    ASM_GSYM(bg3scsize)
    ".byte 0\n"
    ASM_GSYM(bg4scsize)
    ".byte 0\n"
    ASM_GSYM(bg1objptr)
    ".word 0\n"
    ASM_GSYM(bg2objptr)
    ".word 0\n"
    ASM_GSYM(bg3objptr)
    ".word 0\n"
    ASM_GSYM(bg4objptr)
    ".word 0\n"
    ASM_GSYM(bg1scrolx)
    ".word 0\n"
    ASM_GSYM(bg2scrolx)
    ".word 0\n"
    ASM_GSYM(bg3scrolx)
    ".word 0\n"
    ASM_GSYM(bg4scrolx)
    ".word 0\n"
    ASM_GSYM(bg1sx)
    ".word 0\n"
    ASM_GSYM(bg1scroly)
    ".word 0\n"
    ASM_GSYM(bg2scroly)
    ".word 0\n"
    ASM_GSYM(bg3scroly)
    ".word 0\n"
    ASM_GSYM(bg4scroly)
    ".word 0\n"
    ASM_GSYM(addrincr)
    ".word 2\n"
    ASM_GSYM(vramincr)
    ".byte 0\n"
    ASM_GSYM(vramread)
    ".byte 0\n"
    ASM_GSYM(vramaddr)
    ".long 0\n"
    ASM_GSYM(cgaddr)
    ".word 0\n"
    ASM_GSYM(cgmod)
    ".byte 0\n"
    ASM_GSYM(scrnon)
    ".word 0\n"
    ASM_GSYM(scrndist)
    ".byte 0\n"
    ASM_GSYM(resolutn)
    ".word 224\n"
    ASM_GSYM(multa)
    ".byte 0\n"
    ASM_GSYM(diva)
    ".word 0\n"
    ASM_GSYM(divres)
    ".word 0\n"
    ASM_GSYM(multres)
    ".word 0\n"
    ASM_GSYM(latchx)
    ".word 0\n"
    ASM_GSYM(latchy)
    ".word 0\n"
    ASM_GSYM(latchxr)
    ".byte 0\n"
    ASM_GSYM(latchyr)
    ".byte 0\n"
    ASM_GSYM(frskipper)
    ".byte 0\n"
    ASM_GSYM(winl1)
    ".byte 0\n"
    ASM_GSYM(winr1)
    ".byte 0\n"
    ASM_GSYM(winl2)
    ".byte 0\n"
    ASM_GSYM(winr2)
    ".byte 0\n"
    ASM_GSYM(winen)  /* alias: same address as the next symbol */
    ASM_GSYM(winbg1en)
    ".byte 0\n"
    ASM_GSYM(winbg2en)
    ".byte 0\n"
    ASM_GSYM(winbg3en)
    ".byte 0\n"
    ASM_GSYM(winbg4en)
    ".byte 0\n"
    ASM_GSYM(winobjen)
    ".byte 0\n"
    ASM_GSYM(wincolen)
    ".byte 0\n"
    ASM_GSYM(winlogica)
    ".byte 0\n"
    ASM_GSYM(winlogicb)
    ".byte 0\n"
    ASM_GSYM(winenabm)
    ".byte 0\n"
    ASM_GSYM(winenabs)
    ".byte 0\n"
    ASM_GSYM(mode7set)
    ".byte 0\n"
    ASM_GSYM(mode7A)
    ".word 0\n"
    ASM_GSYM(mode7B)
    ".word 0\n"
    ASM_GSYM(mode7C)
    ".word 0\n"
    ASM_GSYM(mode7D)
    ".word 0\n"
    ASM_GSYM(mode7X0)
    ".word 0\n"
    ASM_GSYM(mode7Y0)
    ".word 0\n"
    ASM_GSYM(JoyAPos)
    ".byte 0\n"
    ASM_GSYM(JoyBPos)
    ".byte 0\n"
    ASM_GSYM(compmult)
    ".long 0\n"
    ASM_GSYM(joyalt)
    ".byte 0\n"
    ASM_GSYM(wramrwadr)
    ".long 0\n"
    ASM_GSYM(dmadata)
    ".fill 129, 1, 0x0FF\n"
    ".balign 32, 0x90\n"
    ASM_GSYM(irqon)
    ".byte 0\n"
    ASM_GSYM(nexthdma)
    ".byte 0\n"
    ASM_GSYM(curhdma)
    ".byte 0\n"
    ASM_GSYM(hdmadata)
    /* 8 * sizeof(HDMAInfo), which holds four host pointers: 152 bytes on a
       32-bit build. The assembler does the arithmetic, so the reserve tracks
       the struct instead of the struct having to match a literal. */
    ".fill 8 * (4 * " ASM_STR(__SIZEOF_POINTER__) " + 3), 1, 0\n"
    ".balign 32, 0x90\n"
    ASM_GSYM(hdmatype)
    ".byte 0\n"
    ASM_GSYM(coladdr)
    ".byte 0\n"
    ASM_GSYM(coladdg)
    ".byte 0\n"
    ASM_GSYM(coladdb)
    ".byte 0\n"
    ASM_GSYM(colnull)
    ".byte 0\n"
    ASM_GSYM(scaddset)
    ".byte 0\n"
    ASM_GSYM(scaddtype)
    ".byte 0\n"
    ASM_GSYM(Voice0Disabl2)
    ".byte 1\n"
    ASM_GSYM(Voice1Disabl2)
    ".byte 1\n"
    ASM_GSYM(Voice2Disabl2)
    ".byte 1\n"
    ASM_GSYM(Voice3Disabl2)
    ".byte 1\n"
    ASM_GSYM(Voice4Disabl2)
    ".byte 1\n"
    ASM_GSYM(Voice5Disabl2)
    ".byte 1\n"
    ASM_GSYM(Voice6Disabl2)
    ".byte 1\n"
    ASM_GSYM(Voice7Disabl2)
    ".byte 1\n"
    ASM_GSYM(oamram)
    ".fill 1024, 1, 0\n"
    ASM_GSYM(cgram)
    ".fill 512, 1, 0\n"
    ASM_GSYM(pcgram)
    ".fill 512, 1, 0\n"
    ASM_GSYM(vraminctype)
    ".byte 0\n"
    ASM_GSYM(vramincby8on)
    ".byte 0\n"
    ASM_GSYM(vramincby8left)
    ".byte 0\n"
    ASM_GSYM(vramincby8totl)
    ".byte 0\n"
    ASM_GSYM(vramincby8rowl)
    ".byte 0\n"
    ASM_GSYM(vramincby8ptri)
    ".word 0\n"
    ASM_GSYM(nexthprior)
    ".byte 0\n"
    ASM_GSYM(doirqnext)
    ".byte 0\n"
    ASM_GSYM(vramincby8var)
    ".word 0\n"
    ASM_GSYM(screstype)
    ".byte 0\n"
    ASM_GSYM(extlatch)
    ".byte 0\n"
    ASM_GSYM(cfield)
    ".byte 0\n"
    ASM_GSYM(interlval)
    ".byte 0\n"
    ASM_GSYM(HIRQLoc)
    ".word 0\n"
    ASM_GSYM(KeyOnStA)
    ".byte 0\n"
    ASM_GSYM(KeyOnStB)
    ".byte 0\n"
    ASM_GSYM(SDD1BankA)
    ".byte 0\n"
    ASM_GSYM(SDD1BankB)
    ".byte 1\n"
    ASM_GSYM(SDD1BankC)
    ".byte 2\n"
    ASM_GSYM(SDD1BankD)
    ".byte 3\n"
    ASM_GSYM(vramread2)
    ".byte 0\n"
    ASM_GSYM(nosprincr)
    ".byte 0\n"
    ASM_GSYM(poamaddrs)
    ".word 0\n"
    ASM_GSYM(ioportval)
    ".byte 255\n"
    ASM_GSYM(iohvlatch)
    ".byte 0\n"
    ASM_GSYM(ppustatus)
    ".byte 0\n"
    ASM_GSYM(hdmastartsc)
    ".byte 0\n"
    ASM_GSYM(hdmarestart)
    ".byte 0\n"
    ASM_GSYM(hdmadelay)
    ".byte 0\n"
    ASM_GSYM(nohdmaframe)
    ".byte 0\n"
    ASM_GSYM(rtoflags)
    ".byte 0\n"
    ASM_GSYM(h_dot_counter)
    ".long 0\n"
    ASM_GSYM(tempdat)
    ".fill 473, 1, 0\n"
    /* zstate.c saves this many bytes starting at sndrot, so every distance
       above is part of the save-state format. */
    ASM_GSYM(PHnum2writeppureg)
    ".long . - " ASM_SYMREF(sndrot) "\n"
    ASM_GSYM(scrndis)
    ".byte 0\n"
    ".balign 32, 0x90\n"
    ASM_GSYM(oamaddr)
    ".long 0\n"
    ASM_GSYM(bg1ptrx)
    ".long 0\n"
    ASM_GSYM(bg2ptrx)
    ".long 0\n"
    ASM_GSYM(bg3ptrx)
    ".long 0\n"
    ASM_GSYM(bg4ptrx)
    ".long 0\n"
    ASM_GSYM(bg1ptry)
    ".long 0\n"
    ASM_GSYM(bg2ptry)
    ".long 0\n"
    ASM_GSYM(bg3ptry)
    ".long 0\n"
    ASM_GSYM(bg4ptry)
    ".long 0\n"
    ASM_GSYM(Voice0Disable)
    ".byte 1\n"
    ASM_GSYM(Voice1Disable)
    ".byte 1\n"
    ASM_GSYM(Voice2Disable)
    ".byte 1\n"
    ASM_GSYM(Voice3Disable)
    ".byte 1\n"
    ASM_GSYM(Voice4Disable)
    ".byte 1\n"
    ASM_GSYM(Voice5Disable)
    ".byte 1\n"
    ASM_GSYM(Voice6Disable)
    ".byte 1\n"
    ASM_GSYM(Voice7Disable)
    ".byte 1\n"
    ASM_GSYM(BG116x16t)
    ".byte 0\n"
    ASM_GSYM(BG216x16t)
    ".byte 0\n"
    ASM_GSYM(BG316x16t)
    ".byte 0\n"
    ASM_GSYM(BG416x16t)
    ".byte 0\n"
    ASM_GSYM(SPC700read)
    ".long 0\n"
    ASM_GSYM(SPC700write)
    ".long 0\n"
    ASM_GSYM(JoyCRead)
    ".byte 0\n"
    ASM_GSYM(nssdip1)
    ".byte 0\n"
    ASM_GSYM(nssdip2)
    ".byte 0\n"
    ASM_GSYM(nssdip3)
    ".byte 0\n"
    ASM_GSYM(nssdip4)
    ".byte 0\n"
    ASM_GSYM(nssdip5)
    ".byte 0\n"
    ASM_GSYM(nssdip6)
    ".byte 0\n"
    ASM_SEC_END

    ASM_SEC_DATA(".data")
    ASM_GSYM(JoyARead)
    ".long 0\n"
    ASM_GSYM(JoyBRead)
    ".long 0\n"
    ASM_GSYM(JoyCRead2)
    ".long 0\n"
    ASM_GSYM(JoyDRead)
    ".long 0\n"
    ASM_GSYM(JoyERead)
    ".long 0\n"
    ASM_SEC_END

    ASM_SEC_BSS(".bss")
    ASM_GSYM(MultiTap)
    ".skip 1\n"
    ASM_SEC_END

    ASM_SEC_BSS(".bss")
    ASM_GSYM(hblank)
    ".skip 1\n"
    ASM_SEC_END

    ASM_SEC_BSS(".bss")
    ASM_GSYM(cpu_mdr)
    ".skip 1\n"
    ASM_GSYM(ppu2_mdr)
    ".skip 1\n"
    ASM_SEC_END);

/* clang-format on */
