#include <string.h>

#include "../asm_call.h"
#include "../c_intrf.h"
#include "../cfg.h"
#include "../cpu/execute.h"
#include "../intrf.h"
#include "../macros.h"
#include "../video/procvid.h"
#include "c_gui.h"
#include "c_guimouse.h"
#include "gui.h"
#include "guimouse.h"

#ifndef __MSDOS__
#include "../video/ntsc.h"
#endif

static u1 LastHoldEnable;
static u1 ntscLastVar[6];

static char const guipresstext1[] = "ENTER THE KEY";
static char const guipresstext2[] = "OR BUTTON TO USE";


void ProcessMouse(void)
{
	// Process holds
	if (LastHoldEnable != GUIHold)
	{
		switch (GUIHold)
		{
			case 0:
				if (LastHoldEnable == 7) // ntsc sliders
				{
					u4 const idx = ntscWhVar;
					u1 const cur = ntscCurVar;
					if (ntscLastVar[idx] != cur)
					{
#ifndef __MSDOS__
						NTSCFilterInit();
#endif
						ntscLastVar[idx] = cur;
					}
				}
				Set_MouseXMax(0, 255);
				Set_MouseYMax(0, 223);
				break;

			case 1: // GUI Windows
				Set_MouseXMax( 0, 255);
				Set_MouseYMax(16, 223);
				break;

			case 2: // Colour Slider
			case 5: // Sound Slider
			case 6: // Speed Slider
			case 7: // Video Slider
			case 8: // Scanline Slider
				// Sets min/max move range for mouse once holding slider
				Set_MouseXMax(GUIHoldXlimL, GUIHoldXlimR);
				// Locks pointer on slider
				Set_MouseYMax(GUIHoldYlim, GUIHoldYlim);
				break;

			case 3: // Scrollbars
				Set_MouseXMax(GUIHoldXlimL, GUIHoldXlimR);
				Set_MouseYMax(GUIHoldYlim,  GUIHoldYlimR);
				break;
		}
		LastHoldEnable = GUIHold;
	}
	MouseMoveOkay = 0;
	u4 buttons = Get_MouseData() & 0xFFFF;
	if (lhguimouse == 1)
	{
		asm("call *%1" : "+b" (buttons) : "r" (SwapMouseButtons) : "cc"); // asm_call
	}
	mousebuttonstat = buttons;
	if (lastmouseholded != 0 && !(buttons & 0x01))
	{
		lastmouseholded = 0;
		Set_MousePosition(GUImouseposx, GUImouseposy);
	}
	if (mousewrap == 1)
	{
		asm_call(ProcessMouseWrap);
	}
	else
	{
		u4 const data = Get_MouseData();
		u2       x    = data >> 16 & 0xFF;
		u2       y    = data >> 24;
		if (GUImouseposx != x || GUImouseposy != y) MouseMoveOkay = 1;
		if (x & 0x8000) x =   0;
		if (x >    255) x = 255;
		GUImouseposx = x;
		if (y & 0x8000) y =   0;
		if (y >    223) y = 100;
		GUImouseposy = y;
		asm_call(ProcessMouseButtons);
	}
}


u4 guipresstest(void)
{
	memset(pressed, 0, sizeof(pressed));
	GUIUnBuffer();
	DisplayBoxes();
	DisplayMenu();
	GUIBox3D(75, 95, 180, 131);
	GUIOuttextShadowed(80, 100, guipresstext1);
	GUIOuttextShadowed(80, 110, guipresstext2);
	GUIOuttextShadowed(80, 120, "(ESC TO CLEAR)");
	vidpastecopyscr();
	u1* key;
	do JoyRead(); while (!(key = GetAnyPressedKey()));
	for (u1* i = pressed; i != endof(pressed); ++i)
		if (*i != 0) *i = 2;
	while (Check_Key() != 0) Get_Key();
	return key - pressed;
}


void guipresstestb(void)
{
	GUIUnBuffer();
	DisplayBoxes();
	DisplayMenu();
	GUIBox3D(65, 80, 194, 126);
	GUIOuttextShadowed(70,  85, guipresstext1);
	GUIOuttextShadowed(70,  95, guipresstext2);
	GUIOuttextShadowed(70, 105, guipressptr);
	GUIOuttextShadowed(70, 115, "(ESC TO SKIP)");
	vidpastecopyscr();
	delay(8192);
	do JoyRead(); while (GetAnyPressedKey());

	u1* key;
	do JoyRead(); while (!(key = GetAnyPressedKey()));
	u4 const key_id = key - pressed;
	while (Check_Key() != 0) Get_Key();
	if (key_id != 1 && key_id != 0x3B)
		*guicpressptr = key_id;
}
