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
#include "../c_vcache.h"
#include "../cfg.h"
#include "../cpu/regs.h"
#include "../endmem.h"
#include "../gui/gui.h"
#include "../ui.h"
#include "../vcache.h"
#include "2xsaiw.h"
#include "copyvwin.h"
#include "makevid.h"
#include "newgfx16.h"

#ifdef __WIN32__
#include "../c_intrf.h"
#include "../win/winlink.h"
#endif

u1* WinVidMemStart;
u4 AddEndBytes;
u4 NumBytesPerLine;

static u1* InterPtr = 0;

static void HighResProc(u2** psrc, u1** pdst, u1* ebx)
{
    u2* src = *psrc;
    u1* dst = *pdst;
    u4 ecx = 256;
    if (*ebx != 3 && *ebx != 7) {
        if (*ebx & 4 && scanlines == 0 && cfield & 1) {
            dst += NumBytesPerLine;
        }
        do {
            if (*ebx & 3) {
                do {
                    *(u4*)dst = src[75036 * 2] << 16 | *src;
                    src += 1;
                    dst += 4;
                } while (--ecx != 0);
                if (*ebx & 4 && scanlines == 0) {
                    if (!(cfield & 1))
                        dst += NumBytesPerLine;
                } else {
                    switch (scanlines) {
                    case 1:
                        break;

                    case 3: {
                        dst += AddEndBytes;
                        src -= 256;
                        u4 ecx = 256;
                        do {
                            u4 eax = src[75036 * 2] << 16 | *src;
                            eax = (eax & HalfTrans[0]) >> 1;
                            *(u4*)dst = eax;
                            src += 1;
                            dst += 4;
                        } while (--ecx != 0);
                        break;
                    }

                    case 2: {
                        dst += AddEndBytes;
                        src -= 256;
                        u4 ecx = 256;
                        do {
                            u4 eax = src[75036 * 2] << 16 | *src;
                            eax = (eax & HalfTrans[0]) >> 1;
                            u4 edx = (eax & HalfTrans[0]) >> 1;
                            eax += edx;
                            *(u4*)dst = eax;
                            src += 1;
                            dst += 4;
                        } while (--ecx != 0);
                        break;
                    }

                    default: {
                        dst += AddEndBytes;
                        src -= 256;
                        u4 ecx = 256;
                        do {
                            *(u4*)dst = src[75036 * 2] << 16 | *src;
                            src += 1;
                            dst += 4;
                        } while (--ecx != 0);
                        break;
                    }
                    }
                }
                goto end;
            } else {
                do {
                    *(u4*)dst = *src * 0x00010001;
                    src += 1;
                    dst += 4;
                } while (--ecx != 0);
            }
        } while (scanlines != 0);
        if (!(cfield & 1))
            dst += NumBytesPerLine;
    } else {
        do {
            *(u4*)dst = *src * 0x00010001;
            src += 1;
            dst += 4;
        } while (--ecx != 0);
        dst += AddEndBytes;
        src -= 256;
        u4 ecx = 256;
        src += 75036 * 2;
        do {
            *(u4*)dst = *src * 0x00010001;
            src += 1;
            dst += 4;
        } while (--ecx != 0);
        src -= 75036 * 2;
    }
end:
    *psrc = src;
    *pdst = dst;
}

static u1* SelectTile(void)
{
    return GUIOn == 1 || newengen == 0 ? hirestiledat + 1 : SpecialLine + 1;
}

