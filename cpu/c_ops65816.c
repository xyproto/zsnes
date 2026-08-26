/* 65816 opcode handlers ported from cpu/e65816.inc.
 *
 * The handlers are in cpu/ops65816.h so the difftest can include them next to
 * the original assembly; this file is what the emulator links. */
#include <stdint.h>

#include "../endmem.h"
#include "../types.h"
#include "c_memory.h"
#include "memtable.h"
#include "memory.h"
#include "execute.h"
#include "../gblvars.h"
#include "../initc.h"
#include "../ui.h"
#include "c_irq.h"

extern u4 xa, xx, xy, xs, xd, xdb, xpb;
extern u2 xpc;
extern u1 xe;
extern u4 flagnz, flago, flagc;
extern u2 stackand, stackor;
extern void UpdateDPage(void); /* cpu/c_memory.c */
extern u1 dmadata[129];
extern u2 brkv, brkv8, copv, copv8;
extern u1 intrset, doirqnext, curnmi;

/* Memory accesses go through the seam in cpu/memseam.h, so the handlers here
 * call the bodies directly (cpu/c_memops.c). */
extern uintptr_t MemSeamA, MemSeamB, MemSeamC, MemSeamD;
extern void c_membank0r8(void);
extern void c_membank0r16(void);
extern void c_membank0w8(void);
extern void c_membank0w16(void);

#include "ops65816.h"
