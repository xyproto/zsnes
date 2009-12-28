#include <string.h>

#include "../asm_call.h"
#include "../c_intrf.h"
#include "../cfg.h"
#include "../cpu/execute.h"
#include "../gblvars.h"
#include "../intrf.h"
#include "../link.h"
#include "../macros.h"
#include "../vcache.h"
#include "../video/procvid.h"
#include "../zmovie.h"
#include "c_gui.h"
#include "c_guimouse.h"
#include "gui.h"
#include "guicheat.h"
#include "guicombo.h"
#include "guifuncs.h"
#include "guikeys.h"
#include "guimisc.h"
#include "guimouse.h"
#include "guiwindp.h"

#ifndef __MSDOS__
#include "../video/ntsc.h"
#include "../video/procvidc.h"
#endif

#ifdef __WIN32__
#	include "../win/winlink.h"
#endif


u1 GUIcwinpress;
u1 lastmouseholded;

static u1 LastHoldEnable;
static u1 MouseMoveOkay;
static u1 ntscLastVar[6];
static u2 mousebuttonstat;

static char const guipresstext1[] = "ENTER THE KEY";
static char const guipresstext2[] = "OR BUTTON TO USE";


static void GUIProcVideo(void)
{
	GUICBHold = 0;
	newengen  = 1;
	guiprevideo();
	u1 const prevvid = cvidmode;
	cvidmode = GUIcurrentvideocursloc;
#ifdef __MSDOS__
	ExitFromGUI = 1;
#endif
	initvideo();
#ifdef __MSDOS__
	if (videotroub == 1)
	{
		videotroub = 0;
		cvidmode   = prevvid;
		initvideo();
		GUISetPal();
		guipostvideofail();
	}
	else
#endif
	{
		GUISetPal();
		guipostvideo();
		if (GUIkeydelay == 0)
		{
			videotroub = 0;
			cvidmode   = prevvid;
			initvideo();
			GUISetPal();
			vidpastecopyscr();
		}
	}
	GUIkeydelay = 0;
}


static void GUINTSCReset(void)
{
	if (GUICBHold != 38)
	{
		NTSCBlend  = 0;
		NTSCRef    = 0;
		NTSCHue    = 0;
		NTSCSat    = 0;
		NTSCCont   = 0;
		NTSCBright = 0;
		NTSCSharp  = 0;
	}
	if (GUICBHold != 37)
	{
		NTSCGamma  = 0;
		NTSCRes    = 0;
		NTSCArt    = 0;
		NTSCFringe = 0;
		NTSCBleed  = 0;
		NTSCWarp   = 0;
	}
	GUICBHold = 0;
#ifndef __MSDOS__
	NTSCFilterInit();
#endif
}


static void GUINTSCPreset(void)
{
	switch (GUICBHold)
	{
		case 81: NTSCPresetVar = 0; break;
		case 82: NTSCPresetVar = 1; break;
		case 83: NTSCPresetVar = 2; break;
		case 84: NTSCPresetVar = 3; break;
	}
#ifndef __MSDOS__
	NTSCFilterInit();
#endif
	NTSCPresetVar = 4;
	GUICBHold     = 0;
}


#ifndef __MSDOS__
static void GUIProcCustomVideo(void)
{
	SetCustomXY();
	GUICBHold   = 0;
	GUIInputBox = 0;
#ifdef __WIN32__
	if (cvidmode >= 37)
#else
	if (cvidmode >= 20)
#endif
	{
		changeRes = 1;
		initwinvideo();
		Clear2xSaIBuffer();
	}
}
#endif


void SwitchFullScreen(void)
{
	Clear2xSaIBuffer();
#ifndef __MSDOS__
	if (GUIWFVID[cvidmode] != 0)
	{
		cvidmode = PrevWinMode;
		initvideo();
	}
	else
#endif
	{
#ifndef __MSDOS__
		cvidmode = PrevFSMode;
#endif
		initvideo();
	}
}


static void GUIWindowMove(void)
{
	u1 const id = GUIwinorder[GUIwinptr - 1];
	u4 const rx = GUImouseposx - GUIwinposx[id];
	u4 const ry = GUImouseposy - GUIwinposy[id];
	void (* f)();
	switch (id)
	{
		case  3: f = DisplayGUIInputClick2;       break;
		case  5: f = DisplayGUIVideoClick2;       break;
		case  7: f = DisplayGUICheatClick2;       break;
		case 13: f = DisplayGUICheatSearchClick2; break;
		case 16: f = DisplayGUIComboClick2;       break;
		default: f = DisplayGUIConfirmClick2;     break;
	}
	asm volatile("call *%0" :: "r" (f), "a" (rx), "d" (ry) : "cc", "memory"); // XXX asm_call
}


