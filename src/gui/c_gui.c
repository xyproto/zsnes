#include <string.h>

#include "../asm.h"
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
#include "../video/mode716.h"
#include "../video/procvid.h"
#include "../zstate.h"
#include "../ztimec.h"
#include "c_gui.h"
#include "c_guitools.h"
#include "gui.h"
#include "guicheat.h"
#include "guifuncs.h"
#include "guikeys.h"
#include "guimisc.h"
#include "guimouse.h"
#include "guiwindp.h"

#ifdef __MSDOS__
#	include "../dos/vesa2.h"
#endif

#ifdef __OPENGL__
#	include "../linux/sdlintrf.h"
#endif

#ifdef __WIN32__
#	include "../win/winlink.h"
#endif


static u4 SantaNextT = 36 * 15;
u4        NumSnow;
u4        SnowTimer  = 36 * 30;

u1        savecfgforce;

// The first byte is the number of fields on the right not including the seperators
static u1 MenuDat1[] = { 12, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 2, 0 };
static u1 MenuDat2[] = {  8, 3, 1, 1, 0, 1, 1, 1, 0, 2, 0 };
static u1 MenuDat3[] = { 10, 3, 0, 1, 1, 0, 1, 1, 1, 1, 1, 2, 0 };
static u1 MenuDat4[] = {  2, 3, 1, 2, 0 };
#ifndef __MSDOS__
static u1 MenuDat5[] = {  0, 2, 0, 0 };
#else
static u1 MenuDat5[] = {  1, 3, 2, 0 };
#endif
static u1 MenuDat6[] = {  6, 3, 1, 1, 1, 1, 0, 2, 0 };

static char const* guimsgptr;
static u1          OkaySC;
static u4          SnowMover;


static char GUIGameMenuData[][14] =
{
	{ "\x1" "LOAD        " },
	{ "\x1" "RUN  [ESC]  " },
	{ "\x1" "RESET       " },
	{ "\x0" "------------" },
	{ "\x1" "SAVE STATE  " },
	{ "\x1" "OPEN STATE  " },
	{ "\x1" "PICK STATE  " },
	{ "\x0" "------------" },
	{ "\x1" "QUIT        " }
};

static char GUIConfigMenuData[][14] =
{
	{ "\x1" "INPUT       " },
	{ "\x0" "------------" },
	{ "\x1" "DEVICES     " },
	{ "\x1" "CHIP CFG    " },
	{ "\x0" "------------" },
	{ "\x1" "OPTIONS     " },
	{ "\x1" "VIDEO       " },
	{ "\x1" "SOUND       " },
	{ "\x1" "PATHS       " },
	{ "\x1" "SAVES       " },
	{ "\x1" "SPEED       " }
};

static char GUICheatMenuData[][14] =
{
	{ "\x1" "ADD CODE    " },
	{ "\x1" "BROWSE      " },
	{ "\x1" "SEARCH      " }
};

static char GUINetPlayMenuData[][14] =
{
#ifndef __MSDOS__
	{ "\x1" "INTERNET    " },
	{ "\x0" "------------" }
#else
	{ "\x1" "MODEM       " },
	{ "\x1" "IPX         " }
#endif
};

static char GUIMiscMenuData[][14] =
{
	{ "\x1" "MISC KEYS   " },
	{ "\x1" "GUI OPTS    " },
	{ "\x1" "MOVIE OPT   " },
	{ "\x1" "KEY COMB.   " },
	{ "\x1" "SAVE CFG    " },
	{ "\x0" "------------" },
	{ "\x1" "ABOUT       " }
};


void GUIinit18_2hz(void)
{
	outb(0x43, 0x36);
	outb(0x40, 0x00);
	outb(0x40, 0x00);
}


void GUIinit36_4hz(void)
{
	outb(0x43, 0x36);
	outb(0x40, 0x00);
	outb(0x40, 0x80);
}


