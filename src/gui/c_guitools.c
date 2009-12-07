#include "../ui.h"
#include "../video/procvid.h"
#include "c_guitools.h"
#include "gui.h"
#include "guitools.h"


static void GUIOutputString(u1* dst, char const* text)
{
	for (;; dst += 6, ++text)
	{
		u1 const c = *text;
		if (c == '\0') return;
		asm volatile("call %P0" :: "X" (GUIoutputchar), "S" (dst), "a" (ASCII2Font[c]): "cc", "memory", "ebx", "ecx");
	}
}


void GUIOuttext(u4 const x, u4 const y, char const* const text, u1 const colour)
{
	GUItextcolor[0] = colour;
	u1* const dst = vidbuffer + y * 288 + x + 16;
	GUIOutputString(dst, text);
}
