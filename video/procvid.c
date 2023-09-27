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

#include <stdio.h>
#include <string.h>

#include "../asm.h"
#include "../asm_call.h"
#include "../c_intrf.h"
#include "../c_vcache.h"
#include "../cfg.h"
#include "../cpu/execute.h"
#include "../cpu/regs.h"
#include "../endmem.h"
#include "../gui/c_gui.h"
#include "../gui/gui.h"
#include "../init.h"
#include "../initc.h"
#include "../input.h"
#include "../macros.h"
#include "../ui.h"
#include "../vcache.h"
#include "../zmovie.h"
#include "../zstate.h"
#include "../ztimec.h"
#include "newgfx16.h"
#include "procvid.h"
#include "procvidc.h"
#include "../linux/sdllink.h"

char const *Msgptr;
u1 FPSOn;
u1 ForceNonTransp;
u1 csounddisable;
u1 f3menuen;
u1 mousexdir;
u1 mouseydir;
u1 prevbright;
u1 ssautosw = 0x20;
u2 mousebuttons;
u2 mousexloc = 128;
u2 mousexpos;
u2 mouseyloc = 112;
u2 mouseypos;
u2 tempco0;
u4 MessageOn;
u4 MsgCount = 120;

static u2 allgrnb;
static u2 allgrn;

static u1 textcolor = 128;
static u2 textcolor16b = 0xFFFF;

static u2 cgramback[256];

static u1 curfps = 0; // frame/sec for current screen
static u1 lastfps;	  // stores the last fps encountered

u1 const ASCII2Font[] = {
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x30,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x3E,
	0x33,
	0x31,
	0x3F,
	0x37,
	0x2F,
	0x3D,
	0x3A,
	0x3B,
	0x35,
	0x38,
	0x39,
	0x25,
	0x28,
	0x29,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x07,
	0x08,
	0x09,
	0x0A,
	0x2E,
	0x40,
	0x2A,
	0x32,
	0x2B,
	0x36,
	0x3C,
	0x0B,
	0x0C,
	0x0D,
	0x0E,
	0x0F,
	0x10,
	0x11,
	0x12,
	0x13,
	0x14,
	0x15,
	0x16,
	0x17,
	0x18,
	0x19,
	0x1A,
	0x1B,
	0x1C,
	0x1D,
	0x1E,
	0x1F,
	0x20,
	0x21,
	0x22,
	0x23,
	0x24,
	0x2C,
	0x34,
	0x2D,
	0x42,
	0x26,
	0x41,
	0x0B,
	0x0C,
	0x0D,
	0x0E,
	0x0F,
	0x10,
	0x11,
	0x12,
	0x13,
	0x14,
	0x15,
	0x16,
	0x17,
	0x18,
	0x19,
	0x1A,
	0x1B,
	0x1C,
	0x1D,
	0x1E,
	0x1F,
	0x20,
	0x21,
	0x22,
	0x23,
	0x24,
	0x43,
	0x00,
	0x44,
	0x27,
	0x00,
	0x0D,
	0x1F,
	0x0F,
	0x0B,
	0x0B,
	0x0B,
	0x0B,
	0x0D,
	0x0F,
	0x0F,
	0x0F,
	0x13,
	0x13,
	0x13,
	0x0B,
	0x0B,
	0x0F,
	0x0B,
	0x0B,
	0x19,
	0x19,
	0x19,
	0x1F,
	0x1F,
	0x23,
	0x19,
	0x1F,
	0x0D,
	0x10,
	0x23,
	0x1A,
	0x10,
	0x0B,
	0x4E,
	0x4F,
	0x50,
	0x51,
	0x52,
	0x53,
	0x54,
	0x55,
	0x56,
	0x57,
	0x58,
	0x59,
	0x5A,
	0x5B,
	0x5C,
	0x5D,
	0x5E,
	0x5F,
	0x60,
	0x61,
	0x62,
	0x63,
	0x64,
	0x65,
	0x66,
	0x67,
	0x68,
	0x69,
	0x6A,
	0x6B,
	0x6C,
	0x6D,
	0x6E,
	0x6F,
	0x70,
	0x71,
	0x72,
	0x73,
	0x74,
	0x75,
	0x76,
	0x77,
	0x78,
	0x79,
	0x7A,
	0x7B,
	0x7C,
	0x7D,
	0x7E,
	0x7F,
	0x80,
	0x81,
	0x82,
	0x83,
	0x84,
	0x85,
	0x86,
	0x87,
	0x88,
	0x89,
	0x8A,
	0x8B,
	0x8C,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x4D,
	0x4C,
	0x4B,
	0x4A,
	0x45,
	0x46,
	0x47,
	0x48,
	0x49};