void GUI36hzcall(void)
{
	++GUIt1cc;
	++SnowMover;
	if (GUIEditStringLTxt != 0) --GUIEditStringLTxt;
	if (GUIScrolTim1      != 0) --GUIScrolTim1;
	if (GUIDClickTL       != 0) --GUIDClickTL;
	if (GUIkeydelay       != 0) --GUIkeydelay;
	if (GUIkeydelay2      != 0) --GUIkeydelay2;
	if (GUICTimer         != 0) --GUICTimer;
	GUICCFlash = (GUICCFlash + 1) & 0x0F;
	GUILDFlash = (GUILDFlash + 1) & 0x0F;
}


static void LoadDetermine(void)
{
	GUINetPlayMenuData[0][0] = 2; // Gray out Netplay options
#ifdef __MSDOS__
	GUINetPlayMenuData[1][0] = 2;
#endif
	u1 const v = romloadskip != 0 ? 2 : 1;
	GUIGameMenuData[   1][0] = v;
	GUIGameMenuData[   2][0] = v;
	GUIGameMenuData[   4][0] = v;
	GUIGameMenuData[   5][0] = v;
	GUIGameMenuData[   6][0] = v;
	GUICheatMenuData[  0][0] = v;
	GUICheatMenuData[  1][0] = v;
	GUICheatMenuData[  2][0] = v;
	GUIMiscMenuData[   2][0] = v;
}


static void ProcessSnowVelocity(void)
{
	if (MsgGiftLeft != 0) --MsgGiftLeft;

	if (NumSnow == 200)
	{
		if (SantaNextT != 0)
		{
			--SantaNextT;
		}
		else if (--SantaPos == 0)
		{
			SantaPos   = 272;
			SantaNextT = 36 * 60;
		}
	}
	else
	{
		if (--SnowTimer == 0)
		{
			++NumSnow;
			SnowTimer = 18;
		}
	}

	u4 i = 0;
	u4 n = NumSnow;
	while (n-- != 0)
	{
		SnowData[i * 2]     += SnowVelDist[i * 2] + 4 * (u1)(100 - MusicRelVol);
		SnowData[i * 2 + 1] += SnowVelDist[i * 2 + 1] + 256;
		if (SnowData[i * 2 + 1] <= 0x200)
			SnowVelDist[i * 2] |= 8;
		++i;
	}
}


static void DrawSnow(void)
{
	static u1 const SantaData[] =
	{
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,
		1,0,0,1,0,0,1,0,0,0,1,1,1,0,1,1,
		1,1,0,1,1,0,1,1,0,1,0,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,
		1,1,0,1,1,0,1,1,0,0,1,1,1,1,1,1
	};

	if (OkaySC != 0)
	{
		if (MsgGiftLeft != 0)
		{
			GUItextcolor[0] = 228;
			GUIOuttextwin(20, 210, "A GIFT TO YOU IN THE OPTIONS!");
		}
		u1*       dst = vidbuffer + SantaPos + 60 * 288;
		u1 const* src = SantaData;
		u4        h   = 8;
		do
		{
			u4 w = 16;
			do
			{
				if (*src != 0) *dst = 0;
				++dst;
				++src;
			}
			while (--w != 0);
			dst += 288 - 16;
		}
		while (--h != 0);
	}

	u1* const dst = vidbuffer;
	u4        i   = 0;
	do
	{
		u4 const eax = (SnowData[i * 2 + 1] >> 8) * 288 + (SnowData[i * 2] >> 8) + 16;
		if ((SnowVelDist[i * 2] & 8) != 0)
			dst[eax] = 228 + (SnowVelDist[i * 2] & 0x03);
	}
	while (++i != 200);
	// Change Snow Displacement Values
	for (; SnowMover != 0; --SnowMover)
		ProcessSnowVelocity();
}


static void guipostvideoloop(void)
{
	do
	{
		asm_call(GUIUnBuffer);
		DisplayBoxes();
		DisplayMenu();
		GUIBox3D(43, 90, 213, 163);
		GUIOuttextShadowed(55,  95, "VIDEO MODE CHANGED.");
		GUIOuttextShadowed(55, 150, "  PRESS SPACEBAR.");
		asm_call(vidpastecopyscr);
		// Wait for all mouse and input data to be 0

		if (GUIkeydelay == 0) break;

		// This is to make all ports not register space bar from being pressed earlier
		pressed[0x2C] = 0;

		asm_call(JoyRead);
	}
	while (pressed[0x39] == 0);
	GUIpclicked = 1;
}


