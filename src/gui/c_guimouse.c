#include <stdarg.h>
#include <string.h>

#include "../asm_call.h"
#include "../c_intrf.h"
#include "../cfg.h"
#include "../cpu/execute.h"
#include "../cpu/regs.h"
#include "../gblvars.h"
#include "../input.h"
#include "../link.h"
#include "../macros.h"
#include "../ui.h"
#include "../vcache.h"
#include "../video/procvid.h"
#include "../zmovie.h"
#include "../zpath.h"
#include "../zstate.h"
#include "c_gui.h"
#include "c_guikeys.h"
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


static bool GUIClickArea(s4 const eax, s4 const edx, s4 const p1, s4 const p2, s4 const p3, s4 const p4)
{
	return
		p1 <= eax && eax <= p3 &&
		p2 <= edx && edx <= p4;
}


static void GUIClickCButton(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u1* const p3)
{
	if (GUIClickArea(eax, edx, p1 + 1, p2 + 3, p1 + 6, p2 + 8)) *p3 ^= 1;
}


static void GUIClickCButtonM(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u1* const p3)
{
	if (GUIClickArea(eax, edx, p1 + 1, p2 + 3, p1 + 6, p2 + 8))
	{
		*p3     ^= 1;
		MultiTap = pl12s34 != 1 && (pl3contrl != 0 || pl4contrl != 0 || pl5contrl != 0);
	}
}


#ifdef __MSDOS__

static void GUIClickCButtonID(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u1* const p3)
{
	if (GUIClickArea(eax, edx, p1 + 1, p2 + 3, p1 + 6, p2 + 8))
	{
		*p3 ^= 1
		SetDevice();
	}
}

#endif


static bool GUIClickCButton5(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u1* const p3, u1 const p4)
{
	if (GUIClickArea(eax, edx, p1 + 1, p2 + 3, p1 + 6, p2 + 8))
	{
		*p3 = *p3 == p4 ? *p3 ^ p4 : p4;
		return true;
	}
	return false;
}


static bool GUIClickCButtonL(s4 const eax, s4 const edx, s4 const p1, s4 const p2)
{
	if (GUIClickArea(eax, edx, p1 + 1, p2 + 3, p1 + 6, p2 + 8))
	{
		showallext       ^= 1;
		GUIcurrentfilewin = 0;
		GetLoadData();
		return true;
	}
	return false;
}


static bool GUISlidebarPostImpl(s4 const eax, s4 const edx, u4 const p1, s4 const p2, s4 const p3, s4 const p4, u1 const p7, u4 const p8, u4* const p9, u4* const p10, u4 const* const p11, u4* const p12, u4 const p13, s4 const p14, s4 const p15, s4 const p16, s4 const p17, u4* const p18, u4* const p19, u4 const* const p20, void (* const p23)(s4 eax, s4 edx), u4 const p24) // p1-p13: x1,y1,x2,y2,upjump,downjump,holdpos,scsize,view,cur,listsize, p14-p24: x1,y1,x2,y2,view,curs,num,.scru,.scrd,jumpto,sizeofscreen
{
	if (*p11 == 0) return false;

	if (GUIdispmode != 1)
	{
		if (GUIClickArea(eax, edx, p1, p2 - 7, p3, p2 - 1))
		{
			*p12     = p13;
			GUICHold = p7;
			goto scrollup;
		}
		if (GUIClickArea(eax, edx, p1, p4 + 1, p3, p4 + 7))
		{
			*p12     = p13;
			GUICHold = p7 + 1;
			goto scrolldown;
		}
	}
	else if (GUIClickArea(eax, edx, p1, p2, p3, p4)) // slidebar
	{
		*p12 = p13;
		// displacement = (GUIdispmode * pixeldisp. / (listsize-scsize))
		s8 const edxeax = (s8)(s4)(*p11 - p8) * (s8)(edx - GUIlastypos);
		u4 const ebx    = GUIlastdispval;
		if (ebx != 0 && !(ebx & 0x80000000))
		{
			u4 const eax = edxeax / ebx;
			*p9  = GUIlastvpos + eax;
			*p10 = GUIlastcpos + eax;
			if (*p9  & 0x8000000) *p9  = 0; // XXX probably should be 0x80000000
			if (*p10 & 0x8000000) *p10 = 0; // XXX probably should be 0x80000000
			u4 const eax_ = *p11;
			if (*p10 >= eax_ - 1)  *p10 = eax_ - 1;
			if (*p9  >= eax_ - p8) *p9  = eax_ - p8;
		}
		return true;
	}

	if (p14 <= eax && eax <= p16)
	{
		if (edx == p15)
		{ // Scroll Up
			*p19 = *p18;
scrollup:
			if (GUIScrolTim1 != 0) goto donescrol;
			if (*p19 != 0)
			{
				--*p19;
				if (*p18 != 0) --*p18;
			}
		}
		else if (edx == p17)
		{ // Scroll Down
			if (*p20 > p24) *p19 = *p18 + p24 - 1;
scrolldown:
			if (GUIScrolTim1 != 0) goto donescrol;
			if (*p20 - 1 > *p19)
			{
				++*p19;
				if (*p20 <= p24) goto donescrol;
				u4 const ebx = *p20 - p24;
				if (++*p18 >= ebx) *p18 = ebx;
			}
		}
		else
		{
			return false;
		}

		GUIScrolTim1 = 1;
		if (GUIScrolTim2 >= 4) ++GUIScrolTim1;
		if (GUIScrolTim2 != 0)
		{
			++GUIScrolTim1;
			--GUIScrolTim2;
		}

donescrol:
		p23(eax, edx);
		return true;
	}

	return false;
}


