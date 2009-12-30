#include "../asm_call.h"
#include "../cfg.h"
#include "../cpu/execute.h"
#include "../init.h"
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
