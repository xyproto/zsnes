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

#include <string.h>

#include "../c_init.h"
#include "../cpu/regs.h"
#include "../link.h"
#include "sw_draw.h"

u1* ScreenPtr;
u1* SurfBufD;

void ClearWin16(void)
{
    u1* edi = SurfBufD;
    s4 ebx = 0;
    do
        memset(edi, 0, SurfaceX * 2);
    while (edi += pitch, ++ebx != SurfaceY);
}

void ClearWin32(void)
{
    u1* edi = SurfBufD;
    s4 ebx = 0;
    do
        memset(edi, 0, SurfaceX * 4);
    while (edi += pitch, ++ebx != SurfaceY);
}

void DrawWin256x224x16(void)
{
    u1 const* esi = ScreenPtr;
    u1* edi = SurfBufD;
    u4 eax = 0;
#ifdef __WIN32__
    u4 const edx = resolutn;
#endif
    do {
        memcpy(edi, esi, 512);
        edi += pitch;
        esi += 576;
    }
#ifdef __WIN32__
    while (++eax != edx);
#else
    while (++eax != 223);
#endif
    memset(edi, 0, 512);
}

#ifdef __WIN32__

void DrawWin256x224x32(void)
{
    u4 eax = 0;
    u4 const edx = resolutn;
    u2 const* esi = (u2 const*)ScreenPtr;
    u4* edi = (u4*)SurfBufD;
    do {
        u4 ecx = 256;
        do {
            u4 const px = *esi++;
            *edi++ = (px & 0xF800) << 8 | (px & 0x07E0) << 5 | (px & 0x001F) << 3;
        } while (--ecx != 0);
        edi = (u4*)((u1*)edi + (pitch - 1024));
        esi += (576 - 512) / sizeof(*esi);
    } while (++eax != edx);
}

#endif

void DrawWin320x240x16(void)
{
    u1 const* esi = ScreenPtr;
    u1* edi = SurfBufD;
    u4 const edx = resolutn;

    {
        u4 ebx = 0;
        do
            memset(edi, 0, 640);
        while (edi += pitch, ++ebx != 8);
    }

    {
        u4 ebx = 0;
        do {
            memset(edi, 0, 64);
            memcpy(edi + 64, esi, 512);
            memset(edi + 64 + 512, 0, 64);
        } while (esi += 576, edi += pitch, ++ebx != edx);
    }

    memset(edi, 0, 512);
}