void guipostvideo(void)
{
	memset(vidbufferofsb, 0xFF, 256 * 144 * 4);
	GUIkeydelay = 36 * 10;
	guipostvideoloop();
}


#ifdef __MSDOS__
void guipostvideofail(void)
{
	char guipostvidmsg3b[3][26];
	guipostvidmsg3b[0][0] = '\0';
	guipostvidmsg3b[1][0] = '\0';
	guipostvidmsg3b[2][0] = '\0';

	char const* src = ErrorPointer;
	char const* end = src;
	while (*end != '\0' && *end != '$') ++end;
	char (*guipostvidptr)[26] = guipostvidmsg3b;
	for (;; ++src, ++guipostvidptr)
	{
		u4 n = end - src;
		for (;;)
		{
			if (n == 0) goto notext;
			if (n < lengthof(*guipostvidptr)) break;
			while (src[--n] != ' ') {} // XXX potential buffer underrun
		}
		char* dst = *guipostvidptr;
		do
		{
			char c = *src++;
			if (c == '$') c = '\0';
			*dst++ = c;
		}
		while (--n != 0);
		*dst = '\0';
		if (*src == '\0' || *src == '$') break; // XXX buffer overrun if there are more than 3 lines of text
	}
notext:

	memset(pressed, 0, 256); // XXX maybe should be sizeof(pressed)
	asm_call(GUIUnBuffer);
	DisplayBoxes();
	DisplayMenu();
	GUIBox3D(43, 90, 213, 163);
	GUIOuttextShadowed(55,  95, "VIDEO MODE CHANGE FAILED.");
	GUIOuttextShadowed(55, 107, "UNABLE TO INIT VESA2:");
	GUIOuttextShadowed(55, 118, guipostvidmsg3b[0]);
	GUIOuttextShadowed(55, 128, guipostvidmsg3b[1]);
	GUIOuttextShadowed(55, 138, guipostvidmsg3b[2]);
	GUIOuttextShadowed(55, 151, "PRESS ANY KEY");
	asm_call(vidpastecopyscr);
	asm_call(GUIUnBuffer);
	DisplayBoxes();
	DisplayMenu();
	GUIkeydelay = 0xFFFFFFFF;
	guipostvideoloop();
}
#endif


void CheckMenuItemHelp(u4 const id)
{
	GUIpmenupos = GUIcmenupos;
	GUIcmenupos = 0;
	if (GUIwinactiv[id] != 1)
	{
		GUIwinorder[GUIwinptr++] = id;
		GUIwinactiv[id] = 1;
		if (savewinpos == 0)
		{
			GUIwinposx[id] = GUIwinposxo[id];
			GUIwinposy[id] = GUIwinposyo[id];
		}
	}
	else
	{
		u4 i = 0;
		// look for match
		while (GUIwinorder[i] != id) ++i;
		for (; ++i != GUIwinptr;)
		{
			GUIwinorder[i - 1] = GUIwinorder[i];
		}
		GUIpclicked = 0;
		GUIwinorder[i - 1] = id;
	}
}


static void GUICheckMenuItem(u4 const id, u4 const row)
{
	if (GUIcrowpos == row) CheckMenuItemHelp(id);
}