// bitmap 8x8 font ; char, offset for ASCII2Font
static u1 const FontData[][8] = {
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // ' ', 00
	{0x7C, 0xC6, 0xCE, 0xD6, 0xE6, 0xC6, 0x7C, 0x00}, // 0, 01
	{0x18, 0x38, 0x78, 0x18, 0x18, 0x18, 0x7E, 0x00}, // 1, 02
	{0x7C, 0xC6, 0x0C, 0x18, 0x30, 0x66, 0xFE, 0x00}, // 2, 03
	{0x7C, 0xC6, 0x06, 0x3C, 0x06, 0xC6, 0x7C, 0x00}, // 3, 04
	{0x3C, 0x6C, 0xCC, 0xFE, 0x0C, 0x0C, 0x0C, 0x00}, // 4, 05
	{0xFE, 0xC0, 0xC0, 0xFC, 0x06, 0xC6, 0x7C, 0x00}, // 5, 06
	{0x3C, 0x60, 0xC0, 0xFC, 0xC6, 0xC6, 0x7C, 0x00}, // 6, 07
	{0xFE, 0xC6, 0x06, 0x0C, 0x18, 0x18, 0x18, 0x00}, // 7, 08
	{0x7C, 0xC6, 0xC6, 0x7C, 0xC6, 0xC6, 0x7C, 0x00}, // 8, 09
	{0x7C, 0xC6, 0xC6, 0x7E, 0x06, 0xC6, 0x7C, 0x00}, // 9, 0A
	{0x38, 0x6C, 0xC6, 0xFE, 0xC6, 0xC6, 0xC6, 0x00}, // A, 0B
	{0xFC, 0xC6, 0xC6, 0xFC, 0xC6, 0xC6, 0xFC, 0x00}, // B, 0C
	{0x7C, 0xC6, 0xC0, 0xC0, 0xC0, 0xC6, 0x7C, 0x00}, // C, 0D
	{0xFC, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0xFC, 0x00}, // D, 0E
	{0xFE, 0xC0, 0xC0, 0xF8, 0xC0, 0xC0, 0xFE, 0x00}, // E, 0F
	{0xFE, 0xC0, 0xC0, 0xF8, 0xC0, 0xC0, 0xC0, 0x00}, // F, 10
	{0x7C, 0xC6, 0xC0, 0xC0, 0xCE, 0xC6, 0x7C, 0x00}, // G, 11
	{0xC6, 0xC6, 0xC6, 0xFE, 0xC6, 0xC6, 0xC6, 0x00}, // H, 12
	{0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00}, // I, 13
	{0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0xCC, 0x3C, 0x00}, // J, 14
	{0xCC, 0xD8, 0xF0, 0xE0, 0xF0, 0xD8, 0xCC, 0x00}, // K, 15
	{0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xFE, 0x00}, // L, 16
	{0xC6, 0xEE, 0xFE, 0xD6, 0xC6, 0xC6, 0xC6, 0x00}, // M, 17
	{0xC6, 0xE6, 0xF6, 0xDE, 0xCE, 0xC6, 0xC6, 0x00}, // N, 18
	{0x7C, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0x7C, 0x00}, // O, 19
	{0xFC, 0xC6, 0xC6, 0xFC, 0xC0, 0xC0, 0xC0, 0x00}, // P, 1A
	{0x7C, 0xC6, 0xC6, 0xC6, 0xD6, 0xCE, 0x7E, 0x00}, // Q, 1B
	{0xFC, 0xC6, 0xC6, 0xFC, 0xCC, 0xC6, 0xC6, 0x00}, // R, 1C
	{0x7C, 0xC6, 0xC0, 0x7C, 0x06, 0xC6, 0x7C, 0x00}, // S, 1D
	{0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, // T, 1E
	{0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0x7C, 0x00}, // U, 1F
	{0xC6, 0xC6, 0xC6, 0xC6, 0x6C, 0x38, 0x10, 0x00}, // V, 20
	{0xC6, 0xC6, 0xC6, 0xD6, 0xD6, 0xFE, 0x6C, 0x00}, // W, 21
	{0xC6, 0x6C, 0x38, 0x10, 0x38, 0x6C, 0xC6, 0x00}, // X, 22
	{0xCC, 0xCC, 0x78, 0x30, 0x30, 0x30, 0x30, 0x00}, // Y, 23
	{0xFC, 0x8C, 0x18, 0x30, 0x60, 0xC4, 0xFC, 0x00}, // Z, 24
	{0x00, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x00, 0x00}, // -, 25
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x00}, // _, 26
	{0x70, 0xDC, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00}, // ~, 27
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00}, // ., 28
	{0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00}, // /, 29
	{0x0C, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0C, 0x00}, // <, 2A
	{0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60, 0x00}, // >, 2B
	{0x38, 0x20, 0x20, 0x20, 0x20, 0x20, 0x38, 0x00}, // [, 2C
	{0x38, 0x08, 0x08, 0x08, 0x08, 0x08, 0x38, 0x00}, // ], 2D
	{0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00}, // :, 2E
	{0x18, 0x24, 0x18, 0x3A, 0x44, 0x46, 0x3A, 0x00}  // &, 2F
	// Arrow, 30
	// #, 31  (, 3A  {, 43
	// =, 32  ), 3B  }, 44
	// ", 33  @, 3C  Up,45
	// \, 34  ', 3D  Dn,46
	// *, 35  !, 3E  Lt,47
	// ?, 36  $, 3F  Rt,48
	// %, 37  ;, 40  Bk,49
	// +, 38  `, 41  .5,4A
	// ,, 39  ^, 42
};