static bool GUISlidebarImpl(s4 const eax, s4 const edx, s4 const p1, s4 const p2, s4 const p3, s4 const p4, u4 const* const p5, u4 const p6, u4* const p7, u4* const p8, u4 const* const p9, u4 const p10) // x1,y1,x2,y2,GUI?StA,ScrnSize,ViewLoc,CursLoc,Entries,win#
{
	GUIdispmode = 0;

	if (GUIClickArea(eax, edx, p1, p2, p3, p4))
	{
		if (p5[1] > (u4)(edx - p2))
		{
			*p7 -= p6;
			*p8 -= p6;
			if (*p7 & 0x8000000) // XXX probably should be 0x80000000
			{
				*p7 = 0;
				*p8 = 0;
			}
		}
		else if (p5[2] < (u4)(edx - p2))
		{
			*p7 += p6;
			*p8 += p6;
			u4 const ebx = *p9 - 1;
			if (*p8 >= ebx)
			{
				*p8  = ebx;
				*p7  = ebx - p6 + 1;
			}
			u4 const ebx_ = *p9 - p6;
			if (*p7 >= ebx_) *p7 = ebx_;
		}
		else
		{
			GUIlastypos    = edx;
			GUIdispmode    = 1;
			GUIHoldYlim    = GUIwinposy[p10] + p2;
			GUIHoldYlimR   = GUIwinposy[p10] + p4;
			GUIHoldXlimL   = GUIwinposx[p10] + p1;
			GUIHoldXlimR   = GUIwinposx[p10] + p3;
			GUIlastdispval = *p5;
			GUIlastcpos    = *p8;
			GUIlastvpos    = *p7;
			GUIHold        = 3;
		}
		return true;
	}

	// upper arrow
	if (GUIClickArea(eax, edx, p1, p2 - 7, p3, p2 - 1))
	{
		GUIHoldYlim  = GUIwinposy[p10] + p2 - 7;
		GUIHoldYlimR = GUIwinposy[p10] + p2 - 1;
		GUIHoldXlimL = GUIwinposx[p10] + p1;
		GUIHoldXlimR = GUIwinposx[p10] + p3;
		GUIHold      = 3;
		return true;
	}

	// lower arrow
	if (GUIClickArea(eax, edx, p1, p4 + 1, p3, p4 + 7))
	{
		GUIHoldYlim  = GUIwinposy[p10] + p4 + 1;
		GUIHoldYlimR = GUIwinposy[p10] + p4 + 7;
		GUIHoldXlimL = GUIwinposx[p10] + p1;
		GUIHoldXlimR = GUIwinposx[p10] + p3;
		GUIHold      = 3;
		return true;
	}

	return false;
}


static void GUIPHoldbutton(s4 const eax, s4 const edx, s4 const p1, s4 const p2, s4 const p3, s4 const p4, u1 const p5)
{
	if (GUIClickArea(eax, edx, p1, p2, p3, p4))
	{
		GUIHoldXlimL = p1;
		GUIHoldXlimR = p3;
		GUIHoldYlim  = p2;
		GUIHoldYlimR = p4;
		GUICBHold2   = p5;
		GUIHold      = 4;
	}
}


static void GUIPHoldbutton2(s4 const eax, s4 const edx, s4 const p1, s4 const p2, s4 const p3, s4 const p4, u1 const p5, u1* const p6, s1 const p7, u1 const p8)
{
	if (GUIClickArea(eax, edx, p1, p2, p3, p4) && GUIHold == 0)
	{
		GUIHoldXlimL = p1;
		GUIHoldXlimR = p3;
		GUIHoldYlim  = p2;
		GUIHoldYlimR = p4;
		GUICBHold2   = p5;
		GUIHold      = 4;
		if (*p6 != p8) *p6 += p7;
	}
}