static void interpolate640x480x16bwin(u2* src, u1* dst, u1 dl)
{
    u1* ebx = SelectTile();
    InterPtr = ebx;

    switch (scanlines) {
    case 1: {
        for (;;) {
            u4 ecx = 255;
            if (*ebx > 1) {
                u2* esi_ = src;
                u1* edi_ = dst;
                HighResProc(&esi_, &edi_, ebx);
                src = esi_;
                dst = edi_;

                src += 32;
                ++InterPtr;
                dst += AddEndBytes;
                u4 ecx = 256;
                do { // XXX memset()?
                    *(u4*)dst = 0;
                    dst += 4;
                } while (--ecx != 0);
                dst += AddEndBytes;
                if (--lineleft != 0)
                    continue;
            } else {
                if (*ebx != 1) {
                    do {
                        u4 eax = src[0];
                        u4 ebx = src[1];
                        ebx &= *(u4*)((u1*)HalfTrans + 6); // XXX unaligned?
                        eax &= *(u4*)((u1*)HalfTrans + 6); // XXX unaligned?
                        ebx = (ebx + eax) << 15 & 0xFFFF0000 | *src;
                        *(u4*)dst = ebx;
                        src += 1;
                        dst += 4;
                    } while (--ecx != 0);
                } else {
                    *ebx = 0;
                    if (!(res512switch & 1)) {
                        do {
                            *(u2*)dst = *src;
                            src += 1;
                            dst += 4;
                        } while (--ecx != 0);
                    } else {
                        do {
                            *(u2*)(dst + 2) = *src;
                            src += 1;
                            dst += 4;
                        } while (--ecx != 0);
                    }
                }
                src += 33;
                dst += 4 + AddEndBytes;
                u4 ecx = 256;
                do { // XXX memset()?
                    *(u4*)dst = 0;
                    dst += 4;
                } while (--ecx != 0);
                dst += AddEndBytes;
                ++ebx;
                if (--dl != 0)
                    continue;
                res512switch ^= 1;
            }
            return;
        }
    }

    case 2: {
        lineleft = dl;
        for (;;) {
            u1* ebx = InterPtr;
            if (*ebx > 1) {
                u2* esi_ = src;
                u1* edi_ = dst;
                HighResProc(&esi_, &edi_, ebx);
                src = esi_;
                dst = edi_;

                src += 32;
                ++InterPtr;
                dst += AddEndBytes;
            } else {
                {
                    u4 ecx = 255;
                    u1* edx = spritetablea + 512 * 256;
                    do {
                        u4 eax = src[0];
                        u4 ebx = src[1];
                        ebx &= *(u4*)((u1*)HalfTrans + 6); // XXX unaligned?
                        eax &= *(u4*)((u1*)HalfTrans + 6); // XXX unaligned?
                        ebx = (ebx + eax) << 15 & 0xFFFF0000 | *src;
                        *(u4*)edx = ebx;
                        *(u4*)dst = ebx;
                        src += 1;
                        dst += 4;
                        edx += 4;
                    } while (--ecx != 0);
                }
                dst += AddEndBytes + 4;
                {
                    u4 ecx = 255;
                    u1* edx = spritetablea + 512 * 256;
                    do {
                        u4 eax = (*(u4*)edx & HalfTrans[0]) >> 1;
                        u4 ebx = (eax & HalfTrans[0]) >> 1;
                        *(u4*)dst = eax + ebx;
                        dst += 4;
                        edx += 4;
                    } while (--ecx != 0);
                }
                ++InterPtr;
                src += 33;
                dst += 4 + AddEndBytes;
            }
            if (--lineleft != 0)
                continue;
            return;
        }
    }

    case 3: {
        lineleft = dl;
        do {
            u1* ebx = InterPtr;
            if (*ebx > 1) {
                u2* esi_ = src;
                u1* edi_ = dst;
                HighResProc(&esi_, &edi_, ebx);
                src = esi_;
                dst = edi_;
                src += 32;
                ++InterPtr;
                dst += AddEndBytes;
            } else {
                {
                    u4 ecx = 255;
                    u1* edx = spritetablea + 512 * 256;
                    do {
                        u4 eax = src[0];
                        u4 ebx = src[1];
                        ebx &= *(u4*)((u1*)HalfTrans + 6); // XXX unaligned?
                        eax &= *(u4*)((u1*)HalfTrans + 6); // XXX unaligned?
                        ebx = (ebx + eax) << 15 & 0xFFFF0000 | *src;
                        *(u4*)edx = ebx;
                        *(u4*)dst = ebx;
                        src += 1;
                        dst += 4;
                        edx += 4;
                    } while (--ecx != 0);
                }
                dst += 4 + AddEndBytes;
                {
                    u4 ecx = 255;
                    u1* edx = spritetablea + 512 * 256;
                    do {
                        *(u4*)dst = (*(u4*)edx & HalfTrans[0]) >> 1;
                        dst += 4;
                        edx += 4;
                    } while (--ecx != 0);
                }
                ++InterPtr;
                src += 33;
                dst += 4 + AddEndBytes;
            }
        } while (--lineleft != 0);
        return;
    }

    default: {
        lineleft = dl;
        // do first line
        u4 ecx = 255;
        u1* edx = spritetablea + 512 * 256;
        do {
            u4 eax = src[0];
            u4 ebx = src[1];
            ebx &= *(u4*)((u1*)HalfTrans + 6); // XXX unaligned?
            eax &= *(u4*)((u1*)HalfTrans + 6); // XXX unaligned?
            ebx = (ebx + eax) << 15 & 0xFFFF0000 | *src;
            *(u4*)dst = ebx;
            *(u4*)edx = ebx;
            src += 1;
            dst += 4;
            edx += 4;
        } while (--ecx != 0);
        src += 33;
        dst += AddEndBytes + 4;
        do {
            u1* ebx = InterPtr;
            if (*ebx > 1) {
                u2* esi_ = src;
                u1* edi_ = dst;
                HighResProc(&esi_, &edi_, ebx);
                src = esi_;
                dst = edi_;
                src += 32;
                ++InterPtr;
                dst += AddEndBytes;
            } else {
                {
                    u4 ecx = 255;
                    u1* edx = spritetablea + 512 * 256;
                    do {
                        u4 eax = src[0];
                        u4 ebx = src[1];
                        ebx &= *(u4*)((u1*)HalfTrans + 6); // XXX unaligned?
                        eax &= *(u4*)((u1*)HalfTrans + 6); // XXX unaligned?
                        ebx += eax;
                        ebx <<= 15;
                        u4 eax_ = *(u4*)edx;
                        ebx = ebx & 0xFFFF0000 | *src;
                        *(u4*)edx = ebx;
                        *(u4*)dst = ((eax_ & HalfTrans[0]) >> 1) + ((ebx & HalfTrans[0]) >> 1);
                        src += 1;
                        dst += 4;
                        edx += 4;
                    } while (--ecx != 0);
                    dst += 4 + AddEndBytes;
                }
                {
                    u1* edx = spritetablea + 512 * 256;
                    u4 ecx = 255;
                    do { // XXX memcpy()?
                        *(u4*)dst = *(u4*)edx;
                        edx += 4;
                        dst += 4;
                    } while (--ecx != 0);
                }
                src += 33;
                ++InterPtr;
                dst += AddEndBytes + 4;
            }
        } while (--lineleft != 0);
        return;
    }
    }
}

