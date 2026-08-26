/* C port of the SPC700 state block from cpu/spc700.asm. The opcode handlers
   are already in cpu/c_spc700.c; this is the state they share.

   PHspcsave is the byte distance from SPCRAM to the end of FutureExpandS,
   which zstate.c uses as a save-state block length, so the order and size of
   everything in between is load-bearing - including the 64-byte SPC boot ROM
   that sits at the top of SPCRAM and the copy of it kept in SPCROM, which the
   emulator swaps in and out of that window. Emitted through one inline-asm
   block (see asmdata.h) to pin that layout. */
#include "../asmdata.h"

/* The IPL boot ROM, mapped at $FFC0. Appears twice: once as the tail of SPCRAM
   (where the SPC700 sees it) and once as the pristine copy the register
   handlers restore from. */
#define SPC_BOOT_ROM                                                          \
    ".byte 0xCD,0xEF,0xBD,0xE8,0x00,0xC6,0x1D,0xD0\n"                         \
    ".byte 0xFC,0x8F,0xAA,0xF4,0x8F,0xBB,0xF5,0x78\n"                         \
    ".byte 0xCC,0xF4,0xD0,0xFB,0x2F,0x19,0xEB,0xF4\n"                         \
    ".byte 0xD0,0xFC,0x7E,0xF4,0xD0,0x0B,0xE4,0xF5\n"                         \
    ".byte 0xCB,0xF4,0xD7,0x00,0xFC,0xD0,0xF3,0xAB\n"                         \
    ".byte 0x01,0x10,0xEF,0x7E,0xF4,0x10,0xEB,0xBA\n"                         \
    ".byte 0xF6,0xDA,0x00,0xBA,0xF4,0xC4,0xF4,0xDD\n"                         \
    ".byte 0x5D,0xD0,0xDB,0x1F,0x00,0x00,0xC0,0xFF\n"

/* Holds a host pointer, so the slot follows the pointer width. */
#define PTRSLOT ".balign " ASM_STR(__SIZEOF_POINTER__) "\n"      \
                ".skip " ASM_STR(__SIZEOF_POINTER__) "\n"

/* clang-format off */

__asm__(
    ASM_SEC_DATA(".data")
    ".balign 32\n"
    ASM_GSYM(SPCRAM)
    ".fill 65472, 1, 0xFF\n"
    SPC_BOOT_ROM
    /* 16 bytes past the boot ROM: the assembly's first copy runs 80 bytes, the
       last row being a scratch pattern rather than part of the ROM. */
    ".byte 0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0x00,0x11\n"
    ".byte 0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99\n"
    ASM_GSYM(spcPCRamSt)
    ".long 0\n"
    ASM_GSYM(spcA)
    ".long 0\n"
    ASM_GSYM(spcX)
    ".long 0\n"
    ASM_GSYM(spcY)
    ".long 0\n"
    ASM_GSYM(spcP)
    ".long 0\n"
    ASM_GSYM(spcNZ)
    ".long 0\n"
    ASM_GSYM(spcS)
    ".long 0x1FF\n"
    ASM_GSYM(spcRamDPSt)        /* direct page pointer, as a dword */
    ".long 0\n"
    ASM_GSYM(spcCycle)          /* cycle counter */
    ".long 0\n"
    ASM_GSYM(reg1read)          /* $F4-$F7, read from the 65816 side */
    ".byte 0\n"
    ASM_GSYM(reg2read)
    ".byte 0\n"
    ASM_GSYM(reg3read)
    ".byte 0\n"
    ASM_GSYM(reg4read)
    ".byte 0\n"
    ASM_GSYM(timeron)
    ".byte 0\n"
    ASM_GSYM(timincr0)          /* ticks between increments */
    ".byte 0\n"
    ASM_GSYM(timincr1)
    ".byte 0\n"
    ASM_GSYM(timincr2)
    ".byte 0\n"
    ASM_GSYM(timinl0)           /* ticks left before the next increment */
    ".byte 0\n"
    ASM_GSYM(timinl1)
    ".byte 0\n"
    ASM_GSYM(timinl2)
    ".byte 0\n"
    ASM_GSYM(timrcall)          /* alternates to clock timers 1 and 2 at 8kHz */
    ".byte 0\n"
    ASM_GSYM(spcextraram)       /* scratch the TCALL opcodes use */
    ".fill 64, 1, 0\n"
    ASM_GSYM(FutureExpandS)
    ".fill 192, 1, 0\n"
    ASM_GSYM(PHspcsave)
    ".long . - " ASM_SYMREF(SPCRAM) "\n"
    ASM_GSYM(SPCROM)
    SPC_BOOT_ROM
    ASM_GSYM(timer2upd)
    ".long 0\n"
    ASM_GSYM(spcnumread)
    ".byte 0\n"
    ASM_SEC_END);

/* clang-format on */

__asm__(
    ASM_SEC_BSS(".bss")
    ASM_GSYM(spcPCRam)
    PTRSLOT
    ASM_GSYM(spcRamDP)
    PTRSLOT
    ASM_SEC_END);
