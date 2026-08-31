/* The SA-1's 65816 core, from cpu/se65816.inc - which is cpu/e65816.inc over a
   different register file, differing in 42 of 2012 lines, all EXTSYM lists and
   a debugger hook. So cpu/ops65816.h is included again here with the renamed
   globals macroed onto their SA-1 counterparts.

   What is *not* renamed matters as much: xpc, xe, the memory tables, the
   direct-page and stack masks and the memory routines are shared, and
   membank0r8 dispatches on SA1Enable internally. */
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

#define OP(n) c_SA1##n

#define xa SA1xa
#define xx SA1xx
#define xy SA1xy
#define xs SA1xs
#define xd SA1xd
#define xdb SA1xdb
#define xpb SA1xpb
#define flagnz Sflagnz
#define flago Sflago
#define flagc Sflagc
#define tablead SA1tablead
#define DPageR8 SA1DPageR8
#define DPageR16 SA1DPageR16
#define DPageW8 SA1DPageW8
#define DPageW16 SA1DPageW16
#define UpdateDPage SA1UpdateDPage

extern u4 xa, xx, xy, xs, xd, xdb, xpb;
extern u2 xpc;
extern u1 xe;
extern u4 flagnz, flago, flagc;
extern u2 stackand, stackor;
extern void UpdateDPage(void);
extern u2 brkv, brkv8, copv, copv8;
extern u1 intrset, doirqnext, curnmi;

extern uintptr_t MemSeamA, MemSeamB, MemSeamC, MemSeamD;
extern void c_membank0r8(void);
extern void c_membank0r16(void);
extern void c_membank0w8(void);
extern void c_membank0w16(void);

#include "ops65816_sa1.h"
