#include <stdio.h>
#include <string.h>

#include "../c_init.h"
#include "../c_intrf.h"
#include "../cfg.h"
#include "../input.h"
#include "../ui.h"
#include "../zpath.h"
#include "../zstate.h"
#include "c_gui.h"
#include "c_guiwindp.h"
#include "gui.h"
#include "guifuncs.h"
#include "guikeys.h"
#include "guitools.h"
#include "guiwindp.h"

#ifdef __MSDOS__
#	include "../cpu/dspproc.h"
#	include "../dos/sound.h"
#	include "../dos/vesa2.h"
#endif

#if defined __UNIXSDL__ && defined __OPENGL__
#	include "../linux/gl_draw.h"
#endif


u1 GUIStatesText5 = 0;


static s4 cloadnleft;
static s4 cloadnposb;


static void drawshadow2(u4 const p1, s4 const p2, s4 const p3)
{
	s4 y = GUIwinposy[p1];
	if (y > 223) return;
	if (y < 0) y = 0;
	s4 ebx = y + p3 + 9;
	if (ebx < 0) return;
	if (ebx > 223) ebx = 223;
	ebx = ebx - y + 1;

	s4 x = GUIwinposx[p1];
	s4 ecx = x;
	if (x > 255) return;
	if (x < -3) x = -3;
	ecx += p2;
	if (ecx < 0) return;
	if (ecx > 255) ecx = 255;
	ecx = ecx - x + 1;

	u1* const edi = vidbuffer + (y + 3) * 288 + (x + 3) + 16;
	GUIDrawShadow2(edi, ecx, ebx);
}


static s4 DrawTitleBar(s4 const x1, s4 const x2, s4 ebx)
{
	GUIHLine(x1, x2, ebx++, 46 + 157 + 6 - GUIWincoladd);

	{ u4 edx = 42 + 157 + 4 + 4 - GUIWincoladd;
		u4 n = 8;
		do GUIHLine(x1, x2, ebx++, edx--); while (--n != 0);
	}

	GUIHLine(x1, x2, ebx++, 38 + 157 + 4 - GUIWincoladd);

	ebx -= 10;
	{ u4 const edx = 44 + 157 + 4 - GUIWincoladd;
		u4       esi = 9;
		do GUIHLine(x1, x1, ebx++, edx); while (--esi != 0);
	}

	ebx -= 8;
	{ u4 const edx = 40 + 157 + 4 - GUIWincoladd;
		u4       esi = 9;
		do GUIHLine(x2, x2, ebx++, edx); while (--esi != 0);
	}

	return ebx;
}


static void GUIDrawTArea(u4 const id, u4* const peax, u4* const pebx) // win #id
{
	u4 const edx = GUIWincol + 1;
	s4       eax = GUIwinposx[id];
	s4       ebx = GUIwinposy[id] + 10;
	s4 const ecx = eax + GUIwinsizex[id];
	u4       esi = 12;
	do GUIHLine(eax, ecx, ebx++, edx); while (--esi != 0);
	GUIHLine(eax + 1, ecx, ebx, edx + 3);
	*peax = eax;      // set eax to minX
	*pebx = ebx - 12; // set ebx to minY
}


static void GUIDrawWindowBox(u4 const p1, char const* const p2)
{
	switch (cwindrawn)
	{
		case 0:  GUIWincoladd = 0; GUIWincol = 148;      break;
		case 1:  GUIWincoladd = 4; GUIWincol = 148 +  5; break;
		default: GUIWincoladd = 4; GUIWincol = 148 + 10; break;
	}

	drawshadow2(p1, GUIwinsizex[p1], GUIwinsizey[p1]);

	{ s4 const eax = GUIwinposx[p1];
		s4       ebx = GUIwinposy[p1];
		s4 const ecx = eax + GUIwinsizex[p1];
		ebx = DrawTitleBar(eax, ecx, ebx);

		u4       esi = GUIwinsizey[p1] - 1;
		u4 const edx = GUIWincol + 2;
		do GUIHLine(eax, ecx, ebx++, edx); while (--esi != 0);

		GUIHLine(eax, ecx, ebx++, GUIWincol);
	}

	{ s4 const eax = GUIwinposx[p1];
		s4       ebx = GUIwinposy[p1] + 10;
		u4       esi = GUIwinsizey[p1] - 1;
		u4 const edx = GUIWincol + 3;
		do GUIHLine(eax, eax, ebx++, edx); while (--esi != 0);
	}

	{ s4 const eax = GUIwinposx[p1] + GUIwinsizex[p1];
		s4       ebx = GUIwinposy[p1] + 10;
		u4       esi = GUIwinsizey[p1];
		u4 const edx = GUIWincol + 1;
		do GUIHLine(eax, eax, ebx++, edx); while (--esi != 0);
	}

	{ s4 const ebx = GUIwinposy[p1] + 3;
		s4 const edx = GUIwinposx[p1] + 3;
		GUItextcolor[0] = 184;
		GUIOuttextwin(edx, ebx, p2);
	}

	{ s4 const ebx = GUIwinposy[p1] + 2;
		s4 const edx = GUIwinposx[p1] + 2;
		GUItextcolor[0] = (GUIWincoladd & 0xFF) == 0 ? 220 : 214;
		GUIOuttextwin(edx, ebx, p2);
	}

	{ s4 const eax = GUIwinposx[p1] + GUIwinsizex[p1] - 10;
		s4 const ebx = GUIwinposy[p1];
		GUIoutputiconwin(eax, ebx, GUIIconDataClose);
	}
}


static inline u1* SetVidbufLoc(u4 const ebx)
{
	return vidbuffer + ebx * 288 + 16;
}


static void DrawTabOn(u4 const* const p1, u4* const peax, u4 ebx, u4* const pebp) // p1 = array, ebp = offset, eax = minX, ebx = minY
{
	u4 eax = *peax;
	u4 ebp = *pebp;

	char const* esi = (char const*)p1; // XXX ugly cast
	u4          ecx = 8 + eax;
	while (esi[ebp] == '\0') ++ebp;
	while (esi[ebp] != '\0') ++esi, ecx += 6;

	u4 const edx = GUIWincol;
	GUIHLine(eax + 1, ecx, ebx++, edx + 4);

	{ u4 esi = 12;
		do GUIHLine(eax + 1, ecx, ebx++, edx + 2); while (--esi != 0);
		ebx -= 12;
	}

	{ u4 esi = 11;
		do GUIHLine(eax, eax, ebx++, edx + 3); while (--esi != 0);
		ebx -= 11;
	}

	GUItextcolor[0] = GUIWincol;
	{ u1*         const esi = SetVidbufLoc(ebx + 4);
		char const* const edi = (char const*)p1 + ebp; // XXX ugly cast
		GUIOutputStringwin(eax + 6, esi, edi);
	}

	GUItextcolor[0] = (GUIWincoladd & 0xFF) == 0 ? 163 : 164;
	{ u1*         const esi = SetVidbufLoc(ebx + 3);
		char const* const edi = (char const*)p1 + ebp; // XXX ugly cast
		GUIOutputStringwin(eax + 5, esi, edi);
	}

	GUItextcolor[0] = 217; // restore normal colour
	eax = ecx + 1; // restore and set Xoff for drawing step
	{ u4       esi = 12;
		do GUIHLine(eax, eax, ebx++, edx + 3); while (--esi != 0);
	}

	{ char const* const esi = (char const*)p1; // XXX ugly cast
		while (esi[ebp] != '\0') ++ebp;
	}

	*peax = eax + 1;
	*pebp = ebp;
}


static void DrawTabOff(u4 const* const p1, u4* const peax, u4 ebx, u4* const pebp) // p1 = array, ebp = offset, eax = minX, ebx = minY
{
	u4 eax = *peax;
	u4 ebp = *pebp;

	char const* esi = (char const*)p1; // XXX ugly cast
	u4          ecx = 8 + eax;
	while (esi[ebp] == '\0') ++ebp;
	while (esi[ebp] != '\0') ++esi, ecx += 6;

	u4 const edx = GUIWincol;
	++ebx;
	GUIHLine(eax + 1, ecx, ebx++, edx + 3);

	{ u4 esi = 10;
		do GUIHLine(eax, eax, ebx++, edx + 2); while (--esi != 0);
		ebx -= 10;
	}

	GUItextcolor[0] = (GUIWincoladd & 0xFF) == 0 ? 202 : 196;
	{ u1*         const esi = SetVidbufLoc(ebx + 4);
		char const* const edi = (char const*)p1 + ebp;
		GUIOutputStringwin(eax + 6, esi, edi);
	}

	GUItextcolor[0] += 15;
	{ u1*         const esi = SetVidbufLoc(ebx + 3);
		char const* const edi = (char const*)p1 + ebp;
		GUIOutputStringwin(eax + 5, esi, edi);
	}

	eax = ecx + 1;
	{ u4 esi = 10;
		do GUIHLine(eax, eax, ebx++, edx); while (--esi != 0);
	}

	{ char const* const esi = (char const*)p1;
		while (esi[ebp] != '\0') ++ebp;
	}

	*peax = eax + 1;
	*pebp = ebp;
}


static void GUIDrawTabs(u4 const* const p1, u4* const peax, u4 const ebx) // tabs/label array
{
	u4 ecx = p1[1]; // total #
	if (ecx == 0) return;
	u4 esi = p1[0]; // active tab
	u4 ebp = 8;     // set array offset at top of labels
	do
	{
		if (--esi == 0) // check if tab is the current one
		{
			DrawTabOn(p1, peax, ebx, &ebp); // draws tab, updates eax, ebx & ebp for next tab...
		}
		else
		{
			DrawTabOff(p1, peax, ebx, &ebp); // ... and autosizes the tab for its label
		}
	}
	while (--ecx != 0);
}


/* XXX NOTE: Macro is defective:
 * - if p2 has the form a + b, then 2 * b must be added to p4
 * - if p3 has the form a + b, then 2 * b must be added to p5
 */
static void DrawGUIWinBox(u4 const p1, u4 const p2, u4 const p3, u4 const p4, u4 const p5, u4 const p6)
{
	s4 const eax = GUIwinposx[p1] + p2;
	s4       ebx = GUIwinposy[p1] + p3;
	s4 const ecx = eax + p4 - p2 + 1;
	u4       esi = p5 - p3 + 1;
	do GUIHLine(eax, ecx, ebx++, p6); while (--esi != 0);
}


static void DrawGUIButton(u4 const p1, u4 const p2, u4 const p3, u4 const p4, u4 const p5, char const* const p6, u4 const p7, u4 const p8, u4 const p9)
{
	u1 dl = GUItextcolor[0];
	dl += GUICBHold == p7 ? -18 : -5;
	DrawGUIWinBox(p1, p2,     p3,     p4,     p3, dl);
	dl += GUICBHold == p7 ? +2 : -3;
	DrawGUIWinBox(p1, p2,     p3,     p2,     p5, dl);
	dl += GUICBHold == p7 ? +2 : -3;
	DrawGUIWinBox(p1, p2 + 1, p3 + 1, p4,     p5, dl);
	dl += GUICBHold == p7 ? +2 : -3;
	DrawGUIWinBox(p1, p4 + 1, p3 + 1, p4,     p5, dl);
	dl += GUICBHold == p7 ? +2 : -3;
	DrawGUIWinBox(p1, p2,     p5,     p4 - 1, p5, dl);
	if (GUICBHold != p7)
	{
		GUItextcolor[0] -= 15;
		GUIOuttextwin2(p1, p2 + 5 + p8, p3 + 4 + p9, p6);
		GUItextcolor[0] += 15;
		GUIOuttextwin2(p1, p2 + 4 + p8, p3 + 3 + p9, p6);
	}
	else
	{
		GUItextcolor[0] -= 18;
		GUIOuttextwin2(p1, p2 + 6 + p8, p3 + 5 + p9, p6);
		GUItextcolor[0] += 15;
		GUIOuttextwin2(p1, p2 + 5 + p8, p3 + 4 + p9, p6);
		GUItextcolor[0] += 3;
	}
}


