/* The debug 65816 core, ported from cpu/e65816c.inc.
 *
 * The debugger single-steps through its own dispatch table, so this shares the
 * 65816's register file outright and renames only tableadc. See
 * cpu/ops65816_dbg.h for the handful of handlers that genuinely differ. */
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

#define OP(n) c_dbg##n

/* Only the opcode table differs; the register file is the 65816's own. */
#define tablead tableadc


extern u4 xa, xx, xy, xs, xd, xdb, xpb;
extern u2 xpc;
extern u1 xe;
extern u4 flagnz, flago, flagc;
extern u2 stackand, stackor;
extern void UpdateDPage(void);
extern u1 dmadata[129];
extern u2 brkv, brkv8, copv, copv8;
extern u1 intrset, doirqnext, curnmi;

extern uintptr_t MemSeamA, MemSeamB, MemSeamC, MemSeamD;
extern void c_membank0r8(void);
extern void c_membank0r16(void);
extern void c_membank0w8(void);
extern void c_membank0w16(void);

#include "ops65816_dbg.h"
