#include <string.h>

#include "../asm_call.h"
#include "../cpu/execute.h"
#include "../intrf.h"
#include "../macros.h"
#include "c_gui.h"
#include "c_guimouse.h"
#include "gui.h"
#include "guimouse.h"


u4 guipresstest(void)
{
	memset(pressed, 0, sizeof(pressed));
	asm_call(GUIUnBuffer);
	asm_call(DisplayBoxes);
	asm_call(DisplayMenu);
	GUIBox3D(75, 95, 180, 131);
	GUIOuttextShadowed(80, 100, guipresstext1);
	GUIOuttextShadowed(80, 110, guipresstext2);
	GUIOuttextShadowed(80, 120, guipresstext3);
	asm_call(vidpastecopyscr);
	u1* key;
	do asm_call(JoyRead); while (!(key = GetAnyPressedKey()));
	for (u1* i = pressed; i != endof(pressed); ++i)
		if (*i != 0) *i = 2;
	while ((u1)Check_Key() != 0) // XXX asm_call
		asm_call(Get_Key);
	return key - pressed;
}