static void GUIDisplayTextY(u4 const p1, u4 const p2, u4 const p3, char const* const p4) // Yellow Text&Shadow
{
	GUItextcolor[0] = GUIWincol;
	GUIOuttextwin2(p1, p2, p3, p4);
	GUItextcolor[0] = (GUIWincoladd & 0xFF) == 0 ? 163 : 164;
	GUIOuttextwin2(p1, p2 - 1, p3 - 1, p4);
}


static void GUIDisplayText(u4 const p1, u4 const p2, u4 const p3, char const* const p4) // Text&Shadow
{
	GUItextcolor[0] = (GUIWincoladd & 0xFF) == 0 ? 202 : 196;
	GUIOuttextwin2(p1, p2, p3, p4);
	GUItextcolor[0] += 15;
	GUIOuttextwin2(p1, p2 - 1, p3 - 1, p4);
}


static void GUIDisplayBBox(u4 const p1, u4 const p2, u4 const p3, u4 const p4, u4 const p5, u4 const p6) // Black Box W/Border
{
	GUIWincol =
		cwindrawn == 0 ? 148     :
		cwindrawn == 1 ? 148 + 5 :
		148 + 10;
	DrawGUIWinBox(p1, p2,     p3,         p4,     p5,     p6);
	DrawGUIWinBox(p1, p2,     p3 - 3 + 2, p4,     p3 - 1, GUIWincol);
	DrawGUIWinBox(p1, p2 - 1, p3,         p2 - 2, p5,     GUIWincol + 1);
	DrawGUIWinBox(p1, p2,     p5 + 1,     p4,     p5 + 1, GUIWincol + 4);
	DrawGUIWinBox(p1, p4 + 2, p3,         p4 + 1, p5,     GUIWincol + 3);
}


static void GUIOuttextwin2c(u4 const p1, u4 const p2, u4 const p3, char const* const p4) // Boxed, green text
{
	GUItextcolor[0] = 223;
	GUIOuttextwin2(p1, p2, p3, p4);
	GUItextcolor[0] = (GUIWincoladd & 0xFF) == 0 ? 221 : 222;
	GUIOuttextwin2(p1, p2 - 1, p3 - 1, p4);
}


static void GUIOuttextwin2d(u4 const p1, u4 const p2, u4 const p3, char const* const p4, u4 const p5, char** const p6, u4 const p7) // Boxed, green text, limited to 5th param
{
	char const* ecx = p4; // Move pointer to text into ecx
	while (*ecx != '\0') ++ecx; // Check for null in string
	u4 eax = ecx - p4;      // Subtract pointer from \0 pointer gives us string length
	if (eax > p5) eax = p5; // Restrict to length to display

	GUIOuttextwin2c(p1, p2, p3, ecx - eax);
	if (GUIInputBox == p7 + 1 && p6[p7] == p4)
	{
		if (++GUIBlinkCursorLoop == 30)
		{
			GUIBlinkCursorLoop = 0;
			GUIBlinkCursor[0] = GUIBlinkCursor[0] == ' ' ? '_' : ' ';
		}
		GUIOuttextwin2c(p1, eax * 6 /* 6 pixels */ + p2, p3, GUIBlinkCursor);
	}
}


static void GUIDisplayBBoxS(u4 const p1, u4 const p2, u4 const p3, u4 const p4, u4 const p5, u4 const p6) // Black Box W/Border
{
	// Minus right side
	GUIWincol =
		cwindrawn == 0 ? 148     :
		cwindrawn == 1 ? 148 + 5 :
		148 + 10;
	DrawGUIWinBox(p1, p2,     p3,         p4,     p5,     p6);
	DrawGUIWinBox(p1, p2,     p3 - 3 + 2, p4,     p3 - 1, GUIWincol);
	DrawGUIWinBox(p1, p2 - 1, p3,         p2 - 2, p5,     GUIWincol + 1);
	DrawGUIWinBox(p1, p2,     p5 + 1,     p4,     p5 + 1, GUIWincol + 4);
}


static void DrawGUIWinBox2(u4 const p1, u4 const p2, u4 const p3, u4 const p4, u4 const p5, s4 ebx)
{
	s4 const eax = GUIwinposx[p1] + p2;
	s4 const ecx = GUIwinposx[p1] + p3 + 1;
	u1 const edx = (GUIWincoladd & 0xFF) == 0 ? p5 : p5 + 1;
	ebx += GUIwinposy[p1];
	u4 esi = p4;
	do GUIHLine(eax, ecx, ebx++, edx); while (--esi != 0);
}


static void GUIDisplayTextG(u4 const p1, u4 const p2, u4 const p3, char const* const p4) // Green Text&Shadow
{
	GUItextcolor[0] = 223;
	GUIOuttextwin2(p1, p2, p3, p4);
	GUItextcolor[0] = (GUIWincoladd & 0xFF) == 0 ? 221 : 222;
	GUIOuttextwin2(p1, p2 - 1, p3 - 1, p4);
}


static void GUIDisplayTextu(u4 const p1, u4 const p2, u4 const p3, char const* const p4, u4 const p5) // Text&Shadow With Underline
{
	GUItextcolor[0] = (GUIWincoladd & 0xFF) == 0 ? 202 : 196;
	GUIOuttextwin2u(p1, p2, p3, p4, p5);
	GUItextcolor[0] += 15;
	GUIOuttextwin2(p1, p2 - 1, p3 - 1, p4);
}


static void GUIDisplayCheckboxTn(u4 const p1, u4 const p2, u4 const p3, u1 const* const p4, u4 const p5, char const* const p6) // Variable Checkbox (Text)
{
	GUITemp = (u4)(*p4 == p5 ? GUIIconDataCheckBoxC : GUIIconDataCheckBoxUC); // XXX ugly cast
	GUIDisplayIconWin(p1, p2, p3, (u1 const*)GUITemp); // XXX ugly cast
	GUIDisplayText(p1, p2 + 15, p3 + 5, p6);
}


static void GUIDisplayCheckbox(u4 const p1, u4 const p2, u4 const p3, u1 const* const p4, char const* const p5) // Toggled Checkbox (Text)
{
	GUITemp = (u4)(*p4 != 0 ? GUIIconDataCheckBoxC : GUIIconDataCheckBoxUC); // XXX ugly cast
	GUIDisplayIconWin(p1, p2, p3, (u1 const*)GUITemp); // XXX ugly cast
	GUIDisplayText(p1, p2 + 15, p3 + 5, p5);
}


static void GUIDisplayCheckboxu(u4 const p1, u4 const p2, u4 const p3, u1 const* const p4, char const* const p5, u4 const p6) // Toggled Checkbox (Text Underline)
{
	GUITemp = (u4)(*p4 != 0 ? GUIIconDataCheckBoxC : GUIIconDataCheckBoxUC); // XXX ugly cast
	GUIDisplayIconWin(p1, p2, p3, (u1 const*)GUITemp); // XXX ugly cast
	GUIDisplayTextu(p1, p2 + 15, p3 + 5, p5, p6);
}


static void GUIDisplayCheckboxun(u4 const p1, u4 const p2, u4 const p3, u1 const* const p4, u4 const p5, char const* const p6, u4 const p7) // Set Var. Checkbox (Text Underline)
{
	GUITemp = (u4)(*p4 == p5 ? GUIIconDataCheckBoxC : GUIIconDataCheckBoxUC); // XXX ugly cast
	GUIDisplayIconWin(p1, p2, p3, (u1 const*)GUITemp); // XXX ugly cast
	GUIDisplayTextu(p1, p2 + 15, p3 + 5, p6, p7);
}


static void GUIDisplayButtonHole(u4 const p1, u4 const p2, u4 const p3, u1 const* const p4, u4 const p5)
{
	GUITemp = (u4)(*p4 == p5 ? GUIIconDataButtonFill : GUIIconDataButtonHole); // XXX ugly cast
	GUIDisplayIconWin(p1, p2, p3, (u1 const*)GUITemp); // XXX ugly cast
}


static void GUIDisplayButtonHoleTu(u4 const p1, u4 const p2, u4 const p3, u1 const* const p4, u4 const p5, char const* const p6, u4 const p7)
{
	GUITemp = (u4)(*p4 == p5 ? GUIIconDataButtonFill : GUIIconDataButtonHole); // XXX ugly cast
	GUIDisplayIconWin(p1, p2, p3, (u1 const*)GUITemp);
	GUIDisplayTextu(p1, p2 + 15, p3 + 3, p6, p7);
}


static void GUIDrawSlider(u4 const p1, u4 const p2, u4 const p3, u4 const p4, void const* const p5, u1 (*p7)(void const*), char const* (*p8)(void const*)) // win #id, minX, width, Ypos, var, text, proc1, proc2
{
	s4 const eax = GUIwinposx[p1] + p2;
	s4 const ebx = GUIwinposy[p1] + p4;
	s4 const ecx = eax + p3;
	u4 const edx = 215 - (GUIWincoladd & 0xFF);
	GUIHLine(eax,     ecx,     ebx,     edx);
	GUIHLine(eax + 1, ecx + 1, ebx + 1, edx - 13);
	GUITemp = p2 - 4 + p7(p5); // proc1 == alters var correctly and puts result in al
	GUIDisplayIconWin(p1, GUITemp, p4 - 4, GUIIconDataSlideBar);
	char const* const esi = p8(p5); // proc2 == alters text correctly and puts pointer in esi
	GUITemp = (u4)esi; // Display Value (Green) // XXX ugly cast
	GUIDisplayTextG(p1, p2 + p3 + 6, p4 - 1, (char const*)GUITemp); // XXX ugly cast
}


static void GUIOuttextwin2load(u4 const p1, u4 const p2, u4 const p3, char* const* const eax)
{
	char const* const name = *eax;
	++cloadnposb;
	GUItextcolor[0] = 223;
	GUIOuttextwin2l(p1, p2, p3, name);
	GUItextcolor[0] = (GUIWincoladd & 0xFF) == 0 ? 221 : 222;
	GUIOuttextwin2l(p1, p2 - 1, p3 - 1, name);
	--cloadnleft;
}


static void GUIOuttextwinloadfile(u4 const p1, u4 const p2, u4 const p3)
{
	if (cloadnleft & 0x80000000) return;
	if ((u4)cloadnposb >= (u4)GUIfileentries) return;
	char* const* const eax = &selected_names[cloadnposb];
	GUIOuttextwin2load(p1, p2, p3, eax);
}