static void GUITextBoxInputNach(s4 const eax, s4 const edx, s4 const p1, s4 const p2, s4 const p3, s4 const p4, u4 const p5, u4 const p6, void (* const p7)(void))
{
	if (GUIClickArea(eax, edx, p1, p2, p3, p4))
	{
		p7();
		GUIInputBox   = p5 + 1;
		GUIInputLimit = p6 - 1;
	}
}


static void GUIPButtonHole(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u1* const p3, u1 const p4)
{
	if (GUIClickArea(eax, edx, p1 + 1, p2 + 1, p1 + 7, p2 + 7)) *p3 = p4;
}


static void GUIPButtonHoleLoad(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u1* const p3, u1 const p4)
{
	if (GUIClickArea(eax, edx, p1 + 1, p2 + 1, p1 + 7, p2 + 7))
	{
		*p3 = p4;
		GetLoadData();
	}
}


static bool GUIWinControl(s4 const eax, s4 const edx, s4 const p1, s4 const p2, s4 const p3, s4 const p4, u4* const p5, u4 const* const p6, u4 const* const p7, s4 const p8, u4 const p9, u4* const p10, u4 const p11, u4 const p12, u4 const p13) // x,y,x2,y2,currentwin,vpos,#entries,starty,y/entry,cpos,winval,win#,dclicktick#
{
	if (GUIClickArea(eax, edx, p1, p2, p3, p4) && *p7 != 0)
	{
		*p5 = p11;
		s4 const eax = (u4)(edx - p8) / (u4)p9 + *p6;
		if (eax <= (s4)*p7 - 1) *p10 = eax;
		if (GUIHold == 0)
		{
			if (GUIDClickTL != 0 && GUIDClCWin == p11 && GUIDClCEntry == eax)
			{
				GUIDClickTL = 0;
				if (p12 == 1)
				{
					GUILoadData();
				}
				else
				{
					CheatCodeToggle();
				}
				return true;
			}
			GUIDClickTL  = p13;
			GUIDClCWin   = p11;
			GUIDClCEntry = eax;
		}
		GUIHoldYlim  = GUIwinposy[p12] + p2 - 1;
		GUIHoldYlimR = GUIwinposy[p12] + p4 + 1;
		GUIHoldXlimL = GUIwinposx[p12] + p1;
		GUIHoldXlimR = GUIwinposx[p12] + p3;
		GUIHold      = 3;
		return true;
	}
	return false;
}


static bool DGOptnsProcBox(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u4* const p3)
{
	if (GUIClickArea(eax, edx, p1, p2, p1 + 19, p2 + 6))
	{
		u4 const ebx = guipresstest();
		if (ebx != 0x01 && ebx != 0x3B)
		{
			*p3 = ebx;
#ifndef __MSDOS__
			if (keycontrolval != 0) *keycontrolval = 1;
#endif
		}
		else
		{
			*p3 = 0;
		}
		return true;
	}

	return false;
}


static void GUIPTabClick(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u4 const p3, u4* const p4, ...) // minX, maxX, value, var, vars to zero
{
	if (GUIClickArea(eax, edx, p1 + 1, 11, p2 - 1, 22))
	{
		GUIInputBox = 0;
		p4[0] = p3;
		va_list ap;
		va_start(ap, p4);
		for (;;)
		{
			u4* const p = va_arg(ap, u4*);
			if (!p) break;
			*p = 0;
		}
		GUIFreshInputSelect = 1;
	}
}


static void DisplayGUIConfirmClick_skipscrol(s4 const eax, s4 const edx)
{
	if (GUIClickCButtonL(eax, edx, 10, 187)) return;
	if (GUIWinControl(eax, edx,   5, 27, 144, 26 + 15 * 7, &GUIcurrentfilewin, (u4 const*)&GUIcurrentviewloc,    (u4 const*)&GUIfileentries, 27, 7, (u4*)&GUIcurrentcursloc,    0, 1, 30)) return; // XXX ugly casts
	if (GUIWinControl(eax, edx, 160, 27, 228, 26 + 15 * 7, &GUIcurrentfilewin, (u4 const*)&GUIcurrentdirviewloc, (u4 const*)&GUIdirentries,  27, 7, (u4*)&GUIcurrentdircursloc, 1, 1, 30)) return; // XXX ugly casts
	GUIPHoldbutton(eax, edx, 186, 165, 228, 176, 1);
	GUIPButtonHoleLoad(eax, edx, 9, 163, &GUIloadfntype, 0);
	GUIPButtonHoleLoad(eax, edx, 9, 171, &GUIloadfntype, 1);
#ifdef __MSDOS__
	GUIPButtonHoleLoad(eax, edx, 9, 179, &GUIloadfntype, 2);
#endif
	if (GUIClickCButton5(eax, edx, 144, 177, &ForceROMTiming, 1)) return;
	if (GUIClickCButton5(eax, edx, 144, 187, &ForceROMTiming, 2)) return;
	if (GUIClickCButton5(eax, edx, 184, 177, &ForceHiLoROM,   1)) return;
	if (GUIClickCButton5(eax, edx, 184, 187, &ForceHiLoROM,   2)) return;
}