void copy640x480x16bwin(void)
{
    if (curblank == 0x40)
        return;

    u2* src = (u2*)vidbuffer + 16 + 288;
    u1* dst = WinVidMemStart;
#ifdef __UNIXSDL__
    if (GUIOn != 1 && resolutn == 239)
        src += 8 * 288;
#endif
#ifdef __UNIXSDL__
    u4 dl = 224;
#else
    u4 dl = resolutn;
#endif
    // Check if interpolation mode
    if (FilteredGUI != 0 || GUIOn2 != 1) {
        if (antienab == 1) {
            interpolate640x480x16bwin(src, dst, dl);
            return;
        }
    }
    switch (scanlines) {
    case 1: // scanlines
    {
        u1* ebx = SelectTile();
        do {
            {
                u4 ecx = 256;
                if (*ebx < 1) {
                    do {
                        u4 eax = *src++ * 0x00010001U;
                        *(u4*)dst = eax;
                        dst += 4;
                    } while (--ecx != 0);
                } else if (*ebx == 1) {
                    *ebx = 0;
                    if (res512switch & 1) {
                        do {
                            u2 ax = *src++;
                            *(u2*)(dst + 2) = ax;
                            dst += 4;
                        } while (--ecx != 0);
                    } else {
                        do {
                            u2 ax = *src++;
                            *(u2*)dst = ax;
                            dst += 4;
                        } while (--ecx != 0);
                    }
                } else {
                    u2* src_ = src;
                    u1* dst_ = dst;
                    HighResProc(&src_, &dst_, ebx);
                    src = src_;
                    dst = dst_;
                }
            }
            src += 32;
            dst += AddEndBytes;
            {
                u4 ecx = 256;
                do {
                    *(u4*)dst = 0;
                    dst += 4;
                } while (--ecx != 0);
            }
            dst += AddEndBytes;
            ++ebx;
        } while (--dl != 0);
        res512switch ^= 1;
        break;
    }

    case 2: // quartscanlines
    {
        lineleft = dl;
        u1* ebx = SelectTile();
        do {
            if (*ebx <= 1) {
                {
                    u4 ecx = 256;
                    do {
                        u4 eax = *src++ * 0x00010001U;
                        *(u4*)dst = eax;
                        dst += 4;
                    } while (--ecx != 0);
                }
                {
                    u4 ecx = 256;
                    src -= 256;
                    dst += AddEndBytes;
                    do {
                        u4 eax = (*src++ * 0x00010001U & HalfTrans[0]) >> 1;
                        u4 edx = (eax & HalfTrans[0]) >> 1;
                        eax += edx;
                        *(u4*)dst = eax;
                        dst += 4;
                    } while (--ecx != 0);
                }
            } else {
                u2* src_ = src;
                u1* dst_ = dst;
                HighResProc(&src_, &dst_, ebx);
                src = src_;
                dst = dst_;
            }
            src += 32;
            dst += AddEndBytes;
            ++ebx;
        } while (--lineleft != 0);
        break;
    }

    case 3: // halfscanlines
    {
        u1* ebx = SelectTile();
        do {
            if (*ebx <= 1) {
                {
                    u4 ecx = 256;
                    do {
                        u4 eax = *src++ * 0x00010001U;
                        *(u4*)dst = eax;
                        dst += 4;
                    } while (--ecx != 0);
                }
                {
                    u4 ecx = 256;
                    src -= 256;
                    dst += AddEndBytes;
                    do {
                        u4 eax = (*src++ * 0x00010001U & HalfTrans[0]) >> 1;
                        *(u4*)dst = eax;
                        dst += 4;
                    } while (--ecx != 0);
                }
            } else {
                u2* src_ = src;
                u1* dst_ = dst;
                HighResProc(&src_, &dst_, ebx);
                src = src_;
                dst = dst_;
            }
            src += 32;
            dst += AddEndBytes;
            ++ebx;
        } while (--dl != 0);
        break;
    }

    default: {
        u1* ebx = hirestiledat + 1;
        if (newengen != 0)
            ebx = SpecialLine + 1;
        do {
            u4 ecx = 256;
            if (*ebx < 1) {
                do {
                    u4 eax = *src++ * 0x00010001U;
                    *(u4*)dst = eax;
                    dst += 4;
                } while (--ecx != 0);
                src -= 256;
                dst += AddEndBytes;
                u4 ecx = 256;
                do {
                    u4 eax = *src++ * 0x00010001U;
                    *(u4*)dst = eax;
                    dst += 4;
                } while (--ecx != 0);
            } else if (*ebx == 1) {
                *ebx = 0;
                if (res512switch & 1) {
                    {
                        u4 ebx = NumBytesPerLine;
                        do {
                            u2 ax = *src++;
                            *(u2*)(dst + 2) = ax;
                            *(u2*)(dst + 2 + ebx) = ax;
                            dst += 4;
                        } while (--ecx != 0);
                    }
                    dst += NumBytesPerLine;
                } else {
                    {
                        u4 ebx = NumBytesPerLine;
                        do {
                            u2 ax = *src++;
                            *(u2*)dst = ax;
                            *(u2*)(dst + ebx) = ax;
                            dst += 4;
                        } while (--ecx != 0);
                    }
                    dst += NumBytesPerLine;
                }
            } else {
                u2* src_ = src;
                u1* dst_ = dst;
                HighResProc(&src_, &dst_, ebx);
                src = src_;
                dst = dst_;
            }
            src += 32;
            dst += AddEndBytes;
            ++ebx;
        } while (--dl != 0);
        res512switch ^= 1;
        break;
    }
    }
}
