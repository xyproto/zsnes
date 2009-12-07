#include "../ui.h"
#include "c_guitools.h"
#include "gui.h"
#include "guitools.h"


void GUIOuttext(u4 const x, u4 const y, char const* const text, u1 const colour)
{
	GUItextcolor[0] = colour;
	u1* dst = vidbuffer + y * 288 + x + 16;
	char const* text_ = text;
	asm volatile("call %P2" : "+S" (dst), "+D" (text_) : "X" (GUIOutputString) : "cc", "memory", "eax", "ebx", "ecx");
}
