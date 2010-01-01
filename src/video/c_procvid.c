#include <string.h>

#include "../asm_call.h"
#include "../c_intrf.h"
#include "../cfg.h"
#include "../cpu/execute.h"
#include "../cpu/regs.h"
#include "../endmem.h"
#include "../gui/gui.h"
#include "../init.h"
#include "../input.h"
#include "../macros.h"
#include "../ui.h"
#include "../vcache.h"
#include "../zstate.h"
#include "c_procvid.h"
#include "newgfx16.h"
#include "procvid.h"
#include "procvidc.h"

#ifdef __UNIXSDL__
#	include "../linux/sdllink.h"
#endif

static u2 allgrnb;
static u2 allgrn;

static u1 textcolor    = 128;
static u2 textcolor16b = 0xFFFF;

static u2 cgramback[256];


u4 SwapMouseButtons(u4 const buttons)
{
	return (buttons & ~0x3) | (buttons << 1 & 0x2) | (buttons >> 1 & 0x1);
}


void processmouse1(void)
{
	s2 dx;
	s2 dy;
#ifndef __MSDOS__
	if (MouseCount > 1)
	{
		mouse = 0;
		MultiMouseProcess();
		u4 buttons = MouseButtons[0];
		if (mouse1lh == 1) buttons = SwapMouseButtons(buttons);
		mousebuttons = buttons;
		dx = MouseMoveX[0];
		dy = MouseMoveY[0];
	}
	else
#endif
	{
		u4 buttons = Get_MouseData() & 0xFFFF;
		if (mouse1lh == 1) buttons = SwapMouseButtons(buttons);
		mousebuttons = buttons;
		u4 const eax = Get_MousePositionDisplacement();
		dy = eax >> 16;
		dx = eax;
	}
	if (dx != 0) mousexdir = dx < 0 ? dx = -dx, 1 : 0;
	mousexpos = dx;
	if (dy != 0) mouseydir = dy < 0 ? dy = -dy, 1 : 0;
	mouseypos = dy;
}


void processmouse2(void)
{
	static u1 ssautoswb;

	u4 buttons;
#ifndef __MSDOS__
	if (MouseCount > 1)
	{
		mouse = 1;
		MultiMouseProcess();
		buttons = MouseButtons[1];
	}
	else
#endif
	{
		buttons = Get_MouseData() & 0xFFFF;
	}
	if (mouse2lh == 1) buttons = SwapMouseButtons(buttons);
	mousebuttons = buttons;

	if (device2 == 2)
	{
		if (pressed[SSAutoFire] == 0)
		{
			ssautoswb = 0;
		}
		else if (ssautoswb != 1)
		{
			ssautosw  ^= 0x20;
			ssautoswb  = 1;
			Msgptr     = ssautosw != 0 ? "AUTOFIRE ENABLED." : "AUTOFIRE DISABLED.";
			MessageOn  = MsgCount;
		}
	}

#ifndef __MSDOS__
	s2 dx;
	s2 dy;
	if (MouseCount > 1)
	{
		dx = MouseMoveX[1];
		dy = MouseMoveY[1];
	}
	else
#endif
	{
		u4 const eax = Get_MousePositionDisplacement();
		dy = eax >> 16;
		dx = eax;
	}

	if (2 <= device2 && device2 <= 4)
	{
		s4 x = mousexloc + dx;
		if (x <   0) x =   0;
		if (x > 255) x = 255;
		mousexloc = x;
		latchx    = x + 40;
		extlatch  = 0x40;
	}
	if (dx != 0) mousexdir = dx < 0 ? dx = -dx, 1 : 0;
	mousexpos = dx;

	if (2 <= device2 && device2 <= 4)
	{
		s4 y = mouseyloc + dy;
		if (y <   0) y =   0;
		if (y > 223) y = 223;
		mouseyloc = y;
		latchy    = y;
	}
	if (dy != 0) mouseydir = dy < 0 ? dy = -dy, 1 : 0;
	mouseypos = dy;
}


