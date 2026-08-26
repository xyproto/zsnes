/* C port of the emulation-loop state block from cpu/execute.asm. The routines
   that use it (execloop, pexecs, the rewind helpers) are still there; this is
   only the data they and the rest of the emulator share.

   Two ALIGN32 gaps inside the block are reproduced with an explicit 0x90 fill:
   NASM's ALIGN pads with nop bytes even in a data section, so a plain .balign
   would zero-fill and change the image. Emitted through one inline-asm block
   (see asmdata.h) to pin the layout. */
#include "../asmdata.h"

/* Holds a host pointer, so the slot follows the pointer width. */
#define PTRSLOT ".balign " ASM_STR(__SIZEOF_POINTER__) "\n"      \
                ".skip " ASM_STR(__SIZEOF_POINTER__) "\n"

/* clang-format off */

__asm__(
    ASM_SEC_DATA(".data")
    ASM_GSYM(tempedx)
    ".long 0\n"
    ASM_GSYM(tempesi)
    ".long 0\n"
    ASM_GSYM(tempedi)
    ".long 0\n"
    ASM_GSYM(tempebp)
    ".long 0\n"
    ASM_GSYM(RewindTimer)
    ".long 0\n"
    ASM_GSYM(BackState)
    ".byte 1\n"
    ASM_GSYM(BackStateSize)
    ".long 6\n"
    ASM_GSYM(DblRewTimer)
    ".long 0\n"
    ASM_GSYM(romloadskip)
    ".byte 0\n"
    ASM_GSYM(SSKeyPressed)
    ".long 0\n"
    ASM_GSYM(SPCKeyPressed)
    ".long 0\n"
    ASM_GSYM(NoSoundReinit)
    ".long 0\n"
    ASM_GSYM(NextNGDisplay)
    ".byte 0\n"
    ASM_GSYM(TempVidInfo)
    ".long 0\n"
    ASM_GSYM(tempdh)
    ".byte 0\n"
    ASM_GSYM(invalid)
    ".byte 0\n"
    ASM_GSYM(invopcd)
    ".byte 0\n"
    ASM_GSYM(pressed)
    ".fill 256 + 128 + 64, 1, 0\n"
    ASM_GSYM(exiter)
    ".byte 0\n"
    ASM_GSYM(oldhand9o)
    ".long 0\n"
    ASM_GSYM(oldhand9s)
    ".short 0\n"
    ASM_GSYM(oldhand8o)
    ".long 0\n"
    ASM_GSYM(oldhand8s)
    ".short 0\n"
    /* 24 bytes from here are saved as one run; see zstate.c. */
    ASM_GSYM(opcd_run)
    ASM_GSYM(opcd)
    ".long 0\n"
    ASM_GSYM(pdh)
    ".long 0\n"
    ASM_GSYM(pcury)
    ".long 0\n"
    ASM_GSYM(timercount)
    ".long 0\n"
    ASM_GSYM(initaddrlSt)
    ".long 0\n"
    ASM_GSYM(NetSent)
    ".long 0\n"
    ASM_GSYM(nextframe)
    ".long 0\n"
    ASM_GSYM(HIRQCycNext)
    ".long 0\n"
    ASM_GSYM(HIRQNextExe)
    ".byte 0\n"
    ASM_GSYM(timeradj)
    ".long 65536\n"
    ASM_GSYM(t1cc)
    ".short 0\n"
    /* NASM ALIGN32: nop padding, not zeroes. */
    ".balign 32, 0x90\n"
    ASM_GSYM(soundcycleft)
    ".long 0\n"
    ASM_GSYM(curexecstate)
    ".long 0\n"
    ASM_GSYM(nmiprevaddrl)   /* observed address -5 */
    ".long 0\n"
    ASM_GSYM(nmiprevaddrh)   /* observed address +5 */
    ".long 0\n"
    ASM_GSYM(nmirept)   /* NMI repeat check, if 6 then okay */
    ".long 0\n"
    ASM_GSYM(nmiprevline)   /* previous line */
    ".long 224\n"
    ASM_GSYM(nmistatus)   /* 0 = none, 1 = waiting for nmi location, */
    ".long 0\n"
    ASM_GSYM(joycontren)   /* joystick read control check */
    ".long 0\n"
    ASM_GSYM(NextLineCache)
    ".byte 0\n"
    ASM_GSYM(ZMVZClose)
    ".byte 0\n"
    /* NASM ALIGN32: nop padding, not zeroes. */
    ".balign 32, 0x90\n"
    ASM_GSYM(ExecExitOkay)
    ".byte 1\n"
    ASM_GSYM(JoyABack)
    ".long 0\n"
    ASM_GSYM(JoyBBack)
    ".long 0\n"
    ASM_GSYM(JoyCBack)
    ".long 0\n"
    ASM_GSYM(JoyDBack)
    ".long 0\n"
    ASM_GSYM(JoyEBack)
    ".long 0\n"
    ASM_GSYM(NetCommand)
    ".long 0\n"
    /* 40 bytes from here are saved as one run; see zstate.c. */
    ASM_GSYM(spc700read_run)
    ASM_GSYM(spc700read)
    ".long 0\n"
    ASM_GSYM(lowestspc)
    ".long 0\n"
    ASM_GSYM(highestspc)
    ".long 0\n"
    ASM_GSYM(SA1UBound)
    ".long 0\n"
    ASM_GSYM(SA1LBound)
    ".long 0\n"
    ASM_GSYM(SA1SH)
    ".long 0\n"
    ASM_GSYM(SA1SHb)
    ".long 0\n"
    ASM_GSYM(NumberOfOpcodes2)
    ".long 370\n"
    ASM_GSYM(ChangeOps)
    ".long 0\n"
    ASM_GSYM(SFXProc)
    ".long 0\n"
    ASM_GSYM(EMUPause)
    ".byte 0\n"
    ASM_GSYM(INCRFrame)
    ".byte 0\n"
    ASM_GSYM(NoHDMALine)
    ".byte 0\n"
    ASM_SEC_END);

/* clang-format on */

__asm__(
    ASM_SEC_BSS(".bss")
    ASM_GSYM(initaddrl)
    PTRSLOT
    ASM_SEC_END);
