/* C bodies for the SuperFX opcode handlers migrated out of chips/fxemu2b.asm.
   The seam between the asm core's register ABI and these cdecl functions is
   documented in chips/fx_ops.h. */
#include "../endmem.h"
#include "../types.h"

extern u4 SfxCarry, SfxSignZero, SfxOverflow;

/* The spill slots for the core's live registers. chips/fxemu2b.asm writes them
   on the way in (fxcop) and reads them back on the way out; FxDispatch does
   the reverse around a nested opcode dispatch. */
u1* FxSeamPC;
u4* FxSeamSrc;
u4* FxSeamDst;
u4 FxSeamCX;

void FxDispatch(u4 const* table); /* chips/fxemu2b.asm */

#include "fx_ops.h"
