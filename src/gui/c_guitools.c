#include <string.h>

#include "../cfg.h"
#include "../ui.h"
#include "../video/procvid.h"
#include "c_guitools.h"
#include "gui.h"


static void GUIoutputchar(u1* dst, u1 const glyph)
{
	// XXX better variable names
	// Font Setup (Menus)
	u1      (*font)[5] = newfont == 0 ? GUIFontData : GUIFontData1;
	u1 const* edi      = font[glyph];
	u4        cl       = 5;
	do
	{
		u4 ah = *edi;
		u4 ch = 6;
		do
		{
			if (ah & 0x80) *dst = GUItextcolor[0] - cl - ch + 1;
		}
		while (ah <<= 1, ++dst, --ch != 0);
		dst += 282;
		++edi;
	}
	while (--cl != 0);
}


char const* GUIOutputString(u1* dst, char const* text)
{
	for (;; dst += 6, ++text)
	{
		u1 const c = *text;
		if (c == '\0') return text;
		GUIoutputchar(dst, ASCII2Font[c]);
	}
}


void GUIOuttext(u4 const x, u4 const y, char const* const text, u1 const colour)
{
	GUItextcolor[0] = colour;
	u1* const dst = vidbuffer + y * 288 + x + 16;
	GUIOutputString(dst, text);
}


void GUIDrawBox(u1* dst, u4 const w, u4 h, u1 const colour)
{
	do
	{
		memset(dst, colour, w);
		dst += 288;
	}
	while (--h != 0);
}


void GUIBox(u4 const x1, u4 const y1, u4 const x2, u4 const y2, u1 const colour)
{
	u1* const dst = vidbuffer + x1 + y1 * 288 + 16;
	u4  const w   = x2 - x1 + 1;
	u4  const h   = y2 - y1 + 1;
	GUIDrawBox(dst, w, h, colour);
}


static void GUIDrawShadow(u1* dst, u4 const w, u4 h)
{
	do
	{
		u1* edi = dst;
		u4  ecx = w;
		do
		{
			u1 px = edi[-3 * 288 - 3];
			if (148 <= px && px <= 167)
			{
				edi[-3 * 288 - 3] = px + 20;
			}
			else if (189 <= px && px <= 220)
			{
				edi[-3 * 288 - 3] = 189 + (px - 189) / 2;
			}
			else
			{
				px = edi[0];
				if (px < 32)
					edi[0] = px + 96;
			}
			++edi;
		}
		while (--ecx != 0);
		dst += 288;
	}
	while (--h != 0);
}


void GUIShadow(u4 const x1, u4 const y1, u4 const x2, u4 const y2)
{
	u1* const dst = vidbuffer + x1 + y1 * 288 + 16;
	u4  const w   = x2 - x1 + 1;
	u4  const h   = y2 - y1 + 1;
	GUIDrawShadow(dst, w, h);
}
