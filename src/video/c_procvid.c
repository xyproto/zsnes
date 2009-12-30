#include "../asm_call.h"
#include "../c_intrf.h"
#include "../cfg.h"
#include "../cpu/execute.h"
#include "../init.h"
#include "../input.h"
#include "../ui.h"
#include "c_procvid.h"
#include "procvid.h"


void showvideo(void)
{
	if (++ccud != cacheud) ccud = 0;
	asm_call(copyvid);
	if (pressed[KeyStateSelct] & 1) asm_call(saveselect);
}


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
