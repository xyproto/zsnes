/* The SuperFX (GSU) core, ported from chips/fxemu2.asm, chips/fxemu2b.asm and
   chips/fxemu2c.asm. The bodies are in chips/fx_ops.h, which the difftest
   includes too; this file only supplies the state they share. */
#include <stdint.h>

#include "../endmem.h"
#include "../types.h"

extern u4 SfxCarry, SfxSignZero, SfxOverflow;
extern u4 SfxB, SfxCPB, SfxCROM, SfxRomBuffer, withr15sk;
extern u4 SfxRAMMem, SfxLastRamAdr;
extern u4 SfxCBR, SfxPBR, SfxCacheActive;
extern u4 SfxRAMBR, SfxROMBR, SfxnRamBanks;
extern u1* sfxramdata; /* ui.h */
extern u4 SfxCOLR, SfxPOR, SfxSCMR;
extern u4 fxbit01pcal, fxbit23pcal, fxbit45pcal, fxbit67pcal;
extern u4 sfxclineloc, sfx128lineloc, sfx160lineloc, sfx192lineloc, sfxobjlineloc;
extern u4 SfxSFR, SfxCFGR, SfxPIPE;
extern u4 NumberOfOpcodes, ChangeOps, SFXProc; /* cpu/execute.asm */
extern u4 SfxSCBR, SCBRrel;
extern u4 SfxSREG, SfxDREG;
extern u4 flagnz; /* initdata.c */
extern u4 SfxR0[16]; /* SfxR0..SfxR15 are contiguous (chips/c_fxdata.c) */

/* What the assembly kept in ebp/esi/edi/ecx. MainLoop loads them from the GSU
   register block on entry and writes them back when the loop ends. */
u1* FxSeamPC;
u4* FxSeamSrc;
u4* FxSeamDst;
u4 FxSeamCX;

/* Set by STOP to leave the loop without spending an opcode. */
u4 FxLoopDone;

#include "fx_ops.h"
