/*
Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes
https://zsnes.bountysource.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef MEMTABLE_H
#define MEMTABLE_H

#include <stddef.h>

#include "../init.h"
#include "../types.h"

extern mr8*  memtabler8[256];
extern mw8*  memtablew8[256];
extern mr16* memtabler16[256];
extern mw16* memtablew16[256];

typedef struct
{
    mr8*  memr8;
    mw8*  memw8;
    mr16* memr16;
    mw16* memw16;
} mrwp;

extern mrwp regbank, membank, wrambank, srambank, erambank, sramsbank;
extern mrwp sa1regbank, sa1rambank, sa1rambankb;
extern mrwp dsp1bank, dsp2bank, dsp3bank, dsp4bank;
extern mrwp setabank, setabanka, seta11bank, seta11banka;
extern mrwp sfxbank, sfxbankb, sfxbankc, sfxbankd;
extern mrwp obc1bank, c4bank, SPC7110bank, SPC7110SRAMBank;
extern mrwp stbanka, stbankb;

static inline void rep_stosd_r8 (mr8**  dest, mr8*  fp, size_t n) { while (n--) dest[n] = fp; }
static inline void rep_stosd_w8 (mw8**  dest, mw8*  fp, size_t n) { while (n--) dest[n] = fp; }
static inline void rep_stosd_r16(mr16** dest, mr16* fp, size_t n) { while (n--) dest[n] = fp; }
static inline void rep_stosd_w16(mw16** dest, mw16* fp, size_t n) { while (n--) dest[n] = fp; }

static inline void map_mem(size_t dest, mrwp* src, size_t num)
{
    rep_stosd_r8 (memtabler8  + dest, src->memr8,  num);
    rep_stosd_w8 (memtablew8  + dest, src->memw8,  num);
    rep_stosd_r16(memtabler16 + dest, src->memr16, num);
    rep_stosd_w16(memtablew16 + dest, src->memw16, num);
}

static inline u1 memr8(u1 const bank, u2 const address)
{
    return memtabler8[bank](address);
}

static inline u2 memr16(u1 const bank, u2 const address)
{
    return memtabler16[bank](address);
}

static inline void memw8no_rom(u1 const bank, u2 const address, u1 const val)
{
    memtablew8[bank](address, val);
}

static inline void memw8(u1 const bank, u2 const address, u1 const val)
{
    writeon = 1;
    memw8no_rom(bank, address, val);
    writeon = 0;
}

#endif