static void GUIOuttextwinloaddir(u4 const p1, u4 const p2, u4 const p3)
{
	if (cloadnleft & 0x80000000) return;
	if ((u4)cloadnposb >= (u4)GUIdirentries) return;
	char* const* const eax = &d_names[cloadnposb + 2];
	GUIOuttextwin2load(p1, p2, p3, eax);
}


void DisplayGUILoad(void)
{
	GUIDrawWindowBox(1, "LOAD GAME");

#ifndef __MSDOS__
	char const* const GUILoadText3 = "LONG FILENAME";
#else
	char const* const GUILoadText3 = "WIN9X LONG FILENAME";
#endif
	GUIDisplayText(1, 21, 166, GUILoadText3);
#ifdef __MSDOS__
	GUIDisplayTextY(1,  6, 157, "DISPLAY TYPE:");
	GUIDisplayText( 1, 21, 182, "DOS 8.3 FORMAT");
#endif
	GUIDisplayText(1,  21, 174, "SNES HEADER NAME");
	GUIDisplayText(1,   6,  16, "FILENAME");
	GUIDisplayText(1, 161,  16, "DIRECTORY");
	GUIDisplayText(1, 146, 172, "FORCE");

	u4          ecx = 0;
	char const* esi = ZRomPath;
	while (esi[ecx] != '\0') ++ecx;
	if (ecx > 39) esi += ecx - 39;
	GUITemp = (u4)esi; // XXX ugly cast
	GUIDisplayText(1, 6, 138, (char const*)GUITemp); // XXX ugly cast

	GUItextcolor[0] = (GUIWincoladd & 0xFF) == 0 ? 202 : 196;
	cloadmaxlen = 39;
#ifndef __MSDOS__
	if (GUIcurrentfilewin != 0)
	{
		char const* const eax = d_names[GUIcurrentdircursloc + 2];
		GUITemp = (u4)eax; // XXX ugly cast
		GUIOuttextwin2l(1, 6, 158, (char const*)GUITemp); // XXX ugly cast
		GUItextcolor[0] += 15;
		GUIOuttextwin2l(1, 5, 157, (char const*)GUITemp); // XXX ugly cast
	}
	else if (GUIfileentries != 0)
	{
		s4 const eax = GUIcurrentcursloc;
		if ((u4)eax < (u4)GUIfileentries)
		{
			char const* const name = selected_names[eax];
			GUIOuttextwin2l(1, 6, 158, name);
			GUItextcolor[0] += 15;
			GUIOuttextwin2l(1, 5, 157, name);
		}
	}
#endif

	GUItextcolor[0] = (GUIWincoladd & 0xFF) == 0 ? 217 : 111;
	DrawGUIButton(1, 186, 165, 228, 176, "LOAD", 1, 0, 0);

	// The Three Boxes
	GUIDisplayBBoxS(1,   5,  25, 144, 134, 167); // 126 = 6 * 21,  112 = 7 * 16
	GUIDisplayBBoxS(1, 160,  25, 228, 134, 167); // 78 =  6 * 13
	GUIDisplayBBox( 1,   5, 145, 228, 152, 167); // 126 = 6 * 21,  112 = 7 * 16

	{ u1 const ebx = GUILoadPos; // Flash Code?
		if (GUILDFlash & 8)
		{
			GUILoadTextA[ebx] = '\0';
		}
		else
		{
			GUILoadTextA[ebx]     = '_';
			GUILoadTextA[ebx + 1] = '\0';
		}
	}

	// Check if it's in the Files box
	s4 const ebx = GUIcurrentfilewin == 0 ?
		GUIcurrentcursloc    - GUIcurrentviewloc :
		GUIcurrentdircursloc - GUIcurrentdirviewloc;

	// Draw 2 more boxes?
	s4 const y = 27 + ebx * 7;
	if (GUIcurrentfilewin == 0)
	{
		DrawGUIWinBox2(1, 5, 144, 7, 224, y);
	}
	else
	{
		DrawGUIWinBox2(1, 160, 228, 7, 224, y);
	}

	GUItextcolor[0] = 223; // Green Shadow
	GUIOuttextwin2(1, 8, 148, GUILoadTextA);

	if (GUIfileentries == 0) GUIcurrentfilewin = 1;

	cloadnleft  = GUIfileentries - GUIcurrentviewloc;
	cloadnposb  = GUIcurrentviewloc;
	cloadmaxlen = 23;

	// Text/Shadow for Filename Box
	for (u4 i = 0; i != 15; ++i)
	{
		GUIOuttextwinloadfile(1, 8, 29 + 7 * i);
	}

	cloadnleft  = GUIdirentries - GUIcurrentdirviewloc;
	cloadnposb  = GUIcurrentdirviewloc;
	cloadmaxlen = 11;

	// Text/Shadow for DIR Box
	for (u4 i = 0; i != 15; ++i)
	{
		GUIOuttextwinloaddir(1, 164, 29 + 7 * i);
	}

	GUItextcolor[0] = (GUIWincoladd & 0xFF) == 0 ? 221 : 222; // Green Text
	GUIOuttextwin2(1, 7, 147, GUILoadTextA);

	GUILoadTextA[GUILoadPos] = '\0';

	GUIDisplayButtonHole(1, 9, 163, &GUIloadfntype, 0); // Radio Buttons
	GUIDisplayButtonHole(1, 9, 171, &GUIloadfntype, 1);
#ifdef __MSDOS__
	GUIDisplayButtonHole(1, 9, 179, &GUIloadfntype, 2);
#endif

	GUIDisplayCheckboxTn(1,  10, 187, &showallext,     1, "SHOW ALL EXTENSIONS"); // Checkboxes
	GUIDisplayCheckboxTn(1, 144, 177, &ForceROMTiming, 1, "NTSC");
	GUIDisplayCheckboxTn(1, 144, 187, &ForceROMTiming, 2, "PAL");
	GUIDisplayCheckboxTn(1, 184, 177, &ForceHiLoROM,   1, "LOROM");
	GUIDisplayCheckboxTn(1, 184, 187, &ForceHiLoROM,   2, "HIROM");

	// Slidebar for Files
	// win#,X,Y start, %4-List Loc, %5-List size, %6-Screen size, %7-Bar Size
	DrawSlideBarWin(1, 146, 33, GUIcurrentviewloc, GUIfileentries, 15, 94, GUILStA);
	if ((GUICHold & 0xFF) == 1) GUIWincoladd = GUIWincoladd & 0xFFFFFF00 | (GUIWincoladd + 3) & 0x000000FF;
	GUIDisplayIconWin(1, 146, 25, GUIIconDataUpArrow);
	if ((GUICHold & 0xFF) == 1) GUIWincoladd = GUIWincoladd & 0xFFFFFF00 | (GUIWincoladd - 3) & 0x000000FF;
	if ((GUICHold & 0xFF) == 2) GUIWincoladd = GUIWincoladd & 0xFFFFFF00 | (GUIWincoladd + 3) & 0x000000FF;
	GUIDisplayIconWin(1, 146, 127, GUIIconDataDownArrow);
	if ((GUICHold & 0xFF) == 2) GUIWincoladd = GUIWincoladd & 0xFFFFFF00 | (GUIWincoladd - 3) & 0x000000FF;

	// Slidebar for DIR
	DrawSlideBarWin(1, 230, 33, GUIcurrentdirviewloc, GUIdirentries, 15, 94, GUILStB);
	if ((GUICHold & 0xFF) == 3) GUIWincoladd = GUIWincoladd & 0xFFFFFF00 | (GUIWincoladd + 3) & 0x000000FF;
	GUIDisplayIconWin(1, 230, 25, GUIIconDataUpArrow);
	if ((GUICHold & 0xFF) == 3) GUIWincoladd = GUIWincoladd & 0xFFFFFF00 | (GUIWincoladd - 3) & 0x000000FF;
	if ((GUICHold & 0xFF) == 4) GUIWincoladd = GUIWincoladd & 0xFFFFFF00 | (GUIWincoladd + 3) & 0x000000FF;
	GUIDisplayIconWin(1, 230, 127, GUIIconDataDownArrow);
	if ((GUICHold & 0xFF) == 4) GUIWincoladd = GUIWincoladd & 0xFFFFFF00 | (GUIWincoladd - 3) & 0x000000FF;
}


void DisplayGUIReset(void)
{
	GUIDrawWindowBox(12, "RESET GAME");

	// Red Box around buttons
	u1 dl = 224;
	if ((GUIWincoladd & 0xFF) != 0)
	{
		GUItextcolor[0] = 211;
		++dl;
	}
	u4 const x = GUICResetPos == 0 ? 19 : 79;
	DrawGUIWinBox(12, x, 29, x + 38, 42, dl);

	DrawGUIButton(12, 20, 30,  56, 41, "YES", 2, 0, 0);
	DrawGUIButton(12, 80, 30, 116, 41, "NO",  3, 0, 0);

	GUIDisplayTextY(12, 6, 16, "RESET: ARE YOU SURE ?");
}


void DisplayGUIStates(void)
{
	GUIDrawWindowBox(14, "STATE CONFIRM");

	// Red Box around buttons
	u1 dl = 224;
	if ((GUIWincoladd & 0xFF) != 0)
	{
		GUItextcolor[0] = 211;
		++dl;
	}
	u4 const x = GUICStatePos == 0 ? 19 : 79;
	DrawGUIWinBox(12, x, 29, x + 38, 42, dl);

	DrawGUIButton(14, 20, 30,  56, 41, "YES", 10, 0, 0);
	DrawGUIButton(14, 80, 30, 116, 41, "NO",  11, 0, 0);

	// Determine Load or Save box
	char const* const msg = GUIStatesText5 == 1 ? "OKAY TO LOAD STATE?" : "OKAY TO SAVE STATE?";
	GUIDisplayTextY(14, 6, 16, msg);
}


void DisplayGUIChoseSave(void)
{
	GUIDrawWindowBox(2, "STATE SELECT");

	u1 const ah = current_zst % 10;
	u1 const al = current_zst / 10 + '0';

	GUIDisplayTextY(2, 6, 16, "SELECT SAVE SLOT:");
	u4 x = 0;
	u4 y = 0;
	for (u4 i = 0; i != 10; x += 20, ++i)
	{
		if (i == 5)
		{
			x =  0;
			y = 15;
		}
		GUIChoseSaveText2[0] = '0' + i;
		GUIDisplayText(      2, 21 + x, 31 + y, GUIChoseSaveText2);
		GUIDisplayButtonHole(2, 10 + x, 28 + y, &ah, i);
	}
	GUIDisplayTextY(2, 6, 61, "SLOT LEVEL:");

	GUIChoseSaveText2[0] = ah;
	GUIChoseSlotTextX[0] = al;

	GUIDisplayBBox(2, 72, 59, 90, 66, 167); // Save Slot Frameskip +/- Box
	GUIDisplayTextG(2, 83, 61, GUIChoseSlotTextX);
	GUItextcolor[0] = (GUIWincoladd & 0xFF) == 0 ? 217 : 211;
	DrawGUIButton(2,  94, 59, 102, 67, "+", 80, -2, -1);
	DrawGUIButton(2, 105, 59, 113, 67, "-", 81, -2, -1);
}


