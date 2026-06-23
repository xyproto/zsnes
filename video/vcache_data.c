// vcache_data.c - data tables formerly defined in vcache.asm.
//
// These globals are shared with the assembly renderer (newgfx*.asm,
// mode716*.asm, ...) and several C files.  Sizes and initial values match the
// original NASM definitions exactly so the existing access widths stay valid.
// Symbol names match the plain ELF names the assembly refers to.

#include "../types.h"

// Sprite priority fix flag (hardcoded 1; the sprite path is always priority).
u1 sprprifix = 1;

// Offset-mode scratch values (dword).
u4 OMBGTestVal = 0;
u4 ngptrdat2 = 0;
u4 ofshvaladd = 0;
u4 ofsmtptrs = 0;
u4 ofsmcptr2 = 0;

// Sprite table pointer adder.
u4 addr2add = 0;

// Direct-colour lookup table (two planes of 256 words).
u2 dcolortab[2][256];

// Resolution flags.
u1 res640;
u1 res480;

// Scanline counter used by copyvwin.
u4 lineleft;

// Video-trouble flag.
u4 videotroub = 0;

// VESA / direct-draw colour conversion setup (dword each, as in the asm).
u4 vesa2_clbit = 0;
u4 vesa2_rpos = 0;
u4 vesa2_gpos = 0;
u4 vesa2_bpos = 0;
u4 vesa2_clbitng = 0;
u4 vesa2_clbitng2[2] = { 0, 0 };
u4 vesa2_clbitng3 = 0;
u4 vesa2red10 = 0;
u4 vesa2_rtrcl = 0;
u4 vesa2_rtrcla = 0;
u4 vesa2_rfull = 0;
u4 vesa2_gtrcl = 0;
u4 vesa2_gtrcla = 0;
u4 vesa2_gfull = 0;
u4 vesa2_btrcl = 0;
u4 vesa2_btrcla = 0;
u4 vesa2_bfull = 0;
u4 vesa2_x = 320;
u4 vesa2_y = 240;
u4 vesa2_bits = 8;
u4 vesa2_rposng = 0;
u4 vesa2_gposng = 0;
u4 vesa2_bposng = 0;
u4 vesa2_usbit = 0;