#ifdef __MSDOS__
void outputhex(u1* buf, u1 const val)
{
	for (u4 i = 0; i != 2; buf += 8, ++i)
	{
		u4 const  digit = val >> (i * 4) & 0x0F;
		u1 const* src   = FontData[digit + 1];
		u1*       dst   = buf;
		u4        y     = 8;
		do
		{
			u1 ah = *src++;
			u4 x  = 8;
			do
			{
				if (ah & 0x80)
				{
					dst[0]   = 128;
					dst[289] = 192;
				}
				ah <<= 1;
				++dst;
			}
			while (--x != 0);
			dst += 280;
		}
		while (--y != 0);
	}
}
#endif


static void OutText16bnt(u2* dst, u1 const* src, u4 const edx)
{
	// output text in src to dst
	u1 cl = 9;
	do
	{
		u1 ch  = 9;
		u4 eax = cl != 1 ? src[ 0] << 1 : 0;
		u4 ebx = cl != 9 ? src[-1]      : 0;
		do
		{
			if (eax & 0x100)
			{
				dst[0]         = 0xFFFF;
				dst[75036 * 2] = 0xFFFF;
			}
			else if (ebx & 0x100)
			{
				dst[0]        = (dst[0]        & edx) >> 1;
				dst[75036 *2] = (dst[75036 *2] & edx) >> 1;
			}
			eax <<= 1;
			ebx <<= 1;
			++dst;
		}
		while (--ch != 0);
		dst += 279;
		++src;
	}
	while (--cl != 0);
}


static void OutputText16b(u2* dst, u1 const* src, u4 const edx)
{
	if (ForceNonTransp == 1 || GUIEnableTransp == 0)
	{
		OutText16bnt(dst, src, edx);
	}
	else
	{
		u4 const mask   = edx;
		u4 const colour = edx >> 16;
		// output text in src to dst
		u4       y      = 9;
		do
		{
			u4 eax = y != 1 ? src[ 0] << 1 : 0;
			u4 ebx = y != 9 ? src[-1]      : 0;
			u4 x   = 9;
			do
			{
				if (eax & 0x100)
				{
					dst[0]         = ((dst[0]         & mask) >> 1) + colour;
					dst[75036 * 2] = ((dst[75036 * 2] & mask) >> 1) + colour;
				}
				else if (ebx & 0x100)
				{
					dst[0]         = (dst[0]         & mask) >> 1;
					dst[75036 * 2] = (dst[75036 * 2] & mask) >> 1;
				}
				eax <<= 1;
				ebx <<= 1;
				++dst;
			}
			while (--x != 0);
			dst += 279;
			++src;
		}
		while (--y != 0);
	}
}


void outputhex16(u2* const buf, u1 const val)
{
	u4 const edx = (u2)vesa2_clbitng >> 1 << 16 | (u2)vesa2_clbitng;
	OutputText16b(buf,     FontData[(val >> 4)   + 1], edx);
	OutputText16b(buf + 8, FontData[(val & 0x0F) + 1], edx);
}


#ifdef __MSDOS__
void outputchar(u1* buf, u1 const glyph)
{
	u1 const* src = FontData[glyph];
	u4        y   = 8;
	do
	{
		u1 ah = *src++;
		u4 x  = 8;
		do
		{
			if (ah & 0x80)
			{
				buf[0]   = textcolor;
				buf[289] = 192;
			}
			ah <<= 1;
			++buf;
		}
		while (--x != 0);
		buf += 280;
	}
	while (--y != 0);
}
#endif


void outputchar16b(u2* const buf, u1 const glyph)
{
	u4 const edx = (u2)vesa2_clbitng >> 1 << 16 | (u2)vesa2_clbitng;
	OutputText16b(buf, FontData[glyph], edx);
}


#ifdef __MSDOS__
void outputchar5x5(u1* buf, u1 const glyph)
{
	u1 const* src = GUIFontData[glyph];
	u4        y   = 5;
	do
	{
		u1 ah = *src++;
		u4 x  = 5;
		do
		{
			if (ah & 0x80) *buf = textcolor;
			ah <<= 1;
			++buf;
		}
		while (--x != 0);
		buf += 283;
	}
	while (--y != 0);
}
#endif


void outputchar16b5x5(u2* buf, u1 const glyph)
{
	u2 const  c   = textcolor16b;
	u1 const* src = GUIFontData[glyph];
	u4        y   = 5;
	do
	{
		u1 ah = *src++;
		u1 x  = 5;
		do
		{
			if (ah & 0x80)
			{
				buf[0]         = c;
				buf[75036 * 2] = c;
			}
			ah <<= 1;
			++buf;
		}
		while (--x != 0);
		buf += 283;
	}
	while (--y != 0);
}