static void DisplayGUIConfirmClick(s4 const eax, s4 const edx)
{
	// offset 0 = (ysize-(yend-ystart+1)), offset 1 = starty, offset 2 = endy
	// SlideBar Implementation
	if (GUISlidebarImpl(eax, edx, 146, 33, 153, 33 + 93, GUILStA, 15, (u4*)&GUIcurrentviewloc,    (u4*)&GUIcurrentcursloc,    (u4 const*)&GUIfileentries, 1)) return; // XXX ugly casts
	if (GUISlidebarImpl(eax, edx, 230, 33, 237, 33 + 93, GUILStB, 15, (u4*)&GUIcurrentdirviewloc, (u4*)&GUIcurrentdircursloc, (u4 const*)&GUIdirentries,  1)) return; // XXX ugly casts
	GUIScrolTim1 = 0;
	GUIScrolTim2 = 6;
	DisplayGUIConfirmClick_skipscrol(eax, edx);
}


static void DisplayGUIConfirmClick2(s4 const eax, s4 const edx)
{
	if (GUIfileentries > 1)
	{
		if (GUISlidebarPostImpl(eax, edx, 146, 33, 153, 33 + 93, 1, 15, (u4*)&GUIcurrentviewloc, (u4*)&GUIcurrentcursloc, (u4 const*)&GUIfileentries, &GUIcurrentfilewin, 0, 5, 26, 144, 27 + 15 * 7, (u4*)&GUIcurrentviewloc, (u4*)&GUIcurrentcursloc, (u4 const*)&GUIfileentries, DisplayGUIConfirmClick_skipscrol, 15)) return; // XXX ugly casts
	}
	if (GUIdirentries > 1)
	{
		if (GUISlidebarPostImpl(eax, edx, 230, 33, 237, 33 + 93, 3, 15, (u4*)&GUIcurrentdirviewloc, (u4*)&GUIcurrentdircursloc, (u4 const*)&GUIdirentries, &GUIcurrentfilewin, 1, 160, 26, 228, 27 + 15 * 7, (u4*)&GUIcurrentdirviewloc, (u4*)&GUIcurrentdircursloc, (u4 const*)&GUIdirentries, DisplayGUIConfirmClick_skipscrol, 15)) return; // XXX ugly casts
	}
	DisplayGUIConfirmClick(eax, edx);
}


static void DisplayGUIChoseSaveClick(s4 const eax, s4 const edx)
{
	GUIPHoldbutton2(eax, edx,  94, 59, 102, 67, 80, (u1*)&GUIChoseSlotTextX[0],  1, '9'); // XXX ugly cast
	GUIPHoldbutton2(eax, edx, 105, 59, 113, 67, 81, (u1*)&GUIChoseSlotTextX[0], -1, '0'); // XXX ugly cast
	GUIPButtonHole(eax, edx, 10, 28, (u1*)&GUIChoseSaveText2[0], 0); // XXX ugly cast
	GUIPButtonHole(eax, edx, 30, 28, (u1*)&GUIChoseSaveText2[0], 1); // XXX ugly cast
	GUIPButtonHole(eax, edx, 50, 28, (u1*)&GUIChoseSaveText2[0], 2); // XXX ugly cast
	GUIPButtonHole(eax, edx, 70, 28, (u1*)&GUIChoseSaveText2[0], 3); // XXX ugly cast
	GUIPButtonHole(eax, edx, 90, 28, (u1*)&GUIChoseSaveText2[0], 4); // XXX ugly cast
	GUIPButtonHole(eax, edx, 10, 43, (u1*)&GUIChoseSaveText2[0], 5); // XXX ugly cast
	GUIPButtonHole(eax, edx, 30, 43, (u1*)&GUIChoseSaveText2[0], 6); // XXX ugly cast
	GUIPButtonHole(eax, edx, 50, 43, (u1*)&GUIChoseSaveText2[0], 7); // XXX ugly cast
	GUIPButtonHole(eax, edx, 70, 43, (u1*)&GUIChoseSaveText2[0], 8); // XXX ugly cast
	GUIPButtonHole(eax, edx, 90, 43, (u1*)&GUIChoseSaveText2[0], 9); // XXX ugly cast
	current_zst = (GUIChoseSlotTextX[0] - '0') * 10 + GUIChoseSaveText2[0];
}


