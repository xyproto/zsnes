#include <stdarg.h>
#include <stdbool.h>

#include "../c_init.h"
#include "../c_intrf.h"
#include "../cfg.h"
#include "../cpu/execute.h"
#include "../cpu/regs.h"
#include "../input.h"
#include "../types.h"
#include "../video/procvid.h"
#include "c_gui.h"
#include "c_guikeys.h"
#include "gui.h"
#include "guikeys.h"
#include "guiwindp.h"

#ifdef __MSDOS__
#	include "guimisc.h"
#endif

#ifdef __UNIXSDL__
#	include "../linux/sdllink.h"
#endif

#ifdef __WIN32__
#	include "../win/winlink.h"
#endif


#ifdef __UNIXSDL__
#	define IFKEY(key, a, b) if ((key) == (a) || (numlockptr != 1 && (key) == (b)))
#else
#	define IFKEY(key, a, b) if ((key) == (b))
#endif


u1 GUIDelayB;
u4 GUIkeydelay;

static u1 GUIJoyPadnk[8];
static u4 GUIfirstkey;
static u4 GUIlastkey;
static u4 GUInextkeydelay;


static void GUIqcheckkeys(u4 const p1)
{
	if (pressed[p1] == 1) GUIfirstkey = 1;
}


static void GUIqcheckkeys2(u4 const p1, u4 const p2)
{
	if (pressed[p1] == 1 && GUIJoyPadnk[p2] != 2) GUIfirstkey = 1;
}


static bool GUIgetprkeys(u4 const p1, u4 const p2, char* const al)
{
	if (pressed[p1] == 1)
	{
		GUInextkeydelay = 10;
	}
	else if (pressed[p1] != 2 || GUIfirstkey == 1 || GUIlastkey != p1)
	{
		return false;
	}
	GUIlastkey = p1;
	pressed[p1] = 2;
	if (GUIkeydelay == 0)
	{
		GUIkeydelay     = GUInextkeydelay;
		GUInextkeydelay = 2;
		*al             = p2;
	}
	return true;
}


static bool GUIgetprkeysb(u4 const p1, u4 const p2, char* const al)
{
	if (pressed[p1] != 1)
	{
		if (pressed[p1] == 0) GUIescpress = 0;
	}
	else if (GUIescpress != 1)
	{
		pressed[p1] = 2;
		*al = p2;
		return true;
	}
	return false;
}


static bool GUIgetprkeys2(u4 const p1, u4 const p2, u4 const p3, char* const al)
{
	if (pressed[p1] == 0)
	{
		GUIJoyPadnk[p3] = 0;
		return false;
	}
	else if (GUIJoyPadnk[p3] != 2 && pressed[p1] == 1)
	{
		GUInextkeydelay = 10;
	}
	else if (GUIfirstkey == 1 || GUIlastkey != p1)
	{
		return false;
	}
	GUIlastkey = p1;
	GUIJoyPadnk[p3] = 2;
	if (GUIkeydelay == 0)
	{
		GUIkeydelay     = GUInextkeydelay;
		GUIDelayB       = 5;
		GUInextkeydelay = 2;
		*al             = p2;
	}
	return true;
}


static u1 ToUpperASM(u1 dh)
{
	if ('a' <= dh && dh <= 'z') dh -= 'a' - 'A';
	return dh;
}


static void KeyTabInc(u4* const first, ...) // tab arrays
{
	va_list ap;
	va_start(ap, first);

	u4* a = first;
	for (;;)
	{
		u4 const eax = a[0] / a[1];
		a[0] = a[0] % a[1];
		u4* const b = va_arg(ap, u4*);
		(b ? b : first)[0] |= eax;
		if (eax == 1) break;
		if (eax < a[0]) ++a[0];
		if (a[0] != 0) break;
		if (!b) break;
		a = b;
	}

	va_end(ap);
}


static void GUIKeyCheckbox(u1* const p1, char const p2, char const dh)
{
	if (dh == p2) *p1 ^= 1;
}


