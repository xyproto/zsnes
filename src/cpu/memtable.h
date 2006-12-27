/*
Copyright (C) 1997-2007 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

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

extern void (*memtabler8[256])();
extern void (*memtablew8[256])();
extern void (*memtabler16[256])();
extern void (*memtablew16[256])();

typedef struct
{
  void (*memr8)();
  void (*memw8)();
  void (*memr16)();
  void (*memw16)();
} mrwp;

extern mrwp regbank, membank, wrambank, srambank, erambank, sramsbank;
extern mrwp sa1regbank, sa1rambank, sa1rambankb;
extern mrwp dsp1bank, dsp2bank, dsp3bank, dsp4bank;
extern mrwp setabank, setabanka, seta11bank, seta11banka;
extern mrwp sfxbank, sfxbankb, sfxbankc, sfxbankd;
extern mrwp obc1bank, c4bank, SPC7110bank, SPC7110SRAMBank;
extern mrwp stbanka, stbankb;

/*
rep_stosd is my name for a 'copy <num> times a function pointer <func_ptr> into
a function pointer array <dest>' function, in honour of the almighty asm
instruction rep stosd, which is able to do that (and much more).
Since ZSNES is just full of func pointer arrays, it'll probably come in handy.
*/

static void rep_stosd(void (**dest)(), void (*func_ptr), size_t num)
{
  while (num--) { dest[num] = func_ptr; }
}

static void map_mem(size_t dest, mrwp *src, size_t num)
{
  rep_stosd(memtabler8+dest, src->memr8, num);
  rep_stosd(memtablew8+dest, src->memw8, num);
  rep_stosd(memtabler16+dest, src->memr16, num);
  rep_stosd(memtablew16+dest, src->memw16, num);
}

#endif