static void DisplayGUIMovieClick(s4 const eax, s4 const edx)
{
	if (MovieProcessing < 4 || 6 < MovieProcessing)
	{
		GUIPTabClick(eax, edx, 0, 57, 1, GUIMovieTabs, &GUIDumpingTab[0], (u4*)0);
	}

	if (MovieProcessing < 1 || 3 < MovieProcessing)
	{
		GUIPTabClick(eax, edx, 58, 110, 1, GUIDumpingTab, &GUIMovieTabs[0], (u4*)0);
	}

	GUIPButtonHole(eax, edx,   8, 39, (u1*)&CMovieExt, 'v'); // Radio buttons // XXX ugly cast
	GUIPButtonHole(eax, edx,  28, 39, (u1*)&CMovieExt, '1'); // XXX ugly cast
	GUIPButtonHole(eax, edx,  48, 39, (u1*)&CMovieExt, '2'); // XXX ugly cast
	GUIPButtonHole(eax, edx,  68, 39, (u1*)&CMovieExt, '3'); // XXX ugly cast
	GUIPButtonHole(eax, edx,  88, 39, (u1*)&CMovieExt, '4'); // XXX ugly cast
	GUIPButtonHole(eax, edx, 108, 39, (u1*)&CMovieExt, '5'); // XXX ugly cast
	GUIPButtonHole(eax, edx, 128, 39, (u1*)&CMovieExt, '6'); // XXX ugly cast
	GUIPButtonHole(eax, edx, 148, 39, (u1*)&CMovieExt, '7'); // XXX ugly cast
	GUIPButtonHole(eax, edx, 168, 39, (u1*)&CMovieExt, '8'); // XXX ugly cast
	GUIPButtonHole(eax, edx, 188, 39, (u1*)&CMovieExt, '9'); // XXX ugly cast

	if (GUIMovieTabs[0] == 1)
	{
		if (MovieRecordWinVal != 0) // Overwrite Window
		{
			GUIPHoldbutton(eax, edx, 17, 65,  59, 76, 19);
			GUIPHoldbutton(eax, edx, 70, 65, 112, 76, 20);
			return;
		}

		// Main Window
		GUIPHoldbutton(eax, edx,   7,  80,  49,  91, 16); // Buttons
		GUIPHoldbutton(eax, edx,  55,  80,  97,  91, 17);
		GUIPHoldbutton(eax, edx, 103,  80, 145,  91, 18);
		GUIPHoldbutton(eax, edx, 151,  80, 193,  91, 32);
		GUIPHoldbutton(eax, edx,   7, 108,  50, 119, 29);
		GUIPHoldbutton(eax, edx,  85, 108, 138, 119, 30);
		GUIPHoldbutton(eax, edx, 173, 108, 203, 119, 31);

		GUIPButtonHole(eax, edx,   8, 64, &MovieStartMethod, 0); // Start From
		GUIPButtonHole(eax, edx,  43, 64, &MovieStartMethod, 1);
		GUIPButtonHole(eax, edx,  89, 64, &MovieStartMethod, 2);
		GUIPButtonHole(eax, edx, 135, 64, &MovieStartMethod, 3);

		DGOptnsProcBox(eax, edx,  58, 110, &KeyInsrtChap); // Keyboard Shortcut Boxes
		DGOptnsProcBox(eax, edx, 146, 110, &KeyPrevChap);
		DGOptnsProcBox(eax, edx, 210, 110, &KeyNextChap);

		DGOptnsProcBox(eax, edx, 135, 124, &KeyRTRCycle);

		GUIPButtonHole(eax, edx, 8, 133, &MZTForceRTR, 0);
		GUIPButtonHole(eax, edx, 8, 143, &MZTForceRTR, 1);
		GUIPButtonHole(eax, edx, 8, 153, &MZTForceRTR, 2);

		GUIClickCButton(eax, edx, 8, 163, &MovieDisplayFrame); // Checkbox
	}

	if (GUIDumpingTab[0] == 1)
	{
		GUIPHoldbutton(eax, edx, 165, 178, 200, 189, 34);
		GUIPHoldbutton(eax, edx, 206, 178, 235, 189, 35);

		GUIPButtonHole(eax, edx, 8,  64, &MovieVideoMode, 0); // Movie Options
		GUIPButtonHole(eax, edx, 8,  74, &MovieVideoMode, 1);
		GUIPButtonHole(eax, edx, 8,  84, &MovieVideoMode, 2);
		GUIPButtonHole(eax, edx, 8,  94, &MovieVideoMode, 3);
		GUIPButtonHole(eax, edx, 8, 104, &MovieVideoMode, 4);
		GUIPButtonHole(eax, edx, 8, 114, &MovieVideoMode, 5);

		GUIClickCButton(eax, edx, 130, 62, &MovieAudio);
		GUIClickCButton(eax, edx, 130, 72, &MovieAudioCompress);
		GUIClickCButton(eax, edx, 130, 82, &MovieVideoAudio);

		GUIPButtonHole(eax, edx, 8, 135, &MovieForcedLengthEnabled, 0); // Movie Options
		GUIPButtonHole(eax, edx, 8, 145, &MovieForcedLengthEnabled, 1);
		GUIPButtonHole(eax, edx, 8, 155, &MovieForcedLengthEnabled, 2);

		GUITextBoxInputNach(eax, edx, 136, 144, 205, 154, 0, 11, SetMovieForcedLength);
	}
}