static void GUIKeyButtonHole(u1* const p1, u1 const p2, char const p3, char const dh)
{
	if (dh == p3) *p1 = p2;
}


static void GUIInputKeys(char dh)
{
	dh = ToUpperASM(dh);
	if (dh == 9)
	{
		KeyTabInc(GUIInputTabs, (u4*)0);
		GUIFreshInputSelect = 1;
	}
	GUIKeyCheckbox(&GameSpecificInput, 'G', dh);
	GUIKeyCheckbox(&AllowUDLR,         'A', dh);
	GUIKeyCheckbox(&Turbo30hz,         'T', dh);
	if (dh == 'U')
	{
		pl12s34 ^= 1;
		MultiTap = pl12s34 != 1 && (pl3contrl != 0 || pl4contrl != 0 || pl5contrl != 0);
	}
#ifdef __MSDOS__
	GUIKeyCheckbox(&SidewinderFix, 'S', dh);
	if (dh == 'J')
	{
		switch (cplayernum)
		{
			case 0: pl1p209 ^= 1; break;
			case 1: pl2p209 ^= 1; break;
			case 2: pl3p209 ^= 1; break;
			case 3: pl4p209 ^= 1; break;
			case 4: pl5p209 ^= 1; break;
		}
		SetDevice();
	}
#endif
}


static void GUIOptionKeys(char dh)
{
	if (dh == 9)
	{
		KeyTabInc(GUIOptionTabs, (u4*)0);
	}
	dh = ToUpperASM(dh);
	if (GUIOptionTabs[0] == 1)
	{ // Basic
		if (ShowMMXSupport == 1) GUIKeyCheckbox(&MMXSupport, 'M', dh);
		GUIKeyCheckbox(&Show224Lines,       'L', dh);
		GUIKeyCheckbox(&newengen,           'N', dh);
		GUIKeyCheckbox(&bgfixer,            'A', dh);
		GUIKeyCheckbox(&AutoPatch,          'I', dh);
		GUIKeyCheckbox(&DisplayInfo,        'R', dh);
		GUIKeyCheckbox(&RomInfo,            'G', dh);
#ifdef __WIN32__
		GUIKeyCheckbox(&PauseFocusChange,   'B', dh);
		GUIKeyCheckbox(&HighPriority,       'P', dh);
		CheckPriority();
#endif
		GUIKeyCheckbox(&DisableScreenSaver, 'D', dh);
#ifdef __WIN32__
		CheckScreenSaver();
#endif
	}

	if (GUIOptionTabs[0] == 2)
	{ // More
		GUIKeyCheckbox(&FPSAtStart,            'F', dh);
		GUIKeyCheckbox(&TimerEnable,           'C', dh);
		GUIKeyCheckbox(&TwelveHourClock,       'H', dh);
		GUIKeyCheckbox(&ClockBox,              'X', dh);
		GUIKeyCheckbox(&SmallMsgText,          'S', dh);
		GUIKeyCheckbox(&GUIEnableTransp,       'T', dh);
		GUIKeyButtonHole(&ScreenShotFormat, 0, 'B', dh);
#ifndef NO_PNG
		GUIKeyButtonHole(&ScreenShotFormat, 1, 'P', dh);
#endif
	}
}


static void GUIAboutKeys(char dh)
{
	dh = ToUpperASM(dh);
	GUIKeyCheckbox(&EEgg, 'E', dh);
}


static void GUIAddonKeys(char dh)
{
	dh = ToUpperASM(dh);
	GUIKeyButtonHole(&device1, 0, 'G', dh);
	GUIKeyButtonHole(&device1, 1, 'M', dh);
	GUIKeyButtonHole(&device2, 0, 'A', dh);
	GUIKeyButtonHole(&device2, 1, 'O', dh);
	GUIKeyButtonHole(&device2, 2, 'S', dh);
	GUIKeyButtonHole(&device2, 3, '1', dh);
	GUIKeyButtonHole(&device2, 4, '2', dh);

	GUIKeyCheckbox(&mouse1lh, 'L', dh);
	GUIKeyCheckbox(&mouse2lh, 'E', dh);
}


