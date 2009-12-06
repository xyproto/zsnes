#ifndef VCACHE_H
#define VCACHE_H

#include "types.h"

extern u1 res480;
extern u1 res640;
extern u1 videotroub;
extern u2 vesa2_x;           // Desired screen width
extern u2 vesa2_y;           // Height
extern u4 vesa2_bits;        // Bits per pixel
extern u4 vesa2_bpos;        // Blue bit position
extern u4 vesa2_bposng;      // Blue bit position
extern u4 vesa2_clbitng2[2]; // clear all bit 0's if AND is used
extern u4 vesa2_clbitng3;    // clear all bit 0's if AND is used
extern u4 vesa2_clbitng;     // clear all bit 0's if AND is used
extern u4 vesa2_gpos;        // Green bit position
extern u4 vesa2_gposng;      // Green bit position
extern u4 vesa2_rpos;        // Red bit position
extern u4 vesa2_rposng;      // Red bit position
extern u4 vesa2red10;        // red position at bit 10

#endif
