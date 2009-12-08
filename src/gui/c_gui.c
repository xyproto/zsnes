#include <string.h>

#include "../asm_call.h"
#include "../c_init.h"
#include "../cfg.h"
#include "../cpu/c_execute.h"
#include "../cpu/dspproc.h"
#include "../cpu/execute.h"
#include "../cpu/regs.h"
#include "../effects/burn.h"
#include "../effects/smoke.h"
#include "../effects/water.h"
#include "../endmem.h"
#include "../intrf.h"
#include "../macros.h"
#include "../ui.h"
#include "../vcache.h"
#include "../video/makevid.h"
#include "../video/procvid.h"
#include "../zstate.h"
#include "../ztimec.h"
#include "c_gui.h"
#include "c_guitools.h"
#include "gui.h"
#include "guicheat.h"
#include "guifuncs.h"
#include "guikeys.h"
#include "guimouse.h"
#include "guiwindp.h"

#ifdef __MSDOS__
#	include "../dos/vesa2.h"
#endif

#ifdef __OPENGL__
#	include "../linux/sdlintrf.h"
#endif


static char const* guimsgptr;


static void loadmenuopen(u4 const param1) // XXX better parameter name
{
	GUIpmenupos = GUIcmenupos;
	GUIcmenupos = 0;
	if (GUIwinactiv[param1] != 1)
	{
		GUIwinorder[GUIwinptr++] = param1;
		GUIwinactiv[param1]      = 1;
		if (savewinpos == 0)
		{
			GUIwinposx[param1] = GUIwinposxo[param1];
			GUIwinposy[param1] = GUIwinposyo[param1];
		}
	}
	else
	{
		// look for match
		u4 i = 0;
		u1 bl; // XXX better variable name
		do bl = GUIwinorder[i++]; while (bl != param1);
		for (; i != GUIwinptr; ++i)
			GUIwinorder[i - 1] = GUIwinorder[i];
		GUIpclicked        = 0;
		GUIwinorder[i - 1] = bl;
	}
}


void GUIBox3D(u4 const x1, u4 const y1, u4 const x2, u4 const y2)
{
	GUIBox(x1, y1, x2, y2, 160);
	GUIBox(x1, y1, x2, y1, 162);
	GUIBox(x1, y1, x1, y2, 161);
	GUIBox(x2, y1, x2, y2, 159);
	GUIBox(x1, y2, x2, y2, 158);
}


void GUIOuttextShadowed(u4 const x, u4 const y, char const* const text)
{
	GUIOuttext(x + 1, y + 1, text, 220 - 15);
	GUIOuttext(x,     y,     text, 220);
}


static char const guiftimemsg8[] = "PRESS SPACEBAR TO PROCEED.";


static void guifirsttimemsg(void)
{
	memset(pressed, 0, 256); // XXX maybe should be sizeof(pressed)
	pressed[0x2C] = 0; // XXX redundant

	do
	{
		GUIBox3D(43, 75, 213, 163);
		GUIOuttextShadowed(51,  80, " ONE-TIME USER REMINDER : ");
		GUIOuttextShadowed(51,  95, "  PLEASE BE SURE TO READ  ");
		GUIOuttextShadowed(51, 103, "THE DOCUMENTATION INCLUDED");
		GUIOuttextShadowed(51, 111, " WITH ZSNES FOR IMPORTANT");
		GUIOuttextShadowed(51, 119, " INFORMATION AND ANSWERS");
		GUIOuttextShadowed(51, 127, "    TO COMMON PROBLEMS");
		GUIOuttextShadowed(51, 135, "      AND QUESTIONS.");
		GUIOuttextShadowed(51, 150, guiftimemsg8);
		asm_call(vidpastecopyscr);
		asm_call(GUIUnBuffer);
		asm_call(DisplayBoxes);
		asm_call(DisplayMenu);
		asm_call(JoyRead);
	}
	while (pressed[0x39] == 0);
}