u4 SwapMouseButtons(u4 const buttons) {
	return (buttons & ~0x3) | (buttons << 1 & 0x2) | (buttons >> 1 & 0x1);
}

void processmouse1(void) {
	s2 dx;
	s2 dy;
	if (MouseCount > 1) {
		mouse = 0;
		MultiMouseProcess();
		u4 buttons = MouseButtons[0];
		if (mouse1lh == 1) {
			buttons = SwapMouseButtons(buttons);
		}
		mousebuttons = buttons;
		dx = MouseMoveX[0];
		dy = MouseMoveY[0];
	} else {
		u4 buttons = Get_MouseData() & 0xFFFF;
		if (mouse1lh == 1) {
			buttons = SwapMouseButtons(buttons);
		}
		mousebuttons = buttons;
		u4 const eax = Get_MousePositionDisplacement();
		dy = eax >> 16;
		dx = eax;
	}
	if (dx != 0) {
		mousexdir = dx < 0 ? dx = -dx, 1 : 0;
	}
	mousexpos = dx;
	if (dy != 0) {
		mouseydir = dy < 0 ? dy = -dy, 1 : 0;
	}
	mouseypos = dy;
}

void processmouse2(void) {
	static u1 ssautoswb;

	u4 buttons;
	if (MouseCount > 1) {
		mouse = 1;
		MultiMouseProcess();
		buttons = MouseButtons[1];
	} else {
		buttons = Get_MouseData() & 0xFFFF;
	}
	if (mouse2lh == 1) {
		buttons = SwapMouseButtons(buttons);
	}
	mousebuttons = buttons;

	if (device2 == 2) {
		if (pressed[SSAutoFire] == 0) {
			ssautoswb = 0;
		} else if (ssautoswb != 1) {
			ssautosw ^= 0x20;
			ssautoswb = 1;
			Msgptr = ssautosw != 0 ? "AUTOFIRE ENABLED." : "AUTOFIRE DISABLED.";
			MessageOn = MsgCount;
		}
	}

	s2 dx;
	s2 dy;
	if (MouseCount > 1) {
		dx = MouseMoveX[1];
		dy = MouseMoveY[1];
	} else {
		u4 const eax = Get_MousePositionDisplacement();
		dy = eax >> 16;
		dx = eax;
	}

	if (2 <= device2 && device2 <= 4) {
		s4 x = mousexloc + dx;
		if (x < 0) {
			x = 0;
		}
		if (x > 255) {
			x = 255;
		}
		mousexloc = x;
		latchx = x + 40;
		extlatch = 0x40;
	}
	if (dx != 0) {
		mousexdir = dx < 0 ? dx = -dx, 1 : 0;
	}
	mousexpos = dx;

	if (2 <= device2 && device2 <= 4) {
		s4 y = mouseyloc + dy;
		if (y < 0) {
			y = 0;
		}
		if (y > 223) {
			y = 223;
		}
		mouseyloc = y;
		latchy = y;
	}
	if (dy != 0) {
		mouseydir = dy < 0 ? dy = -dy, 1 : 0;
	}
	mouseypos = dy;
}