static void GUIChipKeys(char dh)
{
	dh = ToUpperASM(dh);
	GUIKeyCheckbox(&nssdip1, '1', dh);
	GUIKeyCheckbox(&nssdip2, '2', dh);
	GUIKeyCheckbox(&nssdip3, '3', dh);
	GUIKeyCheckbox(&nssdip4, '4', dh);
	GUIKeyCheckbox(&nssdip5, '5', dh);
	GUIKeyCheckbox(&nssdip6, '6', dh);
}


static void GUISaveKeys(char dh)
{
	dh = ToUpperASM(dh);
	GUIKeyCheckbox(&AutoIncSaveSlot, 'I', dh);
	GUIKeyCheckbox(&nosaveSRAM,      'D', dh);
	GUIKeyCheckbox(&SRAMSave5Sec,    'C', dh);
	GUIKeyCheckbox(&LatestSave,      'S', dh);
	GUIKeyCheckbox(&AutoState,       'A', dh);
	GUIKeyCheckbox(&SRAMState,       'L', dh);
	GUIKeyCheckbox(&PauseLoad,       'P', dh);
	GUIKeyCheckbox(&PauseRewind,     'R', dh);
}


static void GUISpeedKeys(char dh)
{
	dh = ToUpperASM(dh);
	GUIKeyCheckbox(&FastFwdToggle, 'T', dh);

	if (dh == 'A') // Framerate Checkboxes
	{
		if (frameskip != 0) // 0 = autoframerate / 1-10 = frameskip 0-9
		{
			frameskip = 0;
		}
		else
		{
			FPSOn     = 0;
			frameskip = 1;
		}
	}
}