void OutputGraphicString(u1* buf, char const* text)
{
#ifdef __MSDOS__
	if (cbitmode != 1)
	{
		for (;; buf += 8)
		{
			u1 const al = *text++;
			if (al == '\0') break;
			outputchar(buf, ASCII2Font[al]);
		}
	}
	else
#endif
	{ // XXX probably never reached, callers seem to test cbitmode beforehand
		u2* const buf16 = (u2*)vidbuffer + (buf - vidbuffer);
		switch (textcolor)
		{
			case 128: textcolor16b = 0xFFFF;                                                       break;
			case 129: textcolor16b = 0x0000;                                                       break;
			case 130: textcolor16b = (20 << vesa2_rpos) + (20 << vesa2_gpos) + (20 << vesa2_bpos); break;
			// Color #131, Red
			case 131: textcolor16b = (22 << vesa2_rpos) + ( 5 << vesa2_gpos) + ( 5 << vesa2_bpos); break;
		}
		OutputGraphicString16b(buf16, text);
	}
}


void OutputGraphicString16b(u2* buf, char const* text)
{
	for (;; buf += 8)
	{
		u1 const al = *text++;
		if (al == '\0') break;
		outputchar16b(buf, ASCII2Font[al]);
	}
}


#ifdef __MSDOS__
void OutputGraphicString5x5(u1* buf, char const* text)
{
	if (cbitmode != 1)
	{
		for (;; buf += 6)
		{
			u1 const al = *text++;
			if (al == '\0') break;
			outputchar5x5(buf, ASCII2Font[al]);
		}
	}
	else
	{ // XXX probably never reached, callers seem to test cbitmode beforehand
		u2* const buf16 = (u2*)vidbuffer + (buf - vidbuffer);
		switch (textcolor)
		{
			case 128: textcolor16b = 0xFFFF;                                                       break;
			case 129: textcolor16b = 0x0000;                                                       break;
			case 130: textcolor16b = (20 << vesa2_rpos) + (20 << vesa2_gpos) + (20 << vesa2_bpos); break;
			// Color #131, Red
			case 131: textcolor16b = (22 << vesa2_rpos) + ( 5 << vesa2_gpos) + ( 5 << vesa2_bpos); break;
		}
		OutputGraphicString16b5x5(buf16, text);
	}
}
#endif


void OutputGraphicString16b5x5(u2* buf, char const* text)
{
	for (;; buf += 6)
	{
		u1 const al = *text++;
		if (al == '\0') break;
		outputchar16b5x5(buf, ASCII2Font[al]);
	}
}


#ifdef __MSDOS__
void drawhline(u1* buf, u4 n, u1 const colour)
{
	do *buf++ = colour; while (--n != 0);
}
#endif


void drawhline16b(u2* buf, u4 n, u2 const colour)
{
	do *buf++ = colour; while (--n != 0);
}


#ifdef __MSDOS__
void drawvline(u1* buf, u4 n, u1 const colour)
{
	do { *buf = colour; buf += 288; } while (--n != 0);
}
#endif


void drawvline16b(u2* buf, u4 n, u2 const colour)
{
	do { *buf = colour; buf += 288; } while (--n != 0);
}


static void DetermineNewest(void)
{
	newestfiledate = 0;
	u4 const n     = 10;
	u4 const cur   = current_zst;
	u4 const start = cur / n * n;
	u4       i     = start;
	u4       end   = start + n;
	do
	{
		current_zst = i;
		zst_determine_newest();
	}
	while (++i != end);
	current_zst = cur;
}


static void GetPicture(void)
{
	LoadPicture();

	if (newengen != 0 && nggposng == 5)
	{ // convert to 1:5:5:5
		u2* buf = PrevPicture;
		u4  n   = 64 * 56;
		do
		{
			u2 const px = *buf;
			*buf++ = (px & 0x001F) | (px & 0xFFC0) >> 1;
		}
		while (--n != 0);
	}

	{ // draw border
		u2* dst = (u2*)vidbuffer + 75 + 9 * 288;
		u4  y   = 58;
		do
		{
			u4 x = 66;
			do *dst++ = 0xFFFF; while (--x != 0); // XXX memset?
			dst += -66 + 288;
		}
		while (--y != 0);
	}

	{ // draw picture
		u2*       dst = (u2*)vidbuffer + 76 + 10 * 288;
		u2 const* src = PrevPicture;
		u4        y   = 56;
		do
		{
			u4 x = 64;
			do *dst++ = *src++; while (--x != 0); // XXX memcpy?
			dst += -64 + 288;
		}
		while (--y != 0);
	}
}


