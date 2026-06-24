#ifndef VCACHE_H
#define VCACHE_H

#include "types.h"

extern void c_cachesingle2bng(u4 ecx);
extern void c_cachesingle4bng(u4 ecx);
extern void c_cachesingle8bng(u4 ecx);
extern void cachesprites();
extern void cachetile2b(u4 eax);
extern void cachetile2b16x16(u4 eax);
extern void cachetile4b(u4 eax);
extern void cachetile4b16x16(u4 eax);
extern void cachetile8b(u4 eax);
extern void cachetile8b16x16(u4 eax);
extern void processsprites();

extern u4 lineleft;
extern u1 res480;
extern u1 res640;
extern u1 sprprifix;
extern u4 videotroub;
extern u2 dcolortab[2][256];
extern u4 vesa2_bfull; // red max (or bit*1Fh)
extern u4 vesa2_btrcl; // red transparency clear     (bit+4)
extern u4 vesa2_btrcla; // red transparency (AND) clear (not(bit+4))
extern u4 vesa2_clbit; // clear all bit 0's if AND is used
extern u4 vesa2_gfull; // red max (or bit*1Fh)
extern u4 vesa2_gtrcl; // red transparency clear     (bit+4)
extern u4 vesa2_gtrcla; // red transparency (AND) clear (not(bit+4))
extern u4 vesa2_rfull; // red max (or bit*1Fh)
extern u4 vesa2_rtrcl; // red transparency clear     (bit+4)
extern u4 vesa2_rtrcla; // red transparency (AND) clear (not(bit+4))
extern u4 vesa2_usbit; // Unused bit in proper bit location
extern u4 vesa2_x; // Desired screen width
extern u4 vesa2_y; // Height
extern u4 OMBGTestVal;
extern u4 ngptrdat2;
extern u4 ofshvaladd;
extern u4 ofsmcptr2;
extern u4 ofsmtptrs;
extern u4 vesa2_bits; // Bits per pixel
extern u4 vesa2_bpos; // Blue bit position
extern u4 vesa2_bposng; // Blue bit position
extern u4 vesa2_clbitng2[2]; // clear all bit 0's if AND is used
extern u4 vesa2_clbitng3; // clear all bit 0's if AND is used
extern u4 vesa2_clbitng; // clear all bit 0's if AND is used
extern u4 vesa2_gpos; // Green bit position
extern u4 vesa2_gposng; // Green bit position
extern u4 vesa2_rpos; // Red bit position
extern u4 vesa2_rposng; // Red bit position
extern u4 vesa2red10; // red position at bit 10

#endif