static void ProcessMouseButtons(void)
{
	static u1 GUIOnMenuItm;

	u2 const x = GUImouseposx;
	u2 const y = GUImouseposy;

	if (MouseMoveOkay == 1)
	{
		GUIOnMenuItm = 0;
		if (GUIMenuL < x && x < GUIMenuR && 18 < y && y < GUIMenuD)
		{
			// Mouse Menu
			GUIOnMenuItm = 2;
			u4 const row = (y - 18) / 10;
			if (GUICYLocPtr[row + 1] != 0)
			{
				GUIcrowpos   = row;
				GUIOnMenuItm = 1;
			}
		}
	}

	// Check if mouse is clicked on menu
	u2 const buttons = mousebuttonstat;
	if (GUIRClick == 1)
	{
		if (buttons & 0x02)
		{
			if (romloadskip != 0 || MousePRClick != 0) goto norclick2;
			GUIQuit = 2;
		}
		MousePRClick = 0;
norclick2:;
	}
	if (buttons & 0x01)
	{
		if (GUIpclicked != 1 && GUIOnMenuItm != 2)
		{
			if (GUIOnMenuItm == 1)
			{
				GUITryMenuItem();
			}
			else if (y <= 15)
			{
				if (3 <= y && y <= 14)
				{
					if (233 <= x && x <= 242)
					{
						GUIcwinpress =
#ifndef __UNIXSDL__
							y > 8 ? 3 :
#endif
							1;
						goto noclick;
					}
					else if (244 <= x && x <= 253)
					{
						GUIcwinpress = 2;
						goto noclick;
					}
				}

				if (3 <= y && y <= 13)
				{
					if (4 <= x && x <= 12)
					{
						GUIcmenupos = 1;
						GUIcrowpos  = 0;
						goto noclick;
					}
					else if (17 <= x && x <= 47)
					{
						GUIcmenupos = 2;
						GUIcrowpos  = 0;
						goto noclick;
					}
					else if (52 <= x  && x <= 94)
					{
						GUIcmenupos = 3;
						GUIcrowpos  = 0;
						goto noclick;
					}
					else if (99 <= x  && x <= 135)
					{
						GUIcmenupos = 4;
						GUIcrowpos  = 0;
						goto noclick;
					}
					else if (140 <= x  && x <= 188)
					{
						GUIcmenupos = 5;
						GUIcrowpos  = 0;
						goto noclick;
					}
					else if (193 <= x && x <= 223)
					{
						GUIcmenupos = 6;
						GUIcrowpos  = 0;
						goto noclick;
					}
					else if (224 <= x)
					{
						goto noclick;
					}
				}

#ifndef __MSDOS__
				GUIpclicked  =   1;
				GUIHold      = 255;
				GUIHoldYlim  =   y;
				GUIHoldXlimL =   x;
				MouseWindow();
#endif
			}
			else if (GUIcmenupos == 0)
			{
				GUIpclicked = 1;
				u4 i = GUIwinptr;
				if (i != 0)
				{
					u1 const id = GUIwinorder[--i];
					if (GUIwinposx[id] < x && x < GUIwinposx[id] + GUIwinsizex[id] &&
							GUIwinposy[id] < y && y < GUIwinposy[id] + GUIwinsizey[id] + 10)
					{
						u4 eax;
						u4 ecx;
						u4 edx;
						u4 esi;
						asm volatile("call *%4" : "=a" (eax), "=c" (ecx), "=d" (edx), "=S" (esi) : "r" (GUIWinClicked), "a" (i), "b" (id) : "cc", "memory"); // XXX asm_call
						(void)eax;
						(void)ecx;
						(void)edx;
						(void)esi;
						return;
					}
					while (i != 0)
					{
						u1 const id = GUIwinorder[--i];
						if (x <= GUIwinposx[id] || GUIwinposx[id] + GUIwinsizex[id]      <= x) continue;
						if (y <= GUIwinposy[id] || GUIwinposy[id] + GUIwinsizey[id] + 10 <= y) continue;
						// Shift all following windows downwards by 1
						while (++i != GUIwinptr) GUIwinorder[i - 1] = GUIwinorder[i];
						GUIwinorder[i - 1] = id;
						GUIpclicked        = 0;
						return;
					}
				}
				if (SantaPos != 272 && ShowTimer == 0)
				{
					MsgGiftLeft = 36 * 4;
					ShowTimer   = 1;
				}
				return;
			}
			else
			{
				GUIpmenupos = GUIcmenupos;
				GUIcmenupos = 0;
			}
		}

noclick:;
		GUIpclicked = 1;
		switch (GUIHold)
		{
			case 2:
			{ // Colour Slide Bar Hold
				GUImouseposy = GUIHoldYlim;
				u4 const minx = GUIHoldXlimL;
				if (x < minx) GUImouseposx = minx;
				u4 const maxx = GUIHoldXlimR;
				if (x > maxx) GUImouseposx = maxx;
				lastmouseholded = 1;
				asm_call(DisplayGUIOptnsClick);
				return;
			}

			case 3:
			{ // Box Hold
				u4 const miny = GUIHoldYlim;
				if (y <= miny) GUImouseposy = miny;
				u4 const minx = GUIHoldXlimL;
				if (x <= minx) GUImouseposx = minx;
				u4 const maxy = GUIHoldYlimR;
				if (y >= maxy) GUImouseposy = maxy;
				u4 const maxx = GUIHoldXlimR;
				if (x >= maxx) GUImouseposx = maxx;
				lastmouseholded = 1;
				GUIWindowMove();
				return;
			}

			case 4:
			{
				u1 const id = GUIwinorder[GUIwinptr - 1];
				u4 const rx = x - GUIwinposx[id];
				u4 const ry = y - GUIwinposy[id];
				GUICBHold =
					GUIHoldXlimL <= rx && rx <= GUIHoldXlimR &&
					GUIHoldYlim  <= ry && ry <= GUIHoldYlimR ? GUICBHold2 :
					0;
				return;
			}

			case 5:
			{ // Sound Slide Bar Hold
				GUImouseposy = GUIHoldYlim;
				u4 const minx = GUIHoldXlimL;
				if (x < minx) GUImouseposx = minx;
				u4 const maxx = GUIHoldXlimR;
				if (x > maxx) GUImouseposx = maxx;
				lastmouseholded = 1;
				asm_call(DisplayGUISoundClick);
				return;
			}

			case 6:
			{ // Speed Slide Bar Hold
				GUImouseposy = GUIHoldYlim;
				u4 const minx = GUIHoldXlimL;
				if (x < minx) GUImouseposx = minx;
				u4 const maxx = GUIHoldXlimR;
				if (x > maxx) GUImouseposx = maxx;
				lastmouseholded = 1;
				asm_call(DisplayGUISpeedClick);
				return;
			}

			case 7:
			case 8:
			{ // Video Slide Bar Hold
				GUImouseposy = GUIHoldYlim;
				u4 const minx = GUIHoldXlimL;
				if (x < minx) GUImouseposx = minx;
				u4 const maxx = GUIHoldXlimR;
				if (x > maxx) GUImouseposx = maxx;
				lastmouseholded = 1;
				asm_call(DisplayGUIVideoClick);
				return;
			}

			case 1:
			{
hold:
				if (GUImouseposy < 16)
				{
					if (mousewrap & 1)
					{
						GUImouseposy += 224 - 16;
						goto hold;
					}
					GUImouseposy = 16;
				}
				u1 const id = GUIwinorder[GUIwinptr - 1];
				GUIwinposy[id] = (s2)(GUImouseposy - GUIHoldy + GUIHoldym);
				GUIwinposx[id] = (s2)(GUImouseposx - GUIHoldx + GUIHoldxm);
				return;
			}

			case 255:
			{
				GUImouseposy = GUIHoldYlim;
				GUImouseposx = GUIHoldXlimL;
				Set_MousePosition(GUIHoldXlimL, GUIHoldYlim);
				return;
			}

			default:
				GUICHold = 0;
				return;
		}
	}
	GUICHold    = 0;
	GUIpclicked = 0;
	GUIHold     = 0;

#if defined __UNIXSDL__ || defined __WIN32__
	if (GUIcwinpress == 1)
	{
		GUIcwinpress = 0;
#ifdef __UNIXSDL__
		if (3 <= y && y <= 13)
#else
		if (3 <= y && y <=  7)
#endif
		{
			if (233 <= x && x <= 242)
			{
				SwitchFullScreen();
				return;
			}
		}
	}
#endif

#ifndef __MSDOS__
	if (GUIcwinpress == 2)
	{
		GUIcwinpress = 0;
		if (3 <= y && y <= 13 && 44 <= x && x <= 253)
		{
			GUIQuit = 1;
			return;
		}
	}

	if (GUIcwinpress == 3)
	{
		GUIcwinpress = 0;
		if (9 <= y && y <= 13 && 233 <= x && x <= 242)
		{
#ifdef __WIN32__
			MinimizeWindow();
#endif
			return;
		}
	}
#endif

	// ButtonProcess
	switch (GUICBHold)
	{
		case  1: GUILoadData();              return;
		case  2: GUIProcReset();             return;
		case  3: GUIProcReset();             return;
		case  4: GUIProcVideo();             return; // set video mode
#ifndef __MSDOS__
		case 12: GUIProcCustomVideo();       return; // set custom video mode
#endif
		case 37:
		case 38:
		case 39: GUINTSCReset();             return; // reset ntsc options
		case 81:
		case 82:
		case 83:
		case 84: GUINTSCPreset();            return; // ntsc preset
		case 10:
		case 11: GUIProcStates();            return;
		case  5: asm_call(CheatCodeRemove);  return;
		case  6: asm_call(CheatCodeToggle);  return;
		case  7: CheatCodeSave();            return;
		case  8: CheatCodeLoad();            return;
		case  9: asm_call(ProcessCheatCode); return;
		case 33: asm_call(CheatCodeFix);     return;
		case 14: SetDevice();                return;
		case 15: CalibrateDev1();            return;

		case 16: GUICBHold = 0; MoviePlay();          return; // movie replay
		case 17: GUICBHold = 0; MovieRecord();        return; // movie record
		case 18: GUICBHold = 0; MovieStop();          return; // movie stop
		case 19: GUICBHold = 0; MovieRecord();        return; // overwrite zmv ? yes
		case 20: GUICBHold = 0; SkipMovie();          return; // overwrite zmv ? no
		case 29: GUICBHold = 0; MovieInsertChapter(); return; // insert chapter

		case 30: GUICBHold = 0; GUIQuit = 2; MovieSeekBehind(); return; // back to previous chapter
		case 31: GUICBHold = 0; GUIQuit = 2; MovieSeekAhead();  return; // jump to next chapter
		case 32: GUICBHold = 0; GUIQuit = 2; MovieAppend();     return; // append movie

		case 34: // dump raw
		{
			GUICBHold = 0;
			GUIQuit   = 2;
			SetMovieForcedLength();
			MovieDumpRaw();
			if (MovieVideoMode >= 2 && mencoderExists == 0)
			{
				guimencodermsg();
			}
			if (MovieAudio != 0 && MovieAudioCompress != 0 && lameExists == 0)
			{
				guilamemsg();
			}
			return;
		}

		case 35: GUICBHold = 0; GUIQuit = 2; MovieStop(); return; // stop dump
		case 40: SetAllKeys();                            return;

		case 50: asm_call(CheatCodeSearchInit); break;

		case 60:
			GUIComboTextH[0] = '\0';
			GUINumCombo      = 0;
			GUIComboKey      = 0;
			break;

		case 61: if (NumCombo != 50) asm_call(ComboAdder);   break;
		case 62: if (NumCombo !=  0) asm_call(ComboReplace); break;
		case 63: if (NumCombo !=  0) asm_call(ComboRemoval); break;

		case 51:
			CheatWinMode      = 0;
			CheatSearchStatus = 0;
			break;

		case 52: CheatWinMode = 2;                 break;
		case 53: asm_call(CheatCodeSearchProcess); break;
		case 54: CheatWinMode = 1;                 break;

		case 55:
			if (NumCheatSrc != 0)
			{
				CheatWinMode      = 3;
				CurCStextpos      = 0;
				CSInputDisplay[0] = '_';
				CSInputDisplay[1] = '\0';
				CSDescDisplay[0]  = '\0';
			}
			break;

		case 56: CheatWinMode = 2;         break;
		case 57: asm_call(AddCSCheatCode); break;

#ifndef __MSDOS__
		case 65: ZsnesPage(); break;
		case 66: DocsPage();  break;
#endif
	}
	GUICBHold = 0;
}


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
		u4 const delta = Get_MousePositionDisplacement();

		u2 x = GUImouseposx + delta;
		while (x & 0x8000) x += 256;
		while (x > 255)    x -= 256;
		GUImouseposx = x;

		u2 y = GUImouseposy + (delta >> 16);
		while (y & 0x8000) y += 224;
		while (y >    223) y -= GUIHold == 1 ? 224 - 16 : 224;
		GUImouseposy = y;

		if (delta != 0) MouseMoveOkay = 1;
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
	}
	ProcessMouseButtons();
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