#ifdef __MSDOS__
// draws a 10x10 filled box according to position x
static void drawfillboxsc(u4 const x)
{
	if (zst_exists() != 1) return;

	u1*      buf    = vidbuffer + 76 + 104 * 288 + 11 * x;
	u1 const colour = current_zst == newest_zst ? 208 : 176;
	u4       n      = 10;
	do
	{
		drawhline(buf, 10, colour);
		buf += 288;
	}
	while (--n != 0);
}
#endif


// draws a 10x10 filled box according to position x
static void drawfillboxsc16b(u4 const x)
{
	if (zst_exists() != 1) return;

	u2*      buf    = (u2*)vidbuffer + 76 + 104 * 288 + 11 * x;
	u2 const colour = current_zst == newest_zst ? allgrnb : allgrn;
	u4       n      = 10;
	do
	{
		drawhline16b(buf, 10, colour);
		buf += 288;
	}
	while (--n != 0);
}


#ifdef __MSDOS__
// draws a box according to position x and color colour
static void drawbox(u1 const x, u1 const colour)
{
	u1* const buf = vidbuffer + 75 + 103 * 288 + 11 * x;
	drawhline(buf,            12, colour);
	drawvline(buf,            12, colour);
	drawvline(buf + 11,       12, colour);
	drawhline(buf + 11 * 288, 12, colour);
}
#endif


// draws a box according to position x and color colour
static void drawbox16b(u1 const x, u2 const colour)
{
	u2* const buf = (u2*)vidbuffer + 75 + 103 * 288 + 11 * x;
	drawhline16b(buf,            12, colour);
	drawvline16b(buf,            12, colour);
	drawvline16b(buf + 11,       12, colour);
	drawhline16b(buf + 11 * 288, 12, colour);
}


static u4 testpressed(u4* ebx)
{
	u4 eax = current_zst;
	u4 ten = eax / 10 * 10;
	u4 one = eax % 10;
	u4 res = 1;

#ifdef __UNIXSDL__
	// if numlock on, let's try this first
#	define IFKEY(a, b) if ((numlockptr != 0 && pressed[(a)] & 1) || pressed[(b)] & 1)
#else
#	define IFKEY(a, b) if (pressed[(b)] & 1)
#endif

	IFKEY(0x51,  4) one = 3;
	IFKEY(0x4B,  5) one = 4;
	IFKEY(0x4C,  6) one = 5;
	IFKEY(0x4D,  7) one = 6;
	IFKEY(0x47,  8) one = 7;
	IFKEY(0x48,  9) one = 8;
	IFKEY(0x49, 10) one = 9;
	IFKEY(0x52, 11) one = 0;

#ifdef __UNIXSDL__
	if (numlockptr != 1) // if numlock on, disregard numpad
#endif
	{
		if (pressed[75] & 1)
		{
			one = one == 0 ? one + 9 : one - 1;
			pressed[75] = 2;
		}
		if (pressed[77] & 1)
		{
			one = one == 9 ? one - 9 : one + 1;
			pressed[77] = 2;
		}
		if (pressed[72] & 1)
		{
			ten = ten == 0 ? ten + 90 : ten - 10;
			pressed[72] = 2;
			goto out;
		}
		if (pressed[80] & 1)
		{
			ten = ten == 90 ? ten - 90 : ten + 10;
			pressed[80] = 2;
			goto out;
		}
	}

#ifndef __MSDOS__
#ifdef __UNIXSDL__
	if (pressed[92] & 1)
#else
	if (pressed[0xCB] & 1)
#endif
	{
		one = one == 0 ? one + 9 : one - 1;
#ifdef __UNIXSDL__
		pressed[92] = 2;
#else
		pressed[0xCB] = 2;
#endif
	}
#ifdef __UNIXSDL__
	if (pressed[94] & 1)
#else
	if (pressed[0xCD] & 1)
#endif
	{
		one = one == 9 ? one - 9 : one + 1;
#ifdef __UNIXSDL__
		pressed[94] = 2;
#else
		pressed[0xCD] = 2;
#endif
	}
#ifdef __UNIXSDL__
	if (pressed[90] & 1)
#else
	if (pressed[0xC8] & 1)
#endif
	{
		ten = ten == 0 ? ten + 90 : ten - 10;
#ifdef __UNIXSDL__
		pressed[90] = 2;
#else
		pressed[0xC8] = 2;
#endif
		goto out;
	}
#ifdef __UNIXSDL__
	if (pressed[96] & 1)
#else
	if (pressed[0xD0] & 1)
#endif
	{
		ten = ten == 90 ? ten - 90 : ten + 10;
#ifdef __UNIXSDL__
		pressed[96] = 2;
#else
		pressed[0xD0] = 2;
#endif
		goto out;
	}
#endif
	res         = 0;
out:
	current_zst = ten + one;
	*ebx        = one;
	return res;
}