static void OutText16bnt(u2 *dst, u1 const *src, u4 const edx) {
	// output text in src to dst
	u1 cl = 9;
	do {
		u1 ch = 9;
		u4 eax = cl != 1 ? src[0] << 1 : 0;
		u4 ebx = cl != 9 ? src[-1] : 0;
		do {
			if (eax & 0x100) {
				dst[0] = 0xFFFF;
				dst[75036 * 2] = 0xFFFF;
			} else if (ebx & 0x100) {
				dst[0] = (dst[0] & edx) >> 1;
				dst[75036 * 2] = (dst[75036 * 2] & edx) >> 1;
			}
			eax <<= 1;
			ebx <<= 1;
			++dst;
		} while (--ch != 0);
		dst += 279;
		++src;
	} while (--cl != 0);
}

static void OutputText16b(u2 *dst, u1 const *src, u4 const edx) {
	if (ForceNonTransp == 1 || GUIEnableTransp == 0) {
		OutText16bnt(dst, src, edx);
	} else {
		u4 const mask = edx;
		u4 const colour = edx >> 16;
		// output text in src to dst
		u4 y = 9;
		do {
			u4 eax = y != 1 ? src[0] << 1 : 0;
			u4 ebx = y != 9 ? src[-1] : 0;
			u4 x = 9;
			do {
				if (eax & 0x100) {
					dst[0] = ((dst[0] & mask) >> 1) + colour;
					dst[75036 * 2] = ((dst[75036 * 2] & mask) >> 1) + colour;
				} else if (ebx & 0x100) {
					dst[0] = (dst[0] & mask) >> 1;
					dst[75036 * 2] = (dst[75036 * 2] & mask) >> 1;
				}
				eax <<= 1;
				ebx <<= 1;
				++dst;
			} while (--x != 0);
			dst += 279;
			++src;
		} while (--y != 0);
	}
}

void outputhex16(u2 *const buf, u1 const val) {
	u4 const edx = (u2)vesa2_clbitng >> 1 << 16 | (u2)vesa2_clbitng;
	OutputText16b(buf, FontData[(val >> 4) + 1], edx);
	OutputText16b(buf + 8, FontData[(val & 0x0F) + 1], edx);
}

void outputchar16b(u2 *const buf, u1 const glyph) {
	u4 const edx = (u2)vesa2_clbitng >> 1 << 16 | (u2)vesa2_clbitng;
	OutputText16b(buf, FontData[glyph], edx);
}

static void outputchar16b5x5(u2 *buf, u1 const glyph) {
	u2 const c = textcolor16b;
	u1 const *src = GUIFontData[glyph];
	u4 y = 5;
	do {
		u1 ah = *src++;
		u1 x = 5;
		do {
			if (ah & 0x80) {
				buf[0] = c;
				buf[75036 * 2] = c;
			}
			ah <<= 1;
			++buf;
		} while (--x != 0);
		buf += 283;
	} while (--y != 0);
}