static void horizonfixmsg(void)
{
	memset(pressed, 0, 256); // XXX maybe should be sizeof(pressed)
	pressed[0x2C] = 0; // XXX redundant

	do
	{
		GUIBox3D(43, 75, 213, 163);
		GUIOuttextShadowed(51, 80, "     WELCOME TO ZSNES");
		char const* msg = guimsgptr;
		GUIOuttextShadowed(51, 95, msg);
		msg += 32;
		GUIOuttextShadowed(51, 103, msg);
		msg += 32;
		GUIOuttextShadowed(51, 111, msg);
		msg += 32;
		GUIOuttextShadowed(51, 119, msg);
		GUIOuttextShadowed(51, 150, guiftimemsg8);
		asm_call(vidpastecopyscr);
		asm_call(GUIUnBuffer);
		asm_call(DisplayBoxes);
		asm_call(DisplayMenu);
		asm_call(JoyRead);
	}
	while (pressed[0x39] == 0);
}


void StartGUI(void)
{
	static u1 MouseInitOkay = 0;

#ifdef __OPENGL__
	if (FilteredGUI == 0 && BilinearFilter == 1) blinit = 1;
#endif
	GUILoadPos = 0;
#ifdef __MSDOS__
	if (TripBufAvail == 0) Triplebufen = 0;
#endif
	if (MMXSupport != 1 || newgfx16b == 0)
	{
		En2xSaI  = 0;
		hqFilter = 0;
	}
	if (En2xSaI != 0)
	{
#ifdef __MSDOS__
		Triplebufen = 0;
#endif
		hqFilter  = 0;
		scanlines = 0;
		antienab  = 0;
	}
	if (hqFilter != 0)
	{
		En2xSaI   = 0;
		scanlines = 0;
		antienab  = 0;
	}

	memset(SpecialLine, 0, sizeof(SpecialLine));

	GUIOn    = 1;
	GUIOn2   = 1;
	NumCombo = GUIComboGameSpec != 0 ? NumComboLocl : NumComboGlob;
#ifdef __MSDOS__
	asm_call(ResetTripleBuf);
#endif

	if (GUIwinposx[16] == 0)
	{
		GUIwinposx[16] =  3;
		GUIwinposy[16] = 22;
	}

	GUICTimer = 0;
	{ // Initialize volume
		u4 vol = (u4)MusicRelVol * 128 / 100;
		if (vol > 127) vol = 127;
		MusicVol = vol;
	}
	CheatSearchStatus = 0;
	if (newgfx16b != 0) memset(vidbufferofsb, 0, 256 * 144 * 4);
	ShowTimer = 1;
	if ((GetDate() & 0xFFFF) == 0x0C25) OkaySC = 1;
	lastmouseholded = 1;
	if (GUIwinposx[15] == 0)
	{ // Movie menu fix
		GUIwinposx[15] = 50;
		GUIwinposy[15] = 50;
	}
	PrevResoln = resolutn;
	resolutn   = 224;

	GUIPalConv   = 0;
	MousePRClick = 1;

	if (MouseInitOkay != 1)
	{
		MouseInitOkay = 1;
		if (MouseDis != 1)
		{
			u4 res;
			asm volatile("call %P1" : "=a" (res) : "X" (Init_Mouse) : "cc", "memory", "edx", "ecx");
			if (res == 0) MouseDis = 1;
		}
	}

	if (pressed[KeyQuickLoad] & 1)
	{
		GUIcmenupos = 0;
		loadmenuopen(1);
	}
	memset(pressed, 0, 256 + 128 + 32); // XXX 32 probably should be 64
	pressed[1]  = 2;
	GUIescpress = 1;

	// set Video cursor location
	u4 eax = cvidmode;
	GUIcurrentvideocursloc = eax;
	u4 ebx = NumVideoModes;
	if (ebx > 20)
	{
		ebx -= 20;
		if (eax > ebx) eax = ebx;
		GUIcurrentvideoviewloc = eax;
	}
	else
	{
		GUIcurrentvideoviewloc = 0;
	}

	SaveSramData();
	GUIQuickLoadUpdate();

	asm_call(LoadDetermine);

	if (AutoState != 0 && romloadskip == 0) SaveSecondState();

	asm_call(GUIInit);
	memset(pressed, 0, 256); // XXX probably + 128 + 64 missing, maybe even completely redundant (has been zeroed above)

	if (GUIwinptr != 0)
	{
		GUIcmenupos = 0;
	}
	else if (esctomenu != 0)
	{
		GUIcmenupos = 2;
		GUIcrowpos  = 0;
		GUICYLocPtr = MenuDat2;
		if (esctomenu != 1) GUIcmenupos = 0;
	}
	if (GUIwinactiv[1] != 0)
	{
		GUIcurrentfilewin = 0;
		GetLoadData();
	}
	GUIHold = 0;
	// clear 256 bytes from hirestiledat
	memset(hirestiledat, 0, sizeof(hirestiledat));
	curblank = 0;
	asm_call(InitGUI);

	if (CheatWinMode != 0) LoadCheatSearchFile();

	GUIQuit = 0;
	while (GUIQuit != 2)
	{
		if (GUIQuit == 1)
		{
			asm_call(GUIDeInit);

			resolutn = PrevResoln;
			asm_call(endprog);
			return;
		}
		GUIQuit = 0;
		if (MouseDis != 1)
		{
			asm_call(ProcessMouse);
			if (videotroub == 1) return;
		}
		asm_call(GUIUnBuffer);
		if (GUIEffect == 1) asm_call(DrawSnow);
		if (GUIEffect == 2) DrawWater();
		if (GUIEffect == 3) DrawWater();
		if (GUIEffect == 4) DrawBurn();
		if (GUIEffect == 5) DrawSmoke();

		if (GUIEditStringcWin != 0)
		{
			u1* eax = GUIEditStringcLen;
			if (eax)
			{
				if (GUIEditStringLTxt >= 8)
				{
					eax[0]            = '_';
					eax[1]            = '\0';
					GUIEditStringLstb = 1;
				}
				if (GUIEditStringLTxt == 0) GUIEditStringLTxt = 16;
			}
		}

		asm_call(DisplayBoxes);

		if (GUIEditStringLstb == 1)
		{
			GUIEditStringLstb    = 0;
			GUIEditStringcLen[0] = '\0';
		}

		asm_call(DisplayMenu);
		if (MouseDis != 1) asm_call(DrawMouse);
		if (FirstTimeData == 0)
		{
			guifirsttimemsg();
			FirstTimeData = 1;
		}
		if (!guimsgptr && (GetDate() & 0xFFFF) == 0x0401)
		{
			guimsgptr = (char const*)horizon_get(GetTime());
			horizonfixmsg();
		}
		if (GUICTimer != 0)
		{
			GUIOuttext(21, 211, GUICMessage, 50);
			GUIOuttext(20, 210, GUICMessage, 63);
		}
		asm_call(vidpastecopyscr);
		asm_call(GUIgetcurrentinput);
	}
	memset(spcBuffera, 0, 256 * 1024);
	asm_call(GUIDeInit);
#ifdef __MSDOS__
	asm_call(DOSClearScreen);
	if (cbitmode == 0) asm_call(dosmakepal);
#endif
	t1cc = 1;

	GUISaveVars();

	MousePRClick = 1;
	prevbright   = 0;
	resolutn     = PrevResoln;

	CheatOn = NumCheats != 0;

	if (CopyRamToggle == 1)
	{
		CopyRamToggle = 0;
		// copy 128k ram
		memcpy(vidbuffer + 129600, wramdata, 128 * 1024);
	}

	if (CheatWinMode == 2) CheatWinMode = 1;

	if (CheatWinMode != 0) SaveCheatSearchFile();

	memset(vidbuffer, 0, 288 * 120 * 4);

	memset(vidbufferofsb, 0, 256 * 144 * 4);

	asm_call(AdjustFrequency);
	GUIOn    = 0;
	GUIOn2   = 0;
	GUIReset = 0;
	StartLL  = 0;
	StartLR  = 0;
	continueprog();
}


