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

// Tools for the GUI

#include <string.h>

#include "../cfg.h"
#include "../ui.h"
#include "../video/procvid.h"
#include "c_gui.h"
#include "c_guiwindp.h"
#include "gui.h"
#include "guitools.h"

static void GUIoutputchar(u1* dst, u1 const glyph, u1 const colour)
{
    // XXX better variable names
    // Font Setup (Menus)
    u1 const* edi = newfont == 0 ? GUIFontData[glyph] : GUIFontData1[glyph];
    u4 cl = 5;
    do {
        u4 ah = *edi;
        u4 ch = 6;
        do {
            if (ah & 0x80)
                *dst = colour - cl - ch + 1;
        } while (ah <<= 1, ++dst, --ch != 0);
        dst += 282;
        ++edi;
    } while (--cl != 0);
}

char const* GUIOutputString(u1* dst, char const* text, u1 const colour)
{
    for (;; dst += 6, ++text) {
        u1 const c = *text;
        if (c == '\0')
            return text;
        GUIoutputchar(dst, ASCII2Font[c], colour);
    }
}

void GUIOuttext(u4 const x, u4 const y, char const* const text, u1 const colour)
{
    u1* const dst = vidbuffer + y * 288 + x + 16;
    GUIOutputString(dst, text, colour);
}

void GUIDrawBox(u1* dst, u4 const w, u4 h, u1 const colour)
{
    do {
        memset(dst, colour, w);
        dst += 288;
    } while (--h != 0);
}

void GUIBox(u4 const x1, u4 const y1, u4 const x2, u4 const y2, u1 const colour)
{
    u1* const dst = vidbuffer + x1 + y1 * 288 + 16;
    u4 const w = x2 - x1 + 1;
    u4 const h = y2 - y1 + 1;
    GUIDrawBox(dst, w, h, colour);
}

void GUIHLine(s4 x1, s4 x2, s4 const y, u1 const colour)
{
    if (x2 < 0)
        return;
    if (x1 > 255)
        return;
    if (x1 < 0)
        x1 = 0;
    if (x2 > 255)
        x2 = 255;
    if (y < 0)
        return;
    if (y > 223)
        return;
    memset(vidbuffer + x1 + 16 + y * 288, colour, x2 - x1 + 1);
}

static void GUIDrawShadow(u1* dst, u4 const w, u4 h)
{
    do {
        u1* edi = dst;
        u4 ecx = w;
        do {
            u1 px = edi[-3 * 288 - 3];
            if (148 <= px && px <= 167) {
                edi[-3 * 288 - 3] = px + 20;
            } else if (189 <= px && px <= 220) {
                edi[-3 * 288 - 3] = 189 + (px - 189) / 2;
            } else {
                px = edi[0];
                if (px < 32)
                    edi[0] = px + 96;
            }
            ++edi;
        } while (--ecx != 0);
        dst += 288;
    } while (--h != 0);
}

void GUIShadow(u4 const x1, u4 const y1, u4 const x2, u4 const y2)
{
    u1* const dst = vidbuffer + x1 + y1 * 288 + 16;
    u4 const w = x2 - x1 + 1;
    u4 const h = y2 - y1 + 1;
    GUIDrawShadow(dst, w, h);
}

void GUIDrawShadow2(u1* buf, u4 const w, u4 h)
{
    do {
        u1* b = buf;
        u4 n = w;
        do {
            u1 const c = *b;
            if (c < 32)
                *b = c + 96;
            ++b;
        } while (--n != 0);
        buf += 288;
    } while (--h != 0);
}

static void GUIoutputcharwin(u1* dst, u1 const glyph, u1 const colour)
{
    // Font Setup (Windows)
    u1 const* edi = newfont == 0 ? GUIFontData[glyph] : GUIFontData1[glyph];
    u4 y = 5;
    do {
        if (vidbuffer <= dst && dst < vidbuffer + 224 * 288) // XXX possible buffer overflow by 4
        {
            u4 ah = *edi;
            u4 x = 5;
            do {
                if (ah & 0x80)
                    *dst = colour;
                ah <<= 1;
                ++dst;
            } while (--x != 0);
            dst += 283;
        } else {
            dst += 288;
        }
        ++edi;
    } while (--y != 0);
}

static void GUIOutputStringwin(s4 x, u1* const dst, char const* text, u1 const colour)
{
    for (;; x += 6) {
        u1 const c = *text++;
        if (c == '\0')
            break;
        if (-8 <= x && x <= 255)
            GUIoutputcharwin(dst + x, ASCII2Font[c], colour);
    }
}

static void GUIOutputStringwinl(s4 x, u1* const dst, char const* text, u1 const colour)
{
    u4 n = cloadmaxlen;
    do {
        u1 c = *text++;
#ifndef __MSDOS__
        if (c == '%') {
            u1 v;
            u1 const c0 = text[0];
            if ('0' <= c0 && c0 <= '9')
                v = c0 - '0';
            else if ('A' <= c0 && c0 <= 'F')
                v = c0 - 'A' + 10;
            else if ('a' <= c0 && c0 <= 'f')
                v = c0 - 'a' + 10;
            else
                goto no_number;
            v <<= 4;
            u1 const c1 = text[1];
            if ('0' <= c1 && c1 <= '9')
                v |= c1 - '0';
            else if ('A' <= c1 && c1 <= 'F')
                v |= c1 - 'A' + 10;
            else if ('a' <= c1 && c1 <= 'f')
                v |= c1 - 'a' + 10;
            else
                goto no_number;
            c = v;
            text += 2;
        no_number:;
        }
#endif
        if (c == '\0')
            break;
        if (-8 <= x && x <= 255)
            GUIoutputcharwin(dst + x, ASCII2Font[c], colour);
        x += 6;
    } while (--n != 0);
}