// Defines which menu item calls what window number
void GUITryMenuItem(void)
{
	if (GUIcmenupos == 1)
	{
		if (GUIcrowpos < 10)
		{
			loadquickfname(GUIcrowpos);
			return;
		}
		if (GUIcrowpos == 11)
		{
			prevlfreeze ^= 1;
			memcpy(GUIPrevMenuData + 347, prevlfreeze != 0 ? " ON " : " OFF", 4);
		}
		if (GUIcrowpos == 12 && prevlfreeze == 0)
		{
			memset(prevloadiname,  ' ',  sizeof(prevloadiname));
			memset(prevloaddnamel, '\0', sizeof(prevloaddnamel));
			memset(prevloadfnamel, '\0', sizeof(prevloadfnamel));
			GUIQuickLoadUpdate();
			return;
		}
	}
	if (GUIcmenupos == 2)
	{
		GUICheckMenuItem(1, 0); // Load
		if (GUIcrowpos == 0)
		{
			GUIcurrentfilewin = 0;
			GetLoadData();
			return;
		}
		if (romloadskip == 0)
		{
			if (GUIcrowpos == 1)
			{ // Run
				GUIQuit = 2;
				return;
			}
			GUICheckMenuItem(12, 2); // Reset
			if (GUIcrowpos == 2)
			{
				GUICResetPos = 1;
			}
			if (GUIcrowpos == 4)
			{
				GUIStatesText5 = 0;
				GUICStatePos   = 1;
			}
			if (GUIcrowpos == 5)
			{
				GUIStatesText5 = 1;
				GUICStatePos   = 1;
			}
			GUICheckMenuItem(14, 4); // Save State
			GUICheckMenuItem(14, 5); // Load State
			GUICheckMenuItem( 2, 6); // Select State
		}
		if (GUIcrowpos == 8) GUIQuit = 1;
	}
	if (GUIcmenupos == 3)
	{
		// The number on the left is the window to open
		// the number on the right is where in the drop down box we are
		GUICheckMenuItem( 3, 0); // Input #1-5
		GUICheckMenuItem(17, 2); // Devices
		GUICheckMenuItem(18, 3); // Chip Config
		GUICheckMenuItem( 4, 5); // Options
		if (GUIcrowpos == 6) // Video
		{
			// set Video cursor location
			u4 const v = cvidmode;
			u4 const n = NumVideoModes;
			GUIcurrentvideocursloc = v;
			GUIcurrentvideoviewloc =
				n <= 20    ? 0      :
				n - 20 < v ? n - 20 :
				v;
			CheckMenuItemHelp(5);
		}
		GUICheckMenuItem( 6, 7); // Sound
		GUICheckMenuItem(19, 8); // Paths
		GUICheckMenuItem(20, 9); // Saves
		GUICheckMenuItem(21,10); // Speed
	}
	if (romloadskip == 0 && GUIcmenupos == 4)
	{
		GUICheckMenuItem( 7, 0);
		GUICheckMenuItem( 7, 1);
		GUICheckMenuItem(13, 2);
		if (GUIcrowpos == 0) GUIcurrentcheatwin = 1;
		if (GUIcrowpos == 1) GUIcurrentcheatwin = 0;
	}
	if (GUIcmenupos == 6)
	{
		GUICheckMenuItem( 9, 0);
		GUICheckMenuItem(10, 1);
		if (romloadskip == 0)
		{
			GUICheckMenuItem(15, 2);
			if (GUIcrowpos == 2) MovieRecordWinVal = 0;
		}
		GUICheckMenuItem(16, 3); // Save Config
		if (GUIcrowpos == 4)
		{
			savecfgforce = 1;
			GUISaveVars();
			savecfgforce = 0;

			asm_call(Makemode7Table);
			GUICMessage = "CONFIGURATION FILES SAVED.";
			GUICTimer   = 50;
		}
		GUICheckMenuItem(11, 6);
	}
}


void GUIProcStates(void)
{
	GUIwinactiv[14]          = 0;
	GUIwinorder[--GUIwinptr] = 0;
	u1 const h = GUICBHold;
	GUICBHold = 0;
	if (h != 10) return;
	if (GUIStatesText5 != 1)
	{
		statesaver();
	}
	else
	{
		loadstate2();
	}
}


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
		DisplayBoxes();
		DisplayMenu();
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
		DisplayBoxes();
		DisplayMenu();
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

	LoadDetermine();

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
		if (GUIEffect == 1) DrawSnow();
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

		DisplayBoxes();

		if (GUIEditStringLstb == 1)
		{
			GUIEditStringLstb    = 0;
			GUIEditStringcLen[0] = '\0';
		}

		DisplayMenu();
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
		DisplayBoxes();
		DisplayMenu();
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
		DisplayBoxes();
		DisplayMenu();
		asm_call(JoyRead);
	}
	while (pressed[0x39] == 0);
}


u1* GetAnyPressedKey(void)
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
	DisplayBoxes();
	DisplayMenu();
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
		DisplayBoxes();
		DisplayMenu();
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


