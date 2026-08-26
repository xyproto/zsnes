/* C port of the SuperFX register and state block from chips/fxemu2.asm.
   The opcode handlers moved to chips/c_fxops.c; this is the state they share.

   PHnum2writesfxreg is the byte distance from SfxR0 to the end of the cache
   RAM, which zstate.c uses as a save-state block length, so the order and size
   of everything in between is load-bearing. Emitted through one inline-asm
   block (see asmdata.h) to pin that layout. */
#include "../asmdata.h"

/* Slots holding a host address: pointer-sized rather than the dword the
   assembly reserved. All of them sit after PHnum2writesfxreg, so the
   save-state block above keeps its layout. On i386 this is the same four
   bytes. */
#define PTRSLOT ".balign " ASM_STR(__SIZEOF_POINTER__) "\n" \
                                                       ".skip " ASM_STR(__SIZEOF_POINTER__) "\n"

/* clang-format off */

__asm__(
    ASM_SEC_BSS(".bss")
    ASM_GSYM(tempsfx)
    ".skip 3\n"
    ASM_SEC_END
    ASM_SEC_DATA(".data")
    ASM_GSYM(SfxR0)
    ".long 0\n"
    ASM_GSYM(SfxR1)
    ".long 0\n"
    ASM_GSYM(SfxR2)
    ".long 0\n"
    ASM_GSYM(SfxR3)
    ".long 0\n"
    ASM_GSYM(SfxR4)
    ".long 0\n"
    ASM_GSYM(SfxR5)
    ".long 0\n"
    ASM_GSYM(SfxR6)
    ".long 0\n"
    ASM_GSYM(SfxR7)
    ".long 0\n"
    ASM_GSYM(SfxR8)
    ".long 0\n"
    ASM_GSYM(SfxR9)
    ".long 0\n"
    ASM_GSYM(SfxR10)
    ".long 0\n"
    ASM_GSYM(SfxR11)
    ".long 0\n"
    ASM_GSYM(SfxR12)
    ".long 0\n"
    ASM_GSYM(SfxR13)
    ".long 0\n"
    ASM_GSYM(SfxR14)
    ".long 0\n"
    ASM_GSYM(SfxR15)
    ".long 0\n"
    ASM_GSYM(SfxSFR)
    ".long 0\n"
    ASM_GSYM(SfxBRAMR)
    ".long 0\n"
    ASM_GSYM(SfxPBR)
    ".long 0\n"
    ASM_GSYM(SfxROMBR)
    ".long 0\n"
    ASM_GSYM(SfxCFGR)
    ".long 0\n"
    ASM_GSYM(SfxSCBR)
    ".long 0\n"
    ASM_GSYM(SfxCLSR)
    ".long 0\n"
    ASM_GSYM(SfxSCMR)
    ".long 0\n"
    ASM_GSYM(SfxVCR)
    ".long 0\n"
    ASM_GSYM(SfxRAMBR)
    ".long 0\n"
    ASM_GSYM(SfxCBR)
    ".long 0\n"
    ASM_GSYM(SfxCOLR)
    ".long 0\n"
    ASM_GSYM(SfxPOR)
    ".long 0\n"
    ASM_GSYM(SfxCacheFlags)
    ".long 0\n"
    ASM_GSYM(SfxLastRamAdrSt)
    ".long 0\n"
    ASM_GSYM(SfxDREG)
    ".long 0\n"
    ASM_GSYM(SfxSREG)
    ".long 0\n"
    ASM_GSYM(SfxRomBufferSt)
    ".long 0\n"
    ASM_GSYM(SfxPIPE)
    ".long 0\n"
    ASM_GSYM(SfxPipeAdr)
    ".long 0\n"
    ASM_GSYM(SfxnRamBanks)
    ".long 4\n"
    ASM_GSYM(SfxnRomBanks)
    ".long 0\n"
    ASM_GSYM(SfxvScreenHeight)
    ".long 0\n"
    ASM_GSYM(SfxvScreenSize)
    ".long 0\n"
    ASM_GSYM(SfxCacheActive)
    ".long 0\n"
    ASM_GSYM(SfxCarry)
    ".long 0\n"
    ASM_GSYM(SfxSignZero)
    ".long 0\n"
    ASM_GSYM(SfxB)
    ".long 0\n"
    ASM_GSYM(SfxOverflow)
    ".long 0\n"
    ASM_GSYM(SfxCACHERAM)
    ".fill 512,1,0\n"
    ASM_GSYM(PHnum2writesfxreg)
    ".long . - " ASM_SYMREF(SfxR0) "\n"
    ASM_GSYM(SfxCPB)
    PTRSLOT
    ASM_GSYM(SfxCROM)
    PTRSLOT
    ASM_GSYM(SfxRAMMem)
    PTRSLOT
    ASM_GSYM(withr15sk)
    ".long 0\n"
    ASM_GSYM(sfxclineloc)
    PTRSLOT
    ASM_GSYM(SCBRrel)
    PTRSLOT
    ASM_GSYM(SfxLastRamAdr)
    PTRSLOT
    ASM_GSYM(SfxRomBuffer)
    PTRSLOT
    ASM_GSYM(fxbit01pcal)
    ".long 0\n"
    ASM_GSYM(fxbit23pcal)
    ".long 0\n"
    ASM_GSYM(fxbit45pcal)
    ".long 0\n"
    ASM_GSYM(fxbit67pcal)
    ".long 0\n"
    ASM_SEC_END
    ASM_SEC_BSS(".bss")
    ASM_GSYM(NumberOfOpcodes)
    ".skip 4\n"
    ASM_GSYM(NumberOfOpcodesBU)
    ".skip 4\n"
    ASM_GSYM(sfxwarningb)
    ".skip 1\n"
    ASM_SEC_END
    ASM_SEC_DATA(".data")
    ASM_GSYM(fxtrace)
    ".byte 0\n"
    ASM_SEC_END);

/* clang-format on */
