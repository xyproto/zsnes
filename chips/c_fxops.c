/* C bodies for the SuperFX opcode handlers migrated out of the asm core
   (chips/fxemu2b.asm, chips/fxemu2.asm). The seam between the core's register
   ABI and these cdecl functions is documented in chips/fx_ops.h. */
#include <stdint.h>

#include "../endmem.h"
#include "../types.h"

extern u4 SfxCarry, SfxSignZero, SfxOverflow;
extern u4 SfxB, SfxCPB, SfxCROM, SfxRomBuffer, withr15sk;
extern u4 SfxR0[16]; /* SfxR0..SfxR15 are contiguous (chips/fxemu2.asm) */

/* The spill slots for the core's live registers. The asm writes them on the way
   in (fxcop) and reads them back on the way out; FxDispatch does the reverse
   around a nested opcode dispatch. */
u1* FxSeamPC;
u4* FxSeamSrc;
u4* FxSeamDst;
u4 FxSeamCX;

void FxDispatch(u4 const* table); /* chips/fxemu2b.asm */

#include "fx_ops.h"