// Displays window when item is clicked
void DisplayBoxes(void)
{
	{ u4 i = 0;
		while (GUIwinorder[i] != 0) ++i;
		cwindrawn = i - 1;
	}
	for (u1 const* i = GUIwinorder;; --cwindrawn, ++i)
	{
		switch (*i)
		{
			case  0: return;
			case  1: if (GUIReset != 1) asm_call(DisplayGUILoad); break;
			case  2: asm_call(DisplayGUIChoseSave);  break;
			case  3: asm_call(DisplayGUIInput);      break;
			case  4: asm_call(DisplayGUIOption);     break;
			case  5: asm_call(DisplayGUIVideo);      break;
			case  6: asm_call(DisplayGUISound);      break;
			case  7: asm_call(DisplayGUICheat);      break;
			case  8: asm_call(DisplayNetOptns);      break;
			case  9: asm_call(DisplayGameOptns);     break;
			case 10:
				asm_call(DisplayGUIOptns);
#ifdef __WIN32__
				CheckAlwaysOnTop();
#endif
				break;
			case 11: asm_call(DisplayGUIAbout);      break;
			case 12: asm_call(DisplayGUIReset);      break;
			case 13: asm_call(DisplayGUISearch);     break;
			case 14: asm_call(DisplayGUIStates);     break;
			case 15: asm_call(DisplayGUIMovies);     break;
			case 16: asm_call(DisplayGUICombo);      break;
			case 17: asm_call(DisplayGUIAddOns);     break;
			case 18: asm_call(DisplayGUIChipConfig); break;
			case 19: asm_call(DisplayGUIPaths);      break;
			case 20: asm_call(DisplayGUISave);       break;
			case 21: asm_call(DisplayGUISpeed);      break;
		}
	}
}


static void GUIDMHelp(u4 const x1, u4 const x2, char const* const text, u4 const param4)
{
	GUItextcolor[0] = 46;
	GUItextcolor[1] = 42;
	GUItextcolor[2] = 38;
	GUItextcolor[3] = 44;
	GUItextcolor[4] = 40;
	if (GUIcmenupos == param4)
	{
		GUItextcolor[0] = 38;
		GUItextcolor[1] = 40;
		GUItextcolor[2] = 46;
		GUItextcolor[3] = 40;
		GUItextcolor[4] = 44;
	}
	GUIBox(x1,  3, x2,  3, GUItextcolor[0]);
	GUIBox(x1,  4, x2, 12, GUItextcolor[1]);
	GUIBox(x1, 13, x2, 13, GUItextcolor[2]);
	GUIBox(x1,  3, x1, 12, GUItextcolor[3]);
	GUIBox(x2,  4, x2, 13, GUItextcolor[4]);
	GUIOuttext(x1 + 5, 7, text, 44);
	GUIOuttext(x1 + 4, 6, text, 62);
}


static void GUIDMHelpB(u4 const x1, u4 const x2, char const* const text, u4 const param4)
{
	GUItextcolor[0] = 46;
	GUItextcolor[1] = 42;
	GUItextcolor[2] = 38;
	GUItextcolor[3] = 44;
	GUItextcolor[4] = 40;
	if (GUIcwinpress == param4)
	{
		GUItextcolor[0] = 38;
		GUItextcolor[1] = 40;
		GUItextcolor[2] = 46;
		GUItextcolor[3] = 40;
		GUItextcolor[4] = 44;
	}
	GUIBox(x1,  3, x2,  3, GUItextcolor[0]);
	GUIBox(x1,  4, x2, 13, GUItextcolor[1]);
	GUIBox(x1, 14, x2, 14, GUItextcolor[2]);
	GUIBox(x1,  3, x1, 13, GUItextcolor[3]);
	GUIBox(x2,  4, x2, 14, GUItextcolor[4]);
	GUIOuttext(x1 + 3, 7, text, 44);
	GUIOuttext(x1 + 2, 6, text, 62);
}