static void DGOptnsDrawBox2(s4 const p1, s4 const p2, u4 const p3)
{
	s4 const eax = GUIwinposx[3] + p1;
	s4       ebx = GUIwinposy[3] + p2;
	u4       esi = 7;
	do GUIHLine(eax, eax + 20, ebx++, 167); while (--esi != 0);

	sprintf(GUIGameDisplayKy, "%.3s", ScanCodeListing + p3 * 3);
	GUItextcolor[0] = 223;
	GUIOuttextwin2(3, p1 + 3, p2 + 2, GUIGameDisplayKy);
	GUItextcolor[0] = (GUIWincoladd & 0xFF) == 0 ? 221 : 222;
	GUIOuttextwin2(3, p1 + 2, p2 + 1, GUIGameDisplayKy);
}


#define GUIInputDispAll(p1) \
do \
{ \
	DGOptnsDrawBox2( 45, 102, p1##upk);    /* Up         */ \
	DGOptnsDrawBox2( 45, 112, p1##downk);  /* Down       */ \
	DGOptnsDrawBox2( 45, 122, p1##leftk);  /* Left       */ \
	DGOptnsDrawBox2( 45, 132, p1##rightk); /* Right      */ \
	DGOptnsDrawBox2( 45, 142, p1##startk); /* Start      */ \
	DGOptnsDrawBox2( 45, 152, p1##selk);   /* Select     */ \
	DGOptnsDrawBox2( 85, 102, p1##Ak);     /* A          */ \
	DGOptnsDrawBox2( 85, 112, p1##Bk);     /* B          */ \
	DGOptnsDrawBox2( 85, 122, p1##Xk);     /* X          */ \
	DGOptnsDrawBox2( 85, 132, p1##Yk);     /* Y          */ \
	DGOptnsDrawBox2( 85, 142, p1##Lk);     /* L          */ \
	DGOptnsDrawBox2( 85, 152, p1##Rk);     /* R          */ \
 \
	DGOptnsDrawBox2(125, 102, p1##Xtk);    /* X Turbo    */ \
	DGOptnsDrawBox2(125, 112, p1##Ytk);    /* Y Turbo    */ \
	DGOptnsDrawBox2(125, 122, p1##Ltk);    /* L Turbo    */ \
	DGOptnsDrawBox2(165, 102, p1##Atk);    /* A Turbo    */ \
	DGOptnsDrawBox2(165, 112, p1##Btk);    /* B Turbo    */ \
	DGOptnsDrawBox2(165, 122, p1##Rtk);    /* R Turbo    */ \
 \
	DGOptnsDrawBox2(125, 142, p1##ULk);    /* Up-Left    */ \
	DGOptnsDrawBox2(125, 152, p1##DLk);    /* Down-Left  */ \
	DGOptnsDrawBox2(165, 142, p1##URk);    /* Up-Right   */ \
	DGOptnsDrawBox2(165, 152, p1##DRk);    /* Down-Right */ \
} \
while (0)


static void DGOptnsBorderBox(u4 const p1, u4 const p2, u4 const p3)
{
	GUIWincol =
		cwindrawn == 0 ? 148     :
		cwindrawn == 1 ? 148 + 5 :
		148 + 10;
	DrawGUIWinBox(p1, p2  + 1, p3,     p2 + 20, p3,     GUIWincol);
	DrawGUIWinBox(p1, p2,      p3 + 1, p2 -  1, p3 + 7, GUIWincol + 1);
	DrawGUIWinBox(p1, p2 +  1, p3 + 8, p2 + 20, p3 + 8, GUIWincol + 4);
	DrawGUIWinBox(p1, p2 + 22, p3 + 1, p2 + 21, p3 + 7, GUIWincol + 3);
}


void DisplayGUIInput(void)
{
	GUIDrawWindowBox(3, "INPUT DEVICE");
	cplayernum = GUIInputTabs[0] - 1;

	{ u4 eax;
		u4 ebx;
		GUIDrawTArea(3, &eax, &ebx);
		GUIDrawTabs(GUIInputTabs, &eax, ebx);
	}

	GUIDisplayTextY(3, 6, 26, "DEVICE:");
	u1 const ebx = *GUIInputRefP[cplayernum];
	if (GUIFreshInputSelect != 0)
	{
		GUIFreshInputSelect  = 0;
		GUIJT_viewable       = 5;
		GUIJT_entries        = NumInputDevices;
		GUIJT_offset         = ebx;
		GUIJT_currentviewloc = (s4*)&GUIcurrentinputviewloc; // XXX ugly cast
		GUIJT_currentcursloc = (s4*)&GUIcurrentinputcursloc; // XXX ugly cast
		GUIGenericJumpTo();
	}
	GUITemp = (u4)&GUIInputNames[ebx]; // XXX ugly cast
	GUIDisplayTextY(3, 6 + 54, 83, (char const*)GUITemp); // CDV // XXX ugly cast
	GUIDisplayTextY(3, 6,      83, "CURRENT:");

	GUIDisplayTextY(3,   6,  94, "KEYS:");
	GUIDisplayText( 3,   6, 104, "    UP");
	GUIDisplayText( 3,   6, 114, "  DOWN");
	GUIDisplayText( 3,   6, 124, "  LEFT");
	GUIDisplayText( 3,   6, 134, " RIGHT");
	GUIDisplayText( 3,   6, 144, " START");
	GUIDisplayText( 3,   6, 154, "SELECT");

	GUIDisplayText( 3,  76, 104, "A");
	GUIDisplayText( 3,  76, 114, "B");
	GUIDisplayText( 3,  76, 124, "X");
	GUIDisplayText( 3,  76, 134, "Y");
	GUIDisplayText( 3,  76, 144, "L");
	GUIDisplayText( 3,  76, 154, "R");

	GUIDisplayTextY(3, 116,  94, "TURBO:");
	GUIDisplayText( 3, 156, 104, "A");
	GUIDisplayText( 3, 156, 114, "B");
	GUIDisplayText( 3, 156, 124, "R");
	GUIDisplayText( 3, 116, 104, "X");
	GUIDisplayText( 3, 116, 114, "Y");
	GUIDisplayText( 3, 116, 124, "L");

	GUIDisplayTextY(3, 113, 134, "DIAGONALS:");
	GUIDisplayText( 3, 113, 144, "UL");
	GUIDisplayText( 3, 153, 144, "UR");
	GUIDisplayText( 3, 113, 154, "DL");
	GUIDisplayText( 3, 153, 154, "DR");

#ifdef __MSDOS__
	GUIDisplayCheckboxu(3, 105, 160, &SidewinderFix, "SIDEWINDER FIX", 0);

	char const* const GUIInputTextE5 = "USE JOYSTICK PORT 209H";
	switch (cplayernum)
	{
		case 0: GUIDisplayCheckboxu(3, 5, 190, &pl1p209, GUIInputTextE5, 4); break;
		case 1: GUIDisplayCheckboxu(3, 5, 190, &pl2p209, GUIInputTextE5, 4); break;
		case 2: GUIDisplayCheckboxu(3, 5, 190, &pl3p209, GUIInputTextE5, 4); break;
		case 3: GUIDisplayCheckboxu(3, 5, 190, &pl4p209, GUIInputTextE5, 4); break;
		case 4: GUIDisplayCheckboxu(3, 5, 190, &pl5p209, GUIInputTextE5, 4); break;
	}
#endif

	GUIDisplayCheckboxu(3,   5, 160, &GameSpecificInput, "GAME SPECIFIC",      0);
	GUIDisplayCheckboxu(3,   5, 170, &AllowUDLR,         "ALLOW U+D/L+R",      0);
	GUIDisplayCheckboxu(3, 105, 170, &Turbo30hz,         "TURBO AT 30HZ",      0);
	GUIDisplayCheckboxu(3,   5, 180, &pl12s34,           "USE PL3/4 AS PL1/2", 0);

	DrawGUIButton(3, 123, 34, 153, 45, "SET",      14, 0, 0); // Buttons
	DrawGUIButton(3, 123, 50, 177, 61, "SET KEYS", 40, 0, 0);
#ifdef __MSDOS__
	DrawGUIButton(3, 123, 66, 183, 77, "CALIBRATE", 15, 0, 0);
#endif

	GUIDisplayBBoxS(3, 5, 34, 107, 77, 167); // Main Box
	u4 const eax = GUIcurrentinputcursloc - GUIcurrentinputviewloc;
	DrawGUIWinBox2(3, 5, 107, 7, 224, 36 + eax * 8);

	GUITemp = (u4)&GUIInputNames[GUIcurrentinputviewloc]; // Text&Shadow inside Main Box // XXX ugly cast
	for (u4 i = 0; i != 5; ++i)
	{
		GUIDisplayTextG(3, 11, 38 + 8 * i, (char const*)GUITemp); // XXX ugly cast
		GUITemp += 17;
	}
	GUITemp -= 17;

	// Sidebar
	DrawSlideBarWin(3, 109, 42, GUIcurrentinputviewloc, NumInputDevices, 5, 28, GUIIStA);
	if (GUICHold ==  9) GUIWincoladd = GUIWincoladd & 0xFFFFFF00 | (GUIWincoladd + 3) & 0x000000FF;
	GUIDisplayIconWin(3, 109, 34, GUIIconDataUpArrow);
	if (GUICHold ==  9) GUIWincoladd = GUIWincoladd & 0xFFFFFF00 | (GUIWincoladd - 3) & 0x000000FF;
	if (GUICHold == 10) GUIWincoladd = GUIWincoladd & 0xFFFFFF00 | (GUIWincoladd + 3) & 0x000000FF;
	GUIDisplayIconWin(3, 109, 70, GUIIconDataDownArrow);
	if (GUICHold == 10) GUIWincoladd = GUIWincoladd & 0xFFFFFF00 | (GUIWincoladd - 3) & 0x000000FF;

	// Hotkey Boxes
	switch (cplayernum)
	{
		case 0: GUIInputDispAll(pl1); break;
		case 1: GUIInputDispAll(pl2); break;
		case 2: GUIInputDispAll(pl3); break;
		case 3: GUIInputDispAll(pl4); break;
		case 4: GUIInputDispAll(pl5); break;
	}

	DGOptnsBorderBox(3,  44, 101); // Box borders,  keep them at bottom
	DGOptnsBorderBox(3,  44, 111);
	DGOptnsBorderBox(3,  44, 121);
	DGOptnsBorderBox(3,  44, 131);
	DGOptnsBorderBox(3,  44, 141);
	DGOptnsBorderBox(3,  44, 151);
	DGOptnsBorderBox(3,  84, 101);
	DGOptnsBorderBox(3,  84, 111);
	DGOptnsBorderBox(3,  84, 121);
	DGOptnsBorderBox(3,  84, 131);
	DGOptnsBorderBox(3,  84, 141);
	DGOptnsBorderBox(3,  84, 151);
	DGOptnsBorderBox(3, 124, 101);
	DGOptnsBorderBox(3, 124, 111);
	DGOptnsBorderBox(3, 124, 121);
	DGOptnsBorderBox(3, 124, 141);
	DGOptnsBorderBox(3, 124, 151);
	DGOptnsBorderBox(3, 164, 101);
	DGOptnsBorderBox(3, 164, 111);
	DGOptnsBorderBox(3, 164, 121);
	DGOptnsBorderBox(3, 164, 141);
	DGOptnsBorderBox(3, 164, 151);
}


void DisplayGUIOption(void)
{
	GUIDrawWindowBox(4, "OPTIONS");

	{ u4 eax;
		u4 ebx;
		GUIDrawTArea(4, &eax, &ebx);
		GUIDrawTabs(GUIOptionTabs, &eax, ebx);
	}

	if (GUIOptionTabs[0] == 1)
	{ // Basic
		GUIDisplayTextY(4, 11, 26, "SYSTEM:");
		if (ShowMMXSupport == 1)
		{
			GUIDisplayCheckboxu(4, 11, 31, &MMXSupport, "ENABLE MMX SUPPORT", 7);
		}
		GUIDisplayCheckboxu(4, 11, 41, &Show224Lines, "SHOW 224 LINES", 9);

		GUIDisplayTextY(4, 11, 66, "GFX ENGINES:");
		GUIDisplayCheckboxu(4, 11, 71, &newengen, "USE NEW GFX ENG", 4);
		if (newengen == 0)
		{
			GUIDisplayCheckboxu(4, 11, 81, &bgfixer, "USE ALT OLD GFX ENG", 4);
		}

		GUIDisplayTextY(4, 11, 106, "ROM:");
		GUIDisplayCheckboxu(4, 11, 111, &AutoPatch,   "ENABLE IPS AUTO-PATCHING", 7);
		GUIDisplayCheckboxu(4, 11, 121, &DisplayInfo, "SHOW ROM INFO ON LOAD",    5);
		GUIDisplayCheckboxu(4, 11, 131, &RomInfo,     "LOG ROM INFO",             2);

#ifdef __WIN32__
		GUIDisplayTextY(4, 11, 156, "WINDOWS SPECIFIC:");
		GUIDisplayCheckboxu(4, 11, 161, &PauseFocusChange,   "PAUSE EMU IN BACKGROUND",  13);
		GUIDisplayCheckboxu(4, 11, 171, &HighPriority,       "INCREASE EMU PRIORITY",    13);
#endif
		GUIDisplayCheckboxu(4, 11, 181, &DisableScreenSaver, "DISABLE POWER MANAGEMENT",  0);
	}

	if (GUIOptionTabs[0] == 2)
	{
		GUIDisplayTextY(4, 11, 26, "OVERLAYS:");
		GUIDisplayCheckboxu(4, 11, 31, &FPSAtStart,  "SHOW FPS CNTR ON EMU LOAD", 5);
		GUIDisplayCheckboxu(4, 11, 41, &TimerEnable, "SHOW CLOCK",                5);
		if (TimerEnable == 1)
		{
			GUIDisplayCheckboxu(4, 89, 41, &TwelveHourClock, "12 HOUR MODE",    3);
			GUIDisplayCheckboxu(4, 11, 51, &ClockBox,        "SHOW CLOCK BOX", 13);
		}

		GUIDisplayTextY(4, 11, 76, "MESSAGES:");
		GUIDisplayCheckboxu(4, 11, 81, &SmallMsgText,    "USE SMALL MESSAGE TEXT", 4);
		GUIDisplayCheckboxu(4, 11, 91, &GUIEnableTransp, "USE TRANSPARENT TEXT",   4);

		GUIDisplayTextY(4, 11, 116, "SCREENSHOT FORMAT:");
		GUIDisplayButtonHoleTu(4, 11, 121, &ScreenShotFormat, 0, "BMP", 0);
#ifndef NO_PNG
		GUIDisplayButtonHoleTu(4, 11, 131, &ScreenShotFormat, 1, "PNG", 0);
#endif
	}
}


static u1 glscslidSet(void const* const p1) // slider variable
{
	return *(u1 const*)p1;
}


static char const* glscslidText(void const* const p1) // slider var, text
{
	static char GUIVideoTextB2z[] = "---%";
	sprintf(GUIVideoTextB2z, "%3d", *(u1 const*)p1);
	return GUIVideoTextB2z;
}


static u1 NTSCslidSet(void const* const p1) // slider variable
{
	return *(s1 const*)p1 + 100;
}


static char const* NTSCslidText(void const* const p1) // slider var, text
{
	static char GUIVideoTextCD3[] = "----%";
	sprintf(GUIVideoTextCD3, "%4d", *(s1 const*)p1);
	return GUIVideoTextCD3;
}


void DisplayGUIVideo(void)
{
	// Check features
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

	GUIDrawWindowBox(5, "VIDEO CONFIG");

	if (GUINTVID[cvidmode] == 0)
	{ // not NTSC
		NTSCFilter     = 0;
		GUIVntscTab[0] = 0;
		if ((GUIVideoTabs[0] & 0xFF) == 0) GUIVideoTabs[0] = GUIVideoTabs[0] & 0xFFFFFF00 | 1;
	}

	{ u4 eax;
		u4 ebx;
		GUIDrawTArea(5, &eax, &ebx);
		GUIDrawTabs(GUIVideoTabs, &eax, ebx);
		if (NTSCFilter != 0) GUIDrawTabs(GUIVntscTab, &eax, ebx);
	}

	if (GUIVideoTabs[0] == 1) // Video Modes List/Options Tab
	{
		DrawGUIButton(5, 128, 30, 164, 41, "SET", 4, 0, 0); // Mode Set Button

#ifndef __MSDOS__ // Legend
		GUIDisplayTextY(5, 130, 50, "LEGEND:");
		GUIDisplayText( 5, 130, 58, "D = ALLOW FILTERS");
		GUIDisplayText( 5, 130, 66, "S = STRETCH");
		GUIDisplayText( 5, 130, 74, "R = KEEP 8:7 RATIO");
		GUIDisplayText( 5, 130, 82, "W = WINDOWED");
		GUIDisplayText( 5, 130, 90, "F = FULLSCREEN");
#ifdef __OPENGL__
		GUIDisplayText( 5, 130, 98, "O = USES OPENGL");
#endif

		DrawGUIButton(5, 180, 115, 216, 126, "SET", 12, 0, 0); // Custom Set Button

		GUIDisplayText(5, 130, 120, "CUSTOM:");
		GUIDisplayText(5, 180, 135, "X");
		GUIDisplayBBox(5, 130, 130, 170, 140, 167);
		GUIDisplayBBox(5, 191, 130, 231, 140, 167);

		GetCustomXY();

		GUIOuttextwin2d(5, 138, 133, GUICustomX, 4, GUICustomResTextPtr, 0);
		GUIOuttextwin2d(5, 199, 133, GUICustomY, 4, GUICustomResTextPtr, 1);
#endif

		GUIDisplayBBoxS(5, 5, 26, 115, 189, 167); // Video Modes Box
		DrawSlideBarWin(5, 117, 34, GUIcurrentvideoviewloc, NumVideoModes, 20, 148, GUIVStA);
		// Scrollbar
		if ((GUICHold & 0xFF) == 5) GUIWincoladd = GUIWincoladd & 0xFFFFFF00 | (GUIWincoladd + 3) & 0x000000FF;
		GUIDisplayIconWin(5, 117, 26, GUIIconDataUpArrow);
		if ((GUICHold & 0xFF) == 5) GUIWincoladd = GUIWincoladd & 0xFFFFFF00 | (GUIWincoladd - 3) & 0x000000FF;
		if ((GUICHold & 0xFF) == 6) GUIWincoladd = GUIWincoladd & 0xFFFFFF00 | (GUIWincoladd + 3) & 0x000000FF;
		GUIDisplayIconWin(5, 117, 182, GUIIconDataDownArrow);
		if ((GUICHold & 0xFF) == 6) GUIWincoladd = GUIWincoladd & 0xFFFFFF00 | (GUIWincoladd - 3) & 0x000000FF;

		u4 const ebx = (GUIcurrentvideocursloc - GUIcurrentvideoviewloc) * 8 + 28; // Box
		DrawGUIWinBox2(5, 5, 115, 7, 224, ebx);

		GUItextcolor[0] = 224; // Text in Box
		GUITemp = (u4)GUIVideoModeNames[GUIcurrentvideoviewloc]; // XXX ugly cast
		u4 const n = NumVideoModes < 20 ? NumVideoModes : 20;
		for (u4 i = 0; i != n; ++i)
		{
			GUIDisplayTextG(5, 11, 30 + 8 * i, (char const*)GUITemp); // XXX ugly cast
			GUITemp += 18;
		}

		GUITemp = (u4)GUIVideoModeNames[cvidmode]; // Mode Value // XXX ugly cast
		GUIDisplayTextY(5,  7, 194, "CURRENT:");
		GUIDisplayTextY(5, 91, 194, (char const*)GUITemp); // (5,61,194) // XXX ugly cast
	}

	// Filters tab
	if (GUIVideoTabs[0] == 2)
	{
		// Video Filters
#ifdef __MSDOS__
		if (smallscreenon != 1)
#endif
		{
#ifdef __MSDOS__
			if (ScreenScale != 1)
#endif
			{
				char const* const GUIVideoTextB1 = "VIDEO FILTERS:"; // Filters.Exclusive
#ifndef __MSDOS__
				// Bilinear
				if (GUIBIFIL[cvidmode] != 0)
				{
					GUIDisplayTextY(5, 13, 30, GUIVideoTextB1);
					GUIDisplayCheckboxu(5, 18, 35, &BilinearFilter, "BILINEAR FILTER", 1);
				}
				else
#endif
				{
					// Interpolations
#ifdef __WIN32__
					if (GUIDSIZE[cvidmode] != 0)
#else
					if (GUII2VID[cvidmode] != 0)
#endif
					{
						GUIDisplayTextY(5, 13, 30, GUIVideoTextB1);
						GUIDisplayCheckboxu(5, 18, 35, &antienab, "INTERPOLATION", 0); // -y
					}
#ifdef __MSDOS__ // Eagle Filter
					if (GUIEAVID[cvidmode] != 0)
					{
						GUIDisplayTextY(5, 13, 30, GUIVideoTextB1);
						GUIDisplayCheckboxu(5, 18, 35, &antienab, "EAGLE ENGINE", 9); // same loc at interpolation   -y
					}
#endif
				}

				// NTSC filter
				if (GUINTVID[cvidmode] != 0) GUIDisplayCheckboxu(5, 128, 35, &NTSCFilter, "NTSC FILTER", 0);

				if (MMXSupport != 0)
				{ // Kreed 2x filters
#ifdef __MSDOS__
					if (GUI2xVID[cvidmode] != 0)
#else
					if (GUIDSIZE[cvidmode] != 0)
#endif
					{
						GUIDisplayCheckboxun(5,  18, 45, &En2xSaI, 1, "2XSAI ENGINE", 2); // 2x
						GUIDisplayCheckboxun(5, 128, 45, &En2xSaI, 2, "SUPER EAGLE",  6); // Seagle
						GUIDisplayCheckboxun(5,  18, 55, &En2xSaI, 3, "SUPER 2XSAI",  2); // S2x
					}
				}

				if (MMXSupport != 0)
				{
					// Hq*x
					if (GUIHQ2X[cvidmode] != 0)
					{
#ifdef __MSDOS__
						GUIDisplayCheckboxu(5, 128, 55, &hqFilter, "HQ2X", 1);
#else
						GUIDisplayCheckboxu(5, 128, 55, &hqFilter, "HQ FILTER", 1);
						if (hqFilter != 0)
						{
							GUIDisplayButtonHoleTu(5, 128, 68, &hqFilterlevel, 2, "2X", 1);
							goto hq_x;
						}
#endif
					}
					else
					{
hq_x:;
#ifndef __MSDOS__
						if (GUIHQ3X[cvidmode] != 0) GUIDisplayButtonHoleTu(5, 158, 68, &hqFilterlevel, 3, "3X", 0);
						if (GUIHQ4X[cvidmode] != 0) GUIDisplayButtonHoleTu(5, 188, 68, &hqFilterlevel, 4, "4X", 0);
#endif
					}
				}
			}

			char const* const GUIVideoTextB2 = "SCANLINES:"; // Filters.Scanlines
#ifndef __MSDOS__
			// GL Scanlines
			if (GUIBIFIL[cvidmode] != 0)
			{
				GUIDisplayTextY(5, 13, 80, GUIVideoTextB2); // Scanlines text
				GUIDrawSlider(5, 23, 100, 90, &sl_intensity, glscslidSet, glscslidText);
			}
			else
#endif
			{
				// Scanlines
#ifdef __MSDOS__
				if (GUISLVID[cvidmode] != 0)
#else
				if (GUIDSIZE[cvidmode] != 0)
#endif
				{
					GUIDisplayTextY(5, 13, 80, GUIVideoTextB2); // Scanlines text
					GUIDisplayButtonHoleTu(5,  18, 87, &scanlines, 0, "NONE", 1); // None
					GUIDisplayButtonHoleTu(5, 168, 87, &scanlines, 1, "FULL", 0); // Full
				}
#ifdef __MSDOS__
				if (ScreenScale != 1 && GUIHSVID[cvidmode] != 0)
#else
				if (GUIDSIZE[cvidmode] != 0)
#endif
				{
					GUIDisplayButtonHoleTu(5,  68, 87, &scanlines, 2, "25%", 0); // 25%
					GUIDisplayButtonHoleTu(5, 118, 87, &scanlines, 3, "50%", 0); // 50%
				}
			}
		}

		GUIDisplayTextY(5, 13, 110, "MISC FILTERS:"); // Filters.Other
		GUIDisplayCheckboxu(5, 18, 115, &GrayscaleMode, "GRAYSCALE MODE", 0); // -v8

		// Hires Mode7
		if (GUIM7VID[cvidmode] != 0 && newengen != 0)
		{
			GUIDisplayCheckboxu(5, 128, 115, &Mode7HiRes16b, "HI-RES MODE 7", 0);
		}

		// Monitor Refresh
		// VSync
#if !defined __UNIXSDL__ || defined __OPENGL__
#	ifdef __UNIXSDL__
		if (allow_glvsync == 1 && GUIBIFIL[cvidmode] != 0)
#	endif
		{
			GUIDisplayTextY(5, 13, 140, "MONITOR SYNC:"); // Video.Sync
			GUIDisplayCheckboxu(5, 18, 145, &vsyncon, "VSYNC", 0); // -w
		}
#endif

		// Triple Buffering
#ifndef __UNIXSDL__
		char const* const GUIVideoTextB4b = "TRIPLE BUFFERING"; // -3
#endif
#ifdef __WIN32__
		if (GUIWFVID[cvidmode] != 0)
		{
			GUIDisplayCheckboxu(5, 128, 145, &TripleBufferWin, GUIVideoTextB4b, 0);
		}
#endif
#ifdef __MSDOS__
		if (GUITBVID[cvidmode] != 0 && TripBufAvail != 0)
		{
			GUIDisplayCheckboxu(5, 128, 145, &Triplebufen, GUIVideoTextB4b, 0);
		}
#endif

		char const* const GUIVideoTextB5 = "DISPLAY OPTIONS:"; // Video.Display
#ifndef __MSDOS__
		// Keep 4:3 Ratio
		if (GUIKEEP43[cvidmode] != 0 && Keep43Check())
		{
			GUIDisplayTextY(5, 13, 170, GUIVideoTextB5);
			GUIDisplayCheckboxu(5, 18, 175, &Keep4_3Ratio, "USE 4:3 RATIO", 8);
		}
#else
		// Small Screen
		if (GUISSVID[cvidmode] != 0)
		{
			GUIDisplayTextY(5, 13, 170, GUIVideoTextB5);
			GUIDisplayCheckboxu(5, 18, 175, &smallscreenon, "SMALL SCREEN", 1); // -c
		}

		// Full/Widescreen
		if (GUIWSVID[cvidmode] != 0)
		{
			GUIDisplayTextY(5, 13, 170, GUIVideoTextB5);
			GUIDisplayCheckboxu(5, 128, 175, &ScreenScale, "WIDE SCREEN", 6); // -cc
		}
#endif
	}

	char const* const GUIVideoTextCD1 = "RESET"; // NTSC buttons + counter
	char const* const GUIVideoTextCD2 = "RESET ALL";

	if (GUIVntscTab[0] == 1) // NTSC Tab
	{
		GUIDisplayCheckboxu(5,   5, 25, &NTSCBlend, "BLEND FRAMES", 0); // NTSC Tab
		GUIDisplayCheckboxu(5, 135, 25, &NTSCRef,   "REFRESH",      0);

		DrawGUIButton(5,   8, 166,  67, 177, "COMPOSITE",     81, 0, 0);
		DrawGUIButton(5,  72, 166, 119, 177, "S-VIDEO",       82, 0, 0);
		DrawGUIButton(5, 124, 166, 147, 177, "RGB",           83, 0, 0);
		DrawGUIButton(5, 152, 166, 217, 177, "MONOCHROME",    84, 0, 0);
		DrawGUIButton(5, 102, 186, 137, 197, GUIVideoTextCD1, 37, 0, 0);
		DrawGUIButton(5, 148, 186, 207, 197, GUIVideoTextCD2, 39, 0, 0);

		GUIDisplayTextY(5, 7,  46, "HUE:");
		GUIDisplayTextY(5, 7,  66, "SATURATION:");
		GUIDisplayTextY(5, 7,  86, "CONTRAST:");
		GUIDisplayTextY(5, 7, 106, "BRIGHTNESS:");
		GUIDisplayTextY(5, 7, 126, "SHARPNESS:");
		GUIDisplayTextY(5, 7, 156, "PRESETS:"); // NTSC Presets

		GUIDrawSlider(5, 8, 200,  56, &NTSCHue,    NTSCslidSet, NTSCslidText);
		GUIDrawSlider(5, 8, 200,  76, &NTSCSat,    NTSCslidSet, NTSCslidText);
		GUIDrawSlider(5, 8, 200,  96, &NTSCCont,   NTSCslidSet, NTSCslidText);
		GUIDrawSlider(5, 8, 200, 116, &NTSCBright, NTSCslidSet, NTSCslidText);
		GUIDrawSlider(5, 8, 200, 136, &NTSCSharp,  NTSCslidSet, NTSCslidText);
	}

	if (GUIVntscTab[0] == 2) // Advanced NTSC Options Tab
	{
		DrawGUIButton(5, 102, 186, 137, 197, GUIVideoTextCD1, 38, 0, 0);
		DrawGUIButton(5, 148, 186, 207, 197, GUIVideoTextCD2, 39, 0, 0);

		GUIDisplayTextY(5, 7,  36, "GAMMA:"); // NTSC Adv Tab
		GUIDisplayTextY(5, 7,  56, "RESOLUTION:");
		GUIDisplayTextY(5, 7,  76, "ARTIFACTS:");
		GUIDisplayTextY(5, 7,  96, "FRINGING:");
		GUIDisplayTextY(5, 7, 116, "BLEED:");
		GUIDisplayTextY(5, 7, 136, "HUE WARPING:");

		GUIDrawSlider(5, 8, 200,  46, &NTSCGamma,  NTSCslidSet, NTSCslidText);
		GUIDrawSlider(5, 8, 200,  66, &NTSCRes,    NTSCslidSet, NTSCslidText);
		GUIDrawSlider(5, 8, 200,  86, &NTSCArt,    NTSCslidSet, NTSCslidText);
		GUIDrawSlider(5, 8, 200, 106, &NTSCFringe, NTSCslidSet, NTSCslidText);
		GUIDrawSlider(5, 8, 200, 126, &NTSCBleed,  NTSCslidSet, NTSCslidText);
		GUIDrawSlider(5, 8, 200, 146, &NTSCWarp,   NTSCslidSet, NTSCslidText);
	}
}


void DisplayGUISound(void)
{
	GUIDrawWindowBox(6, "SOUND CONFIG");

	GUIDisplayTextY(6, 6, 16, "SOUND SWITCHES:");
	GUIDisplayCheckboxu(6, 11, 21, &SPCDisable, "DISABLE SPC EMULATION", 0);
	if (SPCDisable == 0)
	{
		GUIDisplayCheckboxu(6, 11, 31, &soundon, "ENABLE SOUND", 0);
		if (soundon == 1)
		{
			GUIDisplayCheckboxu(6, 11, 41, &StereoSound, "ENABLE STEREO SOUND", 7);
			if (StereoSound == 1)
			{
				GUIDisplayCheckboxu(6, 11, 51, &RevStereo, "REVERSE STEREO CHANNELS", 2);
				GUIDisplayCheckboxu(6, 11, 61, &Surround,  "SIMULATE SURROUND SOUND", 2);
			}
#ifdef __MSDOS__
			GUIDisplayCheckboxu(6, 11, 71, &Force8b, "FORCE 8-BIT OUTPUT", 0);
#endif
#ifdef __WIN32__
			GUIDisplayCheckboxu(6, 11, 71, &PrimaryBuffer, "USE PRIMARY BUFFER", 4);
#endif
		}
	}

	char const* const GUISoundTextF = "NONE";

	GUIDisplayTextY(6, 6, 152, "INTERPOLATION:");
	GUIDisplayButtonHoleTu(6, 11, 157, &SoundInterpType, 0, GUISoundTextF,  0);
	GUIDisplayButtonHoleTu(6, 11, 167, &SoundInterpType, 1, "GAUSSIAN", 0);
	GUIDisplayButtonHoleTu(6, 11, 177, &SoundInterpType, 2, "CUBIC SPLINE", 0);
	if (MMXSupport != 0)
	{
		GUIDisplayButtonHoleTu(6, 11, 187, &SoundInterpType, 3, "8-POINT", 0);
	}

	GUIDisplayTextY(6, 106, 152, "LOWPASS:");
	GUIDisplayButtonHoleTu(6, 111, 157, &LowPassFilterType, 0, GUISoundTextF, 1);
	GUIDisplayButtonHoleTu(6, 111, 167, &LowPassFilterType, 1, "SIMPLE",      1);
	GUIDisplayButtonHoleTu(6, 111, 177, &LowPassFilterType, 2, "DYNAMIC",     1);
	if (MMXSupport != 0)
	{
		GUIDisplayButtonHoleTu(6, 111, 187, &LowPassFilterType, 3, "HI QUALITY", 0);
	}

	GUIDisplayTextY(6, 6, 93, "SAMPLING RATE:");
#ifdef __MSDOS__
	if ((SoundQuality & 0xFF) > 2 && (SoundQuality & 0xFF) != 4 && StereoSound == 1 && SBHDMA == 0 && vibracard != 1)
	{
		GUIDisplayBBox(6, 15, 101, 69, 109, 167);
		GUIDisplayTextG(6, 23, 104, "N/A");
	}
	else
#endif
	{
		GUIDisplayBBox(6, 15, 101, 69, 109, 167); // Sampling Rate Box
		static char const GUISoundTextB1[][8] =
		{
			" 8000HZ",
			"11025HZ",
			"22050HZ",
			"44100HZ",
			"16000HZ",
			"32000HZ",
			"48000HZ"
		};
		char const* const eax = GUISoundTextB1[SoundQuality];
		GUITemp = (u4)eax; // XXX ugly cast
		GUIDisplayTextG(6, 23, 104, (char const*)GUITemp); // XXX ugly cast
	}

	GUIDisplayTextY(6, 6, 116, "VOLUME LEVEL:");
	GUIDrawSlider(6, 15, 100, 131, &MusicRelVol, glscslidSet, glscslidText);
}


static char const* DisplayGUICheatConv(u1 const* const c)
{
	char const* const tgl  =
		c[-28] & 0x80 ? "SRC" :
		c[0]   & 0x04 ? "OFF" :
		c[0]   & 0x80 ? "RPL" :
		"ON ";
	char const* const desc = (char const*)(c + 8);

	static char GUICheatTextZ3[6 + 1 + 2 + 1 + 2 + 1 + 3 + 1 + 20 + 1];
	sprintf(GUICheatTextZ3, "%06X %02X %02X %s %.20s", c[4] << 16 | c[3] << 8 | c[2], c[1], c[5], tgl, desc);
	return GUICheatTextZ3;
}


void DisplayGUICheat(void)
{
	GUIDrawWindowBox(7, "CHEAT");

	GUIDisplayText(7,  6,  13, "ADDRESS CV PV TGL DESCRIPTION"); // Top
	GUIDisplayText(7,  6, 132, "ENTER CODE:"); // Text by input boxes
	GUIDisplayText(7,  6, 143, "DESCRIPTION:");
	GUIDisplayText(7, 11, 154, "VALID CODES: GAME GENIE, PAR, AND GF"); // Info for User
	GUIDisplayText(7, 11, 164, "NOTE: YOU MAY HAVE TO RESET THE GAME");
	GUIDisplayText(7, 11, 172, "AFTER ENTERING THE CODE. REMEMBER TO");
	GUIDisplayText(7, 11, 180, "INSERT THE \"-\" FOR GAME GENIE CODES.");

	GUItextcolor[0] = (GUIWincoladd & 0xFF) == 0 ? 217 : 211;
	DrawGUIButton(7,   5, 113,  47, 124, "REMOVE",   5, 0, 0); // Draw Buttons
	DrawGUIButton(7,  52, 113,  94, 124, "TOGGLE",   6, 0, 0);
	DrawGUIButton(7,  99, 113, 141, 124, "SAVE",     7, 0, 0);
	DrawGUIButton(7, 146, 113, 188, 124, "LOAD",     8, 0, 0);
	DrawGUIButton(7, 193, 113, 235, 124, "FIX",     33, 0, 0);
	DrawGUIButton(7, 212, 134, 236, 145, "ADD",      9, 0, 0);

	GUIDisplayBBoxS(7, 5, 20, 229, 108, 167); // Draw Cheat Box

	if (GUIcurrentcheatwin == 0) // Red Highlight for Cheats box
	{
		u4 const ebx = 22 + (GUIcurrentcheatcursloc - GUIcurrentcheatviewloc) * 7;
		DrawGUIWinBox2(7, 5, 229, 7, 224, ebx);
	}

	u1 const* ccheatnpos  = cheatdata + GUIcurrentcheatviewloc * 28; // Green Text
	ccheatnleft = NumCheats - GUIcurrentcheatviewloc - 1;
	for (u4 i = 0; i != 12; ++i)
	{
		if (!(ccheatnleft & 0x80000000))
		{
			u4          const p1             = 12;
			u4          const p2             = 24 + 7 * i;
			char const* const GUICheatTextZ3 = DisplayGUICheatConv(ccheatnpos);
			GUItextcolor[0] = 223;
			GUIOuttextwin2(7, p1, p2, GUICheatTextZ3);
			GUItextcolor[0] = (GUIWincoladd & 0xFF) == 0 ? 221 : 222; // Text
			GUIOuttextwin2(7, p1 - 1, p2 - 1, GUICheatTextZ3);
			ccheatnpos += 28;
			--ccheatnleft;
		}
	}

	// Scrollbar
	DrawSlideBarWin(7, 231, 28, GUIcurrentcheatviewloc, NumCheats, 12, 73, GUICStA);
	if ((GUICHold & 0xFF) == 7) GUIWincoladd = GUIWincoladd & 0xFFFFFF00 | (GUIWincoladd + 3) & 0x000000FF;
	GUIDisplayIconWin(7, 231, 20, GUIIconDataUpArrow);
	if ((GUICHold & 0xFF) == 7) GUIWincoladd = GUIWincoladd & 0xFFFFFF00 | (GUIWincoladd - 3) & 0x000000FF;
	if ((GUICHold & 0xFF) == 8) GUIWincoladd = GUIWincoladd & 0xFFFFFF00 | (GUIWincoladd + 3) & 0x000000FF;
	GUIDisplayIconWin(7, 231, 101, GUIIconDataDownArrow);
	if ((GUICHold & 0xFF) == 8) GUIWincoladd = GUIWincoladd & 0xFFFFFF00 | (GUIWincoladd - 3) & 0x000000FF;

	{ // Code Box
		u1 const dl =
			GUIcurrentcheatwin    != 1 ? 167 :
			(GUIWincoladd & 0xFF) == 0 ? 226 :
			227;
		GUIDisplayBBox(7, 82, 129, 172, 136, dl);
	}

	{ // Description Box
		u1 const dl =
			GUIcurrentcheatwin    != 2 ? 167 :
			(GUIWincoladd & 0xFF) == 0 ? 226 :
			227;
		GUIDisplayBBox(7, 82, 140, 196, 147, dl);
	}

	GUIDisplayTextG(7, 84, 132, GUICheatTextZ1); // Green Text&Shadow
	GUIDisplayTextG(7, 84, 143, GUICheatTextZ2);

	// Code for movement of cursor
	u1 const eax = GUICheatPosA;
	GUICheatTextZ1[eax] = '\0';
	u1 const ebx = GUICheatPosB;
	GUICheatTextZ2[ebx] = '\0';
	if (!(GUICCFlash & 8))
	{
		switch (GUIcurrentcheatwin)
		{
			case 1: GUICheatTextZ1[eax] = '_'; break;
			case 2: GUICheatTextZ2[ebx] = '_'; break;
		}
	}

	char const* const GUICheatTextE1 = "AUTO-LOAD .CHT FILE AT GAME LOAD";
	if (GUIcurrentcheatwin == 0)
	{ // Draw underline only if you don't have an input box selected
		GUIDisplayCheckboxu(7, 11, 186, &AutoLoadCht, GUICheatTextE1, 0);
	}
	else
	{
		GUIDisplayCheckbox(7, 11, 186, &AutoLoadCht, GUICheatTextE1);
	}
}


static void DrawWindowSearch(void)
{
	GUIDrawWindowBox(13, "CHEAT SEARCH");
}


static u4 FindChtSrcRes(u4 edi) // Calculate search results
{
	++edi;
	u1 const* eax = vidbuffer + 129600 + 65536 * 2;
	u4        ecx = 16384;
	u4        esi = 0;
	u4        ebx = 0;
	do
	{
		u1 dl = *eax++;
		u1 dh = 8;
		do
		{
			if (dl & 1)
			{
				++ebx;
				if (--edi == 0) CSStartEntry = esi;
			}
			++esi;
			dl >>= 1;
		}
		while (--dh != 0);
	}
	while (--ecx != 0);
	return ebx;
}


static void DisplayChtSrcResNoSearch(void)
{
	DrawGUIButton(13, 10, 140,  60, 152, "RESTART", 51, 0, 1);
	DrawGUIButton(13, 70, 140, 110, 152, "VIEW",    52, 0, 1);

	// Call and display # of results
	convertnum(GUICSrcTextG1, FindChtSrcRes(0));
	GUIDisplayText(13, 12, 125, "# OF RESULTS:");
	GUIDisplayText(13, 97, 125, GUICSrcTextG1);
	GUIcurrentchtsrcviewloc = 0;
	GUIcurrentchtsrccursloc = 0;
}


static void DisplayChtSrcRes(void) // Buttons (Restart/View/Search)
{
	DrawGUIButton(13, 120, 140, 170, 152, "SEARCH", 53, 0, 1);
	DisplayChtSrcResNoSearch();
}


static void CheatSearchingComp(void) // Comparative search
{
	GUIDisplayTextY(13, 6, 16, "SELECT COMPARISON:");
	GUIDisplayButtonHoleTu(13, 11, 33, &CheatCompareValue, 0, "NEW VALUE IS > OLD VALUE",  0);
	GUIDisplayButtonHoleTu(13, 11, 43, &CheatCompareValue, 1, "NEW VALUE IS < OLD VALUE",  1);
	GUIDisplayButtonHoleTu(13, 11, 53, &CheatCompareValue, 2, "NEW VALUE IS = OLD VALUE",  2);
	GUIDisplayButtonHoleTu(13, 11, 63, &CheatCompareValue, 3, "NEW VALUE IS != OLD VALUE", 5);
	DisplayChtSrcRes();
}


static void CSRemoveFlash(char* const str)
{
	char* const i = strchr(str, '_');
	if (i) *i = '\0';
}


static void CSAddFlash(char* i)
{
	for (; *i != '\0'; ++i)
	{
		if (*i == '_') return;
	}
	i[0] = '_';
	i[1] = '\0';
}


static void CheatSearching(void) // Exact Value Search
{
	if (CheatSrcSearchType == 1)
	{
		CheatSearchingComp();
		return;
	}

	GUIDisplayText(13, 5, 20, "ENTER VALUE:"); // Make Yellow
	GUIDisplayText(13, 5, 65, "MAX VALUE:");

	GUIDisplayBBox(13, 10, 40, 80, 47, 167); // Input Box

	if (!(GUICCFlash & 8)) CSRemoveFlash(CSInputDisplay); // Flash Cursor Code?

	GUItextcolor[0] = CSOverValue == 1 ? 202 /* Alt Color */ : 223 /* Green Shadow */;
	GUIOuttextwin2(13, 13, 42, CSInputDisplay);

	GUItextcolor[0] =
		CSOverValue           == 1 ? 207 : // Alt Color
		(GUIWincoladd & 0xFF) == 0 ? 221 : // Green Text
		222;
	GUIOuttextwin2(13, 12, 41, CSInputDisplay);

	CSAddFlash(CSInputDisplay); // More flash?

	u4    const eax = SrcMask[CheatSrcByteSize]; // Find Max Size
	char* const esi = GUICSrcTextG1;
	if (CheatSrcByteBase != 1)
	{
		convertnum(esi, eax);
	}
	else
	{
		converthex(esi, eax, CheatSrcByteSize + 1);
	}
	GUIDisplayText(13, 71, 65, GUICSrcTextG1); // Max Size Text
	DisplayChtSrcRes();
}


static void Incheatmode(void) // Return and Re-search Window
{
	GUIwinsizex[13] = 180;
	GUIwinsizey[13] = 150;
	DrawWindowSearch();

	GUItextcolor[0] = (GUIWincoladd & 0xFF) == 0 ? 217 : 211; // Text And Shadow
	if (CheatSearchStatus != 1)
	{
		CheatSearching();
	}
	else
	{
		GUIDisplayText(13, 5, 20, "NOW RETURN TO YOUR GAME");
		GUIDisplayText(13, 5, 30, "AND COME BACK WHEN");
		GUIDisplayText(13, 5, 40, "THE NEXT SEARCH");
		GUIDisplayText(13, 5, 50, "SHOULD BE PROCESSED");
		DisplayChtSrcResNoSearch();
	}
}


static void Cheatmodeview(void) // View ResultsWindow
{
	static char GUICSrcTextE[] = "ADDR   VALUE     PVALUE";
	GUICSrcTextE[12] = CheatSrcByteSize == 3 || CheatSrcByteBase != 0 ? ' ' : '\0';

	GUIwinsizex[13] = 185;
	GUIwinsizey[13] = 150;
	DrawWindowSearch();

	GUIDisplayText(13, 10, 12, GUICSrcTextE); // Text

	GUIDisplayBBoxS(13, 5, 20, 171, 108, 167); // Box

	NumCheatSrc = FindChtSrcRes(GUIcurrentchtsrcviewloc);

	GUItextcolor[0] = 223; // Display Window Contents
	u4 eax = NumCheatSrc - GUIcurrentchtsrcviewloc;
	if (eax > 12) eax = 12;
	ccheatnleft  = eax;
	ccheatnleftb = eax;

	CheatSearchYPos = 24;
	CheatSearchXPos = 10;
	CSCurEntry      = CSStartEntry;

	if (ccheatnleft != 0)
	{
		u1 CheatLooped  = 0;
		u4 curentryleft = GUIcurrentchtsrccursloc - GUIcurrentchtsrcviewloc;
		DrawGUIWinBox2(13, 5, 171, 7, 224, 22 + curentryleft * 7);
		do
		{
			do
			{
				if (curentryleft-- == 0) curentryval = CSCurEntry;

				converthex(GUICSrcTextG1, CSCurEntry + 0x7E0000, 3);
				GUIOuttextwin2(13, CheatSearchXPos, CheatSearchYPos, GUICSrcTextG1);

				char* const esi = GUICSrcTextG1;
				u4    const eax = *(u4 const*)(wramdata + CSCurEntry); // XXX ugly cast
				if (CheatSrcByteBase != 0)
				{
					converthex(esi, eax, CheatSrcByteSize + 1);
				}
				else
				{
					convertnum(esi, eax & SrcMask[CheatSrcByteSize]);
				}
				CheatSearchXPos += 42;
				GUIOuttextwin2(13, CheatSearchXPos, CheatSearchYPos, GUICSrcTextG1);

				CheatSearchXPos += 60;
				if (GUICSrcTextE[12] != '\0')
				{
					char* const esi = GUICSrcTextG1;
					u4    const eax = *(u4 const*)(vidbuffer + 129600 + CSCurEntry); // XXX ugly cast
					if (CheatSrcByteBase != 0)
					{
						converthex(esi, eax, CheatSrcByteSize + 1);
					}
					else
					{
						convertnum(esi, eax & SrcMask[CheatSrcByteSize]);
					}
					GUIOuttextwin2(13, CheatSearchXPos, CheatSearchYPos, GUICSrcTextG1);
				}
				CheatSearchXPos -= 102;
				CheatSearchYPos +=   7;

				do // Search for next entry
				{
					++CSCurEntry;
				}
				while (!(vidbuffer[129600 + 65536 * 2 + (CSCurEntry >> 3)] & (1 << (CSCurEntry & 7))));
			}
			while (--ccheatnleft != 0);
			GUItextcolor[0] = (GUIWincoladd & 0xFF) == 0 ? 221 : 222;
			CheatSearchYPos = 23;
			CheatSearchXPos = 11;
			CSCurEntry      = CSStartEntry;
			ccheatnleft     = ccheatnleftb;
		}
		while (++CheatLooped != 2);
	}
	// Slidebar
	// win#,X,Y start, %4-List Loc, %5-List size, %6-Screen size, %7-Bar Size
	DrawSlideBarWin(13, 173, 28, GUIcurrentchtsrcviewloc, NumCheatSrc, 12, 73, GUICSStA);
	if ((GUICHold & 0xFF) == 11) GUIWincoladd = GUIWincoladd & 0xFFFFFF00 | (GUIWincoladd + 3) & 0x000000FF;
	GUIDisplayIconWin(13, 173, 20, GUIIconDataUpArrow);
	if ((GUICHold & 0xFF) == 11) GUIWincoladd = GUIWincoladd & 0xFFFFFF00 | (GUIWincoladd - 3) & 0x000000FF;
	if ((GUICHold & 0xFF) == 12) GUIWincoladd = GUIWincoladd & 0xFFFFFF00 | (GUIWincoladd + 3) & 0x000000FF;
	GUIDisplayIconWin(13, 173, 101, GUIIconDataDownArrow);
	if ((GUICHold & 0xFF) == 12) GUIWincoladd = GUIWincoladd & 0xFFFFFF00 | (GUIWincoladd - 3) & 0x000000FF;
	GUItextcolor[0] = (GUIWincoladd & 0xFF) == 0 ? 217 : 211;
	DrawGUIButton(13,  70, 140, 130, 152, "RETURN", 54, 0, 1);
	DrawGUIButton(13, 140, 140, 180, 152, "ADD",    55, 0, 1);
}


static void Cheatmodeadd(void) // Add Window
{
	GUIwinsizex[13] = 170;
	GUIwinsizey[13] = 165;
	DrawWindowSearch();

	GUIDisplayText(13, 5, 20, "ENTER NEW VALUE:"); // Text
	GUIDisplayText(13, 5, 45, "ENTER CHEAT DESCRIPTION:");
	GUIDisplayText(13, 5, 70, "PAR CODE EQUIVALENT:");

	GUIDisplayCheckbox(13, 8, 139, &CheatUpperByteOnly, "USE ONLY UPPER BYTE"); // Checkbox

	GUIDisplayBBox(13, 10, 30,  80,  37, 167); // Boxes
	GUIDisplayBBox(13, 10, 55, 126,  62, 167);
	GUIDisplayBBox(13, 10, 80,  80, 120, 167);

	DrawGUIButton(13,  60, 155, 120, 167, "RETURN", 56, 0, 1); // Buttons
	DrawGUIButton(13, 130, 155, 160, 167, "ADD",    57, 0, 1);

	GUIDisplayText(13, 5, 130, "MAX VALUE:"); // Max Value Text
	u4    const eax = SrcMask[CheatSrcByteSize];
	char* const esi = GUICSrcTextG1;
	if (CheatSrcByteBase != 1)
	{ // dec
		convertnum(esi, eax);
	}
	else
	{ // hex
		converthex(esi, eax, CheatSrcByteSize + 1);
	}
	GUIDisplayText(13, 71, 130, GUICSrcTextG1);

	// Cheat Input
	if (CurCStextpos != 0 || !(GUICCFlash & 8)) CSRemoveFlash(CSInputDisplay);
	GUItextcolor[0] = CSOverValue == 1 ? 202 : 223;
	GUIOuttextwin2(13, 13, 32, CSInputDisplay);
	GUItextcolor[0] =
		CSOverValue           == 1 ? 207 :
		(GUIWincoladd & 0xFF) == 0 ? 221 :
		222;
	GUIOuttextwin2(13, 12, 31, CSInputDisplay);
	CSAddFlash(CSInputDisplay);

	// Cheat Desc. Input
	if (CurCStextpos == 1 && !(GUICCFlash & 8)) CSAddFlash(CSDescDisplay);
	GUIDisplayTextG(13, 13, 57, CSDescDisplay);
	CSRemoveFlash(CSDescDisplay);

	if (CSOverValue != 1 && CSInputDisplay[0] != '_')
	{
		CheatSearchYPos = 83; // PAR Code?
		curaddrvalcs = curentryval;
		u4 const eax = CSCurValue;
		curvaluecs = eax;
		u1 ecx = CheatSrcByteSize + 1;
		if (CheatUpperByteOnly != 0)
		{
			ecx = 1;
			while (curvaluecs > 0xFF)
			{
				curvaluecs >>= 8;
				++curaddrvalcs;
			}
		}
		do
		{ // Max Value Display?
			converthex(GUICSrcTextG1,     curaddrvalcs + 0x7E0000, 3);
			converthex(GUICSrcTextG1 + 6, curvaluecs,              1);
			curvaluecs >>= 8;
			GUItextcolor[0] = 223;
			++CheatSearchYPos;
			GUIOuttextwin2(13, 13, CheatSearchYPos, GUICSrcTextG1);
			--CheatSearchYPos;
			GUItextcolor[0] = (GUIWincoladd & 0xFF) == 0 ? 221 : 222;
			GUIOuttextwin2(13, 12, CheatSearchYPos, GUICSrcTextG1);
			CheatSearchYPos += 10;
			++curaddrvalcs;
		}
		while (--ecx != 0);
	}
}


void DisplayGUISearch(void)
{
	switch (CheatWinMode) // Determine which CS window we're on
	{
		case 1: Incheatmode();   return;
		case 2: Cheatmodeview(); return;
		case 3: Cheatmodeadd();  return;
	}

	// Opening Screen
	GUIwinsizex[13] = 170;
	GUIwinsizey[13] = 150;
	DrawWindowSearch();

	// Radio Buttons
	GUIDisplayTextY(       13,  6, 16,                       "SELECT SIZE AND FORMAT:");
	GUIDisplayButtonHoleTu(13, 11, 28, &CheatSrcByteSize, 0, "1 BYTE  [0..255]",        0);
	GUIDisplayButtonHoleTu(13, 11, 38, &CheatSrcByteSize, 1, "2 BYTES [0..65535]",      0);
	GUIDisplayButtonHoleTu(13, 11, 48, &CheatSrcByteSize, 2, "3 BYTES [0..16777215]",   0);
	GUIDisplayButtonHoleTu(13, 11, 58, &CheatSrcByteSize, 3, "4 BYTES [0..4294967295]", 0);
	GUIDisplayButtonHoleTu(13, 11, 73, &CheatSrcByteBase, 0, "DEC (BASE 10)",           0);
	GUIDisplayButtonHoleTu(13, 11, 83, &CheatSrcByteBase, 1, "HEX (BASE 16)",           0);

	GUIDisplayTextY(       13,  6, 101,                         "SELECT SEARCH TYPE:");
	GUIDisplayButtonHoleTu(13, 11, 113, &CheatSrcSearchType, 0, "EXACT VALUE SEARCH", 0);
	GUIDisplayButtonHoleTu(13, 11, 123, &CheatSrcSearchType, 1, "COMPARATIVE SEARCH", 0);

	GUItextcolor[0] = (GUIWincoladd & 0xFF) == 0 ? 217 : 211; // Button
	DrawGUIButton(13, 95, 140, 140, 152, "START", 50, 0, 1);
}