void OutputGraphicString(u1 *buf, char const *text) {
	u2 *const buf16 = (u2 *)vidbuffer + (buf - vidbuffer);
	switch (textcolor) {
	case 128:
		textcolor16b = 0xFFFF;
		break;
	case 129:
		textcolor16b = 0x0000;
		break;
	case 130:
		textcolor16b = (20 << vesa2_rpos) + (20 << vesa2_gpos) + (20 << vesa2_bpos);
		break;
	// Color #131, Red
	case 131:
		textcolor16b = (22 << vesa2_rpos) + (5 << vesa2_gpos) + (5 << vesa2_bpos);
		break;
	}
	OutputGraphicString16b(buf16, text);
}

void OutputGraphicString16b(u2 *buf, char const *text) {
	for (;; buf += 8) {
		u1 const al = *text++;
		if (al == '\0') {
			break;
		}
		outputchar16b(buf, ASCII2Font[al]);
	}
}

void OutputGraphicString16b5x5(u2 *buf, char const *text) {
	for (;; buf += 6) {
		u1 const al = *text++;
		if (al == '\0') {
			break;
		}
		outputchar16b5x5(buf, ASCII2Font[al]);
	}
}

void drawhline16b(u2 *buf, u4 n, u2 const colour) {
	do {
		*buf++ = colour;
	} while (--n != 0);
}

void drawvline16b(u2 *buf, u4 n, u2 const colour) {
	do {
		*buf = colour;
		buf += 288;
	} while (--n != 0);
}

static void DetermineNewest(void) {
	newestfiledate = 0;
	u4 const n = 10;
	u4 const cur = current_zst;
	u4 const start = cur / n * n;
	u4 i = start;
	u4 end = start + n;
	do {
		current_zst = i;
		zst_determine_newest();
	} while (++i != end);
	current_zst = cur;
}

static void GetPicture(void) {
	LoadPicture();

	if (newengen != 0 && nggposng == 5) { // convert to 1:5:5:5
		u2 *buf = PrevPicture;
		u4 n = 64 * 56;
		do {
			u2 const px = *buf;
			*buf++ = (px & 0x001F) | (px & 0xFFC0) >> 1;
		} while (--n != 0);
	}

	{ // draw border
		u2 *dst = (u2 *)vidbuffer + 75 + 9 * 288;
		u4 y = 58;
		do {
			u4 x = 66;
			do {
				*dst++ = 0xFFFF;
			} while (--x != 0); // XXX memset?
			dst += -66 + 288;
		} while (--y != 0);
	}

	{ // draw picture
		u2 *dst = (u2 *)vidbuffer + 76 + 10 * 288;
		u2 const *src = PrevPicture;
		u4 y = 56;
		do {
			u4 x = 64;
			do {
				*dst++ = *src++;
			} while (--x != 0); // XXX memcpy?
			dst += -64 + 288;
		} while (--y != 0);
	}
}

// draws a 10x10 filled box according to position x
static void drawfillboxsc16b(u4 const x) {
	if (zst_exists() != 1) {
		return;
	}

	u2 *buf = (u2 *)vidbuffer + 76 + 104 * 288 + 11 * x;
	u2 const colour = current_zst == newest_zst ? allgrnb : allgrn;
	u4 n = 10;
	do {
		drawhline16b(buf, 10, colour);
		buf += 288;
	} while (--n != 0);
}

// draws a box according to position x and color colour
static void drawbox16b(u1 const x, u2 const colour) {
	u2 *const buf = (u2 *)vidbuffer + 75 + 103 * 288 + 11 * x;
	drawhline16b(buf, 12, colour);
	drawvline16b(buf, 12, colour);
	drawvline16b(buf + 11, 12, colour);
	drawhline16b(buf + 11 * 288, 12, colour);
}