#ifdef __WIN32__
static void GUIDMHelpB2(u4 const x1, u4 const x2, char const* const text, u4 const param4)
{
	GUItextcolor[0] = 46;
	GUItextcolor[1] = 42;
	GUItextcolor[2] = 38;
	GUItextcolor[3] = 44;
	GUItextcolor[4] = 40;
	if (GUIcwinpress == param4)
	{
		GUItextcolor[0] = 38;
		GUItextcolor[1] = 40;
		GUItextcolor[2] = 46;
		GUItextcolor[3] = 40;
		GUItextcolor[4] = 44;
	}
	GUIBox(x1, 3, x2, 3, GUItextcolor[0]);
	GUIBox(x1, 4, x2, 6, GUItextcolor[1]);
	GUIBox(x1, 7, x2, 7, GUItextcolor[2]);
	GUIBox(x1, 3, x1, 6, GUItextcolor[3]);
	GUIBox(x2, 4, x2, 7, GUItextcolor[4]);
	GUIOuttext(x1 + 3, 5, text, 44);
	GUIOuttext(x1 + 2, 4, text, 62);
}


static void GUIDMHelpB3(u4 const x1, u4 const x2, char const* const text, u4 const param4)
{
	GUItextcolor[0] = 46;
	GUItextcolor[1] = 42;
	GUItextcolor[2] = 38;
	GUItextcolor[3] = 44;
	GUItextcolor[4] = 40;
	if (GUIcwinpress == param4)
	{
		GUItextcolor[0] = 38;
		GUItextcolor[1] = 40;
		GUItextcolor[2] = 46;
		GUItextcolor[3] = 40;
		GUItextcolor[4] = 44;
	}
	GUIBox(x1,  9, x2,  9, GUItextcolor[0]);
	GUIBox(x1, 10, x2, 12, GUItextcolor[1]);
	GUIBox(x1, 13, x2, 13, GUItextcolor[2]);
	GUIBox(x1,  9, x1, 12, GUItextcolor[3]);
	GUIBox(x2, 10, x2, 13, GUItextcolor[4]);
	GUIOuttext(x1 + 3, 11, text, 44);
	GUIOuttext(x1 + 2, 10, text, 62);
}
#endif


static void GUIMenuDisplay(u4 const n_cols, u4 n_rows, u1* dst, char const* text)
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


static void GUIDrawMenuM(u4 const x1, u4 const y1, u4 const p3, u4 const p4, char const* const text, u4 const p6, u4 const p7, u4 const p8, u4 const p9, u4 const p10)
{
	GUIShadow(p7, p8, p7 + 4 + p3 * 6, p8 + 3 + p4 * 10);
	GUIBox(x1, y1, x1 + 4 + p3 * 6, y1 + 3 + p4 * 10, 43);

	u1* dst = vidbuffer + GUIcrowpos * 2880 + x1 + 17 + 18 * 288;
	GUIDrawBox(dst,           6 * p3 + 3, 1, 73);
	GUIDrawBox(dst + 288,     6 * p3 + 3, 7, 72);
	GUIDrawBox(dst + 288 * 8, 6 * p3 + 3, 1, 73);

	GUIBox(x1 + p10,        y1,     x1 + 4 + p3 * 6, y1, 47);
	GUIBox(x1,              y1,     x1,              p9, 45);
	GUIBox(x1,              p9,     x1 + 4 + p3 * 6, p9, 39);
	GUIBox(x1 + 4 + p3 * 6, 1 + y1, x1 + 4 + p3 * 6, p9, 41);
	GUIMenuDisplay(6 * p3, p4, vidbuffer + 16 + p6 + 20 * 288, text);

	GUIMenuL = x1 + 1;
	GUIMenuR = x1 + 6 * p3 + 3;
	GUIMenuD = 18 + p4 * 10;
}


