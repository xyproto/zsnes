/*
 * Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/* C port of init.asm's data: the 65816 register file and emulation globals.
   Each symbol keeps the exact width and initial value of its NASM declaration. */

#include "types.h"

u1 regsbackup[3019];
u1 forceromtype;
u1 autoloadstate;
u1 autoloadmovie;
u1 ZMVRawDump;

u1 romtype;
u2 resetv;
u2 abortv;
u2 nmiv2;
u2 nmiv;
u2 irqv;
u2 irqv2;
u2 brkv;
u2 copv;
u2 abortv8;
u2 nmiv8;
u2 irqv8;
u2 brkv8;
u2 copv8;
u1 cycpb268 = 109;
u1 cycpb358 = 149;
u1 cycpbl2 = 109;
u1 cycpblt2 = 149;
u1 writeon;
u2 totlines = 263;

u1 curcyc;
u1 cacheud = 1;
u1 ccud;
u1 spcon;

/* 65816 registers (native widths) */
u2 xat;
u1 xdbt;
u1 xpbt;
u2 xst;
u2 xdt;
u2 xxt;
u2 xyt;
u2 xpc;
u1 debugger;
u1 curnmi;

u4 cycpbl = 110;
u4 cycpblt = 110;

u4 xa;
u4 xdb;
u4 xpb;
u4 xs;
u4 xd;
u4 xx;
u4 xy;
u4 flagnz;
u4 flago;
u4 flagc;
u4 bankkp;
u4 Sflagnz;
u4 Sflago;
u4 Sflagc;

u1 disablespcclr;
u1 ENVDisable;

u1 IPSPatched;
u1 SramExists;
u4 NumofBanks;
u4 NumofBytes;

u1 DSP1Type;
u1 yesoutofmemory;