static u4 testpressed(u4 *ebx) {
	u4 eax = current_zst;
	u4 ten = eax / 10 * 10;
	u4 one = eax % 10;
	u4 res = 1;

// if numlock on, let's try this first
#define IFKEY(a, b) if ((numlockptr != 0 && pressed[(a)] & 1) || pressed[(b)] & 1)

	IFKEY(0x51, 4)
	one = 3;
	IFKEY(0x4B, 5)
	one = 4;
	IFKEY(0x4C, 6)
	one = 5;
	IFKEY(0x4D, 7)
	one = 6;
	IFKEY(0x47, 8)
	one = 7;
	IFKEY(0x48, 9)
	one = 8;
	IFKEY(0x49, 10)
	one = 9;
	IFKEY(0x52, 11)
	one = 0;

	if (numlockptr != 1) { // if numlock on, disregard numpad
		if (pressed[75] & 1) {
			one = one == 0 ? one + 9 : one - 1;
			pressed[75] = 2;
		}
		if (pressed[77] & 1) {
			one = one == 9 ? one - 9 : one + 1;
			pressed[77] = 2;
		}
		if (pressed[72] & 1) {
			ten = ten == 0 ? ten + 90 : ten - 10;
			pressed[72] = 2;
			goto out;
		}
		if (pressed[80] & 1) {
			ten = ten == 90 ? ten - 90 : ten + 10;
			pressed[80] = 2;
			goto out;
		}
	}

	if (pressed[92] & 1) {
		one = one == 0 ? one + 9 : one - 1;
		pressed[92] = 2;
	}
	if (pressed[94] & 1) {
		one = one == 9 ? one - 9 : one + 1;
		pressed[94] = 2;
	}
	if (pressed[90] & 1) {
		ten = ten == 0 ? ten + 90 : ten - 10;
		pressed[90] = 2;
		goto out;
	}
	if (pressed[96] & 1) {
		ten = ten == 90 ? ten - 90 : ten + 10;
		pressed[96] = 2;
		goto out;
	}
	res = 0;
out:
	current_zst = ten + one;
	*ebx = one;
	return res;
}

