#include "../cfg.h"
#include "../ui.h"
#include "../video/procvid.h"
#include "c_guitools.h"
#include "gui.h"


void GUIoutputchar(u1* dst, u1 const glyph)
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


static void GUIOutputString(u1* dst, char const* text)
{
	for (;; dst += 6, ++text)
	{
		u1 const c = *text;
		if (c == '\0') return;
		GUIoutputchar(dst, ASCII2Font[c]);
	}
}


void GUIOuttext(u4 const x, u4 const y, char const* const text, u1 const colour)
{
	GUItextcolor[0] = colour;
	u1* const dst = vidbuffer + y * 288 + x + 16;
	GUIOutputString(dst, text);
}