void GUIgetcurrentinput(void)
{
	char UseExtKey = '\0';
	char ch        = '\0';
	if (Check_Key() != 0)
	{
		char const al = Get_Key();
		ch = al;
		if (al == 0)
		{ // Extended key
			UseExtKey = Get_Key();
			ch = '\0';
		}
	}
	char dh = ch;
	GUIDelayB = 0;
	// Convert pressed to keys
	GUIfirstkey = 0;

#ifdef __UNIXSDL___
	GUIqcheckkeys(90); // UP
	GUIqcheckkeys(96); // DOWN
	GUIqcheckkeys(92); // LEFT
	GUIqcheckkeys(94); // RIGHT
	GUIqcheckkeys(89); // HOME
	GUIqcheckkeys(91); // PGUP
	GUIqcheckkeys(95); // END
	GUIqcheckkeys(97); // PGDOWN
#endif
	GUIqcheckkeys(72); // NUMPAD STUFF
	GUIqcheckkeys(80);
	GUIqcheckkeys(75);
	GUIqcheckkeys(77);
	GUIqcheckkeys(73);
	GUIqcheckkeys(81);
	GUIqcheckkeys(71);
	GUIqcheckkeys(79);
	GUIqcheckkeys(1);
	GUIqcheckkeys(0x1C);
#ifndef __MSDOS__
	GUIqcheckkeys(0xC8);
	GUIqcheckkeys(0xD0);
	GUIqcheckkeys(0xCB);
	GUIqcheckkeys(0xCD);
	GUIqcheckkeys(0xC9);
	GUIqcheckkeys(0xD1);
	GUIqcheckkeys(0xC7);
	GUIqcheckkeys(0xCF);
	GUIqcheckkeys(0x9C);
#endif

	if (JoyPad1Move != 0)
	{
		JoyRead();
		GUIqcheckkeys2(pl1upk,    0);
		GUIqcheckkeys2(pl1downk,  1);
		GUIqcheckkeys2(pl1leftk,  2);
		GUIqcheckkeys2(pl1rightk, 3);
		GUIqcheckkeys2(pl1Lk,     4);
		GUIqcheckkeys2(pl1Rk,     5);
		GUIqcheckkeys2(pl1Bk,     6);
		GUIqcheckkeys2(pl1Ak,     7);
	}

	char al = '\0';
#ifdef __UNIXSDL__
	if (GUIgetprkeys(90, 90, &al)) goto done; // UP
	if (GUIgetprkeys(96, 96, &al)) goto done; // DOWN
	if (GUIgetprkeys(92, 92, &al)) goto done; // LEFT
	if (GUIgetprkeys(94, 94, &al)) goto done; // RIGHT
	if (GUIgetprkeys(89, 89, &al)) goto done; // HOME
	if (GUIgetprkeys(91, 91, &al)) goto done; // PGUP
	if (GUIgetprkeys(95, 95, &al)) goto done; // END
	if (GUIgetprkeys(97, 97, &al)) goto done; // PGDOWN

	if (GUIgetprkeys(72, 72, &al)) goto done; // KP8
	if (GUIgetprkeys(80, 80, &al)) goto done; // KP2
	if (GUIgetprkeys(75, 75, &al)) goto done; // KP4
	if (GUIgetprkeys(77, 77, &al)) goto done; // KP6
	if (GUIgetprkeys(71, 71, &al)) goto done; // KP7
	if (GUIgetprkeys(79, 79, &al)) goto done; // KP9
	if (GUIgetprkeys(81, 81, &al)) goto done; // KP1
	if (GUIgetprkeys(73, 73, &al)) goto done; // KP3
#else
	if (GUIgetprkeys(72, 72, &al)) goto done;
	if (GUIgetprkeys(80, 80, &al)) goto done;
	if (GUIgetprkeys(75, 75, &al)) goto done;
	if (GUIgetprkeys(77, 77, &al)) goto done;
	if (GUIgetprkeys(71, 71, &al)) goto done;
	if (GUIgetprkeys(79, 79, &al)) goto done;
	if (GUIgetprkeys(81, 81, &al)) goto done;
	if (GUIgetprkeys(73, 73, &al)) goto done;
#endif
	if (GUIgetprkeysb(1, 27, &al)) goto done;
	if (GUIgetprkeys(0x1C, 13, &al)) goto done;

	if (UseExtKey == 72 || UseExtKey == 80) al = UseExtKey; // Extended key

#ifdef __UNIXSDL__
	if (GUIgetprkeys(90, 90, &al)) goto done; // UP
	if (GUIgetprkeys(96, 96, &al)) goto done; // DOWN
	if (GUIgetprkeys(92, 92, &al)) goto done; // LEFT
	if (GUIgetprkeys(94, 94, &al)) goto done; // RIGHT
	if (GUIgetprkeys(89, 89, &al)) goto done; // HOME
	if (GUIgetprkeys(91, 91, &al)) goto done; // PGUP
	if (GUIgetprkeys(95, 95, &al)) goto done; // END
	if (GUIgetprkeys(97, 97, &al)) goto done; // PGDOWN

	if (GUIgetprkeys(72, 72, &al)) goto done; // KP8
	if (GUIgetprkeys(80, 80, &al)) goto done; // KP2
	if (GUIgetprkeys(75, 75, &al)) goto done; // KP4
	if (GUIgetprkeys(77, 77, &al)) goto done; // KP6
	if (GUIgetprkeys(71, 71, &al)) goto done; // KP7
	if (GUIgetprkeys(79, 79, &al)) goto done; // KP9
	if (GUIgetprkeys(81, 81, &al)) goto done; // KP1
	if (GUIgetprkeys(73, 73, &al)) goto done; // KP3

	if (GUIgetprkeys(0x09C, 13, &al)) goto done; // ENTER
#endif
#ifdef __WIN32__
	//if (GUIgetprkeys(0xC8, 72, &al)) goto done;
	//if (GUIgetprkeys(0xD0, 80, &al)) goto done;
	if (GUIgetprkeys(0xCB, 75, &al)) goto done;
	if (GUIgetprkeys(0xCD, 77, &al)) goto done;
	if (GUIgetprkeys(0xC7, 71, &al)) goto done;
	if (GUIgetprkeys(0xCF, 79, &al)) goto done;
	if (GUIgetprkeys(0xD1, 81, &al)) goto done;
	if (GUIgetprkeys(0xC9, 73, &al)) goto done;
	if (GUIgetprkeys(0x9C, 13, &al)) goto done;
#endif

	if (JoyPad1Move != 0)
	{
#ifndef __WIN32__
		if (GUIgetprkeys2(pl1upk,    72, 0, &al)) goto done;
		if (GUIgetprkeys2(pl1downk,  80, 1, &al)) goto done;
#endif
		if (GUIgetprkeys2(pl1leftk,  75, 2, &al)) goto done;
		if (GUIgetprkeys2(pl1rightk, 77, 3, &al)) goto done;
		if (GUIgetprkeys2(pl1Lk,     73, 4, &al)) goto done;
		if (GUIgetprkeys2(pl1Rk,     81, 5, &al)) goto done;
		if (GUIgetprkeys2(pl1Bk,     27, 6, &al)) goto done;
		if (GUIgetprkeys2(pl1Ak,     13, 7, &al)) goto done;
	}

	if (dh == ' ') al = ' ';
	GUInextkeydelay = 10;
	GUIkeydelay     =  0;

done:
	if (GUIcmenupos == 0)
	{
		if (al == 27 && GUIwinptr == 0)
		{
			if (romloadskip == 0) GUIQuit = 2;
		}
		else
		{
			u4 const eax = GUIwinptr - 1;
			u4 const ebx = GUIwinorder[eax];
			if (al == 27 && (ebx != 13 || CheatWinMode <= 1))
			{ // Close window
				GUIwinorder[eax] = 0;
				GUIwinactiv[ebx] = 0;
				if (--GUIwinptr == 0) GUIcmenupos = GUIpmenupos;
			}
			else
			{
				eop* f;
				switch (ebx)
				{
					case  1: f = GUILoadKeys;        break;
					case  2: f = GUIStateSelKeys;    break;
					case  3: GUIInputKeys(dh);       return;
					case  4: GUIOptionKeys(dh);      return;
					case  5: f = GUIVideoKeys;       break;
					case  6: f = GUISoundKeys;       break;
					case  7: f = GUICheatKeys;       break;
					case 10: f = GUIGUIOptnsKeys;    break;
					case 11: GUIAboutKeys(dh);       return;
					case 12: f = GUIResetKeys;       break;
					case 13: f = GUICheatSearchKeys; break;
					case 14: f = GUIStateKeys;       break;
					case 15: f = GUIMovieKeys;       break;
					case 16: f = GUIComboKeys;       break;
					case 17: GUIAddonKeys(dh);       return;
					case 18: GUIChipKeys(dh);        return;
					case 19: f = GUIPathKeys;        break;
					case 20: GUISaveKeys(dh);        return;
					case 21: GUISpeedKeys(dh);       return;
					case  8: f = GUIGetInputLine;    break;
					default: return;
				}
				u4 eax = al;
				u4 edx = dh << 8 | al;
				asm volatile("call %A2" : "+a" (eax), "+d" (edx) : "g" (f) : "cc", "memory", "ecx", "ebx", "esi", "edi");
			}
		}
	}
	else
	{
		// Process menu
		dh = ToUpperASM(dh);

		switch (dh) // Main Menu Hotkeys
		{
			case 'X': GUIcmenupos = 0; GUIcrowpos = 0; break; // Close
			case 'R': GUIcmenupos = 1; GUIcrowpos = 0; break; // Recent
			case 'G': GUIcmenupos = 2; GUIcrowpos = 0; break; // Game
			case 'C': GUIcmenupos = 3; GUIcrowpos = 0; break; // Config
			case 'H': GUIcmenupos = 4; GUIcrowpos = 0; break; // Cheat
			case 'N': GUIcmenupos = 5; GUIcrowpos = 0; break; // Netplay
			case 'M': GUIcmenupos = 6; GUIcrowpos = 0; break; // Misc
		}

		if (GUIcmenupos == 1)
		{ // Recently Played Hotkeys
			switch (dh)
			{
				case '1': GUIcrowpos =  0; break;
				case '2': GUIcrowpos =  1; break;
				case '3': GUIcrowpos =  2; break;
				case '4': GUIcrowpos =  3; break;
				case '5': GUIcrowpos =  4; break;
				case '6': GUIcrowpos =  5; break;
				case '7': GUIcrowpos =  6; break;
				case '8': GUIcrowpos =  7; break;
				case '9': GUIcrowpos =  8; break;
				case '0': GUIcrowpos =  9; break;
				case 'F': GUIcrowpos = 11; break;
				case 'L': GUIcrowpos = 12; break;
			}
		}

		if (GUIcmenupos == 2)
		{ // Game Hotkeys
			switch (dh)
			{
				case 'L': GUIcrowpos = 0; break;
				case 'E': GUIcrowpos = 2; break;
				case 'S': GUIcrowpos = 4; break;
				case 'O': GUIcrowpos = 5; break;
				case 'P': GUIcrowpos = 6; break;
				case 'Q': GUIcrowpos = 8; break;
			}
		}

		if (GUIcmenupos == 3)
		{ // Config Hotkeys
			switch (dh)
			{
				case 'I': GUIcrowpos =  0; break;
				case 'D': GUIcrowpos =  2; break;
				case 'F': GUIcrowpos =  3; break;
				case 'O': GUIcrowpos =  5; break;
				case 'V': GUIcrowpos =  6; break;
				case 'S': GUIcrowpos =  7; break;
				case 'P': GUIcrowpos =  8; break;
				case 'A': GUIcrowpos =  9; break;
				case 'E': GUIcrowpos = 10; break;
			}
		}

		if (GUIcmenupos == 4)
		{ // Cheat Hotkeys
			switch (dh)
			{
				case 'A': GUIcrowpos = 0; break;
				case 'B': GUIcrowpos = 1; break;
				case 'S': GUIcrowpos = 2; break;
			}
		}

		if (GUIcmenupos == 6)
		{ // Misc Hotkeys
			switch (dh)
			{
				case 'K': GUIcrowpos = 0;
				case 'U': GUIcrowpos = 1;
				case 'O': GUIcrowpos = 2;
				case 'E': GUIcrowpos = 3;
				case 'S': GUIcrowpos = 4;
				case 'A': GUIcrowpos = 6;
			}
		}

		if (romloadskip == 0 && al == 27) GUIQuit = 2;

		if (al == 13) GUITryMenuItem();

		IFKEY(al, 92, 75)
		{
			GUIcrowpos  = 0;
			GUIcmenupos = GUIcmenupos > 1 ? GUIcmenupos - 1 : 6;
		}

		IFKEY(al, 94, 77)
		{
			GUIcrowpos  = 0;
			GUIcmenupos = GUIcmenupos != 6 ? GUIcmenupos + 1 : 1;
		}

		if (GUIcmenupos != 0)
		{
			IFKEY(al, 96, 80)
			{
				u4              eax = GUIcrowpos;
				u1 const* const ebx = GUICYLocPtr;
				if (ebx[eax + 1] == 2)
				{ // Bottom
					GUIcrowpos = 0;
				}
				else
				{
					do ++eax; while (ebx[eax + 1] == 0);
					GUIcrowpos = eax;
				}
				return;
			}

			IFKEY(al, 90, 72)
			{
				u4              eax = GUIcrowpos;
				u1 const* const ebx = GUICYLocPtr;
				if ((u1)eax == 0 || ebx[eax + 1] == 3)
				{ // Top
					GUIcrowpos = *ebx;
				}
				else
				{
					do --eax; while (ebx[eax + 1] == 0);
					GUIcrowpos = eax;
				}
				return;
			}
		}
	}
}
