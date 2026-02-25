/*
 * init_data.c — C replacement for init.asm
 *
 * Pure data declarations: CPU registers, vectors, cycle counters, and
 * emulator state variables.  Every symbol matches the NEWSYM in init.asm
 * exactly (name, size, initial value).
 *
 * Built only when NO_ASM=1.
 */

#include "types.h"

/* ── .data (explicit initial values) ── */

u1 regsbackup[3019] = { 0 };
u1 forceromtype   = 0;
u1 autoloadstate  = 0;
u1 autoloadmovie  = 0;
u1 ZMVRawDump     = 0;

u1 romtype  = 0;
u2 resetv   = 0;
u2 abortv   = 0;
u2 nmiv2    = 0;
u2 nmiv     = 0;
u2 irqv     = 0;
u2 irqv2    = 0;
u2 brkv     = 0;
u2 copv     = 0;
u2 abortv8  = 0;
u2 nmiv8    = 0;
u2 irqv8    = 0;
u2 brkv8    = 0;
u2 copv8    = 0;
u1 cycpb268 = 109;
u1 cycpb358 = 149;
u1 cycpbl2  = 109;
u1 cycpblt2 = 149;
u1 writeon  = 0;
u2 totlines = 263;

/* Saved in states */
u1 curcyc   = 0;
u1 cacheud  = 1;
u1 ccud     = 0;
u1 spcon    = 0;

/* 65816 registers (16-bit temporaries) */
u2 xat  = 0;
u1 xdbt = 0;
u1 xpbt = 0;
u2 xst  = 0;
u2 xdt  = 0;
u2 xxt  = 0;
u2 xyt  = 0;
u2 xpc  = 0;
u1 debugger = 0;
u1 curnmi   = 0;

/* 32-bit cycle counters (32-byte aligned in ASM) */
__attribute__((aligned(32)))
u4 cycpbl  = 110;
u4 cycpblt = 110;

/* 65816 registers (32-bit working copies, 32-byte aligned) */
__attribute__((aligned(32)))
u4 xa     = 0;
u4 xdb    = 0;
u4 xpb    = 0;
u4 xs     = 0;
u4 xd     = 0;
u4 xx     = 0;
u4 xy     = 0;
u4 flagnz = 0;
u4 flago  = 0;
u4 flagc  = 0;
u4 bankkp = 0;
u4 Sflagnz = 0;
u4 Sflago  = 0;
u4 Sflagc  = 0;

u1 disablespcclr = 0;
u1 ENVDisable    = 0;

/* ── .bss (zero-initialized) ── */

u1 IPSPatched;
u1 SramExists;
u4 NumofBanks;
u4 NumofBytes;

u1 DSP1Type;
u1 yesoutofmemory;