// Allows user to select save state slot
static void saveselect(void) {
	static char const stringa[] = "PLEASE SELECT";
	static char const stringb[] = "SAVE STATE SLOT";
	static char const stringb2[] = "SLOT LEVEL:";
	static char const stringc[] = "USE CURSOR KEYS";
	static char const stringd[] = "TO MOVE AND";
	static char const stringe[] = "ENTER TO SELECT";
	static char slotlevelnum[] = "-";

	f3menuen = 1;
	ForceNonTransp = 1;
	if (MessageOn != 0) {
		MessageOn = 1;
	}
	u1 const al = newengen;
	newengen = 0;
	copyvid();
	newengen = al;
	StopSound();
	if (soundon != 0) {
		csounddisable = 1;
	}
	{
		// Start 16-bit stuff here
		if (newengen != 0) {
			GUIOn = 1;
		}
		{ // draw shadow behind box
			u2 *esi = (u2 *)vidbuffer + 80 + 90 * 288;
			u1 al = 70;
			do {
				u4 ecx = 150;
				do {
					*esi = (*esi & vesa2_clbit) >> 1;
					++esi;
				} while (--ecx != 0);
				esi += 288 - 150;
			} while (--al != 0);
		}

	updatescreen16b:
		allgrn = 0x18 << vesa2_rpos;
		allgrnb = 25 << vesa2_rpos | 12 << vesa2_gpos;
		u2 const allred = 0x1F << vesa2_rpos;
		u2 dx = 0x12 << vesa2_bpos;
		u2 const bx = 0x01 << vesa2_gpos | 0x01 << vesa2_rpos;

		{ // draw a small blue box with a white border
			u2 *esi = (u2 *)vidbuffer + 70 + 70 * 288;
			u1 al = 80;
			u1 ah = 5;
			do {
				u4 ecx = 150;
				do {
					*esi++ = dx;
				} while (--ecx != 0);
				esi += 288 - 150;
				if (--ah == 0) {
					dx += bx;
					ah = 5;
				}
			} while (--al != 0);
		}

		{ // draw filled boxes for existing files
			DetermineNewest();
			u4 const eax = current_zst;
			slotlevelnum[0] = '0' + eax / 10;
			for (u4 i = 0; i != 10; ++i) {
				current_zst = current_zst / 10 * 10 + i;
				drawfillboxsc16b(i);
			}
			current_zst = eax;
		}

		OutputGraphicString16b((u2 *)vidbuffer + 75 + 73 * 288, stringa);
		OutputGraphicString16b((u2 *)vidbuffer + 75 + 83 * 288, stringb);
		OutputGraphicString16b((u2 *)vidbuffer + 75 + 93 * 288, stringb2);
		OutputGraphicString16b((u2 *)vidbuffer + 171 + 93 * 288, slotlevelnum);
		OutputGraphicString16b((u2 *)vidbuffer + 75 + 118 * 288, stringc);
		OutputGraphicString16b((u2 *)vidbuffer + 75 + 128 * 288, stringd);
		OutputGraphicString16b((u2 *)vidbuffer + 75 + 138 * 288, stringe);
		{
			u2 const ax = 0xFFFF;
			drawhline16b((u2 *)vidbuffer + 70 + 70 * 288, 150, ax);
			drawvline16b((u2 *)vidbuffer + 70 + 70 * 288, 80, ax);
			drawhline16b((u2 *)vidbuffer + 70 + 149 * 288, 150, ax);
			drawvline16b((u2 *)vidbuffer + 219 + 70 * 288, 80, ax);
			drawhline16b((u2 *)vidbuffer + 75 + 103 * 288, 111, ax);
			drawhline16b((u2 *)vidbuffer + 75 + 114 * 288, 111, ax);
			u2 *esi = (u2 *)vidbuffer + 75 + 104 * 288;
			u2 bl = 11;
			do {
				drawvline16b(esi, 10, ax);
				esi += 11;
			} while (--bl != 0);
		}
		{
			u2 *esi = (u2 *)vidbuffer + 78 + 106 * 288;
			u4 al = 1;
			outputchar16b(esi, al);
			u4 bl = 9;
			do {
				esi += 11;
				outputchar16b(esi, ++al);
			} while (--bl != 0);
		}

		curblank = 0;
		{
			u1 const al = newengen;
			newengen = 0;
			copyvid();
			newengen = al;
		}
		// wait until esc/enter is pressed

		u4 PrevPictureVal = 0xFF;
		u4 ebx = 0;
		for (;;) {
			if (PrevPictureVal != current_zst) {
				PrevPictureVal = current_zst;
				GetPicture();
			}

			drawbox16b(ebx, 0xFFFF);
			delay(2500);
			if (testpressed(&ebx)) {
				goto updatescreen16b;
			}
			if (pressed[1] & 1) {
				goto esc16b;
			}
			if (pressed[28] & 1) {
				goto enter16b;
			}
			delay(2500);
			if (testpressed(&ebx)) {
				goto updatescreen16b;
			}
			if (pressed[1] & 1) {
				goto esc16b;
			}
			if (pressed[28] & 1) {
				goto enter16b;
			}
			{
				u1 const al = newengen;
				newengen = 0;
				copyvid();
				newengen = al;
			}
			delay(2500);
			if (testpressed(&ebx)) {
				goto updatescreen16b;
			}
			if (pressed[1] & 1) {
				goto esc16b;
			}
			if (pressed[28] & 1) {
				goto enter16b;
			}
			delay(2500);
			if (testpressed(&ebx)) {
				goto updatescreen16b;
			}
			if (pressed[1] & 1) {
				goto esc16b;
			}
			if (pressed[28] & 1) {
				goto enter16b;
			}
			drawbox16b(ebx, allred);
			{
				u1 const al = newengen;
				newengen = 0;
				copyvid();
				newengen = al;
			}
		}
	enter16b:
		pressed[28] = 2;
	esc16b:;
		u1 *eax = pressed;
		u4 ecx = 256;
		do {
			if (*eax == 1) {
				*eax = 2;
			}
			++eax;
		} while (--ecx != 0);
		t1cc = 0;
		csounddisable = 0;
		StartSound();
		f3menuen = 0;
		ForceNonTransp = 0;
		GUIOn = 0;
	}
}

void showvideo(void) {
	if (++ccud != cacheud) {
		ccud = 0;
	}
	copyvid();
	if (pressed[KeyStateSelct] & 1) {
		saveselect();
	}
}

