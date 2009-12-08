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


static char const guiftimemsg8[] = "PRESS SPACEBAR TO PROCEED.";


static void guifirsttimemsg(void)
{
	static char const guiftimemsg1[] = " ONE-TIME USER REMINDER : ";
	static char const guiftimemsg2[] = "  PLEASE BE SURE TO READ  ";
	static char const guiftimemsg3[] = "THE DOCUMENTATION INCLUDED";
	static char const guiftimemsg4[] = " WITH ZSNES FOR IMPORTANT";
	static char const guiftimemsg5[] = " INFORMATION AND ANSWERS";
	static char const guiftimemsg6[] = "    TO COMMON PROBLEMS";
	static char const guiftimemsg7[] = "      AND QUESTIONS.";

	memset(pressed, 0, 256); // XXX maybe should be sizeof(pressed)
	pressed[0x2C] = 0; // XXX redundant

	do
	{
		GUIBox( 43,  75, 213, 163, 160);
		GUIBox( 43,  75, 213,  75, 162);
		GUIBox( 43,  75,  43, 163, 161);
		GUIBox(213,  75, 213, 163, 159);
		GUIBox( 43, 163, 213, 163, 158);
		GUIOuttext(52,  81, guiftimemsg1, 220 - 15);
		GUIOuttext(51,  80, guiftimemsg1, 220);
		GUIOuttext(52,  96, guiftimemsg2, 220 - 15);
		GUIOuttext(51,  95, guiftimemsg2, 220);
		GUIOuttext(52, 104, guiftimemsg3, 220 - 15);
		GUIOuttext(51, 103, guiftimemsg3, 220);
		GUIOuttext(52, 112, guiftimemsg4, 220 - 15);
		GUIOuttext(51, 111, guiftimemsg4, 220);
		GUIOuttext(52, 120, guiftimemsg5, 220 - 15);
		GUIOuttext(51, 119, guiftimemsg5, 220);
		GUIOuttext(52, 128, guiftimemsg6, 220 - 15);
		GUIOuttext(51, 127, guiftimemsg6, 220);
		GUIOuttext(52, 136, guiftimemsg7, 220 - 15);
		GUIOuttext(51, 135, guiftimemsg7, 220);
		GUIOuttext(52, 151, guiftimemsg8, 220 - 15);
		GUIOuttext(51, 150, guiftimemsg8, 220);
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
	static char const guimsgmsg[] = "     WELCOME TO ZSNES";

	memset(pressed, 0, 256); // XXX maybe should be sizeof(pressed)
	pressed[0x2C] = 0; // XXX redundant

	do
	{
		GUIBox( 43,  75, 213, 163, 160);
		GUIBox( 43,  75, 213,  75, 162);
		GUIBox( 43,  75,  43, 163, 161);
		GUIBox(213,  75, 213, 163, 159);
		GUIBox( 43, 163, 213, 163, 158);
		GUIOuttext(52, 81, guimsgmsg,   220 - 15);
		GUIOuttext(51, 80, guimsgmsg,   220);
		char const* msg = guimsgptr;
		GUIOuttext(52, 96, msg, 220 - 15);
		GUIOuttext(51, 95, msg, 220);
		msg += 32;
		GUIOuttext(52, 104, msg, 220 - 15);
		GUIOuttext(51, 103, msg, 220);
		msg += 32;
		GUIOuttext(52, 112, msg, 220 - 15);
		GUIOuttext(51, 111, msg, 220);
		msg += 32;
		GUIOuttext(52, 120, msg, 220 - 15);
		GUIOuttext(51, 119, msg, 220);
		GUIOuttext(52, 151, guiftimemsg8, 220 - 15);
		GUIOuttext(51, 150, guiftimemsg8, 220);
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