// Allows user to select save state slot
static void saveselect(void)
{
	static char const stringa[]      = "PLEASE SELECT";
	static char const stringb[]      = "SAVE STATE SLOT";
	static char const stringb2[]     = "SLOT LEVEL:";
	static char const stringc[]      = "USE CURSOR KEYS";
	static char const stringd[]      = "TO MOVE AND";
	static char const stringe[]      = "ENTER TO SELECT";
	static char       slotlevelnum[] = "-";

	f3menuen       = 1;
	ForceNonTransp = 1;
	if (MessageOn != 0) MessageOn = 1;
	u1 const al = newengen;
	newengen = 0;
	asm_call(copyvid);
	newengen = al;
	StopSound();
	if (soundon != 0)
	{
		csounddisable = 1;
#ifdef __MSDOS__
		asm_call(SB_blank);
#endif
	}
#ifdef __MSDOS__
	if (cbitmode != 1)
	{
updatescreen8b:
		asm_call(saveselectpal);
		{ // draw a small blue box with a white border
			u1* esi = vidbuffer + 70 + 70 * 288;
			u1  al  = 80;
			do
			{
				u4 ecx = 150;
				do *esi++ = 144; while (--ecx != 0);
				esi += 288 - 150;
			}
			while (--al != 0);
		}

		{ // draw filled boxes for existing files
			DetermineNewest();
			u4 const eax = current_zst;
			slotlevelnum[0] = '0' + eax / 10;
			for (u4 i = 0; i != 10; ++i)
			{
				current_zst = current_zst / 10 * 10 + i;
				drawfillboxsc(i);
			}
			current_zst = eax;
		}

		OutputGraphicString(vidbuffer +  75 +  73 * 288, stringa);
		OutputGraphicString(vidbuffer +  75 +  83 * 288, stringb);
		OutputGraphicString(vidbuffer +  75 +  93 * 288, stringb2);
		OutputGraphicString(vidbuffer + 171 +  93 * 288, slotlevelnum);
		OutputGraphicString(vidbuffer +  75 + 118 * 288, stringc);
		OutputGraphicString(vidbuffer +  75 + 128 * 288, stringd);
		OutputGraphicString(vidbuffer +  75 + 138 * 288, stringe);
		{ u1 const al = 128;
			drawhline(vidbuffer +  70 +  70 * 288, 150, al);
			drawvline(vidbuffer +  70 +  70 * 288,  80, al);
			drawhline(vidbuffer +  70 + 149 * 288, 150, al);
			drawvline(vidbuffer + 219 +  70 * 288,  80, al);
			drawhline(vidbuffer +  75 + 103 * 288, 111, al);
			drawhline(vidbuffer +  75 + 114 * 288, 111, al);
			u1* esi = vidbuffer + 75 + 104 * 288;
			u1 bl = 11;
			do
			{
				drawvline(esi, 10, al);
				esi += 11;
			}
			while (--bl != 0);
		}
		{ u1* esi = vidbuffer + 78 + 106 * 288;
			u4  al  = 1;
			outputchar(esi, al);
			u4  bl  = 9;
			do
			{
				esi += 11;
				outputchar(esi, ++al);
			}
			while (--bl != 0);
		}
		curblank = 0;

		{ u4 ebx = 0;
			drawbox(ebx, 160);
			asm_call(copyvid);
			// wait until esc/enter is pressed
			for (;;)
			{
				drawbox(ebx, 128);
				delay(2500);
				if (testpressed(&ebx)) goto updatescreen8b;
				if (pressed[ 1] & 1)   goto esc;
				if (pressed[28] & 1)   goto enter;
				delay(2500);
				if (testpressed(&ebx)) goto updatescreen8b;
				if (pressed[ 1] & 1)   goto esc;
				if (pressed[28] & 1)   goto enter;
				asm_call(copyvid);
				delay(2500);
				if (testpressed(&ebx)) goto updatescreen8b;
				if (pressed[ 1] & 1)   goto esc;
				if (pressed[28] & 1)   goto enter;
				delay(2500);
				if (testpressed(&ebx)) goto updatescreen8b;
				if (pressed[ 1] & 1)   goto esc;
				if (pressed[28] & 1)   goto enter;
				drawbox(ebx, 160);
				asm_call(copyvid);
			}
enter:;
			pressed[28] = 2;
esc:;
		}

		{ u1* eax = pressed;
			u4  ecx = 256;
			do
			{
				if (*eax == 1) *eax = 2;
				++eax;
			}
			while (--ecx != 0);
		}
		pressed[1] = 0;

		t1cc          = 0;
		csounddisable = 0;
		StartSound();

		dosmakepal();
		f3menuen       = 0;
		ForceNonTransp = 0;
	}
	else
#endif
	{
		// Start 16-bit stuff here
		if (newengen != 0) GUIOn = 1;
		{ // draw shadow behind box
			u2* esi = (u2*)vidbuffer + 80 + 90 * 288;
			u1  al  = 70;
			do
			{
				u4 ecx = 150;
				do
				{
					*esi = (*esi & vesa2_clbit) >> 1;
					++esi;
				}
				while (--ecx != 0);
				esi += 288 - 150;
			}
			while (--al != 0);
		}

updatescreen16b:
		allgrn  = 0x18 << vesa2_rpos;
		allgrnb =   25 << vesa2_rpos | 12 << vesa2_gpos;
		u2 const allred  = 0x1F << vesa2_rpos;
		u2       dx      = 0x12 << vesa2_bpos;
		u2 const bx      = 0x01 << vesa2_gpos | 0x01 << vesa2_rpos;

		{ // draw a small blue box with a white border
			u2* esi = (u2*)vidbuffer + 70 + 70 * 288;
			u1  al  = 80;
			u1  ah  =  5;
			do
			{
				u4 ecx = 150;
				do *esi++ = dx; while (--ecx != 0);
				esi += 288 - 150;
				if (--ah == 0)
				{
					dx += bx;
					ah  = 5;
				}
			}
			while (--al != 0);
		}

		{ // draw filled boxes for existing files
			DetermineNewest();
			u4 const eax = current_zst;
			slotlevelnum[0] = '0' + eax / 10;
			for (u4 i = 0; i != 10; ++i)
			{
				current_zst = current_zst / 10 * 10 + i;
				drawfillboxsc16b(i);
			}
			current_zst = eax;
		}

		OutputGraphicString16b((u2*)vidbuffer +  75 +  73 * 288, stringa);
		OutputGraphicString16b((u2*)vidbuffer +  75 +  83 * 288, stringb);
		OutputGraphicString16b((u2*)vidbuffer +  75 +  93 * 288, stringb2);
		OutputGraphicString16b((u2*)vidbuffer + 171 +  93 * 288, slotlevelnum);
		OutputGraphicString16b((u2*)vidbuffer +  75 + 118 * 288, stringc);
		OutputGraphicString16b((u2*)vidbuffer +  75 + 128 * 288, stringd);
		OutputGraphicString16b((u2*)vidbuffer +  75 + 138 * 288, stringe);
		{ u2 const ax = 0xFFFF;
			drawhline16b((u2*)vidbuffer +  70 +  70 * 288, 150, ax);
			drawvline16b((u2*)vidbuffer +  70 +  70 * 288,  80, ax);
			drawhline16b((u2*)vidbuffer +  70 + 149 * 288, 150, ax);
			drawvline16b((u2*)vidbuffer + 219 +  70 * 288,  80, ax);
			drawhline16b((u2*)vidbuffer +  75 + 103 * 288, 111, ax);
			drawhline16b((u2*)vidbuffer +  75 + 114 * 288, 111, ax);
			u2* esi = (u2*)vidbuffer + 75 + 104 * 288;
			u2   bl  = 11;
			do
			{
				drawvline16b(esi, 10, ax);
				esi += 11;
			}
			while (--bl != 0);
		}
		{ u2* esi = (u2*)vidbuffer + 78 + 106 * 288;
			u4  al  = 1;
			outputchar16b(esi, al);
			u4  bl  = 9;
			do
			{
				esi += 11;
				outputchar16b(esi, ++al);
			}
			while (--bl != 0);
		}

		curblank = 0;
		{ u1 const al = newengen;
			newengen = 0;
			asm_call(copyvid);
			newengen = al;
		}
		// wait until esc/enter is pressed

		u4 PrevPictureVal = 0xFF;
		u4 ebx            = 0;
		for (;;)
		{
			if (PrevPictureVal != current_zst)
			{
				PrevPictureVal = current_zst;
				GetPicture();
			}

			drawbox16b(ebx, 0xFFFF);
			delay(2500);
			if (testpressed(&ebx)) goto updatescreen16b;
			if (pressed[ 1] & 1)   goto esc16b;
			if (pressed[28] & 1)   goto enter16b;
			delay(2500);
			if (testpressed(&ebx)) goto updatescreen16b;
			if (pressed[ 1] & 1)   goto esc16b;
			if (pressed[28] & 1)   goto enter16b;
			{ u1 const al = newengen;
				newengen = 0;
				asm_call(copyvid);
				newengen = al;
			}
			delay(2500);
			if (testpressed(&ebx)) goto updatescreen16b;
			if (pressed[ 1] & 1)   goto esc16b;
			if (pressed[28] & 1)   goto enter16b;
			delay(2500);
			if (testpressed(&ebx)) goto updatescreen16b;
			if (pressed[ 1] & 1)   goto esc16b;
			if (pressed[28] & 1)   goto enter16b;
			drawbox16b(ebx, allred);
			{ u1 const al = newengen;
				newengen = 0;
				asm_call(copyvid);
				newengen = al;
			}
		}
enter16b:
		pressed[28] = 2;
esc16b:;
		u1* eax = pressed;
		u4  ecx = 256;
		do
		{
			if (*eax == 1) *eax = 2;
			++eax;
		}
		while (--ecx != 0);
#ifdef __MSDOS__
		pressed[1] = 0;
#endif
		t1cc           = 0;
		csounddisable  = 0;
		StartSound();
		f3menuen       = 0;
		ForceNonTransp = 0;
		GUIOn          = 0;
		Clear2xSaIBuffer();
	}
}