void guimencodermsg(void)
{
	memset(pressed, 0, 256); // XXX maybe should be sizeof(pressed)
	pressed[0x2C] = 0; // XXX redundant

	do
	{
		GUIBox3D(43, 75, 213, 163);
		GUIOuttextShadowed(51,  95, " MENCODER IS MISSING: ");
		GUIOuttextShadowed(51, 133, "PRESS SPACE TO PROCEED");
		asm_call(vidpastecopyscr);
		asm_call(GUIUnBuffer);
		asm_call(DisplayBoxes);
		asm_call(DisplayMenu);
		asm_call(JoyRead);
	}
	while (pressed[0x39] == 0);
}


void guilamemsg(void)
{
	memset(pressed, 0, 256); // XXX maybe should be sizeof(pressed)
	pressed[0x2C] = 0; // XXX redundant

	do
	{
		GUIBox3D(43, 75, 213, 163);
		GUIOuttextShadowed(51, 95, " LAME IS MISSING: ");
		GUIOuttextShadowed(51,133, "PRESS SPACE TO PROCEED");
		asm_call(vidpastecopyscr);
		asm_call(GUIUnBuffer);
		asm_call(DisplayBoxes);
		asm_call(DisplayMenu);
		asm_call(JoyRead);
	}
	while (pressed[0x39] == 0);
}