void DisplayMenu(void)
{
	// Draw Shadow
	GUIShadow(5, 7, 235, 21);
	// Display Top Border
	GUIBox(0,  1, 229,  1, 71);
	GUIBox(0,  2, 229,  2, 70);
	GUIBox(0,  3, 229,  3, 69);
	GUIBox(0,  4, 229,  4, 68);
	GUIBox(0,  5, 229,  5, 67);
	GUIBox(0,  6, 229,  6, 66);
	GUIBox(0,  7, 229,  7, 65);
	GUIBox(0,  8, 229,  8, 64);
	GUIBox(0,  9, 229,  9, 65);
	GUIBox(0, 10, 229, 10, 66);
	GUIBox(0, 11, 229, 11, 67);
	GUIBox(0, 12, 229, 12, 68);
	GUIBox(0, 13, 229, 13, 69);
	GUIBox(0, 14, 229, 14, 70);
	GUIBox(0, 15, 229, 15, 71);

#ifdef __UNIXSDL__
	GUIShadow(238, 9, 247, 20);
	GUIShadow(249, 9, 257, 20);
#endif
#ifdef __WIN32__
	GUIShadow(238,  9, 247, 14);
	GUIShadow(238, 16, 247, 20);
	GUIShadow(249,  9, 257, 20);
#endif

#ifdef __UNIXSDL__
	GUIMenuItem[36] = 247;
	GUIDMHelpB(233, 242, GUIMenuItem + 36, 1);
	GUIMenuItem[36] = 'x';
	GUIDMHelpB(244, 253, GUIMenuItem + 36, 2);
#endif

#ifdef __WIN32__
	GUIMenuItem[36] = 249;
	GUIDMHelpB2(233, 242, GUIMenuItem + 36, 1);
	GUIMenuItem[36] = 248;
	GUIDMHelpB3(233, 242, GUIMenuItem + 36, 3);
	GUIMenuItem[36] = 'x';
	GUIDMHelpB( 244, 253, GUIMenuItem + 36, 2);
#endif

	// Display upper-left box
	GUIMenuItem[36] = 25;
	GUIDMHelp(4, 12, GUIMenuItem + 6, 1);
	GUIOuttext(4 + 3, 7, GUIMenuItem + 36, 44);
	GUIOuttext(4 + 2, 6, GUIMenuItem + 36, 62);
	// Display boxes
	GUIDMHelp( 17,  47, GUIMenuItem,      2);
	GUIDMHelp( 52,  94, GUIMenuItem +  7, 3);
	GUIDMHelp( 99, 135, GUIMenuItem + 14, 4);
	GUIDMHelp(140, 188, GUIMenuItem + 21, 5);
	GUIDMHelp(193, 223, GUIMenuItem + 29, 6);

	GUIMenuL = 0;
	GUIMenuR = 0;
	GUIMenuD = 0;

	/* format : x pos, y pos, #charx, #chary, name, xpos+2, xpos+5,22,
	 *          19+#chary*10, length of top menu box */
	if (GUIcmenupos == 1)
	{
		GUIDrawMenuM(4, 16, 30, 13, GUIPrevMenuData, 6, 9, 22, 149, 8); // 19+13*10
		GUICYLocPtr = MenuDat1;
	}
	if (GUIcmenupos == 2)
	{
		GUIDrawMenuM(17, 16, 10, 9, GUIGameMenuData[0], 19, 22, 22, 109, 30); // 19+9*10
		GUICYLocPtr = MenuDat2;
	}
	if (GUIcmenupos == 3)
	{
		GUIDrawMenuM(52, 16, 8, 11, GUIConfigMenuData[0], 54, 57, 22, 129, 42); // 19+11*10
		GUICYLocPtr = MenuDat3;
	}
	if (GUIcmenupos == 4)
	{
		GUIDrawMenuM(99, 16, 8, 3, GUICheatMenuData[0], 101, 104, 22, 49, 36); // 19+3*10
		GUICYLocPtr = MenuDat4;
	}
	if (GUIcmenupos == 5)
	{
#ifdef __MSDOS__
		GUIDrawMenuM(140, 16, 10, 2, GUINetPlayMenuData[0], 142, 145, 22, 39, 48); // 19+2*10
#else
		GUIDrawMenuM(140, 16, 10, 1, GUINetPlayMenuData[0], 142, 145, 22, 29, 48); // 19+1*10
#endif
		GUICYLocPtr = MenuDat5;
	}
	if (GUIcmenupos == 6)
	{
		GUIDrawMenuM(193, 16, 9, 7, GUIMiscMenuData[0], 195, 198, 22, 89, 30); // 19+5*10
		GUICYLocPtr = MenuDat6;
	}
}
