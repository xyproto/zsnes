/* 65816 opcode handlers ported from cpu/e65816.inc.
 *
 * The handlers are in cpu/ops65816.h so the difftest can include them next to
 * the original assembly; this file is what the emulator links. */
#include <stdint.h>

#include "../endmem.h"
#include "../types.h"

extern u4 xa, xx, xy, xs, xd, xdb, xpb;
extern u2 xpc;
extern u1 xe;
extern u4 flagnz, flago, flagc;
extern u2 stackand, stackor;
extern void UpdateDPage(void); /* cpu/memory.asm */

/* Memory accesses go through the same seam cpu/memory.asm's memcop thunk uses,
 * so the handlers here call the C halves directly (cpu/c_memops.c). */
extern u4 MemSeamA, MemSeamB, MemSeamC, MemSeamD;
extern void c_membank0r8(void);
extern void c_membank0r16(void);
extern void c_membank0w8(void);

#include "ops65816.h"