void GUIOuttextwin2(u4 const win_id, u4 x, u4 y, char const* const text, u1 const colour)
{
    x += GUIwinposx[win_id];
    y += GUIwinposy[win_id];
    u1* const dst = vidbuffer + y * 288 + 16;
    GUIOutputStringwin(x, dst, text, colour);
}

void GUIOuttextwin2l(u4 const win_id, u4 x, u4 y, char const* const text, u1 const colour)
{
    x += GUIwinposx[win_id];
    y += GUIwinposy[win_id];
    u1* const dst = vidbuffer + y * 288 + 16;
    GUIOutputStringwinl(x, dst, text, colour);
}

static void OutputUnder(s4 const edx, u1* const esi)
{
    if (-8 <= edx && edx <= 255) {
        memset(esi + edx + 5 * 288 - 1, 0xE8, 5);
    }
}

void GUIOuttextwin(u4 x, u4 const y, char const* const text, u1 const colour)
{
    GUIOutputStringwin(x, vidbuffer + y * 288 + 16, text, colour);
}

void GUIOuttextwin2u(u4 const win_id, u4 x, u4 y, char const* const text, u1 const colour, u4 const under_pos)
{
    x += GUIwinposx[win_id];
    y += GUIwinposy[win_id];
    u1* const dst = vidbuffer + y * 288 + 16;
    OutputUnder(x + under_pos * 6, dst);
    GUIOutputStringwin(x, dst, text, colour);
}

void GUIoutputiconwin(s4 const x, u4 const y, u1 const* src)
{
    if (x < -9 || 256 <= x)
        return;
    u1* dst = y * 288 + 16 + vidbuffer + x;
    u4 cl = 10;
    do {
        if (vidbuffer <= dst && dst < vidbuffer + 224 * 288) {
            u4 ch = 10;
            do {
                u1 al = *src;
                if (al != 0) {
                    if (al <= 189) {
                        if (GUIWincoladd != 0)
                            ++al;
                    } else {
                        al -= GUIWincoladd + 1;
                    }
                    *dst = al;
                }
                ++src;
                ++dst;
            } while (--ch != 0);
            dst += 278;
        } else {
            dst += 288;
        }
    } while (--cl != 0);
}

void GUIDisplayIconWin(u4 const win_id, u4 const x, u4 const y, u1 const* icon)
{
    GUIoutputiconwin(GUIwinposx[win_id] + x, GUIwinposy[win_id] + y, icon);
}

static void GUIDrawSlideBar(s4 const x, u4 const y, u4 h, u4 starty, u4 endy)
{
    if (x < -10 || 256 < x)
        return;
    u1* const vbuflimtop = vidbuffer;
    u1* const vbuflimbot = vidbuffer + 288 * 224 - 26;
    u1* dst = vidbuffer + y * 288 + x + 16;
    u4 draw_slider = 0;
    do {
        --endy;
        if (vbuflimtop <= dst && dst <= vbuflimbot) {
            if (starty == 0) {
                u1 const al = 202 - GUIWincoladd;
                dst[0] = al;
                dst[1] = al + 2;
                dst[2] = al + 2;
                dst[3] = al + 2;
                dst[4] = al + 2;
                dst[5] = al + 2;
                dst[6] = al + 2;
                dst[7] = al + 2;
                draw_slider = 1;
            } else if (endy == 0) {
                u1 const al = 196 - GUIWincoladd;
                dst[0] = al;
                dst[1] = al;
                dst[2] = al;
                dst[3] = al;
                dst[4] = al;
                dst[5] = al;
                dst[6] = al;
                dst[7] = al + 2;
                draw_slider = 0;
            } else if (draw_slider != 1) {
                u1 const al = 197 - GUIWincoladd;
                dst[0] = al;
                dst[1] = al - 2;
                dst[2] = al - 3;
                dst[3] = al - 4;
                dst[4] = al - 4;
                dst[5] = al - 3;
                dst[6] = al - 2;
                dst[7] = al;
            } else {
                u1 const al = 202 - GUIWincoladd;
                dst[0] = al;
                dst[1] = al - 2;
                dst[2] = al - 2;
                dst[3] = al - 2;
                dst[4] = al - 2;
                dst[5] = al - 2;
                dst[6] = al - 2;
                dst[7] = al;
            }
        }
        dst += 288;
        --starty;
    } while (--h != 0);
}

void DrawSlideBarWin(u4 const win_id, u4 const x, u4 const y, u4 list_loc, u4 list_size, u4 const screen_size, u4 const bar_size, u4* const bar_dims)
{
    if (list_size < screen_size)
        list_size = screen_size;
    if (list_loc > list_size - screen_size)
        list_loc = list_size - screen_size;
    u4 scrollbar_size = screen_size * bar_size / list_size;
    if (scrollbar_size < 5)
        scrollbar_size = 5;
    u4 const gap = list_size - screen_size;
    u4 const starty = gap != 0 ? (bar_size - scrollbar_size) * list_loc / gap : 0;
    u4 const endy = scrollbar_size + starty;
    bar_dims[0] = bar_size - scrollbar_size - 1;
    bar_dims[1] = starty;
    bar_dims[2] = endy;
    GUIDrawSlideBar(GUIwinposx[win_id] + x, GUIwinposy[win_id] + y, bar_size, starty, endy);
}