static u1* GetAnyPressedKey(void)
{
	for (u1* i = pressed; i != endof(pressed); ++i)
		if (*i != 0) return i;
	return 0;
}


static u4 GetMouseButtons(void)
{
	if (MouseDis == 1) return 0;

	u4 ebx;
	asm("call *%1" : "=b" (ebx) : "r" (Get_MouseData) : "cc", "memory", "ecx", "edx");
	if (lhguimouse == 1)
		asm("call *%1" : "+b" (ebx) : "r" (SwapMouseButtons) : "cc");
	return ebx;
}


void guiprevideo(void)
{
	memset(pressed, 0, 256); // XXX maybe should be sizeof(pressed)

	asm_call(GUIUnBuffer);
	asm_call(DisplayBoxes);
	asm_call(DisplayMenu);
	GUIBox3D(43, 90, 213, 163);
	GUIOuttextShadowed(55,  95, "ZSNES WILL NOW ATTEMPT");
	GUIOuttextShadowed(55, 103, " TO CHANGE YOUR VIDEO");
	GUIOuttextShadowed(55, 111, " MODE.  IF THE CHANGE");
	GUIOuttextShadowed(55, 119, "IS UNSUCCESSFUL,  WAIT");
	GUIOuttextShadowed(55, 127, " 10 SECONDS AND VIDEO");
	GUIOuttextShadowed(55, 135, "MODE WILL BE RESTORED.");
	GUIOuttextShadowed(55, 150, "    PRESS ANY KEY.");
	asm_call(vidpastecopyscr);
	pressed[0x2C] = 0; // XXX redundant
	for (;;)
	{
		asm_call(JoyRead);

		u1* const key = GetAnyPressedKey();
		if (key)
		{
			*key = 0;
			return;
		}
		if (GetMouseButtons() & 0x01) return;
	}
}


void guicheaterror(void)
{
	memset(pressed, 0, sizeof(pressed));

	for (;;)
	{
		asm_call(GUIUnBuffer);
		asm_call(DisplayBoxes);
		asm_call(DisplayMenu);
		GUIBox3D(75, 95, 192, 143);
		GUIOuttextShadowed(80, 100, "INVALID CODE!  YOU");
		GUIOuttextShadowed(80, 108, "MUST ENTER A VALID");
		GUIOuttextShadowed(80, 116, "GAME GENIE,PAR, OR");
		GUIOuttextShadowed(80, 124, "GOLD FINGER CODE.");
		GUIOuttextShadowed(80, 134, "PRESS ANY KEY.");
		asm_call(vidpastecopyscr);
		asm_call(JoyRead);

		if (GetAnyPressedKey())       break;
		if (GetMouseButtons() & 0x01) break;
	}
	while ((u1)Check_Key() != 0) // XXX asm_call
		asm_call(Get_Key);
	GUIcurrentcheatwin = 1;
	GUIpclicked        = 1;
}


void GUIMenuDisplay(u4 const n_cols, u4 n_rows, u1* dst, char const* text)
{
	u4 row = 0;
	do
	{
		u1 const al = *text;
		if (al != '\0')
		{
			++text;
			if (al != 2)
			{
				GUItextcolor[0] = 44;
				if (GUIcrowpos != row)
					GUIOutputString(dst + 289, text);
				GUItextcolor[0] = 63;
			}
			else
			{
				GUItextcolor[0] = 42;
				if (GUIcrowpos != row)
					GUIOutputString(dst + 289, text);
				GUItextcolor[0] = 57;
			}
			text = GUIOutputString(dst, text) + 1;
		}
		else
		{
			u1* d    = dst + 4 * 288;
			u4  cols = n_cols;
			do
			{
				d[   0] = 45;
				d[-289] = 40;
				d[ 289] = 42;
				++d;
			}
			while (--cols != 0);
			text += 14;
		}
		dst += 10 * 288;
		++row;
	}
	while (--n_rows != 0);
}