void showvideo(void)
{
	if (++ccud != cacheud) ccud = 0;
	asm_call(copyvid);
	if (pressed[KeyStateSelct] & 1) saveselect();
}


void doveg(void)
{
	// backup cgram
	memcpy(cgramback, cgram, sizeof(cgramback));
	{ u1 const grey = (coladdr + coladdg + coladdb) / 3 & 0x1F;
		coladdr = grey;
		coladdg = grey;
		coladdb = grey;
	}
	u2* i = cgram;
	do
	{
		u4 const px   = *i;
		u4 const grey = ((px >> 10 & 0x1F) + (px >> 5 & 0x1F) + (px & 0x1F)) / 3;
		*i = grey << 10 | grey << 5 | grey;
	}
	while (++i != endof(cgram));
}


void dovegrest(void)
{
	// backup cgram
	memcpy(cgram, cgramback, sizeof(cgram));
}


#ifdef __MSDOS__
void dosmakepal(void)
{
	if (V8Mode == 1) doveg();

	tempco0 = cgram[0];
	if ((scaddtype & 0xA0) == 0x20)
	{
		u2 const c = cgram[0];
		u2       r = (c       & 0x1F) + coladdr;
		if (r > 0x1F) r = 0x1F;
		u2       g = (c >>  5 & 0x1F) + coladdg;
		if (g > 0x1F) g = 0x1F;
		u2       b = (c >> 10 & 0x1F) + coladdb;
		if (b > 0x1F) b = 0x1F;
		cgram[0] = b << 10 | g << 5 | r;
	}
	if (Palette0 != 0) cgram[0] = 0;
	asm_call(makepalb);
}
#endif
