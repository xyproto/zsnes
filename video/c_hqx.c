/*
 * Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
 *
 * http://www.zsnes.com
 * http://sourceforge.net/projects/zsnes
 * https://zsnes.bountysource.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * C ports of the gutted hqNx asm scalers: each is a scalar nearest-neighbor
 * block doubler (NxN pixel replication), not the real hq filter.
 */

#include "c_hqx.h"
#include "../c_vcache.h"
#include "../cpu/regs.h"
#include "../ui.h"
#include "copyvwin.h"

// 16->32bit color lookup table (4096 + 65536*16 bytes), defined in ui.c.
extern u1* BitConv32Ptr;

// Source pixels start past the top/left border; 256 pixels are read per line
// then 32 (64 bytes) are skipped to the next line. Skip drawing when blanked.
#define SRC_START ((u2 const*)(vidbuffer + 16 * 2 + 256 * 2 + 32 * 2))
#define SRC_LINE_SKIP 32 // u2 units

void hq2x_16b(void)
{
    if (curblank == 0x40)
        return;
    u2 const* src = SRC_START;
    u1* dst = WinVidMemStart;
    u4 const ebx = NumBytesPerLine;
    u1 lines = resolutn;
    do {
        u4 ecx = 256;
        do {
            u4 const px = *src * 0x00010001u; // pixel in both halves
            *(u4*)dst = px;
            *(u4*)(dst + ebx) = px;
            src += 1;
            dst += 4;
        } while (--ecx != 0);
        dst += AddEndBytes + ebx;
        src += SRC_LINE_SKIP;
    } while (--lines != 0);
}

void hq2x_32b(void)
{
    if (curblank == 0x40)
        return;
    u4 const* conv = (u4 const*)BitConv32Ptr;
    u2 const* src = SRC_START;
    u1* dst = WinVidMemStart;
    u4 const ebx = NumBytesPerLine;
    u1 lines = resolutn;
    do {
        u4 ecx = 256;
        do {
            u4 const px = conv[*src];
            *(u4*)dst = px;
            *(u4*)(dst + 4) = px;
            *(u4*)(dst + ebx) = px;
            *(u4*)(dst + ebx + 4) = px;
            src += 1;
            dst += 8;
        } while (--ecx != 0);
        dst += AddEndBytes + ebx;
        src += SRC_LINE_SKIP;
    } while (--lines != 0);
}

void hq3x_16b(void)
{
    if (curblank == 0x40)
        return;
    u2 const* src = SRC_START;
    u1* dst = WinVidMemStart;
    u4 const ebx = NumBytesPerLine;
    u1 lines = resolutn;
    do {
        u4 ecx = 256;
        do {
            u4 const px = *src * 0x00010001u;
            u2 const p = *src;
            *(u4*)dst = px;
            *(u4*)(dst + ebx) = px;
            *(u4*)(dst + ebx * 2) = px;
            *(u2*)(dst + 4) = p;
            *(u2*)(dst + ebx + 4) = p;
            *(u2*)(dst + ebx * 2 + 4) = p;
            src += 1;
            dst += 6;
        } while (--ecx != 0);
        dst += AddEndBytes + ebx * 2;
        src += SRC_LINE_SKIP;
    } while (--lines != 0);
}

void hq3x_32b(void)
{
    if (curblank == 0x40)
        return;
    u4 const* conv = (u4 const*)BitConv32Ptr;
    u2 const* src = SRC_START;
    u1* dst = WinVidMemStart;
    u4 const ebx = NumBytesPerLine;
    u1 lines = resolutn;
    do {
        u4 ecx = 256;
        do {
            u4 const px = conv[*src];
            *(u4*)dst = px;
            *(u4*)(dst + 4) = px;
            *(u4*)(dst + 8) = px;
            *(u4*)(dst + ebx) = px;
            *(u4*)(dst + ebx + 4) = px;
            *(u4*)(dst + ebx + 8) = px;
            *(u4*)(dst + ebx * 2) = px;
            *(u4*)(dst + ebx * 2 + 4) = px;
            *(u4*)(dst + ebx * 2 + 8) = px;
            src += 1;
            dst += 12;
        } while (--ecx != 0);
        dst += AddEndBytes + ebx * 2;
        src += SRC_LINE_SKIP;
    } while (--lines != 0);
}

void hq4x_16b(void)
{
    if (curblank == 0x40)
        return;
    u2 const* src = SRC_START;
    u1* dst = WinVidMemStart;
    u4 const ebx = NumBytesPerLine;
    u1 lines = resolutn;
    do {
        u4 ecx = 256;
        do {
            u4 const px = *src * 0x00010001u;
            *(u4*)dst = px;
            *(u4*)(dst + 4) = px;
            *(u4*)(dst + ebx) = px;
            *(u4*)(dst + ebx + 4) = px;
            *(u4*)(dst + ebx * 2) = px;
            *(u4*)(dst + ebx * 2 + 4) = px;
            *(u4*)(dst + ebx * 3) = px;
            *(u4*)(dst + ebx * 3 + 4) = px;
            src += 1;
            dst += 8;
        } while (--ecx != 0);
        dst += AddEndBytes + ebx * 3;
        src += SRC_LINE_SKIP;
    } while (--lines != 0);
}

void hq4x_32b(void)
{
    if (curblank == 0x40)
        return;
    u4 const* conv = (u4 const*)BitConv32Ptr;
    u2 const* src = SRC_START;
    u1* dst = WinVidMemStart;
    u4 const ebx = NumBytesPerLine;
    u1 lines = resolutn;
    do {
        u4 ecx = 256;
        do {
            u4 const px = conv[*src];
            *(u4*)dst = px;
            *(u4*)(dst + 4) = px;
            *(u4*)(dst + 8) = px;
            *(u4*)(dst + 12) = px;
            *(u4*)(dst + ebx) = px;
            *(u4*)(dst + ebx + 4) = px;
            *(u4*)(dst + ebx + 8) = px;
            *(u4*)(dst + ebx + 12) = px;
            *(u4*)(dst + ebx * 2) = px;
            *(u4*)(dst + ebx * 2 + 4) = px;
            *(u4*)(dst + ebx * 2 + 8) = px;
            *(u4*)(dst + ebx * 2 + 12) = px;
            *(u4*)(dst + ebx * 3) = px;
            *(u4*)(dst + ebx * 3 + 4) = px;
            *(u4*)(dst + ebx * 3 + 8) = px;
            *(u4*)(dst + ebx * 3 + 12) = px;
            src += 1;
            dst += 16;
        } while (--ecx != 0);
        dst += AddEndBytes + ebx * 3;
        src += SRC_LINE_SKIP;
    } while (--lines != 0);
}