#define GUIInputSetIndKey(p1) \
do \
{ \
	/* Check if controller is set */ \
	if (*(u4 const*)keycontrolval == 0) return; /* XXX cast makes no sense */ \
 \
	DGOptnsProcBox(eax, edx,  45, 102, &p1 ## upk);    /* Up */ \
	DGOptnsProcBox(eax, edx,  45, 112, &p1 ## downk);  /* Down */ \
	DGOptnsProcBox(eax, edx,  45, 122, &p1 ## leftk);  /* Left */ \
	DGOptnsProcBox(eax, edx,  45, 132, &p1 ## rightk); /* Right */ \
	DGOptnsProcBox(eax, edx,  45, 142, &p1 ## startk); /* Start */ \
	DGOptnsProcBox(eax, edx,  45, 152, &p1 ## selk);   /* Select */ \
	DGOptnsProcBox(eax, edx,  85, 102, &p1 ## Ak);     /* A */ \
	DGOptnsProcBox(eax, edx,  85, 112, &p1 ## Bk);     /* B */ \
	DGOptnsProcBox(eax, edx,  85, 122, &p1 ## Xk);     /* X */ \
	DGOptnsProcBox(eax, edx,  85, 132, &p1 ## Yk);     /* Y */ \
	DGOptnsProcBox(eax, edx,  85, 142, &p1 ## Lk);     /* L */ \
	DGOptnsProcBox(eax, edx,  85, 152, &p1 ## Rk);     /* R */ \
 \
	DGOptnsProcBox(eax, edx, 125, 102, &p1 ## Xtk);    /* X Turbo */ \
	DGOptnsProcBox(eax, edx, 125, 112, &p1 ## Ytk);    /* Y Turbo */ \
	DGOptnsProcBox(eax, edx, 125, 122, &p1 ## Ltk);    /* L Turbo */ \
	DGOptnsProcBox(eax, edx, 165, 102, &p1 ## Atk);    /* A Turbo */ \
	DGOptnsProcBox(eax, edx, 165, 112, &p1 ## Btk);    /* B Turbo */ \
	DGOptnsProcBox(eax, edx, 165, 122, &p1 ## Rtk);    /* R Turbo */ \
 \
	DGOptnsProcBox(eax, edx, 125, 142, &p1 ## ULk);    /* Up-Left */ \
	DGOptnsProcBox(eax, edx, 125, 152, &p1 ## DLk);    /* Down-Left */ \
	DGOptnsProcBox(eax, edx, 165, 142, &p1 ## URk);    /* Up-Right */ \
	DGOptnsProcBox(eax, edx, 165, 152, &p1 ## DRk);    /* Down-Right */ \
} \
while (0)


static void DisplayGUIInputClick_skipscrol(s4 const eax, s4 const edx)
{
	// x,y,x2,y2,currentwin,vpos,#entries,starty,y/entry,cpos,winval,win#,dclicktick#
	GUIWinControl(eax, edx, 5, 36, 107, 34 + 5 * 8, &GUIBlankVar, &GUIcurrentinputviewloc, &GUINumValue, 35, 8, &GUIcurrentinputcursloc, 4, 3, 0);

	GUIPTabClick(eax, edx,  0,  21, 1, GUIInputTabs, (u4*)0);
	GUIPTabClick(eax, edx, 22,  43, 2, GUIInputTabs, (u4*)0);
	GUIPTabClick(eax, edx, 44,  65, 3, GUIInputTabs, (u4*)0);
	GUIPTabClick(eax, edx, 66,  87, 4, GUIInputTabs, (u4*)0);
	GUIPTabClick(eax, edx, 88, 109, 5, GUIInputTabs, (u4*)0);

	GUIPHoldbutton(eax, edx, 123, 34, 153, 45, 14); // Buttons
	GUIPHoldbutton(eax, edx, 123, 50, 177, 61, 40);
#ifdef __MSDOS__
	GUIPHoldbutton(eax, edx, 123, 66, 183, 77, 15);
#endif

	switch (cplayernum)
	{
		case 0: keycontrolval = &pl1contrl; GUIInputSetIndKey(pl1); break;
		case 1: keycontrolval = &pl2contrl; GUIInputSetIndKey(pl2); break;
		case 2: keycontrolval = &pl3contrl; GUIInputSetIndKey(pl3); break;
		case 3: keycontrolval = &pl4contrl; GUIInputSetIndKey(pl4); break;
		case 4: keycontrolval = &pl5contrl; GUIInputSetIndKey(pl5); break;
	}

#ifdef __MSDOS__
	switch (cplayernum)
	{
		case 0: GUIClickCButtonID(eax, edx, 5, 190, &pl1p209); break;
		case 1: GUIClickCButtonID(eax, edx, 5, 190, &pl2p209); break;
		case 1: GUIClickCButtonID(eax, edx, 5, 190, &pl3p209); break;
		case 1: GUIClickCButtonID(eax, edx, 5, 190, &pl4p209); break;
		case 1: GUIClickCButtonID(eax, edx, 5, 190, &pl5p209); break;
	}
	GUIClickCButton(eax, edx, 105, 160, &SidewinderFix);
#endif

	GUIClickCButton( eax, edx,   5, 160, &GameSpecificInput);
	GUIClickCButton( eax, edx,   5, 170, &AllowUDLR);
	GUIClickCButton( eax, edx, 105, 170, &Turbo30hz);
	GUIClickCButtonM(eax, edx,   5, 180, &pl12s34);
}


static void DisplayGUIInputClick(s4 const eax, s4 const edx)
{
	// SlideBar Implementation
	GUINumValue = NumInputDevices;
	if (GUISlidebarImpl(eax, edx, 109, 42, 116, 69, GUIIStA, 5, &GUIcurrentinputviewloc, &GUIcurrentinputcursloc, &GUINumValue, 3)) return;
	DisplayGUIInputClick_skipscrol(eax, edx);
}


static void DisplayGUIInputClick2(s4 const eax, s4 const edx)
{
	GUINumValue = NumInputDevices;
	if (GUISlidebarPostImpl(eax, edx, 109, 42, 116, 69, 9, 5, &GUIcurrentinputviewloc, &GUIcurrentinputcursloc, &GUINumValue, &GUIBlankVar, 1, 5, 35, 107, 35 + 5 * 8, &GUIcurrentinputviewloc, &GUIcurrentinputcursloc, &GUINumValue, DisplayGUIInputClick_skipscrol, 5)) return;
	DisplayGUIInputClick(eax, edx);
}


static void GUIWindowMove(void)
{
	u1 const id = GUIwinorder[GUIwinptr - 1];
	u4 const rx = GUImouseposx - GUIwinposx[id];
	u4 const ry = GUImouseposy - GUIwinposy[id];
	void (* f)();
	switch (id)
	{
		case  3: DisplayGUIInputClick2(  rx, ry); return;
		case  5: f = DisplayGUIVideoClick2;       break;
		case  7: f = DisplayGUICheatClick2;       break;
		case 13: f = DisplayGUICheatSearchClick2; break;
		case 16: f = DisplayGUIComboClick2;       break;
		default: DisplayGUIConfirmClick2(rx, ry); return;
	}
	asm volatile("call *%0" :: "r" (f), "a" (rx), "d" (ry) : "cc", "memory"); // XXX asm_call
}


static void GUIWinClicked(u4 const i, u4 const id)
{
	u4 const rx  = GUImouseposx - GUIwinposx[id];
	u4 const ry  = GUImouseposy - GUIwinposy[id];
	s4 const esi = rx - GUIwinsizex[id] + 10;
	if (0 <= esi && esi < 10 && 0 < ry && ry < 10)
	{
		GUIwinorder[i]  = 0;
		GUIwinactiv[id] = 0;
		GUIInputBox     = 0;
		--GUIwinptr;
		init_save_paths();
		SetMovieForcedLength();
#ifndef __MSDOS__
		SetCustomXY();
#endif
	}
	else if (ry < 10)
	{
		GUIHold   = 1;
		GUIHoldxm = (short)GUIwinposx[id];
		GUIHoldym = (short)GUIwinposy[id];
		GUIHoldx  = GUImouseposx;
		GUIHoldy  = GUImouseposy;
	}
	else
	{
		GUIInputBox = 0;
		void (* f)();
		switch (id)
		{
			case  1: DisplayGUIConfirmClick(  rx, ry); return;
			case  2: DisplayGUIChoseSaveClick(rx, ry); return;
			case  3: DisplayGUIInputClick(    rx, ry); return;
			case  4: f = DisplayGUIOptionClick;      break;
			case  5: f = DisplayGUIVideoClick;       break;
			case  6: f = DisplayGUISoundClick;       break;
			case  7: f = DisplayGUICheatClick;       break;
			case  8: f = DisplayNetOptnsClick;       break;
			case  9: f = DisplayGameOptnsClick;      break;
			case 10: f = DisplayGUIOptnsClick;       break;
			case 11: f = DisplayGUIAboutClick;       break;
			case 12: f = DisplayGUIResetClick;       break;
			case 13: f = DisplayGUICheatSearchClick; break;
			case 14: f = DisplayGUIStatesClick;      break;
			case 15: DisplayGUIMovieClick(    rx, ry); return;
			case 16: f = DisplayGUIComboClick;       break;
			case 17: f = DisplayGUIAddOnClick;       break;
			case 18: f = DisplayGUIChipClick;        break;
			case 19: f = DisplayGUIPathsClick;       break;
			case 20: f = DisplayGUISaveClick;        break;
			case 21: f = DisplayGUISpeedClick;       break;
			default: return;
		}
		asm volatile("call *%0" :: "r" (f), "a" (rx), "d" (ry) : "cc", "memory"); // XXX asm_call
	}
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
						GUIWinClicked(i, id);
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
		case  1: GUILoadData();        return;
		case  2: GUIProcReset();       return;
		case  3: GUIProcReset();       return;
		case  4: GUIProcVideo();       return; // set video mode
#ifndef __MSDOS__
		case 12: GUIProcCustomVideo(); return; // set custom video mode
#endif
		case 37:
		case 38:
		case 39: GUINTSCReset();       return; // reset ntsc options
		case 81:
		case 82:
		case 83:
		case 84: GUINTSCPreset();      return; // ntsc preset
		case 10:
		case 11: GUIProcStates();      return;
		case  5: CheatCodeRemove();    return;
		case  6: CheatCodeToggle();    return;
		case  7: CheatCodeSave();      return;
		case  8: CheatCodeLoad();      return;
		case  9: ProcessCheatCode();   return;
		case 33: CheatCodeFix();       return;
		case 14: SetDevice();          return;
		case 15: CalibrateDev1();      return;

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

		case 50: CheatCodeSearchInit(); break;

		case 60:
			GUIComboTextH[0] = '\0';
			GUINumCombo      = 0;
			GUIComboKey      = 0;
			break;

		case 61: if (NumCombo != 50) ComboAdder();   break;
		case 62: if (NumCombo !=  0) ComboReplace(); break;
		case 63: if (NumCombo !=  0) ComboRemoval(); break;

		case 51:
			CheatWinMode      = 0;
			CheatSearchStatus = 0;
			break;

		case 52: CheatWinMode = 2;         break;
		case 53: CheatCodeSearchProcess(); break;
		case 54: CheatWinMode = 1;         break;

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

		case 56: CheatWinMode = 2; break;
		case 57: AddCSCheatCode(); break;

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
	if (lhguimouse == 1) buttons = SwapMouseButtons(buttons);
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


void guipresstestb(u4* const guicpressptr, char const* const guipressptr)
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


void DrawMouse(void)
{
	static u1 const GUIMousePtr[] =
	{
		50,47,45,43,40, 0, 0, 0,
		53,52,46,42, 0, 0, 0, 0,
		55,54,54,44, 0, 0, 0, 0,
		57,57,56,52,45, 0, 0, 0,
		59, 0, 0,55,50,45, 0, 0,
		 0, 0, 0, 0,55,50,45, 0,
		 0, 0, 0, 0, 0,55,50,47,
		 0, 0, 0, 0, 0, 0,52, 0
	};

	u1*       dst = vidbuffer + 16 + GUImouseposx + GUImouseposy * 288;
	u1 const* src = GUIMousePtr;
	u4        y   = 8;
	do
	{
		u4 x = 8;
		do
		{
			u1 const al = *src++;
			if (al == 0) continue;

			if (mouseshad & 1)
			{
				u1* const px = &dst[288 * 10 + 8];
				if (*px < 32)
				{
					*px = 96;
				}
				else
				{
					u1* const px = &dst[288 * 4 + 3];
					u1  const c  = *px;
					if (32 <= c && c <= 63)
					{
						*px = (c - 32U) / 2 + 32U;
					}
					else if ((c & 0xF0) == 64)
					{
						*px = c + 16;
					}
					else
					{
						u1* const px = &dst[288 * 7 + 5];
						u1  const c  = *px;
						if (148 <= c && c <= 167)
						{
							*px = c + 20;
						}
						else if (189 <= c && c <= 220)
						{
							*px = (c - 189U) / 2 + 189U;
						}
					}
				}
			}
			*dst = al + 88;
		}
		while (++dst, --x != 0);
		dst += 288 - 8;
	}
	while (--y != 0);
}
