#include <string.h>

#include "../asm_call.h"
#include "../cpu/execute.h"
#include "../intrf.h"
#include "../macros.h"
#include "c_gui.h"
#include "c_guimouse.h"
#include "gui.h"
#include "guimouse.h"


static char const guipresstext1[] = "ENTER THE KEY";
static char const guipresstext2[] = "OR BUTTON TO USE";


u4 guipresstest(void)
{
	memset(pressed, 0, sizeof(pressed));
	asm_call(GUIUnBuffer);
	asm_call(DisplayBoxes);
	DisplayMenu();
	GUIBox3D(75, 95, 180, 131);
	GUIOuttextShadowed(80, 100, guipresstext1);
	GUIOuttextShadowed(80, 110, guipresstext2);
	GUIOuttextShadowed(80, 120, "(ESC TO CLEAR)");
	asm_call(vidpastecopyscr);
	u1* key;
	do asm_call(JoyRead); while (!(key = GetAnyPressedKey()));
	for (u1* i = pressed; i != endof(pressed); ++i)
		if (*i != 0) *i = 2;
	while ((u1)Check_Key() != 0) // XXX asm_call
		asm_call(Get_Key);
	return key - pressed;
}


void guipresstestb(void)
{
	asm_call(GUIUnBuffer);
	asm_call(DisplayBoxes);
	DisplayMenu();
	GUIBox3D(65, 80, 194, 126);
	GUIOuttextShadowed(70,  85, guipresstext1);
	GUIOuttextShadowed(70,  95, guipresstext2);
	GUIOuttextShadowed(70, 105, guipressptr);
	GUIOuttextShadowed(70, 115, "(ESC TO SKIP)");
	asm_call(vidpastecopyscr);
	asm volatile("call *%0" :: "r" (delay), "c" (8192) : "eax");
	do asm_call(JoyRead); while (GetAnyPressedKey());

	u1* key;
	do asm_call(JoyRead); while (!(key = GetAnyPressedKey()));
	u4 const key_id = key - pressed;
	while ((u1)Check_Key() != 0) // XXX asm_call
		asm_call(Get_Key);
	if (key_id != 1 && key_id != 0x3B)
		*guicpressptr = key_id;
}