static void showfps(void) {
	u4 limit = romispal != 0 ? 50 : 60;
	++curfps;
	if (nextframe >= limit) {
		lastfps = curfps;
		curfps = 0;
		nextframe -= limit;
	}
	if (SloMo != 0) {
		limit /= SloMo + 1;
	}

	u2 *buf = (u2 *)vidbuffer + 208 * 288 + 48;
	u4 fps = lastfps;
	do {
		buf -= 8;
		outputchar16b(buf, ASCII2Font['0' + (fps % 10)]);
		fps /= 10;
	} while (fps != 0);

	outputchar16b((u2 *)vidbuffer + 208 * 288 + 48, 41); // '/'
	outputhex16((u2 *)vidbuffer + 208 * 288 + 56, limit / 10 << 4 | limit % 10);
}

static void ClockOutput(void) {
	u2 *buf = (u2 *)vidbuffer + 215 * 288 + 32 + 192;
	if (ForceNonTransp == 1 || ClockBox == 1) {
		u4 y = 7;
		do {
			memset(buf - 1, 0, sizeof(*buf) * 49);
			memset(buf - 1 + 75036 * 2, 0, sizeof(*buf) * 49);
			buf += 288;
		} while (--y != 0);
	}

	u4 t = GetTimeInSeconds();
	u4 const s = t % 60;
	t /= 60;
	u4 const m = t % 60;
	u4 h = t / 60;
	if (TwelveHourClock == 1) { // check to see if it's 12 PM
		if (h > 12) {
			h -= 12;
		}
		if (h == 0) {
			h += 12;
		}
	}

	char hrbuffer[9];
	sprintf(hrbuffer, "%02d:%02d:%02d", h, m, s);
	OutputGraphicString16b5x5((u2 *)vidbuffer + 216 * 288 + 32 + 192, hrbuffer);
}

static void vidpaste(void) {
	if (FPSOn != 0 && curblank == 0) {
		showfps();
	}
	if (TimerEnable != 0 && ShowTimer != 0) {
		ClockOutput();
	}

	if (device2 == 2) {
		static u1 const SScopeCursor[] = {
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			1,
			1,
			1,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			1,
			0,
			0,
			1,
			0,
			0,
			1,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			1,
			1,
			1,
			1,
			1,
			1,
			1,
			1,
			1,
			1,
			1,
			1,
			1,
			1,
			1,
			1,
			1,
			1,
			1,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			1,
			0,
			0,
			1,
			0,
			0,
			1,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			1,
			1,
			1,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			1,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0};

		u1 const *src = SScopeCursor;
		u4 pos = mouseyloc * 288 + mousexloc + 6;
		u2 const SSRedCo = 31 << vesa2_rpos;
		u2 *const dst = (u2 *)vidbuffer;
		u4 y = 20;
		do {
			u4 x = 20;
			do {
				if (*src++ == 0) {
					continue;
				}
				if (pos < 288 * 10) {
					continue;
				}
				dst[pos - 288 * 10] = SSRedCo;
			} while (++pos, --x != 0);
			pos -= 288 - 20;
		} while (--y != 0);
	}

	DrawScreen();
}

void copyvid(void) {
	if (MessageOn != 0) {
		char const *const msg = Msgptr;
		u2 *const buf = (u2 *)vidbuffer + 192 * 288 + 32;
		if (msg == CSStatus) {
			OutputGraphicString16b5x5(buf, msg);
			OutputGraphicString16b5x5((u2 *)vidbuffer + 200 * 288 + 32, CSStatus2);
			OutputGraphicString16b5x5((u2 *)vidbuffer + 208 * 288 + 32, CSStatus3);
			OutputGraphicString16b5x5((u2 *)vidbuffer + 216 * 288 + 32, CSStatus4);
		} else if (SmallMsgText == 1) {
			OutputGraphicString16b5x5(buf, msg);
		} else {
			OutputGraphicString16b(buf, msg);
		}
		--MessageOn;
	}

	if (MovieProcessing != 0 && MovieDisplayFrame != 0) {
		GetMovieFrameStr();
		char const *const msg = MovieFrameStr;
		OutputGraphicString16b5x5((u2 *)vidbuffer + 216 * 288 + 32, msg);
	}
	vidpaste();
}
