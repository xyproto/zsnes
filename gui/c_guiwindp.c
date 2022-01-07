#include <stdio.h>
#include <string.h>

#include "../asm_call.h"
#include "../c_init.h"
#include "../c_intrf.h"
#include "../cfg.h"
#include "../cpu/regs.h"
#include "../input.h"
#include "../macros.h"
#include "../ui.h"
#include "../ver.h"
#include "../zmovie.h"
#include "../zpath.h"
#include "../zstate.h"
#include "../ztimec.h"
#include "c_gui.h"
#include "c_guiwindp.h"
#include "gui.h"
#include "guicombo.h"
#include "guifuncs.h"
#include "guikeys.h"
#include "guitools.h"
#include "guiwindp.h"

#ifdef __MSDOS__
#include "../cpu/dspproc.h"
#include "../dos/c_sound.h"
#include "../dos/vesa2.h"
#endif

#if defined __UNIXSDL__ && defined __OPENGL__
#include "../linux/gl_draw.h"
#endif

char CMovieExt = 'v';
char GUICheatTextZ1[16] = "_";
char GUICheatTextZ2[23] = "_";
char GUIChoseSaveText2[] = "-";
char GUIChoseSlotTextX[] = "-";
char GUIComboTextH[21];
char GUILoadTextA[38];
u1 GUIFreshInputSelect = 1;
u1 GUILoadPos;
u1 GUIStatesText5 = 0;
u1 GUIWincoladd;
u1 ShowMMXSupport = 2;
u1* const GUIInputRefP[] = { &pl1contrl, &pl2contrl, &pl3contrl, &pl4contrl, &pl5contrl };
u4 GUIIStA[3];
u4 GUILStA[3];
u4 GUILStB[3];
u4 GUIVStA[3];
u4 GUIcurrentinputcursloc;
u4 GUIcurrentinputviewloc;
u4 GUIcurrentvideocursloc;
u4 GUIcurrentvideoviewloc;

static s4 cloadnleft;
static s4 cloadnposb;
static u4 CSStartEntry;
static u4 GUIWincol;

static void drawshadow2(u4 const p1, s4 const p2, s4 const p3)
{
    s4 y = GUIwinposy[p1];
    if (y > 223)
        return;
    if (y < 0)
        y = 0;
    s4 ebx = y + p3 + 9;
    if (ebx < 0)
        return;
    if (ebx > 223)
        ebx = 223;
    ebx = ebx - y + 1;

    s4 x = GUIwinposx[p1];
    s4 ecx = x;
    if (x > 255)
        return;
    if (x < -3)
        x = -3;
    ecx += p2;
    if (ecx < 0)
        return;
    if (ecx > 255)
        ecx = 255;
    ecx = ecx - x + 1;

    u1* const edi = vidbuffer + (y + 3) * 288 + (x + 3) + 16;
    GUIDrawShadow2(edi, ecx, ebx);
}

static void GUIRect(s4 const x1, s4 const x2, s4 y, u4 h, u1 const colour)
{
    do
        GUIHLine(x1, x2, y++, colour);
    while (--h != 0);
}

static s4 DrawTitleBar(s4 const x1, s4 const x2, s4 ebx)
{
    GUIHLine(x1, x2, ebx++, 46 + 157 + 6 - GUIWincoladd);

    {
        u4 edx = 42 + 157 + 4 + 4 - GUIWincoladd;
        u4 n = 8;
        do
            GUIHLine(x1, x2, ebx++, edx--);
        while (--n != 0);
    }

    GUIHLine(x1, x2, ebx++, 38 + 157 + 4 - GUIWincoladd);

    GUIRect(x1, x1, ebx - 10, 9, 44 + 157 + 4 - GUIWincoladd);
    GUIRect(x2, x2, ebx - 9, 9, 40 + 157 + 4 - GUIWincoladd);

    return ebx;
}

static void GUIDrawTArea(u4 const id, u4* const peax, u4* const pebx) // win #id
{
    u4 const edx = GUIWincol + 1;
    s4 const eax = GUIwinposx[id];
    s4 const ebx = GUIwinposy[id] + 10;
    s4 const ecx = eax + GUIwinsizex[id];
    GUIRect(eax, ecx, ebx, 12, edx);
    GUIHLine(eax + 1, ecx, ebx + 12, edx + 3);
    *peax = eax; // set eax to minX
    *pebx = ebx; // set ebx to minY
}

static void GUIDrawWindowBox(u4 const p1, char const* const p2)
{
    switch (cwindrawn) {
    case 0:
        GUIWincoladd = 0;
        GUIWincol = 148;
        break;
    case 1:
        GUIWincoladd = 4;
        GUIWincol = 148 + 5;
        break;
    default:
        GUIWincoladd = 4;
        GUIWincol = 148 + 10;
        break;
    }

    drawshadow2(p1, GUIwinsizex[p1], GUIwinsizey[p1]);

    {
        s4 const eax = GUIwinposx[p1];
        s4 ebx = GUIwinposy[p1];
        s4 const ecx = eax + GUIwinsizex[p1];
        ebx = DrawTitleBar(eax, ecx, ebx);

        u4 const esi = GUIwinsizey[p1] - 1;
        GUIRect(eax, ecx, ebx, esi, GUIWincol + 2);

        GUIHLine(eax, ecx, ebx + esi, GUIWincol);
    }

    {
        s4 const eax = GUIwinposx[p1];
        s4 const ebx = GUIwinposy[p1] + 10;
        u4 const esi = GUIwinsizey[p1] - 1;
        GUIRect(eax, eax, ebx, esi, GUIWincol + 3);
    }

    {
        s4 const eax = GUIwinposx[p1] + GUIwinsizex[p1];
        s4 const ebx = GUIwinposy[p1] + 10;
        u4 const esi = GUIwinsizey[p1];
        GUIRect(eax, eax, ebx, esi, GUIWincol + 1);
    }

    {
        s4 const ebx = GUIwinposy[p1] + 3;
        s4 const edx = GUIwinposx[p1] + 3;
        GUIOuttextwin(edx, ebx, p2, 184);
    }

    {
        s4 const ebx = GUIwinposy[p1] + 2;
        s4 const edx = GUIwinposx[p1] + 2;
        GUIOuttextwin(edx, ebx, p2, GUIWincoladd == 0 ? 220 : 214);
    }

    {
        s4 const eax = GUIwinposx[p1] + GUIwinsizex[p1] - 10;
        s4 const ebx = GUIwinposy[p1];
        static u1 const GUIIconDataClose[] = {
            /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            /**/ 0, 216, 216, 216, 216, 216, 216, 216, 0, 0,
            /**/ 214, 212, 202, 212, 212, 212, 202, 212, 210, 0,
            /**/ 214, 212, 212, 200, 212, 200, 212, 212, 210, 202,
            /**/ 214, 212, 212, 212, 198, 212, 212, 212, 210, 202,
            /**/ 214, 212, 212, 196, 212, 196, 212, 212, 210, 200,
            /**/ 214, 212, 194, 212, 212, 212, 194, 212, 210, 200,
            /**/ 0, 208, 208, 208, 208, 208, 208, 208, 198, 198,
            /**/ 0, 0, 198, 198, 198, 198, 198, 198, 198, 0,
            /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        };
        GUIoutputiconwin(eax, ebx, GUIIconDataClose);
    }
}

static void DrawTabOn(u4 const* const p1, u4* const peax, u4 ebx, u4* const pebp) // p1 = array, ebp = offset, eax = minX, ebx = minY
{
    u4 eax = *peax;
    u4 ebp = *pebp;

    char const* esi = (char const*)p1; // XXX ugly cast
    u4 ecx = 8 + eax;
    while (esi[ebp] == '\0')
        ++ebp;
    while (esi[ebp] != '\0')
        ++esi, ecx += 6;

    u4 const edx = GUIWincol;
    GUIHLine(eax + 1, ecx, ebx++, edx + 4);

    GUIRect(eax + 1, ecx, ebx, 12, edx + 2);
    GUIRect(eax, eax, ebx, 11, edx + 3);

    char const* const label = (char const*)p1 + ebp; // XXX ugly cast
    GUIOuttextwin(eax + 6, ebx + 4, label, GUIWincol);
    GUIOuttextwin(eax + 5, ebx + 3, label, GUIWincoladd == 0 ? 163 : 164);

    eax = ecx + 1; // restore and set Xoff for drawing step
    GUIRect(eax, eax, ebx, 12, edx + 3);

    {
        char const* const esi = (char const*)p1; // XXX ugly cast
        while (esi[ebp] != '\0')
            ++ebp;
    }

    *peax = eax + 1;
    *pebp = ebp;
}

static void DrawTabOff(u4 const* const p1, u4* const peax, u4 ebx, u4* const pebp) // p1 = array, ebp = offset, eax = minX, ebx = minY
{
    u4 eax = *peax;
    u4 ebp = *pebp;

    char const* esi = (char const*)p1; // XXX ugly cast
    u4 ecx = 8 + eax;
    while (esi[ebp] == '\0')
        ++ebp;
    while (esi[ebp] != '\0')
        ++esi, ecx += 6;

    u4 const edx = GUIWincol;
    ++ebx;
    GUIHLine(eax + 1, ecx, ebx++, edx + 3);

    GUIRect(eax, eax, ebx, 10, edx + 2);

    char const* const label = (char const*)p1 + ebp; // XXX ugly cast
    u1 const colour = GUIWincoladd == 0 ? 202 : 196;
    GUIOuttextwin(eax + 6, ebx + 4, label, colour);
    GUIOuttextwin(eax + 5, ebx + 3, label, colour + 15);

    eax = ecx + 1;
    GUIRect(eax, eax, ebx, 10, edx);

    {
        char const* const esi = (char const*)p1;
        while (esi[ebp] != '\0')
            ++ebp;
    }

    *peax = eax + 1;
    *pebp = ebp;
}

static void GUIDrawTabs(u4 const* const p1, u4* const peax, u4 const ebx) // tabs/label array
{
    u4 ecx = p1[1]; // total #
    if (ecx == 0)
        return;
    u4 esi = p1[0]; // active tab
    u4 ebp = 8; // set array offset at top of labels
    do {
        if (--esi == 0) // check if tab is the current one
        {
            DrawTabOn(p1, peax, ebx, &ebp); // draws tab, updates eax, ebx & ebp for next tab...
        } else {
            DrawTabOff(p1, peax, ebx, &ebp); // ... and autosizes the tab for its label
        }
    } while (--ecx != 0);
}

/* XXX NOTE: Macro is defective:
 * - if p2 has the form a + b, then 2 * b must be added to p4
 * - if p3 has the form a + b, then 2 * b must be added to p5
 */
static void DrawGUIWinBox(u4 const p1, u4 const p2, u4 const p3, u4 const p4, u4 const p5, u4 const p6)
{
    s4 const eax = GUIwinposx[p1] + p2;
    s4 const ebx = GUIwinposy[p1] + p3;
    s4 const ecx = eax + p4 - p2 + 1;
    u4 const esi = p5 - p3 + 1;
    GUIRect(eax, ecx, ebx, esi, p6);
}

static void DrawGUIButton(u4 const p1, u4 const p2, u4 const p3, u4 const p4, u4 const p5, char const* const p6, u4 const p7, u4 const p8, u4 const p9)
{
    u1 const colour = GUIWincoladd == 0 ? 217 : 211;
    bool const held = GUICBHold == p7;
    DrawGUIWinBox(p1, p2, p3, p4, p3, colour + (held ? -18 : -5));
    DrawGUIWinBox(p1, p2, p3, p2, p5, colour + (held ? -16 : -8));
    DrawGUIWinBox(p1, p2 + 1, p3 + 1, p4, p5, colour + (held ? -14 : -11));
    DrawGUIWinBox(p1, p4 + 1, p3 + 1, p4, p5, colour + (held ? -12 : -14));
    DrawGUIWinBox(p1, p2, p5, p4 - 1, p5, colour + (held ? -10 : -17));
    if (!held) {
        GUIOuttextwin2(p1, p2 + 5 + p8, p3 + 4 + p9, p6, colour - 15);
        GUIOuttextwin2(p1, p2 + 4 + p8, p3 + 3 + p9, p6, colour);
    } else {
        GUIOuttextwin2(p1, p2 + 6 + p8, p3 + 5 + p9, p6, colour - 18);
        GUIOuttextwin2(p1, p2 + 5 + p8, p3 + 4 + p9, p6, colour - 3);
    }
}

static void GUIDisplayTextY(u4 const p1, u4 const p2, u4 const p3, char const* const p4) // Yellow Text&Shadow
{
    GUIOuttextwin2(p1, p2, p3, p4, GUIWincol);
    GUIOuttextwin2(p1, p2 - 1, p3 - 1, p4, GUIWincoladd == 0 ? 163 : 164);
}

static void GUIDisplayText(u4 const p1, u4 const p2, u4 const p3, char const* const p4) // Text&Shadow
{
    u1 const colour = GUIWincoladd == 0 ? 202 : 196;
    GUIOuttextwin2(p1, p2, p3, p4, colour);
    GUIOuttextwin2(p1, p2 - 1, p3 - 1, p4, colour + 15);
}

static void GUIDisplayBBox(u4 const p1, u4 const p2, u4 const p3, u4 const p4, u4 const p5, u4 const p6) // Black Box W/Border
{
    GUIWincol = cwindrawn == 0 ? 148 : cwindrawn == 1 ? 148 + 5
                                                      : 148 + 10;
    DrawGUIWinBox(p1, p2, p3, p4, p5, p6);
    DrawGUIWinBox(p1, p2, p3 - 3 + 2, p4, p3 - 1, GUIWincol);
    DrawGUIWinBox(p1, p2 - 1, p3, p2 - 2, p5, GUIWincol + 1);
    DrawGUIWinBox(p1, p2, p5 + 1, p4, p5 + 1, GUIWincol + 4);
    DrawGUIWinBox(p1, p4 + 2, p3, p4 + 1, p5, GUIWincol + 3);
}

static void GUIDisplayTextG(u4 const p1, u4 const p2, u4 const p3, char const* const p4) // Green Text&Shadow
{
    GUIOuttextwin2(p1, p2, p3, p4, 223);
    GUIOuttextwin2(p1, p2 - 1, p3 - 1, p4, GUIWincoladd == 0 ? 221 : 222);
}

static void GUIOuttextwin2d(u4 const p1, u4 const p2, u4 const p3, char const* const p4, u4 const p5, char** const p6, u4 const p7) // Boxed, green text, limited to 5th param
{
    char const* ecx = p4; // Move pointer to text into ecx
    while (*ecx != '\0')
        ++ecx; // Check for null in string
    u4 eax = ecx - p4; // Subtract pointer from \0 pointer gives us string length
    if (eax > p5)
        eax = p5; // Restrict to length to display

    GUIDisplayTextG(p1, p2, p3, ecx - eax);
    if (GUIInputBox == p7 + 1 && p6[p7] == p4) {
        static u1 GUIBlinkCursorLoop = 0;
        if (++GUIBlinkCursorLoop == 60)
            GUIBlinkCursorLoop = 0;
        if (GUIBlinkCursorLoop < 30) {
            GUIDisplayTextG(p1, eax * 6 /* 6 pixels */ + p2, p3, "_");
        }
    }
}

static void GUIDisplayBBoxS(u4 const p1, u4 const p2, u4 const p3, u4 const p4, u4 const p5, u4 const p6) // Black Box W/Border
{
    // Minus right side
    GUIWincol = cwindrawn == 0 ? 148 : cwindrawn == 1 ? 148 + 5
                                                      : 148 + 10;
    DrawGUIWinBox(p1, p2, p3, p4, p5, p6);
    DrawGUIWinBox(p1, p2, p3 - 3 + 2, p4, p3 - 1, GUIWincol);
    DrawGUIWinBox(p1, p2 - 1, p3, p2 - 2, p5, GUIWincol + 1);
    DrawGUIWinBox(p1, p2, p5 + 1, p4, p5 + 1, GUIWincol + 4);
}

static void DrawGUIWinBox2(u4 const p1, u4 const p2, u4 const p3, u4 const p4, u4 const p5, s4 const ebx)
{
    s4 const eax = GUIwinposx[p1] + p2;
    s4 const ecx = GUIwinposx[p1] + p3 + 1;
    u1 const edx = GUIWincoladd == 0 ? p5 : p5 + 1;
    GUIRect(eax, ecx, ebx + GUIwinposy[p1], p4, edx);
}

static void GUIDisplayTextu(u4 const p1, u4 const p2, u4 const p3, char const* const p4, u4 const p5) // Text&Shadow With Underline
{
    u1 const colour = GUIWincoladd == 0 ? 202 : 196;
    GUIOuttextwin2u(p1, p2, p3, p4, colour, p5);
    GUIOuttextwin2(p1, p2 - 1, p3 - 1, p4, colour + 15);
}

static u1 const GUIIconDataCheckBoxUC[] = {
    /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /**/ 0, 220, 219, 218, 217, 216, 215, 0, 0, 0,
    /**/ 0, 219, 218, 217, 216, 215, 214, 202, 0, 0,
    /**/ 0, 218, 217, 216, 215, 214, 213, 202, 0, 0,
    /**/ 0, 217, 216, 215, 214, 213, 212, 202, 0, 0,
    /**/ 0, 216, 215, 214, 213, 212, 211, 202, 0, 0,
    /**/ 0, 215, 214, 213, 212, 211, 210, 202, 0, 0,
    /**/ 0, 0, 202, 202, 202, 202, 202, 202, 0, 0
};

static u1 const GUIIconDataCheckBoxC[] = {
    /**/ 0, 0, 0, 0, 0, 0, 0, 0, 165, 0,
    /**/ 0, 0, 0, 0, 0, 0, 0, 165, 0, 0,
    /**/ 0, 0, 0, 0, 0, 0, 165, 0, 0, 0,
    /**/ 0, 220, 219, 218, 217, 165, 215, 0, 0, 0,
    /**/ 0, 165, 165, 217, 165, 165, 214, 202, 0, 0,
    /**/ 0, 218, 165, 216, 165, 214, 213, 202, 0, 0,
    /**/ 0, 217, 165, 165, 165, 213, 212, 202, 0, 0,
    /**/ 0, 216, 215, 165, 213, 212, 211, 202, 0, 0,
    /**/ 0, 215, 214, 165, 212, 211, 210, 202, 0, 0,
    /**/ 0, 0, 202, 202, 202, 202, 202, 202, 0, 0
};

static void GUIDisplayCheckboxTn(u4 const p1, u4 const p2, u4 const p3, u1 const* const p4, u4 const p5, char const* const p6) // Variable Checkbox (Text)
{
    u1 const* const icon = *p4 == p5 ? GUIIconDataCheckBoxC : GUIIconDataCheckBoxUC;
    GUIDisplayIconWin(p1, p2, p3, icon);
    GUIDisplayText(p1, p2 + 15, p3 + 5, p6);
}

static void GUIDisplayCheckbox(u4 const p1, u4 const p2, u4 const p3, u1 const* const p4, char const* const p5) // Toggled Checkbox (Text)
{
    u1 const* const icon = *p4 != 0 ? GUIIconDataCheckBoxC : GUIIconDataCheckBoxUC;
    GUIDisplayIconWin(p1, p2, p3, icon);
    GUIDisplayText(p1, p2 + 15, p3 + 5, p5);
}

static void GUIDisplayCheckboxu(u4 const p1, u4 const p2, u4 const p3, u1 const* const p4, char const* const p5, u4 const p6) // Toggled Checkbox (Text Underline)
{
    u1 const* const icon = *p4 != 0 ? GUIIconDataCheckBoxC : GUIIconDataCheckBoxUC;
    GUIDisplayIconWin(p1, p2, p3, icon);
    GUIDisplayTextu(p1, p2 + 15, p3 + 5, p5, p6);
}

static void GUIDisplayCheckboxun(u4 const p1, u4 const p2, u4 const p3, u1 const* const p4, u4 const p5, char const* const p6, u4 const p7) // Set Var. Checkbox (Text Underline)
{
    u1 const* const icon = *p4 == p5 ? GUIIconDataCheckBoxC : GUIIconDataCheckBoxUC;
    GUIDisplayIconWin(p1, p2, p3, icon);
    GUIDisplayTextu(p1, p2 + 15, p3 + 5, p6, p7);
}

static void GUIDisplayButtonHole(u4 const p1, u4 const p2, u4 const p3, u1 const* const p4, u4 const p5)
{
    static u1 const GUIIconDataButtonFill[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 207, 209, 207, 0, 0, 0, 0,
        0, 0, 207, 211, 212, 211, 207, 0, 0, 0,
        0, 207, 211, 214, 216, 214, 211, 207, 0, 0,
        0, 207, 212, 216, 217, 216, 212, 207, 0, 0,
        0, 207, 211, 214, 216, 214, 211, 207, 0, 0,
        0, 0, 207, 211, 212, 211, 207, 0, 0, 0,
        0, 0, 0, 207, 209, 207, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    static u1 const GUIIconDataButtonHole[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 207, 205, 207, 0, 0, 0, 0,
        0, 0, 207, 203, 202, 203, 207, 0, 0, 0,
        0, 207, 203, 200, 198, 200, 203, 207, 0, 0,
        0, 207, 202, 198, 197, 198, 202, 207, 0, 0,
        0, 207, 203, 200, 198, 200, 203, 207, 0, 0,
        0, 0, 207, 203, 202, 203, 207, 0, 0, 0,
        0, 0, 0, 207, 205, 207, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    u1 const* const icon = *p4 == p5 ? GUIIconDataButtonFill : GUIIconDataButtonHole;
    GUIDisplayIconWin(p1, p2, p3, icon);
}

static void GUIDisplayButtonHoleTu(u4 const p1, u4 const p2, u4 const p3, u1 const* const p4, u4 const p5, char const* const p6, u4 const p7)
{
    GUIDisplayButtonHole(p1, p2, p3, p4, p5);
    GUIDisplayTextu(p1, p2 + 15, p3 + 3, p6, p7);
}

static void GUIDrawSlider(u4 const p1, u4 const p2, u4 const p3, u4 const p4, void const* const p5, u1 (*p7)(void const*), char const* (*p8)(void const*)) // win #id, minX, width, Ypos, var, text, proc1, proc2
{
    s4 const eax = GUIwinposx[p1] + p2;
    s4 const ebx = GUIwinposy[p1] + p4;
    s4 const ecx = eax + p3;
    u4 const edx = 215 - GUIWincoladd;
    GUIHLine(eax, ecx, ebx, edx);
    GUIHLine(eax + 1, ecx + 1, ebx + 1, edx - 13);
    u4 const x = p2 - 4 + p7(p5); // proc1 == alters var correctly and puts result in al
    static u1 const GUIIconDataSlideBar[] = {
        0, 0, 0, 0, 216, 0, 0, 0, 0, 0,
        0, 0, 0, 212, 216, 220, 0, 0, 0, 0,
        0, 0, 0, 212, 216, 220, 202, 0, 0, 0,
        0, 0, 212, 212, 216, 218, 220, 0, 0, 0,
        0, 0, 212, 214, 216, 218, 220, 202, 0, 0,
        0, 0, 212, 214, 216, 218, 220, 202, 0, 0,
        0, 0, 0, 212, 216, 220, 202, 202, 0, 0,
        0, 0, 0, 212, 216, 220, 202, 0, 0, 0,
        0, 0, 0, 0, 216, 202, 202, 0, 0, 0,
        0, 0, 0, 0, 0, 202, 0, 0, 0, 0
    };
    GUIDisplayIconWin(p1, x, p4 - 4, GUIIconDataSlideBar);
    char const* const esi = p8(p5); // proc2 == alters text correctly and puts pointer in esi
    GUIDisplayTextG(p1, p2 + p3 + 6, p4 - 1, esi); // Display Value (Green)
}

static void GUIOuttextwin2load(u4 const p1, u4 const p2, u4 const p3, char* const* const eax)
{
    char const* const name = *eax;
    ++cloadnposb;
    GUIOuttextwin2l(p1, p2, p3, name, 223);
    GUIOuttextwin2l(p1, p2 - 1, p3 - 1, name, GUIWincoladd == 0 ? 221 : 222);
    --cloadnleft;
}

static void GUIOuttextwinloadfile(u4 const p1, u4 const p2, u4 const p3)
{
    if (cloadnleft & 0x80000000)
        return;
    if ((u4)cloadnposb >= (u4)GUIfileentries)
        return;
    char* const* const eax = &selected_names[cloadnposb];
    GUIOuttextwin2load(p1, p2, p3, eax);
}

static void GUIOuttextwinloaddir(u4 const p1, u4 const p2, u4 const p3)
{
    if (cloadnleft & 0x80000000)
        return;
    if ((u4)cloadnposb >= (u4)GUIdirentries)
        return;
    char* const* const eax = &d_names[cloadnposb + 2];
    GUIOuttextwin2load(p1, p2, p3, eax);
}

static void DrawSlideBar(u4 const p1, u4 const p2, u4 const p3, u4 const p4, u4 const p5, u4 const p6, u4 const p7, u4* const p8, u4 const p9, u4 const p10)
{
    DrawSlideBarWin(p1, p2, p3 + 8, p4, p5, p6, p7 - 16, p8);
    if ((GUICHold & 0xFF) == p9)
        GUIWincoladd += 3;
    static u1 const GUIIconDataUpArrow[] = {
        201, 209, 209, 209, 209, 209, 209, 200, 0, 0,
        207, 205, 205, 202, 203, 205, 205, 203, 0, 0,
        207, 205, 201, 202, 203, 202, 205, 203, 0, 0,
        207, 200, 205, 202, 203, 205, 201, 203, 0, 0,
        207, 205, 205, 202, 203, 205, 205, 203, 0, 0,
        207, 205, 205, 202, 203, 205, 205, 203, 0, 0,
        207, 205, 205, 202, 203, 205, 205, 203, 0, 0,
        199, 201, 201, 201, 201, 201, 201, 198, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    GUIDisplayIconWin(p1, p2, p3, GUIIconDataUpArrow);
    if ((GUICHold & 0xFF) == p9)
        GUIWincoladd -= 3;

    if ((GUICHold & 0xFF) == p10)
        GUIWincoladd += 3;
    static u1 const GUIIconDataDownArrow[] = {
        201, 209, 209, 209, 209, 209, 209, 200, 0, 0,
        207, 205, 205, 202, 203, 205, 205, 203, 0, 0,
        207, 205, 205, 202, 203, 205, 205, 203, 0, 0,
        207, 205, 205, 202, 203, 205, 205, 203, 0, 0,
        207, 200, 205, 202, 203, 205, 201, 203, 0, 0,
        207, 205, 201, 202, 203, 202, 205, 203, 0, 0,
        207, 205, 205, 202, 203, 205, 205, 203, 0, 0,
        199, 201, 201, 201, 201, 201, 201, 198, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    GUIDisplayIconWin(p1, p2, p3 + p7 - 8, GUIIconDataDownArrow);
    if ((GUICHold & 0xFF) == p10)
        GUIWincoladd -= 3;
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
    GUIDisplayTextY(1, 6, 157, "DISPLAY TYPE:");
    GUIDisplayText(1, 21, 182, "DOS 8.3 FORMAT");
#endif
    GUIDisplayText(1, 21, 174, "SNES HEADER NAME");
    GUIDisplayText(1, 6, 16, "FILENAME");
    GUIDisplayText(1, 161, 16, "DIRECTORY");
    GUIDisplayText(1, 146, 172, "FORCE");

    u4 ecx = 0;
    char const* esi = ZRomPath;
    while (esi[ecx] != '\0')
        ++ecx;
    if (ecx > 39)
        esi += ecx - 39;
    GUIDisplayText(1, 6, 138, esi);

    cloadmaxlen = 39;
#ifndef __MSDOS__
    u1 const colour = GUIWincoladd == 0 ? 202 : 196;
    if (GUIcurrentfilewin != 0) {
        char const* const eax = d_names[GUIcurrentdircursloc + 2];
        GUIOuttextwin2l(1, 6, 158, eax, colour);
        GUIOuttextwin2l(1, 5, 157, eax, colour + 15);
    } else if (GUIfileentries != 0) {
        s4 const eax = GUIcurrentcursloc;
        if ((u4)eax < (u4)GUIfileentries) {
            char const* const name = selected_names[eax];
            GUIOuttextwin2l(1, 6, 158, name, colour);
            GUIOuttextwin2l(1, 5, 157, name, colour + 15);
        }
    }
#endif

    DrawGUIButton(1, 186, 165, 228, 176, "LOAD", 1, 0, 0);

    // The Three Boxes
    GUIDisplayBBoxS(1, 5, 25, 144, 134, 167); // 126 = 6 * 21,  112 = 7 * 16
    GUIDisplayBBoxS(1, 160, 25, 228, 134, 167); // 78 =  6 * 13
    GUIDisplayBBox(1, 5, 145, 228, 152, 167); // 126 = 6 * 21,  112 = 7 * 16

    {
        u1 const ebx = GUILoadPos; // Flash Code?
        if (GUILDFlash & 8) {
            GUILoadTextA[ebx] = '\0';
        } else {
            GUILoadTextA[ebx] = '_';
            GUILoadTextA[ebx + 1] = '\0';
        }
    }

    // Check if it's in the Files box
    s4 const ebx = GUIcurrentfilewin == 0 ? GUIcurrentcursloc - GUIcurrentviewloc : GUIcurrentdircursloc - GUIcurrentdirviewloc;

    // Draw 2 more boxes?
    s4 const y = 27 + ebx * 7;
    if (GUIcurrentfilewin == 0) {
        DrawGUIWinBox2(1, 5, 144, 7, 224, y);
    } else {
        DrawGUIWinBox2(1, 160, 228, 7, 224, y);
    }

    GUIDisplayTextG(1, 8, 148, GUILoadTextA);

    if (GUIfileentries == 0)
        GUIcurrentfilewin = 1;

    cloadnleft = GUIfileentries - GUIcurrentviewloc;
    cloadnposb = GUIcurrentviewloc;
    cloadmaxlen = 23;

    // Text/Shadow for Filename Box
    for (u4 i = 0; i != 15; ++i) {
        GUIOuttextwinloadfile(1, 8, 29 + 7 * i);
    }

    cloadnleft = GUIdirentries - GUIcurrentdirviewloc;
    cloadnposb = GUIcurrentdirviewloc;
    cloadmaxlen = 11;

    // Text/Shadow for DIR Box
    for (u4 i = 0; i != 15; ++i) {
        GUIOuttextwinloaddir(1, 164, 29 + 7 * i);
    }

    GUILoadTextA[GUILoadPos] = '\0';

    GUIDisplayButtonHole(1, 9, 163, &GUIloadfntype, 0); // Radio Buttons
    GUIDisplayButtonHole(1, 9, 171, &GUIloadfntype, 1);
#ifdef __MSDOS__
    GUIDisplayButtonHole(1, 9, 179, &GUIloadfntype, 2);
#endif

    GUIDisplayCheckboxTn(1, 10, 187, &showallext, 1, "SHOW ALL EXTENSIONS"); // Checkboxes
    GUIDisplayCheckboxTn(1, 144, 177, &ForceROMTiming, 1, "NTSC");
    GUIDisplayCheckboxTn(1, 144, 187, &ForceROMTiming, 2, "PAL");
    GUIDisplayCheckboxTn(1, 184, 177, &ForceHiLoROM, 1, "LOROM");
    GUIDisplayCheckboxTn(1, 184, 187, &ForceHiLoROM, 2, "HIROM");

    // Slidebar for Files
    // win#,X,Y start, %4-List Loc, %5-List size, %6-Screen size, %7-Bar Size
    DrawSlideBar(1, 146, 25, GUIcurrentviewloc, GUIfileentries, 15, 110, GUILStA, 1, 2);

    // Slidebar for DIR
    DrawSlideBar(1, 230, 25, GUIcurrentdirviewloc, GUIdirentries, 15, 110, GUILStB, 3, 4);
}

void DisplayGUIReset(void)
{
    GUIDrawWindowBox(12, "RESET GAME");

    // Red Box around buttons
    u1 const dl = GUIWincoladd == 0 ? 225 : 224;
    u4 const x = GUICResetPos == 0 ? 19 : 79;
    DrawGUIWinBox(12, x, 29, x + 38, 42, dl);

    DrawGUIButton(12, 20, 30, 56, 41, "YES", 2, 0, 0);
    DrawGUIButton(12, 80, 30, 116, 41, "NO", 3, 0, 0);

    GUIDisplayTextY(12, 6, 16, "RESET: ARE YOU SURE ?");
}

void DisplayGUIStates(void)
{
    GUIDrawWindowBox(14, "STATE CONFIRM");

    // Red Box around buttons
    u1 const dl = GUIWincoladd == 0 ? 225 : 224;
    u4 const x = GUICStatePos == 0 ? 19 : 79;
    DrawGUIWinBox(12, x, 29, x + 38, 42, dl);

    DrawGUIButton(14, 20, 30, 56, 41, "YES", 10, 0, 0);
    DrawGUIButton(14, 80, 30, 116, 41, "NO", 11, 0, 0);

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
    for (u4 i = 0; i != 10; x += 20, ++i) {
        if (i == 5) {
            x = 0;
            y = 15;
        }
        GUIChoseSaveText2[0] = '0' + i;
        GUIDisplayText(2, 21 + x, 31 + y, GUIChoseSaveText2);
        GUIDisplayButtonHole(2, 10 + x, 28 + y, &ah, i);
    }
    GUIDisplayTextY(2, 6, 61, "SLOT LEVEL:");

    GUIChoseSaveText2[0] = ah;
    GUIChoseSlotTextX[0] = al;

    GUIDisplayBBox(2, 72, 59, 90, 66, 167); // Save Slot Frameskip +/- Box
    GUIDisplayTextG(2, 83, 61, GUIChoseSlotTextX);
    DrawGUIButton(2, 94, 59, 102, 67, "+", 80, -2, -1);
    DrawGUIButton(2, 105, 59, 113, 67, "-", 81, -2, -1);
}

static void PrintKey(u4 const id, u4 const x, u4 const y, u4 const key)
{
    char GUIGameDisplayKy[4];
    sprintf(GUIGameDisplayKy, "%.3s", ScanCodeListing + key * 3);
    GUIDisplayTextG(id, x, y, GUIGameDisplayKy);
}

static void DGOptnsBorderBox(u4 const p1, u4 const p2, u4 const p3)
{
    GUIWincol = cwindrawn == 0 ? 148 : cwindrawn == 1 ? 148 + 5
                                                      : 148 + 10;
    DrawGUIWinBox(p1, p2 + 1, p3, p2 + 20, p3, GUIWincol);
    DrawGUIWinBox(p1, p2, p3 + 1, p2 - 1, p3 + 7, GUIWincol + 1);
    DrawGUIWinBox(p1, p2 + 1, p3 + 8, p2 + 20, p3 + 8, GUIWincol + 4);
    DrawGUIWinBox(p1, p2 + 22, p3 + 1, p2 + 21, p3 + 7, GUIWincol + 3);
}

static void DDrawBox(u4 const p1, s4 const p2, s4 const p3, u4 const* const p4)
{
    s4 const eax = GUIwinposx[p1] + p2 + 1;
    s4 const ebx = GUIwinposy[p1] + p3 + 1;
    GUIRect(eax, eax + 20, ebx, 7, 167);
    PrintKey(p1, p2 + 4, p3 + 3, *p4);
    DGOptnsBorderBox(p1, p2, p3);
}

#define GUIInputDispAll(p1)                                 \
    do {                                                    \
        DDrawBox(3, 44, 101, &p1##upk); /* Up         */    \
        DDrawBox(3, 44, 111, &p1##downk); /* Down       */  \
        DDrawBox(3, 44, 121, &p1##leftk); /* Left       */  \
        DDrawBox(3, 44, 131, &p1##rightk); /* Right      */ \
        DDrawBox(3, 44, 141, &p1##startk); /* Start      */ \
        DDrawBox(3, 44, 151, &p1##selk); /* Select     */   \
        DDrawBox(3, 84, 101, &p1##Ak); /* A          */     \
        DDrawBox(3, 84, 111, &p1##Bk); /* B          */     \
        DDrawBox(3, 84, 121, &p1##Xk); /* X          */     \
        DDrawBox(3, 84, 131, &p1##Yk); /* Y          */     \
        DDrawBox(3, 84, 141, &p1##Lk); /* L          */     \
        DDrawBox(3, 84, 151, &p1##Rk); /* R          */     \
                                                            \
        DDrawBox(3, 124, 101, &p1##Xtk); /* X Turbo    */   \
        DDrawBox(3, 124, 111, &p1##Ytk); /* Y Turbo    */   \
        DDrawBox(3, 124, 121, &p1##Ltk); /* L Turbo    */   \
        DDrawBox(3, 164, 101, &p1##Atk); /* A Turbo    */   \
        DDrawBox(3, 164, 111, &p1##Btk); /* B Turbo    */   \
        DDrawBox(3, 164, 121, &p1##Rtk); /* R Turbo    */   \
                                                            \
        DDrawBox(3, 124, 141, &p1##ULk); /* Up-Left    */   \
        DDrawBox(3, 124, 151, &p1##DLk); /* Down-Left  */   \
        DDrawBox(3, 164, 141, &p1##URk); /* Up-Right   */   \
        DDrawBox(3, 164, 151, &p1##DRk); /* Down-Right */   \
    } while (0)

void DisplayGUIInput(void)
{
    GUIDrawWindowBox(3, "INPUT DEVICE");
    cplayernum = GUIInputTabs[0] - 1;

    {
        u4 eax;
        u4 ebx;
        GUIDrawTArea(3, &eax, &ebx);
        GUIDrawTabs(GUIInputTabs, &eax, ebx);
    }

    GUIDisplayTextY(3, 6, 26, "DEVICE:");
    u1 const ebx = *GUIInputRefP[cplayernum];
    if (GUIFreshInputSelect != 0) {
        GUIFreshInputSelect = 0;
        GUIJT_viewable = 5;
        GUIJT_entries = NumInputDevices;
        GUIJT_offset = ebx;
        GUIJT_currentviewloc = (s4*)&GUIcurrentinputviewloc; // XXX ugly cast
        GUIJT_currentcursloc = (s4*)&GUIcurrentinputcursloc; // XXX ugly cast
        GUIGenericJumpTo();
    }
    GUIDisplayTextY(3, 6 + 54, 83, GUIInputNames[ebx]); // CDV
    GUIDisplayTextY(3, 6, 83, "CURRENT:");

    GUIDisplayTextY(3, 6, 94, "KEYS:");
    GUIDisplayText(3, 6, 104, "    UP");
    GUIDisplayText(3, 6, 114, "  DOWN");
    GUIDisplayText(3, 6, 124, "  LEFT");
    GUIDisplayText(3, 6, 134, " RIGHT");
    GUIDisplayText(3, 6, 144, " START");
    GUIDisplayText(3, 6, 154, "SELECT");

    GUIDisplayText(3, 76, 104, "A");
    GUIDisplayText(3, 76, 114, "B");
    GUIDisplayText(3, 76, 124, "X");
    GUIDisplayText(3, 76, 134, "Y");
    GUIDisplayText(3, 76, 144, "L");
    GUIDisplayText(3, 76, 154, "R");

    GUIDisplayTextY(3, 116, 94, "TURBO:");
    GUIDisplayText(3, 156, 104, "A");
    GUIDisplayText(3, 156, 114, "B");
    GUIDisplayText(3, 156, 124, "R");
    GUIDisplayText(3, 116, 104, "X");
    GUIDisplayText(3, 116, 114, "Y");
    GUIDisplayText(3, 116, 124, "L");

    GUIDisplayTextY(3, 113, 134, "DIAGONALS:");
    GUIDisplayText(3, 113, 144, "UL");
    GUIDisplayText(3, 153, 144, "UR");
    GUIDisplayText(3, 113, 154, "DL");
    GUIDisplayText(3, 153, 154, "DR");

#ifdef __MSDOS__
    GUIDisplayCheckboxu(3, 105, 160, &SidewinderFix, "SIDEWINDER FIX", 0);

    char const* const GUIInputTextE5 = "USE JOYSTICK PORT 209H";
    switch (cplayernum) {
    case 0:
        GUIDisplayCheckboxu(3, 5, 190, &pl1p209, GUIInputTextE5, 4);
        break;
    case 1:
        GUIDisplayCheckboxu(3, 5, 190, &pl2p209, GUIInputTextE5, 4);
        break;
    case 2:
        GUIDisplayCheckboxu(3, 5, 190, &pl3p209, GUIInputTextE5, 4);
        break;
    case 3:
        GUIDisplayCheckboxu(3, 5, 190, &pl4p209, GUIInputTextE5, 4);
        break;
    case 4:
        GUIDisplayCheckboxu(3, 5, 190, &pl5p209, GUIInputTextE5, 4);
        break;
    }
#endif

    GUIDisplayCheckboxu(3, 5, 160, &GameSpecificInput, "GAME SPECIFIC", 0);
    GUIDisplayCheckboxu(3, 5, 170, &AllowUDLR, "ALLOW U+D/L+R", 0);
    GUIDisplayCheckboxu(3, 105, 170, &Turbo30hz, "TURBO AT 30HZ", 0);
    GUIDisplayCheckboxu(3, 5, 180, &pl12s34, "USE PL3/4 AS PL1/2", 0);

    DrawGUIButton(3, 123, 34, 153, 45, "SET", 14, 0, 0); // Buttons
    DrawGUIButton(3, 123, 50, 177, 61, "SET KEYS", 40, 0, 0);
#ifdef __MSDOS__
    DrawGUIButton(3, 123, 66, 183, 77, "CALIBRATE", 15, 0, 0);
#endif

    GUIDisplayBBoxS(3, 5, 34, 107, 77, 167); // Main Box
    u4 const eax = GUIcurrentinputcursloc - GUIcurrentinputviewloc;
    DrawGUIWinBox2(3, 5, 107, 7, 224, 36 + eax * 8);

    // Text&Shadow inside Main Box
    char const(*name)[17] = GUIInputNames + GUIcurrentinputviewloc;
    for (u4 i = 0; i != 5; ++i) {
        GUIDisplayTextG(3, 11, 38 + 8 * i, *name++);
    }

    DrawSlideBar(3, 109, 34, GUIcurrentinputviewloc, NumInputDevices, 5, 44, GUIIStA, 9, 10);

    // Hotkey Boxes
    switch (cplayernum) {
    case 0:
        GUIInputDispAll(pl1);
        break;
    case 1:
        GUIInputDispAll(pl2);
        break;
    case 2:
        GUIInputDispAll(pl3);
        break;
    case 3:
        GUIInputDispAll(pl4);
        break;
    case 4:
        GUIInputDispAll(pl5);
        break;
    }
}

void DisplayGUIOption(void)
{
    GUIDrawWindowBox(4, "OPTIONS");

    {
        u4 eax;
        u4 ebx;
        GUIDrawTArea(4, &eax, &ebx);
        GUIDrawTabs(GUIOptionTabs, &eax, ebx);
    }

    if (GUIOptionTabs[0] == 1) { // Basic
        GUIDisplayTextY(4, 11, 26, "SYSTEM:");
        if (ShowMMXSupport == 1) {
            GUIDisplayCheckboxu(4, 11, 31, &MMXSupport, "ENABLE MMX SUPPORT", 7);
        }
        GUIDisplayCheckboxu(4, 11, 41, &Show224Lines, "SHOW 224 LINES", 9);

        GUIDisplayTextY(4, 11, 66, "GFX ENGINES:");
        GUIDisplayCheckboxu(4, 11, 71, &newengen, "USE NEW GFX ENG", 4);
        if (newengen == 0) {
            GUIDisplayCheckboxu(4, 11, 81, &bgfixer, "USE ALT OLD GFX ENG", 4);
        }

        GUIDisplayTextY(4, 11, 106, "ROM:");
        GUIDisplayCheckboxu(4, 11, 111, &AutoPatch, "ENABLE IPS AUTO-PATCHING", 7);
        GUIDisplayCheckboxu(4, 11, 121, &DisplayInfo, "SHOW ROM INFO ON LOAD", 5);
        GUIDisplayCheckboxu(4, 11, 131, &RomInfo, "LOG ROM INFO", 2);

#ifdef __WIN32__
        GUIDisplayTextY(4, 11, 156, "WINDOWS SPECIFIC:");
        GUIDisplayCheckboxu(4, 11, 161, &PauseFocusChange, "PAUSE EMU IN BACKGROUND", 13);
        GUIDisplayCheckboxu(4, 11, 171, &HighPriority, "INCREASE EMU PRIORITY", 13);
#endif
        GUIDisplayCheckboxu(4, 11, 181, &DisableScreenSaver, "DISABLE POWER MANAGEMENT", 0);
    }

    if (GUIOptionTabs[0] == 2) {
        GUIDisplayTextY(4, 11, 26, "OVERLAYS:");
        GUIDisplayCheckboxu(4, 11, 31, &FPSAtStart, "SHOW FPS CNTR ON EMU LOAD", 5);
        GUIDisplayCheckboxu(4, 11, 41, &TimerEnable, "SHOW CLOCK", 5);
        if (TimerEnable == 1) {
            GUIDisplayCheckboxu(4, 89, 41, &TwelveHourClock, "12 HOUR MODE", 3);
            GUIDisplayCheckboxu(4, 11, 51, &ClockBox, "SHOW CLOCK BOX", 13);
        }

        GUIDisplayTextY(4, 11, 76, "MESSAGES:");
        GUIDisplayCheckboxu(4, 11, 81, &SmallMsgText, "USE SMALL MESSAGE TEXT", 4);
        GUIDisplayCheckboxu(4, 11, 91, &GUIEnableTransp, "USE TRANSPARENT TEXT", 4);

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
    if (TripBufAvail == 0)
        Triplebufen = 0;
#endif

    if (MMXSupport != 1 || newgfx16b == 0) {
        En2xSaI = 0;
        hqFilter = 0;
    }

    if (En2xSaI != 0) {
#ifdef __MSDOS__
        Triplebufen = 0;
#endif
        hqFilter = 0;
        scanlines = 0;
        antienab = 0;
    }

    if (hqFilter != 0) {
        En2xSaI = 0;
        scanlines = 0;
        antienab = 0;
    }

    GUIDrawWindowBox(5, "VIDEO CONFIG");

    if (GUINTVID[cvidmode] == 0) { // not NTSC
        NTSCFilter = 0;
        GUIVntscTab[0] = 0;
        if ((GUIVideoTabs[0] & 0xFF) == 0)
            GUIVideoTabs[0] = GUIVideoTabs[0] & 0xFFFFFF00 | 1;
    }

    {
        u4 eax;
        u4 ebx;
        GUIDrawTArea(5, &eax, &ebx);
        GUIDrawTabs(GUIVideoTabs, &eax, ebx);
        if (NTSCFilter != 0)
            GUIDrawTabs(GUIVntscTab, &eax, ebx);
    }

    if (GUIVideoTabs[0] == 1) // Video Modes List/Options Tab
    {
        DrawGUIButton(5, 128, 30, 164, 41, "SET", 4, 0, 0); // Mode Set Button

#ifndef __MSDOS__ // Legend
        GUIDisplayTextY(5, 130, 50, "LEGEND:");
        GUIDisplayText(5, 130, 58, "D = ALLOW FILTERS");
        GUIDisplayText(5, 130, 66, "S = STRETCH");
        GUIDisplayText(5, 130, 74, "R = KEEP 8:7 RATIO");
        GUIDisplayText(5, 130, 82, "W = WINDOWED");
        GUIDisplayText(5, 130, 90, "F = FULLSCREEN");
#ifdef __OPENGL__
        GUIDisplayText(5, 130, 98, "O = USES OPENGL");
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
        DrawSlideBar(5, 117, 26, GUIcurrentvideoviewloc, NumVideoModes, 20, 164, GUIVStA, 5, 6);

        u4 const ebx = (GUIcurrentvideocursloc - GUIcurrentvideoviewloc) * 8 + 28; // Box
        DrawGUIWinBox2(5, 5, 115, 7, 224, ebx);

        char const(*name)[18] = GUIVideoModeNames + GUIcurrentvideoviewloc;
        u4 const n = NumVideoModes < 20 ? NumVideoModes : 20;
        for (u4 i = 0; i != n; ++i) {
            GUIDisplayTextG(5, 11, 30 + 8 * i, *name++);
        }

        GUIDisplayTextY(5, 7, 194, "CURRENT:");
        GUIDisplayTextY(5, 91, 194, GUIVideoModeNames[cvidmode]); // (5,61,194) // Mode Value
    }

    // Filters tab
    if (GUIVideoTabs[0] == 2) {
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
                if (GUIBIFIL[cvidmode] != 0) {
                    GUIDisplayTextY(5, 13, 30, GUIVideoTextB1);
                    GUIDisplayCheckboxu(5, 18, 35, &BilinearFilter, "BILINEAR FILTER", 1);
                } else
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
                    if (GUIEAVID[cvidmode] != 0) {
                        GUIDisplayTextY(5, 13, 30, GUIVideoTextB1);
                        GUIDisplayCheckboxu(5, 18, 35, &antienab, "EAGLE ENGINE", 9); // same loc at interpolation   -y
                    }
#endif
                }

                // NTSC filter
                if (GUINTVID[cvidmode] != 0)
                    GUIDisplayCheckboxu(5, 128, 35, &NTSCFilter, "NTSC FILTER", 0);

                if (MMXSupport != 0) { // Kreed 2x filters
#ifdef __MSDOS__
                    if (GUI2xVID[cvidmode] != 0)
#else
                    if (GUIDSIZE[cvidmode] != 0)
#endif
                    {
                        GUIDisplayCheckboxun(5, 18, 45, &En2xSaI, 1, "2XSAI ENGINE", 2); // 2x
                        GUIDisplayCheckboxun(5, 128, 45, &En2xSaI, 2, "SUPER EAGLE", 6); // Seagle
                        GUIDisplayCheckboxun(5, 18, 55, &En2xSaI, 3, "SUPER 2XSAI", 2); // S2x
                    }
                }

                if (MMXSupport != 0) {
                    // Hq*x
                    if (GUIHQ2X[cvidmode] != 0) {
#ifdef __MSDOS__
                        GUIDisplayCheckboxu(5, 128, 55, &hqFilter, "HQ2X", 1);
#else
                        GUIDisplayCheckboxu(5, 128, 55, &hqFilter, "HQ FILTER", 1);
                        if (hqFilter != 0) {
                            GUIDisplayButtonHoleTu(5, 128, 68, &hqFilterlevel, 2, "2X", 1);
                            goto hq_x;
                        }
#endif
                    } else {
                    hq_x:;
#ifndef __MSDOS__
                        if (GUIHQ3X[cvidmode] != 0)
                            GUIDisplayButtonHoleTu(5, 158, 68, &hqFilterlevel, 3, "3X", 0);
                        if (GUIHQ4X[cvidmode] != 0)
                            GUIDisplayButtonHoleTu(5, 188, 68, &hqFilterlevel, 4, "4X", 0);
#endif
                    }
                }
            }

            char const* const GUIVideoTextB2 = "SCANLINES:"; // Filters.Scanlines
#ifndef __MSDOS__
            // GL Scanlines
            if (GUIBIFIL[cvidmode] != 0) {
                GUIDisplayTextY(5, 13, 80, GUIVideoTextB2); // Scanlines text
                GUIDrawSlider(5, 23, 100, 90, &sl_intensity, glscslidSet, glscslidText);
            } else
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
                    GUIDisplayButtonHoleTu(5, 18, 87, &scanlines, 0, "NONE", 1); // None
                    GUIDisplayButtonHoleTu(5, 168, 87, &scanlines, 1, "FULL", 0); // Full
                }
#ifdef __MSDOS__
                if (ScreenScale != 1 && GUIHSVID[cvidmode] != 0)
#else
                if (GUIDSIZE[cvidmode] != 0)
#endif
                {
                    GUIDisplayButtonHoleTu(5, 68, 87, &scanlines, 2, "25%", 0); // 25%
                    GUIDisplayButtonHoleTu(5, 118, 87, &scanlines, 3, "50%", 0); // 50%
                }
            }
        }

        GUIDisplayTextY(5, 13, 110, "MISC FILTERS:"); // Filters.Other
        GUIDisplayCheckboxu(5, 18, 115, &GrayscaleMode, "GRAYSCALE MODE", 0); // -v8

        // Hires Mode7
        if (GUIM7VID[cvidmode] != 0 && newengen != 0) {
            GUIDisplayCheckboxu(5, 128, 115, &Mode7HiRes16b, "HI-RES MODE 7", 0);
        }

        // Monitor Refresh
        // VSync
#if !defined __UNIXSDL__ || defined __OPENGL__
#ifdef __UNIXSDL__
        if (allow_glvsync == 1 && GUIBIFIL[cvidmode] != 0)
#endif
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
        if (GUIWFVID[cvidmode] != 0) {
            GUIDisplayCheckboxu(5, 128, 145, &TripleBufferWin, GUIVideoTextB4b, 0);
        }
#endif
#ifdef __MSDOS__
        if (GUITBVID[cvidmode] != 0 && TripBufAvail != 0) {
            GUIDisplayCheckboxu(5, 128, 145, &Triplebufen, GUIVideoTextB4b, 0);
        }
#endif

        char const* const GUIVideoTextB5 = "DISPLAY OPTIONS:"; // Video.Display
#ifndef __MSDOS__
        // Keep 4:3 Ratio
        if (GUIKEEP43[cvidmode] != 0 && Keep43Check()) {
            GUIDisplayTextY(5, 13, 170, GUIVideoTextB5);
            GUIDisplayCheckboxu(5, 18, 175, &Keep4_3Ratio, "USE 4:3 RATIO", 8);
        }
#else
        // Small Screen
        if (GUISSVID[cvidmode] != 0) {
            GUIDisplayTextY(5, 13, 170, GUIVideoTextB5);
            GUIDisplayCheckboxu(5, 18, 175, &smallscreenon, "SMALL SCREEN", 1); // -c
        }

        // Full/Widescreen
        if (GUIWSVID[cvidmode] != 0) {
            GUIDisplayTextY(5, 13, 170, GUIVideoTextB5);
            GUIDisplayCheckboxu(5, 128, 175, &ScreenScale, "WIDE SCREEN", 6); // -cc
        }
#endif
    }

    char const* const GUIVideoTextCD1 = "RESET"; // NTSC buttons + counter
    char const* const GUIVideoTextCD2 = "RESET ALL";

    if (GUIVntscTab[0] == 1) // NTSC Tab
    {
        GUIDisplayCheckboxu(5, 5, 25, &NTSCBlend, "BLEND FRAMES", 0); // NTSC Tab
        GUIDisplayCheckboxu(5, 135, 25, &NTSCRef, "REFRESH", 0);

        DrawGUIButton(5, 8, 166, 67, 177, "COMPOSITE", 81, 0, 0);
        DrawGUIButton(5, 72, 166, 119, 177, "S-VIDEO", 82, 0, 0);
        DrawGUIButton(5, 124, 166, 147, 177, "RGB", 83, 0, 0);
        DrawGUIButton(5, 152, 166, 217, 177, "MONOCHROME", 84, 0, 0);
        DrawGUIButton(5, 102, 186, 137, 197, GUIVideoTextCD1, 37, 0, 0);
        DrawGUIButton(5, 148, 186, 207, 197, GUIVideoTextCD2, 39, 0, 0);

        GUIDisplayTextY(5, 7, 46, "HUE:");
        GUIDisplayTextY(5, 7, 66, "SATURATION:");
        GUIDisplayTextY(5, 7, 86, "CONTRAST:");
        GUIDisplayTextY(5, 7, 106, "BRIGHTNESS:");
        GUIDisplayTextY(5, 7, 126, "SHARPNESS:");
        GUIDisplayTextY(5, 7, 156, "PRESETS:"); // NTSC Presets

        GUIDrawSlider(5, 8, 200, 56, &NTSCHue, NTSCslidSet, NTSCslidText);
        GUIDrawSlider(5, 8, 200, 76, &NTSCSat, NTSCslidSet, NTSCslidText);
        GUIDrawSlider(5, 8, 200, 96, &NTSCCont, NTSCslidSet, NTSCslidText);
        GUIDrawSlider(5, 8, 200, 116, &NTSCBright, NTSCslidSet, NTSCslidText);
        GUIDrawSlider(5, 8, 200, 136, &NTSCSharp, NTSCslidSet, NTSCslidText);
    }

    if (GUIVntscTab[0] == 2) // Advanced NTSC Options Tab
    {
        DrawGUIButton(5, 102, 186, 137, 197, GUIVideoTextCD1, 38, 0, 0);
        DrawGUIButton(5, 148, 186, 207, 197, GUIVideoTextCD2, 39, 0, 0);

        GUIDisplayTextY(5, 7, 36, "GAMMA:"); // NTSC Adv Tab
        GUIDisplayTextY(5, 7, 56, "RESOLUTION:");
        GUIDisplayTextY(5, 7, 76, "ARTIFACTS:");
        GUIDisplayTextY(5, 7, 96, "FRINGING:");
        GUIDisplayTextY(5, 7, 116, "BLEED:");
        GUIDisplayTextY(5, 7, 136, "HUE WARPING:");

        GUIDrawSlider(5, 8, 200, 46, &NTSCGamma, NTSCslidSet, NTSCslidText);
        GUIDrawSlider(5, 8, 200, 66, &NTSCRes, NTSCslidSet, NTSCslidText);
        GUIDrawSlider(5, 8, 200, 86, &NTSCArt, NTSCslidSet, NTSCslidText);
        GUIDrawSlider(5, 8, 200, 106, &NTSCFringe, NTSCslidSet, NTSCslidText);
        GUIDrawSlider(5, 8, 200, 126, &NTSCBleed, NTSCslidSet, NTSCslidText);
        GUIDrawSlider(5, 8, 200, 146, &NTSCWarp, NTSCslidSet, NTSCslidText);
    }
}

void DisplayGUISound(void)
{
    GUIDrawWindowBox(6, "SOUND CONFIG");

    GUIDisplayTextY(6, 6, 16, "SOUND SWITCHES:");
    GUIDisplayCheckboxu(6, 11, 21, &SPCDisable, "DISABLE SPC EMULATION", 0);
    if (SPCDisable == 0) {
        GUIDisplayCheckboxu(6, 11, 31, &soundon, "ENABLE SOUND", 0);
        if (soundon == 1) {
            GUIDisplayCheckboxu(6, 11, 41, &StereoSound, "ENABLE STEREO SOUND", 7);
            if (StereoSound == 1) {
                GUIDisplayCheckboxu(6, 11, 51, &RevStereo, "REVERSE STEREO CHANNELS", 2);
                GUIDisplayCheckboxu(6, 11, 61, &Surround, "SIMULATE SURROUND SOUND", 2);
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
    GUIDisplayButtonHoleTu(6, 11, 157, &SoundInterpType, 0, GUISoundTextF, 0);
    GUIDisplayButtonHoleTu(6, 11, 167, &SoundInterpType, 1, "GAUSSIAN", 0);
    GUIDisplayButtonHoleTu(6, 11, 177, &SoundInterpType, 2, "CUBIC SPLINE", 0);
    if (MMXSupport != 0) {
        GUIDisplayButtonHoleTu(6, 11, 187, &SoundInterpType, 3, "8-POINT", 0);
    }

    GUIDisplayTextY(6, 106, 152, "LOWPASS:");
    GUIDisplayButtonHoleTu(6, 111, 157, &LowPassFilterType, 0, GUISoundTextF, 1);
    GUIDisplayButtonHoleTu(6, 111, 167, &LowPassFilterType, 1, "SIMPLE", 1);
    GUIDisplayButtonHoleTu(6, 111, 177, &LowPassFilterType, 2, "DYNAMIC", 1);
    if (MMXSupport != 0) {
        GUIDisplayButtonHoleTu(6, 111, 187, &LowPassFilterType, 3, "HI QUALITY", 0);
    }

    GUIDisplayTextY(6, 6, 93, "SAMPLING RATE:");
#ifdef __MSDOS__
    if ((SoundQuality & 0xFF) > 2 && (SoundQuality & 0xFF) != 4 && StereoSound == 1 && SBHDMA == 0 && vibracard != 1) {
        GUIDisplayBBox(6, 15, 101, 69, 109, 167);
        GUIDisplayTextG(6, 23, 104, "N/A");
    } else
#endif
    {
        GUIDisplayBBox(6, 15, 101, 69, 109, 167); // Sampling Rate Box
        static char const GUISoundTextB1[][8] = {
            " 8000HZ",
            "11025HZ",
            "22050HZ",
            "44100HZ",
            "16000HZ",
            "32000HZ",
            "48000HZ"
        };
        GUIDisplayTextG(6, 23, 104, GUISoundTextB1[SoundQuality]);
    }

    GUIDisplayTextY(6, 6, 116, "VOLUME LEVEL:");
    GUIDrawSlider(6, 15, 100, 131, &MusicRelVol, glscslidSet, glscslidText);
}

static char const* DisplayGUICheatConv(u1 const* const c)
{
    char const* const tgl = c[-28] & 0x80 ? "SRC" : c[0] & 0x04 ? "OFF"
        : c[0] & 0x80                                           ? "RPL"
                                                                : "ON ";
    char const* const desc = (char const*)(c + 8);

    static char GUICheatTextZ3[6 + 1 + 2 + 1 + 2 + 1 + 3 + 1 + 20 + 1];
    sprintf(GUICheatTextZ3, "%06X %02X %02X %s %.20s", c[4] << 16 | c[3] << 8 | c[2], c[1], c[5], tgl, desc);
    return GUICheatTextZ3;
}

void DisplayGUICheat(void)
{
    GUIDrawWindowBox(7, "CHEAT");

    GUIDisplayText(7, 6, 13, "ADDRESS CV PV TGL DESCRIPTION"); // Top
    GUIDisplayText(7, 6, 132, "ENTER CODE:"); // Text by input boxes
    GUIDisplayText(7, 6, 143, "DESCRIPTION:");
    GUIDisplayText(7, 11, 154, "VALID CODES: GAME GENIE, PAR, AND GF"); // Info for User
    GUIDisplayText(7, 11, 164, "NOTE: YOU MAY HAVE TO RESET THE GAME");
    GUIDisplayText(7, 11, 172, "AFTER ENTERING THE CODE. REMEMBER TO");
    GUIDisplayText(7, 11, 180, "INSERT THE \"-\" FOR GAME GENIE CODES.");

    DrawGUIButton(7, 5, 113, 47, 124, "REMOVE", 5, 0, 0); // Draw Buttons
    DrawGUIButton(7, 52, 113, 94, 124, "TOGGLE", 6, 0, 0);
    DrawGUIButton(7, 99, 113, 141, 124, "SAVE", 7, 0, 0);
    DrawGUIButton(7, 146, 113, 188, 124, "LOAD", 8, 0, 0);
    DrawGUIButton(7, 193, 113, 235, 124, "FIX", 33, 0, 0);
    DrawGUIButton(7, 212, 134, 236, 145, "ADD", 9, 0, 0);

    GUIDisplayBBoxS(7, 5, 20, 229, 108, 167); // Draw Cheat Box

    if (GUIcurrentcheatwin == 0) // Red Highlight for Cheats box
    {
        u4 const ebx = 22 + (GUIcurrentcheatcursloc - GUIcurrentcheatviewloc) * 7;
        DrawGUIWinBox2(7, 5, 229, 7, 224, ebx);
    }

    u1 const* ccheatnpos = cheatdata + GUIcurrentcheatviewloc * 28; // Green Text
    u4 ccheatnleft = NumCheats - GUIcurrentcheatviewloc - 1;
    for (u4 i = 0; i != 12; ++i) {
        if (!(ccheatnleft & 0x80000000)) {
            u4 const p1 = 12;
            u4 const p2 = 24 + 7 * i;
            char const* const GUICheatTextZ3 = DisplayGUICheatConv(ccheatnpos);
            GUIDisplayTextG(7, p1, p2, GUICheatTextZ3);
            ccheatnpos += 28;
            --ccheatnleft;
        }
    }

    DrawSlideBar(7, 231, 20, GUIcurrentcheatviewloc, NumCheats, 12, 89, GUICStA, 7, 8);

    { // Code Box
        u1 const dl = GUIcurrentcheatwin != 1 ? 167 : GUIWincoladd == 0 ? 226
                                                                        : 227;
        GUIDisplayBBox(7, 82, 129, 172, 136, dl);
    }

    { // Description Box
        u1 const dl = GUIcurrentcheatwin != 2 ? 167 : GUIWincoladd == 0 ? 226
                                                                        : 227;
        GUIDisplayBBox(7, 82, 140, 196, 147, dl);
    }

    GUIDisplayTextG(7, 84, 132, GUICheatTextZ1); // Green Text&Shadow
    GUIDisplayTextG(7, 84, 143, GUICheatTextZ2);

    // Code for movement of cursor
    u1 const eax = GUICheatPosA;
    GUICheatTextZ1[eax] = '\0';
    u1 const ebx = GUICheatPosB;
    GUICheatTextZ2[ebx] = '\0';
    if (!(GUICCFlash & 8)) {
        switch (GUIcurrentcheatwin) {
        case 1:
            GUICheatTextZ1[eax] = '_';
            break;
        case 2:
            GUICheatTextZ2[ebx] = '_';
            break;
        }
    }

    char const* const GUICheatTextE1 = "AUTO-LOAD .CHT FILE AT GAME LOAD";
    if (GUIcurrentcheatwin == 0) { // Draw underline only if you don't have an input box selected
        GUIDisplayCheckboxu(7, 11, 186, &AutoLoadCht, GUICheatTextE1, 0);
    } else {
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
    u4 ecx = 16384;
    u4 esi = 0;
    u4 ebx = 0;
    do {
        u1 dl = *eax++;
        u1 dh = 8;
        do {
            if (dl & 1) {
                ++ebx;
                if (--edi == 0)
                    CSStartEntry = esi;
            }
            ++esi;
            dl >>= 1;
        } while (--dh != 0);
    } while (--ecx != 0);
    return ebx;
}

static void DisplayChtSrcResNoSearch(void)
{
    DrawGUIButton(13, 10, 140, 60, 152, "RESTART", 51, 0, 1);
    DrawGUIButton(13, 70, 140, 110, 152, "VIEW", 52, 0, 1);

    // Call and display # of results
    char GUICSrcTextG1[11];
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
    GUIDisplayButtonHoleTu(13, 11, 33, &CheatCompareValue, 0, "NEW VALUE IS > OLD VALUE", 0);
    GUIDisplayButtonHoleTu(13, 11, 43, &CheatCompareValue, 1, "NEW VALUE IS < OLD VALUE", 1);
    GUIDisplayButtonHoleTu(13, 11, 53, &CheatCompareValue, 2, "NEW VALUE IS = OLD VALUE", 2);
    GUIDisplayButtonHoleTu(13, 11, 63, &CheatCompareValue, 3, "NEW VALUE IS != OLD VALUE", 5);
    DisplayChtSrcRes();
}

static void CSRemoveFlash(char* const str)
{
    char* const i = strchr(str, '_');
    if (i)
        *i = '\0';
}

static void CSAddFlash(char* i)
{
    for (; *i != '\0'; ++i) {
        if (*i == '_')
            return;
    }
    i[0] = '_';
    i[1] = '\0';
}

static void CheatSearching(void) // Exact Value Search
{
    if (CheatSrcSearchType == 1) {
        CheatSearchingComp();
        return;
    }

    GUIDisplayText(13, 5, 20, "ENTER VALUE:"); // Make Yellow
    GUIDisplayText(13, 5, 65, "MAX VALUE:");

    GUIDisplayBBox(13, 10, 40, 80, 47, 167); // Input Box

    if (!(GUICCFlash & 8))
        CSRemoveFlash(CSInputDisplay); // Flash Cursor Code?

    u1 const col_shadow = CSOverValue == 1 ? 202 /* Alt Color */ : 223 /* Green Shadow */;
    u1 const col_text = CSOverValue == 1 ? 207 : // Alt Color
        GUIWincoladd == 0 ? 221
                          : // Green Text
        222;
    GUIOuttextwin2(13, 13, 42, CSInputDisplay, col_shadow);
    GUIOuttextwin2(13, 12, 41, CSInputDisplay, col_text);

    CSAddFlash(CSInputDisplay); // More flash?

    char GUICSrcTextG1[11];
    u4 const eax = SrcMask[CheatSrcByteSize]; // Find Max Size
    char* const esi = GUICSrcTextG1;
    if (CheatSrcByteBase != 1) {
        convertnum(esi, eax);
    } else {
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

    if (CheatSearchStatus != 1) {
        CheatSearching();
    } else {
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

    // Display Window Contents
    u4 ccheatnleft = NumCheatSrc - GUIcurrentchtsrcviewloc;
    if (ccheatnleft > 12)
        ccheatnleft = 12;

    u4 CheatSearchXPos = 10;
    u4 CheatSearchYPos = 24;
    u4 CSCurEntry = CSStartEntry;

    if (ccheatnleft != 0) {
        u4 curentryleft = GUIcurrentchtsrccursloc - GUIcurrentchtsrcviewloc;
        DrawGUIWinBox2(13, 5, 171, 7, 224, 22 + curentryleft * 7);
        do {
            if (curentryleft-- == 0)
                curentryval = CSCurEntry;

            char GUICSrcTextG1[11];
            converthex(GUICSrcTextG1, CSCurEntry + 0x7E0000, 3);
            GUIDisplayTextG(13, CheatSearchXPos, CheatSearchYPos, GUICSrcTextG1);

            char* const esi = GUICSrcTextG1;
            u4 const eax = *(u4 const*)(wramdata + CSCurEntry); // XXX ugly cast
            if (CheatSrcByteBase != 0) {
                converthex(esi, eax, CheatSrcByteSize + 1);
            } else {
                convertnum(esi, eax & SrcMask[CheatSrcByteSize]);
            }
            CheatSearchXPos += 42;
            GUIDisplayTextG(13, CheatSearchXPos, CheatSearchYPos, GUICSrcTextG1);

            CheatSearchXPos += 60;
            if (GUICSrcTextE[12] != '\0') {
                char* const esi = GUICSrcTextG1;
                u4 const eax = *(u4 const*)(vidbuffer + 129600 + CSCurEntry); // XXX ugly cast
                if (CheatSrcByteBase != 0) {
                    converthex(esi, eax, CheatSrcByteSize + 1);
                } else {
                    convertnum(esi, eax & SrcMask[CheatSrcByteSize]);
                }
                GUIDisplayTextG(13, CheatSearchXPos, CheatSearchYPos, GUICSrcTextG1);
            }
            CheatSearchXPos -= 102;
            CheatSearchYPos += 7;

            do // Search for next entry
            {
                ++CSCurEntry;
            } while (!(vidbuffer[129600 + 65536 * 2 + (CSCurEntry >> 3)] & (1 << (CSCurEntry & 7))));
        } while (--ccheatnleft != 0);
    }
    // win#,X,Y start, %4-List Loc, %5-List size, %6-Screen size, %7-Bar Size
    DrawSlideBar(13, 173, 20, GUIcurrentchtsrcviewloc, NumCheatSrc, 12, 89, GUICSStA, 11, 12);
    DrawGUIButton(13, 70, 140, 130, 152, "RETURN", 54, 0, 1);
    DrawGUIButton(13, 140, 140, 180, 152, "ADD", 55, 0, 1);
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

    GUIDisplayBBox(13, 10, 30, 80, 37, 167); // Boxes
    GUIDisplayBBox(13, 10, 55, 126, 62, 167);
    GUIDisplayBBox(13, 10, 80, 80, 120, 167);

    DrawGUIButton(13, 60, 155, 120, 167, "RETURN", 56, 0, 1); // Buttons
    DrawGUIButton(13, 130, 155, 160, 167, "ADD", 57, 0, 1);

    GUIDisplayText(13, 5, 130, "MAX VALUE:"); // Max Value Text
    char GUICSrcTextG1[11];
    u4 const eax = SrcMask[CheatSrcByteSize];
    char* const esi = GUICSrcTextG1;
    if (CheatSrcByteBase != 1) { // dec
        convertnum(esi, eax);
    } else { // hex
        converthex(esi, eax, CheatSrcByteSize + 1);
    }
    GUIDisplayText(13, 71, 130, GUICSrcTextG1);

    // Cheat Input
    if (CurCStextpos != 0 || !(GUICCFlash & 8))
        CSRemoveFlash(CSInputDisplay);
    u1 const col_shadow = CSOverValue == 1 ? 202 : 223;
    u1 const col_text = CSOverValue == 1 ? 207 : GUIWincoladd == 0 ? 221
                                                                   : 222;
    GUIOuttextwin2(13, 13, 32, CSInputDisplay, col_shadow);
    GUIOuttextwin2(13, 12, 31, CSInputDisplay, col_text);
    CSAddFlash(CSInputDisplay);

    // Cheat Desc. Input
    if (CurCStextpos == 1 && !(GUICCFlash & 8))
        CSAddFlash(CSDescDisplay);
    GUIDisplayTextG(13, 13, 57, CSDescDisplay);
    CSRemoveFlash(CSDescDisplay);

    if (CSOverValue != 1 && CSInputDisplay[0] != '_') {
        u4 CheatSearchYPos = 83; // PAR Code?
        curaddrvalcs = curentryval;
        u4 const eax = CSCurValue;
        curvaluecs = eax;
        u1 ecx = CheatSrcByteSize + 1;
        if (CheatUpperByteOnly != 0) {
            ecx = 1;
            while (curvaluecs > 0xFF) {
                curvaluecs >>= 8;
                ++curaddrvalcs;
            }
        }
        do { // Max Value Display?
            converthex(GUICSrcTextG1, curaddrvalcs + 0x7E0000, 3);
            converthex(GUICSrcTextG1 + 6, curvaluecs, 1);
            curvaluecs >>= 8;
            GUIDisplayTextG(13, 13, CheatSearchYPos + 1, GUICSrcTextG1);
            CheatSearchYPos += 10;
            ++curaddrvalcs;
        } while (--ecx != 0);
    }
}

void DisplayGUISearch(void)
{
    switch (CheatWinMode) // Determine which CS window we're on
    {
    case 1:
        Incheatmode();
        return;
    case 2:
        Cheatmodeview();
        return;
    case 3:
        Cheatmodeadd();
        return;
    }

    // Opening Screen
    GUIwinsizex[13] = 170;
    GUIwinsizey[13] = 150;
    DrawWindowSearch();

    // Radio Buttons
    GUIDisplayTextY(13, 6, 16, "SELECT SIZE AND FORMAT:");
    GUIDisplayButtonHoleTu(13, 11, 28, &CheatSrcByteSize, 0, "1 BYTE  [0..255]", 0);
    GUIDisplayButtonHoleTu(13, 11, 38, &CheatSrcByteSize, 1, "2 BYTES [0..65535]", 0);
    GUIDisplayButtonHoleTu(13, 11, 48, &CheatSrcByteSize, 2, "3 BYTES [0..16777215]", 0);
    GUIDisplayButtonHoleTu(13, 11, 58, &CheatSrcByteSize, 3, "4 BYTES [0..4294967295]", 0);
    GUIDisplayButtonHoleTu(13, 11, 73, &CheatSrcByteBase, 0, "DEC (BASE 10)", 0);
    GUIDisplayButtonHoleTu(13, 11, 83, &CheatSrcByteBase, 1, "HEX (BASE 16)", 0);

    GUIDisplayTextY(13, 6, 101, "SELECT SEARCH TYPE:");
    GUIDisplayButtonHoleTu(13, 11, 113, &CheatSrcSearchType, 0, "EXACT VALUE SEARCH", 0);
    GUIDisplayButtonHoleTu(13, 11, 123, &CheatSrcSearchType, 1, "COMPARATIVE SEARCH", 0);

    DrawGUIButton(13, 95, 140, 140, 152, "START", 50, 0, 1);
}

void DisplayNetOptns(void) { }

void DisplayGameOptns(void)
{
    GUIDrawWindowBox(9, "MISC KEYS");

    GUIDisplayTextY(9, 6, 16, "BG DISABLES:");
    GUIDisplayText(9, 9, 25, "BG1");
    GUIDisplayText(9, 9 + 45, 25, "BG2");
    GUIDisplayText(9, 9 + 90, 25, "BG3");
    GUIDisplayText(9, 9 + 135, 25, "BG4");
    GUIDisplayText(9, 9 + 180, 25, "OBJ");

    GUIDisplayTextY(9, 6, 34, "SOUND KEYS:");
    GUIDisplayText(9, 9, 43, "CH1");
    GUIDisplayText(9, 9 + 45, 43, "CH2");
    GUIDisplayText(9, 9 + 45 * 2, 43, "CH3");
    GUIDisplayText(9, 9 + 45 * 3, 43, "CH4");
    GUIDisplayText(9, 9 + 45 * 4, 43, "+VOL");
    GUIDisplayText(9, 9, 52, "CH5");
    GUIDisplayText(9, 9 + 45, 52, "CH6");
    GUIDisplayText(9, 9 + 45 * 2, 52, "CH7");
    GUIDisplayText(9, 9 + 45 * 3, 52, "CH8");
    GUIDisplayText(9, 9 + 45 * 4, 52, "-VOL");

    GUIDisplayTextY(9, 6, 61, "QUICK KEYS:");
    GUIDisplayText(9, 9, 72, "LOAD");
    GUIDisplayText(9, 9 + 52, 72, "RESET");
    GUIDisplayText(9, 9 + 109, 72, "EXIT");
    GUIDisplayText(9, 9 + 160, 72, "CLOCK");
    GUIDisplayText(9, 9, 82, "CHAT");
    GUIDisplayText(9, 9 + 52, 82, "SNAPSHOT");
    GUIDisplayText(9, 137, 82, "SAVE SPC");

    GUIDisplayTextY(9, 6, 93, "MISC TOGGLES:");
    GUIDisplayText(9, 9, 102, "USE PL12/34");
    GUIDisplayText(9, 9, 112, "PANIC KEY");
    GUIDisplayText(9, 9, 122, "DISPLAY FPS");
#ifndef __MSDOS__
    GUIDisplayText(9, 9, 132, "BATT POWER");
#endif

    GUIDisplayTextY(9, 119, 93, "GFX TOGGLES:");
    GUIDisplayText(9, 122, 102, "NEW GFX ENG");
    GUIDisplayText(9, 122, 112, "BG WINDOW");
    GUIDisplayText(9, 122, 122, "OFFSET MODE");
    GUIDisplayText(9, 122, 132, "+ GAMMA");
    GUIDisplayText(9, 122, 142, "- GAMMA");

    // Draw black boxes
    DDrawBox(9, 26, 22, &KeyBGDisble0);
    DDrawBox(9, 71, 22, &KeyBGDisble1);
    DDrawBox(9, 116, 22, &KeyBGDisble2);
    DDrawBox(9, 161, 22, &KeyBGDisble3);
    DDrawBox(9, 206, 22, &KeySprDisble);

    DDrawBox(9, 26, 40, &KeyDisableSC0);
    DDrawBox(9, 71, 40, &KeyDisableSC1);
    DDrawBox(9, 116, 40, &KeyDisableSC2);
    DDrawBox(9, 161, 40, &KeyDisableSC3);
    DDrawBox(9, 213, 40, &KeyVolUp);
    DDrawBox(9, 26, 49, &KeyDisableSC4);
    DDrawBox(9, 71, 49, &KeyDisableSC5);
    DDrawBox(9, 116, 49, &KeyDisableSC6);
    DDrawBox(9, 161, 49, &KeyDisableSC7);
    DDrawBox(9, 213, 49, &KeyVolDown);

    DDrawBox(9, 32, 69, &KeyQuickLoad);
    DDrawBox(9, 90, 69, &KeyQuickRst);
    DDrawBox(9, 141, 69, &KeyQuickExit);
    DDrawBox(9, 199, 69, &KeyQuickClock);
    DDrawBox(9, 32, 79, &KeyQuickChat);
    DDrawBox(9, 109, 79, &KeyQuickSnapShot);
    DDrawBox(9, 185, 79, &KeyQuickSaveSPC);

    DDrawBox(9, 77, 99, &KeyUsePlayer1234);
    DDrawBox(9, 77, 109, &KeyResetAll);
    DDrawBox(9, 77, 119, &KeyDisplayFPS);
#ifndef __MSDOS__
    DDrawBox(9, 77, 129, &KeyDisplayBatt);
#endif

    DDrawBox(9, 190, 99, &KeyNewGfxSwt);
    DDrawBox(9, 190, 109, &KeyWinDisble);
    DDrawBox(9, 190, 119, &KeyOffsetMSw);
    DDrawBox(9, 190, 129, &KeyIncreaseGamma);
    DDrawBox(9, 190, 139, &KeyDecreaseGamma);
}

static u1 GUICslidSet(void const* const p1) // slider var
{
    return 2 + *(u1 const*)p1 * 4;
}

static char const* GUICslidText(void const* p1) // slider var, text
{
    static char GUIGUIOptnsTextD2[3];
    sprintf(GUIGUIOptnsTextD2, "%2u", *(u1 const*)p1);
    return GUIGUIOptnsTextD2;
}

void DisplayGUIOptns(void)
{
#ifdef __WIN32__ // If Windows, extend window down
    GUIwinsizey[10] = 192;
#endif
    GUIDrawWindowBox(10, "GUI OPTIONS");

    // Setup Colors
    u1 TRVal2;
    u1 TGVal2;
    u1 TBVal2;
    switch (CurPalSelect) {
    default:
        TRVal2 = GUIRAdd;
        TGVal2 = GUIGAdd;
        TBVal2 = GUIBAdd;
        break;
    case 1:
        TRVal2 = GUITRAdd;
        TGVal2 = GUITGAdd;
        TBVal2 = GUITBAdd;
        break;
    case 2:
        TRVal2 = GUIWRAdd;
        TGVal2 = GUIWGAdd;
        TBVal2 = GUIWBAdd;
        break;
    }

    GUIDrawSlider(10, 25, 127, 124, &TRVal2, GUICslidSet, GUICslidText);
    GUIDrawSlider(10, 25, 127, 136, &TGVal2, GUICslidSet, GUICslidText);
    GUIDrawSlider(10, 25, 127, 148, &TBVal2, GUICslidSet, GUICslidText);

    GUIDisplayTextY(10, 6, 16, "GUI SWITCHES:");

    // Checkboxes
    GUIDisplayCheckboxu(10, 12, 23, &GUIRClick, "RCLICK OPENS GUI", 1);
    GUIDisplayCheckboxu(10, 12, 33, &lhguimouse, "SWAP L/R MBUTTONS", 6);
    GUIDisplayCheckboxu(10, 12, 43, &mouseshad, "SHOW MOUSE SHADOW", 0);
    GUIDisplayCheckboxu(10, 12, 53, &mousewrap, "MICE WRAP GUI WIN", 0);
#ifdef __WIN32__
    GUIDisplayCheckboxu(10, 12, 63, &TrapMouseCursor, "TRAP MOUSE CURSOR", 3);
    GUIDisplayCheckboxu(10, 12, 73, &MouseWheel, "WHEEL MICE SCROLL", 1);
#endif

    GUIDisplayCheckboxu(10, 129, 23, &esctomenu, "ESC TO GAME MENU", 7);
    GUIDisplayCheckboxu(10, 129, 33, &JoyPad1Move, "CTRL GUI W/GPAD1", 6);
    GUIDisplayCheckboxu(10, 129, 43, &FilteredGUI, "FILTERED GUI", 0);
    GUIDisplayCheckboxu(10, 129, 53, &newfont, "USE CUSTOM FONT", 12);
    GUIDisplayCheckboxu(10, 129, 63, &savewinpos, "SAVE GUI WIN POS", 9);

    GUIDisplayTextY(10, 6, 91, "BG EFFECTS:");
    GUIDisplayButtonHoleTu(10, 72, 88, &GUIEffect, 0, "NONE", 3);
    GUIDisplayButtonHoleTu(10, 122, 88, &GUIEffect, 1, "SNOW", 1);
    GUIDisplayButtonHoleTu(10, 182, 88, &GUIEffect, 4, "BURNING", 2);
    GUIDisplayButtonHoleTu(10, 72, 98, &GUIEffect, 5, "SMOKE", 3);
    GUIDisplayButtonHoleTu(10, 122, 98, &GUIEffect, 2, "WATER A", 6);
    GUIDisplayButtonHoleTu(10, 182, 98, &GUIEffect, 3, "WATER B", 6);

    GUIDisplayTextY(10, 6, 111, "COLOR:");
    GUIDisplayText(10, 60, 111, "BACK");
    GUIDisplayText(10, 100, 111, "TITLE");
    GUIDisplayText(10, 145, 111, "WIN");

#ifdef __WIN32__
    GUIDisplayTextY(10, 6, 161, "MAIN WINDOW OPTIONS:");
    GUIDisplayCheckboxu(10, 12, 168, &AlwaysOnTop, "EMU ALWAYS ON TOP", 14);
    GUIDisplayCheckboxu(10, 12, 178, &SaveMainWindowPos, "SAVE MAIN WINDOW POSITION", 2);
    GUIDisplayCheckboxu(10, 12, 188, &AllowMultipleInst, "ALLOW MULTIPLE INSTANCES OF EMU", 1);
#endif

    GUIDisplayText(10, 16, 123, "R");
    GUIDisplayText(10, 16, 135, "G");
    GUIDisplayText(10, 16, 147, "B");
    // Radio Buttons
    GUIDisplayButtonHole(10, 48, 108, &CurPalSelect, 0);
    GUIDisplayButtonHole(10, 88, 108, &CurPalSelect, 1);
    GUIDisplayButtonHole(10, 133, 108, &CurPalSelect, 2);
}

void DisplayGUIAbout(void)
{
    // This will attach compile date onto the end of GUIGUIAboutText1
    static char GUIGUIAboutTextA1[] = "ZSNES V" ZVER "             "; // Need room for date
    VERSION_STR = GUIGUIAboutTextA1;
    placedate();

#if defined __MSDOS__
    char const* const GUIGUIAboutTextA2 = "DOS VERSION";
#elif defined __WIN32__
    char const* const GUIGUIAboutTextA2 = "WIN VERSION";
#elif defined __UNIXSDL__
    char const* const GUIGUIAboutTextA2 = "SDL VERSION";
#endif

    GUIDrawWindowBox(11, "ABOUT");
    if (EEgg != 1) {
        GUIDisplayText(11, 6, 16, GUIGUIAboutTextA1); // Text
        // GUIDisplayText(11, 6, 36, GUIGUIAboutTextA2);
        GUIDisplayTextY(11, 6, 46, "CODED BY:");
        GUIDisplayText(11, 6, 56, "    ZSKNIGHT      _DEMO_");
        GUIDisplayText(11, 6, 66, "    PAGEFAULT     NACH");
        GUIDisplayTextY(11, 6, 76, "ASSISTANT CODERS:");
        GUIDisplayText(11, 6, 86, "    PHAROS        STATMAT");
        GUIDisplayText(11, 6, 96, "    TEUF          HPSOLO");
        GUIDisplayText(11, 6, 106, "    THEODDONE33   SILOH");
        GUIDisplayText(11, 6, 116, "    IPHER         GRINVADER");
        GUIDisplayText(11, 6, 126, "    JONAS QUINN   DEATHLIKE");
        GUIDisplayText(11, 15, 151, "ZSNES is released under");
        GUIDisplayText(11, 15, 161, "the GPL2 license. See the");
        GUIDisplayText(11, 15, 171, "contents of the `COPYING`");
        GUIDisplayText(11, 15, 181, "file for more information.");

        DrawGUIButton(11, 90, 27, 175, 37, "WWW.ZSNES.COM", 65, 0, 0);
        DrawGUIButton(11, 90, 38, 175, 48, "DOCUMENTATION", 66, 0, 0);
    } else { // Playground
        GUIDisplayText(11, 42, 36, "HIDDEN MESSAGE!");
        GUIDisplayText(11, 30, 96, "PRESS 'E' TO RETURN");
        GUIDisplayText(11, 39, 106, "TO THE ABOUT BOX");
    }
}

void DisplayGUIMovies(void)
{
    GUIDrawWindowBox(15, "MOVIE OPTIONS"); // Display Window

    u4 eax;
    u4 ebx;
    GUIDrawTArea(15, &eax, &ebx);
    if (MovieProcessing < 4 || 6 < MovieProcessing)
        GUIDrawTabs(GUIMovieTabs, &eax, ebx);
    if (MovieProcessing < 1 || 3 < MovieProcessing)
        GUIDrawTabs(GUIDumpingTab, &eax, ebx);

    if (RawDumpInProgress) {
        GUIMovieTabs[0] = 0;
        GUIDumpingTab[0] = 1;
    }

    if (MovieRecordWinVal != 0) {
        GUIDisplayText(15, 9, 26, "WARNING: THIS MOVIE"); // Overwrite Message Box
        GUIDisplayText(15, 9, 36, "FILE ALREADY EXISTS");
        GUIDisplayText(15, 9, 51, "OKAY TO OVERWRITE ?");

        DrawGUIButton(15, 17, 65, 59, 76, "YES", 19, 0, 0); // Yes/No Buttons
        DrawGUIButton(15, 70, 65, 112, 76, "NO", 20, 0, 0);
    } else {
        // Main Window
        GUIDisplayTextY(15, 8, 31, "SELECT MOVIE:"); // Slot text
        GUIDisplayText(15, 20, 42, "0");
        GUIDisplayText(15, 40, 42, "1");
        GUIDisplayText(15, 60, 42, "2");
        GUIDisplayText(15, 80, 42, "3");
        GUIDisplayText(15, 100, 42, "4");
        GUIDisplayText(15, 120, 42, "5");
        GUIDisplayText(15, 140, 42, "6");
        GUIDisplayText(15, 160, 42, "7");
        GUIDisplayText(15, 180, 42, "8");
        GUIDisplayText(15, 200, 42, "9");

        // Display Radio buttons
        GUIDisplayButtonHole(15, 8, 39, (u1 const*)&CMovieExt, 'v'); // XXX ugly cast
        GUIDisplayButtonHole(15, 28, 39, (u1 const*)&CMovieExt, '1'); // XXX ugly cast
        GUIDisplayButtonHole(15, 48, 39, (u1 const*)&CMovieExt, '2'); // XXX ugly cast
        GUIDisplayButtonHole(15, 68, 39, (u1 const*)&CMovieExt, '3'); // XXX ugly cast
        GUIDisplayButtonHole(15, 88, 39, (u1 const*)&CMovieExt, '4'); // XXX ugly cast
        GUIDisplayButtonHole(15, 108, 39, (u1 const*)&CMovieExt, '5'); // XXX ugly cast
        GUIDisplayButtonHole(15, 128, 39, (u1 const*)&CMovieExt, '6'); // XXX ugly cast
        GUIDisplayButtonHole(15, 148, 39, (u1 const*)&CMovieExt, '7'); // XXX ugly cast
        GUIDisplayButtonHole(15, 168, 39, (u1 const*)&CMovieExt, '8'); // XXX ugly cast
        GUIDisplayButtonHole(15, 188, 39, (u1 const*)&CMovieExt, '9'); // XXX ugly cast

        // Determine and Display Status
        char const* status;
        switch (MovieProcessing) {
        default:
            status = "INACTIVE        ";
            break;
        case 1:
            status = "PLAYING         ";
            break;
        case 2:
            status = "RECORDING       ";
            break;
        case 3:
            status = "OLD PLAYING     ";
            break;
        case 4:
            status = "DUMPING ENDING  ";
            break;
        case 5:
            status = "DUMPING         ";
            break;
        case 6:
            status = "DUMPING OLD     ";
            break;
        }
        static char GUIMovieTextZ[] = "STATUS:                  ";
        strcpy(GUIMovieTextZ + 8, status);
        GUIDisplayTextY(15, 6, 192, GUIMovieTextZ);

        if (GUIMovieTabs[0] == 1) {
            GUIDisplayTextY(15, 8, 56, "RECORD FROM:"); // "Start From" Section
            GUIDisplayTextY(15, 8, 100, "CHAPTERS:"); // Chapters

            GUIDisplayButtonHoleTu(15, 8, 64, &MovieStartMethod, 0, "NOW", 0);
            GUIDisplayButtonHoleTu(15, 43, 64, &MovieStartMethod, 1, "POWER", 0);
            GUIDisplayButtonHoleTu(15, 89, 64, &MovieStartMethod, 2, "RESET", 0);
            GUIDisplayButtonHoleTu(15, 135, 64, &MovieStartMethod, 3, "POWER+SRAM CLEAR", 6);

            DrawGUIButton(15, 7, 80, 49, 91, "PLAY", 16, 0, 0); // Draw Buttons
            DrawGUIButton(15, 55, 80, 97, 91, "RECORD", 17, 0, 0);
            DrawGUIButton(15, 103, 80, 145, 91, "STOP", 18, 0, 0);
            DrawGUIButton(15, 151, 80, 193, 91, "APPEND", 32, 0, 0);
            DrawGUIButton(15, 7, 108, 50, 119, "INSERT", 29, 0, 0);
            DrawGUIButton(15, 85, 108, 138, 119, "PREVIOUS", 30, 0, 0);
            DrawGUIButton(15, 173, 108, 203, 119, "NEXT", 31, 0, 0);

            DDrawBox(15, 57, 109, &KeyInsrtChap); // Chapter Keyboard Shortcut Boxes
            DDrawBox(15, 145, 109, &KeyPrevChap);
            DDrawBox(15, 210, 109, &KeyNextChap);

            GUIDisplayTextY(15, 8, 125, "ON MOVIE STATE LOAD:"); // Movie State Load

            GUIDisplayButtonHoleTu(15, 8, 133, &MZTForceRTR, 0, "DO NOT SWITCH MODES", 14);
            GUIDisplayButtonHoleTu(15, 8, 143, &MZTForceRTR, 1, "SWITCH TO RECORD", 12);
            GUIDisplayButtonHoleTu(15, 8, 153, &MZTForceRTR, 2, "SWITCH TO PLAYBACK", 14);

            DDrawBox(15, 134, 123, &KeyRTRCycle); // MZT Load Shortcut Box

            GUIDisplayCheckboxu(15, 8, 163, &MovieDisplayFrame, "DISPLAY FRAME COUNTER", 0); // Checkbox
        }

        if (GUIDumpingTab[0] == 1) {
            GUIDisplayTextY(15, 8, 56, "VIDEO OPTIONS:"); // Video Section
            GUIDisplayButtonHoleTu(15, 8, 64, &MovieVideoMode, 0, "NO VIDEO DUMP", 1);
            GUIDisplayButtonHoleTu(15, 8, 74, &MovieVideoMode, 1, "RAW VIDEO", 2);
            GUIDisplayButtonHoleTu(15, 8, 84, &MovieVideoMode, 2, "FFV1", 0);
            GUIDisplayButtonHoleTu(15, 8, 94, &MovieVideoMode, 3, "X264 LOSSLESS", 9);
            GUIDisplayButtonHoleTu(15, 8, 104, &MovieVideoMode, 4, "XVID LOSSLESS", 0);
            GUIDisplayButtonHoleTu(15, 8, 114, &MovieVideoMode, 5, "CUSTOM", 0);

            GUIDisplayTextY(15, 129, 56, "AUDIO OPTIONS:"); // Audio Section
            GUIDisplayTextY(15, 162, 171, "DUMPING:");

            if (MovieVideoMode != 5) {
                GUIDisplayCheckboxu(15, 130, 62, &MovieAudio, "DUMP AUDIO", 5);
                if (MovieAudio == 1)
                    goto mux;
            } else {
            mux:
                GUIDisplayCheckboxu(15, 130, 72, &MovieAudioCompress, "COMPRESS AUDIO", 2);
                if ((s1)MovieVideoMode >= 2 && MovieVideoMode != 5) // XXX ugly cast
                {
                    GUIDisplayCheckboxu(15, 130, 82, &MovieVideoAudio, "MERGE WITH VIDEO", 11);
                }
            }

            DrawGUIButton(15, 165, 178, 200, 189, "START", 34, 0, 0);
            DrawGUIButton(15, 206, 178, 235, 189, "STOP", 35, 0, 0);

            GUIDisplayTextY(15, 8, 127, "DUMPING LENGTH:"); // Video Section
            GUIDisplayButtonHoleTu(15, 8, 135, &MovieForcedLengthEnabled, 0, "ZMV LENGTH", 0);
            GUIDisplayButtonHoleTu(15, 8, 145, &MovieForcedLengthEnabled, 1, "DUMP # OF FRAMES", 11);
            GUIDisplayButtonHoleTu(15, 8, 155, &MovieForcedLengthEnabled, 2, "UNTIL STOP", 0);

            GUIDisplayBBox(15, 136, 144, 205, 154, 167);

            GetMovieForcedLength();

            GUIOuttextwin2d(15, 139, 148, GUIMovieForcedText, 10, GUIMovieTextPtr, 0);
        }
    }
}

static void DrawBorderedBox(u4 const p1, u4 const p2, u4 const p3, u4 const p4, u4 const p5)
{
    // draw borders
    DrawGUIWinBox(p1, p2, p3 - 1, p4, p3, GUIWincol);
    DrawGUIWinBox(p1, p2 - 1, p3, p2 - 2, p5, GUIWincol + 1);
    DrawGUIWinBox(p1, p2, p5, p4, p5 + 1, GUIWincol + 4);
    DrawGUIWinBox(p1, p4, p3, p4 + 1, p5, GUIWincol + 3);
    DrawGUIWinBox(p1, p2, p3, p4, p5, 167);
}

static void DrawBorderedBoxB(u4 const p1, u4 const p2, u4 const p3, u4 const p4, u4 const p5, char const* const p7) // Special function for combo displays
{
    DrawBorderedBox(p1, p2, p3, p4, p5);
    GUIDisplayTextG(p1, p2 + 5, p3 + 2, p7);
}

static void DrawBorderedBoxB2(u4 const p1, u4 const p2, u4 const p3, u4 const p4, u4 const p5, char const* const p7) // Special function for combo displays
{
    DrawBorderedBox(p1, p2, p3, p4, p5);
    GUIDisplayTextG(p1, p2 + 2, p3 + 2, p7);
}

// Key types: Up, Down, Left, Right, A, B, X, Y, L, R (Press/Relase/P+R)
// Frame delays: 1 frame, 2, 3, 4, 5, 1 sec., 2, 3, 4, 5
void DisplayGUICombo(void)
{
    if (GUIccomblcursloc != GUIccombcursloc) {
        GUIccomblcursloc = GUIccombcursloc;
        // copy contents into temporary variables
        ComboData const* const esi = &(GUIComboGameSpec == 0 ? CombinDataGlob : CombinDataLocl)[GUIccombcursloc];
        memcpy(GUIComboTextH, esi->name, sizeof(esi->name));
        memcpy(GUIComboData, esi->combo, sizeof(esi->combo));
        GUIComboKey = GUIComboKey & 0xFFFF0000 | esi->key;
        GUIComboPNum = esi->player;
        GUIComboLHorz = esi->ff;
        // determine length of combo data
        u1 const* eax = GUIComboData;
        u4 ecx = 0;
        while (*eax++ != 0)
            ++ecx;
        GUINumCombo = ecx;
    }

    // copy into data if description equal
    {
        ComboData* const edi = &(GUIComboGameSpec == 0 ? CombinDataGlob : CombinDataLocl)[GUIccombcursloc];
        if (strncmp(edi->name, GUIComboTextH, 20) == 0) {
            ComboClip();
            memcpy(edi->name, GUIComboTextH, sizeof(edi->name));
            memcpy(edi->combo, GUIComboData, sizeof(edi->combo));
            edi->key = GUIComboKey;
            edi->player = GUIComboPNum;
            edi->ff = GUIComboLHorz;
        }
    }

    GUIDrawWindowBox(16, "KEY COMBINATION EDITOR");

    DrawBorderedBox(16, 10, 20, 190, 80);
    DrawBorderedBox(16, 37, 91, 157, 99);
    DrawBorderedBox(16, 10, 91, 32, 99);
    DrawBorderedBox(16, 10, 110, 220, 146);

    // win#,X,Y start,List Loc,List size,# Lines,Bar Size(Y),UpArrowResource#,DownArrowRes#
    DrawSlideBar(16, 192, 20, GUIccombviewloc, NumCombo, 8, 61, GUICSStC, 13, 14);

    // Draw control boxes
    DrawBorderedBoxB(16, 75, 150, 85, 157, "\xFB");
    DrawBorderedBoxB(16, 89, 150, 99, 157, "\xFC");
    DrawBorderedBoxB(16, 103, 150, 113, 157, "\xFD");
    DrawBorderedBoxB(16, 117, 150, 127, 157, "\xFE");
    DrawBorderedBoxB(16, 131, 150, 141, 157, "A");
    DrawBorderedBoxB(16, 145, 150, 155, 157, "B");
    DrawBorderedBoxB(16, 159, 150, 169, 157, "X");
    DrawBorderedBoxB(16, 173, 150, 183, 157, "Y");
    DrawBorderedBoxB(16, 187, 150, 197, 157, "L");
    DrawBorderedBoxB(16, 201, 150, 211, 157, "R");
    DrawBorderedBoxB2(16, 215, 150, 227, 157, "ST");
    DrawBorderedBoxB2(16, 231, 150, 243, 157, "SL");

    DrawBorderedBoxB(16, 75, 160, 85, 167, "\xFB");
    DrawBorderedBoxB(16, 89, 160, 99, 167, "\xFC");
    DrawBorderedBoxB(16, 103, 160, 113, 167, "\xFD");
    DrawBorderedBoxB(16, 117, 160, 127, 167, "\xFE");
    DrawBorderedBoxB(16, 131, 160, 141, 167, "A");
    DrawBorderedBoxB(16, 145, 160, 155, 167, "B");
    DrawBorderedBoxB(16, 159, 160, 169, 167, "X");
    DrawBorderedBoxB(16, 173, 160, 183, 167, "Y");
    DrawBorderedBoxB(16, 187, 160, 197, 167, "L");
    DrawBorderedBoxB(16, 201, 160, 211, 167, "R");
    DrawBorderedBoxB2(16, 215, 160, 227, 167, "ST");
    DrawBorderedBoxB2(16, 231, 160, 243, 167, "SL");

    DrawBorderedBoxB(16, 75, 170, 85, 177, "\xFB");
    DrawBorderedBoxB(16, 89, 170, 99, 177, "\xFC");
    DrawBorderedBoxB(16, 103, 170, 113, 177, "\xFD");
    DrawBorderedBoxB(16, 117, 170, 127, 177, "\xFE");
    DrawBorderedBoxB(16, 131, 170, 141, 177, "A");
    DrawBorderedBoxB(16, 145, 170, 155, 177, "B");
    DrawBorderedBoxB(16, 159, 170, 169, 177, "X");
    DrawBorderedBoxB(16, 173, 170, 183, 177, "Y");
    DrawBorderedBoxB(16, 187, 170, 197, 177, "L");
    DrawBorderedBoxB(16, 201, 170, 211, 177, "R");
    DrawBorderedBoxB2(16, 215, 170, 227, 177, "ST");
    DrawBorderedBoxB2(16, 231, 170, 243, 177, "SL");

    DrawBorderedBoxB(16, 10, 189, 20, 196, "1");
    DrawBorderedBoxB(16, 24, 189, 34, 196, "2");
    DrawBorderedBoxB(16, 38, 189, 48, 196, "3");
    DrawBorderedBoxB(16, 52, 189, 62, 196, "4");
    DrawBorderedBoxB(16, 66, 189, 76, 196, "5");
    DrawBorderedBoxB(16, 80, 189, 90, 196, "9");

    DrawBorderedBoxB(16, 107, 189, 117, 196, "\xFA");
    DrawBorderedBoxB(16, 121, 189, 131, 196, "1");
    DrawBorderedBoxB(16, 135, 189, 145, 196, "2");
    DrawBorderedBoxB(16, 149, 189, 159, 196, "3");
    DrawBorderedBoxB(16, 163, 189, 173, 196, "4");
    DrawBorderedBoxB(16, 177, 189, 187, 196, "5");

    DrawBorderedBoxB(16, 204, 189, 218, 196, "\xFF");

    GUIDisplayText(16, 10, 13, "DESCRIPTION"); // Text
    GUIDisplayText(16, 138, 13, "KEY P# LH");
    GUIDisplayText(16, 38, 84, "DESCRIPTION:");
    GUIDisplayText(16, 10, 84, "KEY:");
    GUIDisplayText(16, 10, 103, "COMBINATION KEYS:");
    GUIDisplayText(16, 10, 152, "PRESS+REL");
    GUIDisplayText(16, 10, 162, "PRESS ONLY");
    GUIDisplayText(16, 10, 172, "REL ONLY");
    GUIDisplayText(16, 10, 182, "FRAME DELAY");
    GUIDisplayText(16, 114, 182, "SECOND DELAY");
    GUIDisplayText(16, 204, 182, "DEL");
    GUIDisplayText(16, 145, 85, "P#  1  2  3  4  5");

    char const* const GUIComboTextA = "CLEAR";
    char const* const GUIComboTextB = "ADD";
    char const* const GUIComboTextC = "REPLACE";
    char const* const GUIComboTextD = "DELETE";

    // XXX twice? see below
    DrawGUIButton(16, 202, 20, 246, 31, GUIComboTextA, 60, -1, 0); // Buttons
    DrawGUIButton(16, 202, 35, 246, 46, GUIComboTextB, 61, -1, 0);
    DrawGUIButton(16, 202, 50, 246, 61, GUIComboTextC, 62, -1, 0);
    DrawGUIButton(16, 202, 65, 246, 76, GUIComboTextD, 63, -1, 0);

    // Calculate Text Cursor Position / Draw Box Text
    {
        u4 const eax = strlen(GUIComboTextH);
        GUIComboPos = eax;
        if (!(GUICCFlash & 8)) {
            GUIComboTextH[eax] = '_';
            GUIComboTextH[eax + 1] = '\0';
        }
        GUIDisplayTextG(16, 39, 94, GUIComboTextH);
        GUIComboTextH[GUIComboPos] = '\0';
    }

    // Display Current Combo Key
    PrintKey(16, 14, 94, GUIComboKey);

    // Buttons
    DrawGUIButton(16, 202, 20, 246, 31, GUIComboTextA, 60, -1, 0);
    DrawGUIButton(16, 202, 35, 246, 46, GUIComboTextB, 61, -1, 0);
    DrawGUIButton(16, 202, 50, 246, 61, GUIComboTextC, 62, -1, 0);
    DrawGUIButton(16, 202, 65, 246, 76, GUIComboTextD, 63, -1, 0);

    // Radio Switches
    GUIDisplayButtonHole(16, 158, 82, &GUIComboPNum, 0);
    GUIDisplayButtonHole(16, 176, 82, &GUIComboPNum, 1);
    GUIDisplayButtonHole(16, 194, 82, &GUIComboPNum, 2);
    GUIDisplayButtonHole(16, 212, 82, &GUIComboPNum, 3);
    GUIDisplayButtonHole(16, 230, 82, &GUIComboPNum, 4);
    // Checkboxes
    GUIDisplayCheckbox(16, 163, 88, &GUIComboLHorz, "\xFE = LAST \xFD/\xFE");
    GUIDisplayCheckbox(16, 163, 96, &GUIComboGameSpec, "GAME SPECFIC");

    // Draw Combination Keys (Each 15x11 -> 210x36)
    u1 eax = GUINumCombo;
    if (eax != 0) {
        u4 ebx = 0;
        u4 ecx = 11;
        u4 edx = 112;
        do {
            u1 al = GUIComboData[ebx++] - 1;
            u1 const* esi;
            if (al < 12) {
                static u1 const GUIIconDataComboPressRelease[] = {
                    0, 166, 0, 0, 166, 0, 0, 0, 0, 0,
                    0, 166, 0, 166, 166, 166, 0, 0, 0, 0,
                    0, 166, 0, 0, 166, 0, 0, 0, 0, 0,
                    166, 166, 166, 0, 166, 0, 0, 0, 0, 0,
                    0, 166, 0, 0, 166, 0, 0, 0, 0, 0,
                    0, 234, 234, 234, 234, 0, 0, 0, 0, 0,
                    234, 234, 234, 234, 234, 234, 0, 0, 0, 0,
                    234, 234, 234, 234, 234, 234, 0, 0, 0, 0,
                    235, 234, 234, 234, 234, 235, 0, 0, 0, 0,
                    0, 235, 235, 235, 235, 0, 0, 0, 0, 0
                };
                esi = GUIIconDataComboPressRelease;
            } else if ((al -= 12) < 12) {
                static u1 const GUIIconDataComboPress[] = {
                    0, 0, 166, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 166, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 166, 0, 0, 0, 0, 0, 0, 0,
                    0, 166, 166, 166, 0, 0, 0, 0, 0, 0,
                    0, 0, 166, 0, 0, 0, 0, 0, 0, 0,
                    0, 236, 236, 236, 236, 0, 0, 0, 0, 0,
                    236, 236, 236, 236, 236, 236, 0, 0, 0, 0,
                    236, 236, 236, 236, 236, 236, 0, 0, 0, 0,
                    237, 236, 236, 236, 236, 237, 0, 0, 0, 0,
                    0, 237, 237, 237, 237, 0, 0, 0, 0, 0
                };
                esi = GUIIconDataComboPress;
            } else if ((al -= 12) < 12) {
                static u1 const GUIIconDataComboRelease[] = {
                    0, 0, 0, 166, 0, 0, 0, 0, 0, 0,
                    0, 0, 166, 166, 166, 0, 0, 0, 0, 0,
                    0, 0, 0, 166, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 166, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 166, 0, 0, 0, 0, 0, 0,
                    0, 238, 238, 238, 238, 0, 0, 0, 0, 0,
                    238, 238, 238, 238, 238, 238, 0, 0, 0, 0,
                    238, 238, 238, 238, 238, 238, 0, 0, 0, 0,
                    239, 238, 238, 238, 238, 239, 0, 0, 0, 0,
                    0, 239, 239, 239, 239, 0, 0, 0, 0, 0
                };
                esi = GUIIconDataComboRelease;
            } else {
                static u1 const GUIIconDataComboFrame[] = {
                    0, 0, 0, 0, 58, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 56, 46, 0, 0, 0, 0,
                    0, 0, 0, 0, 54, 44, 0, 0, 0, 0,
                    0, 0, 0, 52, 0, 42, 0, 0, 0, 0,
                    0, 0, 0, 50, 40, 0, 0, 0, 0, 0,
                    0, 0, 0, 48, 38, 0, 0, 0, 0, 0,
                    0, 0, 0, 46, 36, 0, 0, 0, 0, 0,
                    0, 0, 44, 0, 34, 0, 0, 0, 0, 0,
                    0, 0, 42, 32, 0, 0, 0, 0, 0, 0,
                    0, 0, 40, 32, 0, 0, 0, 0, 0, 0
                };
                esi = GUIIconDataComboFrame;
                if (al == 17) {
                    al = 20;
                } else if (al >= 18) {
                    al -= 7;
                    static u1 const GUIIconDataComboSecond[] = {
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 51, 45, 0, 0, 0, 0, 0, 0,
                        0, 0, 51, 45, 0, 0, 0, 0, 0, 0,
                        0, 54, 52, 48, 46, 0, 0, 0, 0, 0,
                        53, 46, 50, 50, 48, 45, 0, 0, 0, 0,
                        52, 50, 45, 48, 46, 44, 0, 0, 0, 0,
                        51, 50, 50, 46, 50, 43, 0, 0, 0, 0,
                        50, 50, 50, 50, 50, 42, 0, 0, 0, 0,
                        0, 48, 46, 44, 42, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0
                    };
                    esi = GUIIconDataComboSecond;
                    if (al == 11)
                        al = 250 - 37;
                }
            }
            if (al > 11)
                al += 37;
            switch (al) {
            case 0:
                al = '\xFB';
                break;
            case 1:
                al = '\xFC';
                break;
            case 2:
                al = '\xFD';
                break;
            case 3:
                al = '\xFE';
                break;
            case 4:
                al = 'A';
                break;
            case 5:
                al = 'B';
                break;
            case 6:
                al = 'X';
                break;
            case 7:
                al = 'Y';
                break;
            case 8:
                al = 'L';
                break;
            case 9:
                al = 'R';
                break;
            case 10:
                al = 'T';
                break;
            case 11:
                al = 'E';
                break;
            }
            static char GUIComboText3[] = " ";
            GUIComboText3[0] = al;

            GUIDisplayTextG(16, ecx + 8, edx + 5, GUIComboText3);

            GUIDisplayIconWin(16, ecx, edx, esi);
            ecx += 15;
            if (ecx == 11 + 15 * 14) {
                ecx -= 15 * 14;
                edx += 11;
            }
        } while (--eax != 0);
    }

    // Display Bordered Box
    if (NumCombo != 0) {
        DrawGUIWinBox2(16, 10, 190, 7, 224, 23 + (GUIccombcursloc - GUIccombviewloc) * 7);
    }

    // Display Scroll Lines
    // Copy Description to GUIScrolBufA, Others to GUIScrolBufB
    s4 ebx = NumCombo - GUIccombviewloc;
    if (ebx > 0) {
        if (ebx > 8)
            ebx = 8;
        ComboData const* edi = (GUIComboGameSpec == 0 ? CombinDataGlob : CombinDataLocl) + GUIccombviewloc;
        u4 ecx = 12;
        u4 eax = 25;
        do {
            static char GUIScrolBufB[9];
            sprintf(GUIScrolBufB, "%.3s %c  %c", ScanCodeListing + edi->key * 3, '1' + edi->player, edi->ff != 0 ? 'Y' : 'N');
            static char GUIScrolBufA[21];
            memcpy(GUIScrolBufA, edi->name, sizeof(edi->name));

            GUIDisplayTextG(16, ecx, eax, GUIScrolBufA);
            GUIDisplayTextG(16, ecx + 128, eax, GUIScrolBufB);
        } while (eax += 7, ++edi, --ebx != 0);
    }
}

static bool EEMode(void)
{
    return GetTime() <= 360;
}

void DisplayGUIAddOns(void)
{
    if (EEMode()) { // You know it!
        GUIDrawWindowBox(17, "TOASTER OPTIONS");

        GUIDisplayText(17, 20, 15, "SLOT 1:");
        GUIDisplayText(17, 112, 15, "SLOT 2:");

        char const* const GUIAddOnText3Alt = "OFF";
        char const* const GUIAddOnText4Alt = "WHITE BREAD";
        GUIDisplayButtonHoleTu(17, 9, 23, &device1, 0, GUIAddOnText3Alt, 0);
        GUIDisplayButtonHoleTu(17, 9, 33, &device1, 1, GUIAddOnText4Alt, 0);

        GUIDisplayButtonHoleTu(17, 100, 23, &device2, 0, GUIAddOnText3Alt, 1);
        GUIDisplayButtonHoleTu(17, 100, 33, &device2, 1, GUIAddOnText4Alt, 1);
        GUIDisplayButtonHoleTu(17, 100, 43, &device2, 2, "RYE BREAD", 0);
        GUIDisplayButtonHoleTu(17, 100, 53, &device2, 3, "WHEAT BREAD", 0);
        GUIDisplayButtonHoleTu(17, 100, 63, &device2, 4, "ENGLISH MUFFIN", 0);

        GUIDisplayText(17, 12, 78, "POP TART");
        GUIDisplayText(17, 103, 78, "WAFFLE");

        char const* const GUIAddOnTextDAlt = "QUICK TOAST";
        GUIDisplayCheckboxu(17, 9, 83, &mouse1lh, GUIAddOnTextDAlt, 0);
        GUIDisplayCheckboxu(17, 100, 83, &mouse2lh, GUIAddOnTextDAlt, 1);

        GUIDisplayTextY(17, 12, 100, "COFFEE?");
        GUIDisplayText(17, 12, 110, "CREAM");
        GUIDisplayText(17, 103, 110, "SUGAR");
    } else { // Regular
        GUIDrawWindowBox(17, "DEVICES SELECTOR");

        GUIDisplayTextY(17, 20, 15, "PORT 1:");
        GUIDisplayTextY(17, 112, 15, "PORT 2:");

        char const* const GUIAddOnText3 = "GAMEPAD";
        char const* const GUIAddOnText4 = "MOUSE";
        GUIDisplayButtonHoleTu(17, 9, 23, &device1, 0, GUIAddOnText3, 0);
        GUIDisplayButtonHoleTu(17, 9, 33, &device1, 1, GUIAddOnText4, 0);

        GUIDisplayButtonHoleTu(17, 100, 23, &device2, 0, GUIAddOnText3, 1);
        GUIDisplayButtonHoleTu(17, 100, 33, &device2, 1, GUIAddOnText4, 1);
        GUIDisplayButtonHoleTu(17, 100, 43, &device2, 2, "SUPER SCOPE", 0);
        GUIDisplayButtonHoleTu(17, 100, 53, &device2, 3, "1 JUSTIFIER", 0);
        GUIDisplayButtonHoleTu(17, 100, 63, &device2, 4, "2 JUSTIFIERS", 0);

        GUIDisplayText(17, 12, 78, "CYCLE P1:");
        GUIDisplayText(17, 103, 78, "CYCLE P2:");

        char const* const GUIAddOnTextD = "LEFT HANDED";
        GUIDisplayCheckboxu(17, 9, 83, &mouse1lh, GUIAddOnTextD, 0);
        GUIDisplayCheckboxu(17, 100, 83, &mouse2lh, GUIAddOnTextD, 1);

        GUIDisplayTextY(17, 12, 100, "SUPER SCOPE KEYS:");
        GUIDisplayText(17, 12, 110, "AUTO-FIRE");
        GUIDisplayText(17, 103, 110, "SS PAUSE");
    }

    DDrawBox(17, 73, 75, &KeyExtraEnab1);
    DDrawBox(17, 164, 75, &KeyExtraEnab2);

    DDrawBox(17, 73, 107, &SSAutoFire);
    DDrawBox(17, 164, 107, &SSPause);
}

void DisplayGUIChipConfig(void)
{
    GUIDrawWindowBox(18, "CONFIGURE CHIPS");

    if (EEMode()) { // You know it!
        GUIDisplayText(18, 10, 38, "EXTRAS");
        GUIDisplayCheckbox(18, 9, 43, &nssdip1, "SALT");
        GUIDisplayCheckbox(18, 59, 43, &nssdip2, "GARLIC");
        GUIDisplayCheckbox(18, 109, 43, &nssdip3, "PEPPER");
        GUIDisplayCheckbox(18, 9, 53, &nssdip4, "CHEESE");
        GUIDisplayCheckbox(18, 59, 53, &nssdip5, "BUTTER");
        GUIDisplayCheckbox(18, 109, 53, &nssdip6, "CATSUP");
    } else { // Regular Shadow
        GUIDisplayTextY(18, 10, 38, "SUPER SYSTEM:");
        GUIDisplayCheckboxu(18, 9, 43, &nssdip1, "DIP 1", 4); // Checkboxes
        GUIDisplayCheckboxu(18, 59, 43, &nssdip2, "DIP 2", 4);
        GUIDisplayCheckboxu(18, 109, 43, &nssdip3, "DIP 3", 4);
        GUIDisplayCheckboxu(18, 9, 53, &nssdip4, "DIP 4", 4);
        GUIDisplayCheckboxu(18, 59, 53, &nssdip5, "DIP 5", 4);
        GUIDisplayCheckboxu(18, 109, 53, &nssdip6, "DIP 6", 4);
    }
}

void DisplayGUIPaths(void)
{
    GUIDrawWindowBox(19, "SETUP PATHS");
    u4 eax;
    u4 ebx;
    GUIDrawTArea(19, &eax, &ebx);
    GUIDrawTabs(GUIPathTabs, &eax, ebx);

    if (GUIPathTabs[0] == 1) { // General
        GUIDisplayText(19, 8, 31, "SAVES:"); // Text
        GUIDisplayText(19, 8, 66, "SAVESTATES:");
        GUIDisplayText(19, 8, 101, "MOVIES:");
        GUIDisplayText(19, 8, 136, "IPS:");
        GUIDisplayText(19, 8, 171, "RELATIVE PATH BASE:");

        GUIDisplayBBox(19, 8, 41, 236, 51, 167); // Input boxes
        GUIDisplayBBox(19, 8, 76, 236, 86, 167);
        GUIDisplayBBox(19, 8, 111, 236, 121, 167);
        GUIDisplayBBox(19, 8, 146, 236, 156, 167);
        // Green Text
        GUIOuttextwin2d(19, 10, 45, SRAMPath, 37, GUIPathsTab1Ptr, 0);
        GUIOuttextwin2d(19, 10, 80, SStatePath, 37, GUIPathsTab1Ptr, 1);
        GUIOuttextwin2d(19, 10, 115, MoviePath, 37, GUIPathsTab1Ptr, 2);
        GUIOuttextwin2d(19, 10, 150, IPSPath, 37, GUIPathsTab1Ptr, 3);

        // Display Radio buttons
        char const* const GUIPathsTextA5A = "CONFIG DIR";
        char const* const GUIPathsTextA5B = "ROM DIR";
        GUIDisplayText(19, 23, 181, GUIPathsTextA5A);
        GUIDisplayText(19, 103, 181, GUIPathsTextA5B);

        GUIDisplayButtonHole(19, 8, 178, &RelPathBase, 0);
        GUIDisplayButtonHole(19, 88, 178, &RelPathBase, 1);

        if (GUIInputBox == 0) {
            GUIDisplayButtonHoleTu(19, 8, 178, &RelPathBase, 0, GUIPathsTextA5A, 0);
            GUIDisplayButtonHoleTu(19, 88, 178, &RelPathBase, 1, GUIPathsTextA5B, 0);
        }
    }

    if (GUIPathTabs[0] == 2) { // More paths
        GUIDisplayText(19, 8, 31, "SNAPSHOTS:");
        GUIDisplayText(19, 8, 66, "SPCS:");
        GUIDisplayText(19, 8, 101, "CHEATS:");
        GUIDisplayText(19, 8, 136, "COMBOS:");
        GUIDisplayText(19, 8, 171, "GAME SPECIFIC INPUT:");

        GUIDisplayBBox(19, 8, 41, 236, 51, 167);
        GUIDisplayBBox(19, 8, 76, 236, 86, 167);
        GUIDisplayBBox(19, 8, 111, 236, 121, 167);
        GUIDisplayBBox(19, 8, 146, 236, 156, 167);
        GUIDisplayBBox(19, 8, 181, 236, 191, 167);

        GUIOuttextwin2d(19, 10, 45, SnapPath, 37, GUIPathsTab2Ptr, 0);
        GUIOuttextwin2d(19, 10, 80, SPCPath, 37, GUIPathsTab2Ptr, 1);
        GUIOuttextwin2d(19, 10, 115, CHTPath, 37, GUIPathsTab2Ptr, 2);
        GUIOuttextwin2d(19, 10, 150, ComboPath, 37, GUIPathsTab2Ptr, 3);
        GUIOuttextwin2d(19, 10, 185, INPPath, 37, GUIPathsTab2Ptr, 4);
    }

    if (GUIPathTabs[0] == 3) { // bc
        GUIDisplayText(19, 8, 31, "BS-X:");
        GUIDisplayText(19, 8, 66, "SUFAMI TURBO:");
        GUIDisplayText(19, 8, 101, "SD GUNDAM G-NEXT:");
        GUIDisplayText(19, 8, 136, "SAME GAME:");

        GUIDisplayBBox(19, 8, 41, 236, 51, 167);
        GUIDisplayBBox(19, 8, 76, 236, 86, 167);
        GUIDisplayBBox(19, 8, 111, 236, 121, 167);
        GUIDisplayBBox(19, 8, 146, 236, 156, 167);

        GUIOuttextwin2d(19, 10, 45, BSXPath, 37, GUIPathsTab3Ptr, 0);
        GUIOuttextwin2d(19, 10, 80, STPath, 37, GUIPathsTab3Ptr, 1);
        GUIOuttextwin2d(19, 10, 115, GNextPath, 37, GUIPathsTab3Ptr, 2);
        GUIOuttextwin2d(19, 10, 150, SGPath, 37, GUIPathsTab3Ptr, 3);
    }
}

void DisplayGUISave(void)
{
    GUIDrawWindowBox(20, "SAVE OPTIONS");

    GUIDisplayText(20, 8, 19, "# OF REWIND STATES"); // Text
    GUIDisplayText(20, 8, 31, "1/5 SECONDS PER REWIND");
    GUIDisplayText(20, 9, 159, "SAVE");
    GUIDisplayText(20, 9 + 57, 159, "LOAD");
    GUIDisplayText(20, 9 + 114, 159, "PICK");
    GUIDisplayText(20, 9, 168, "REWIND");

    GUIDisplayTextY(20, 6, 123, "STATE SHORTCUTS:");
    GUIDisplayText(20, 9, 132, "ST0");
    GUIDisplayText(20, 9 + 45, 132, "ST1");
    GUIDisplayText(20, 9 + 45 * 2, 132, "ST2");
    GUIDisplayText(20, 9 + 45 * 3, 132, "ST3");
    GUIDisplayText(20, 9, 141, "ST4");
    GUIDisplayText(20, 9 + 45, 141, "ST5");
    GUIDisplayText(20, 9 + 45 * 2, 141, "ST6");
    GUIDisplayText(20, 9 + 45 * 3, 141, "ST7");
    GUIDisplayText(20, 9, 150, "ST8");
    GUIDisplayText(20, 9 + 45, 150, "ST9");
    GUIDisplayText(20, 9 + 45 * 2, 150, "ST+");
    GUIDisplayText(20, 9 + 45 * 3, 150, "ST-");

    GUIDisplayCheckboxu(20, 11, 38, &nosaveSRAM, "DO NOT SAVE SRAM", 0);
    if (nosaveSRAM == 0) {
        GUIDisplayCheckboxu(20, 11, 48, &SRAMSave5Sec, "SRAM CHECK+SAVE", 5); // Checkboxes
    }
    GUIDisplayCheckboxu(20, 11, 58, &SRAMState, "LOAD SAVESTATE W/SRAM", 0);
    GUIDisplayCheckboxu(20, 11, 68, &LatestSave, "START AT LATEST SAVE", 0);
    GUIDisplayCheckboxu(20, 11, 78, &AutoIncSaveSlot, "AUTO INCREMENT SAVE SLOT", 5);
    GUIDisplayCheckboxu(20, 11, 88, &AutoState, "AUTO STATE SAVE/LOAD", 0);
    GUIDisplayCheckboxu(20, 11, 98, &PauseLoad, "PAUSE AFTER LOADING STATE", 0);
    GUIDisplayCheckboxu(20, 11, 108, &PauseRewind, "PAUSE AFTER REWIND", 12);

    char GUISaveTextZ3[3];

    GUIDisplayBBox(20, 150, 17, 165, 24, 167); // Rewind States Box
    sprintf(GUISaveTextZ3, "%02u", RewindStates);
    GUIDisplayTextG(20, 154, 19, GUISaveTextZ3);

    GUIDisplayBBox(20, 150, 29, 165, 36, 167); // Second/Rewind Box
    sprintf(GUISaveTextZ3, "%02u", RewindFrames);
    GUIDisplayTextG(20, 154, 31, GUISaveTextZ3);

    DDrawBox(20, 26, 129, &KeyStateSlc0); // Boxes for State section
    DDrawBox(20, 71, 129, &KeyStateSlc1);
    DDrawBox(20, 116, 129, &KeyStateSlc2);
    DDrawBox(20, 161, 129, &KeyStateSlc3);
    DDrawBox(20, 26, 138, &KeyStateSlc4);
    DDrawBox(20, 71, 138, &KeyStateSlc5);
    DDrawBox(20, 116, 138, &KeyStateSlc6);
    DDrawBox(20, 161, 138, &KeyStateSlc7);
    DDrawBox(20, 26, 147, &KeyStateSlc8);
    DDrawBox(20, 71, 147, &KeyStateSlc9);
    DDrawBox(20, 116, 147, &KeyIncStateSlot);
    DDrawBox(20, 161, 147, &KeyDecStateSlot);
    DDrawBox(20, 32, 156, &KeySaveState);
    DDrawBox(20, 89, 156, &KeyLoadState);
    DDrawBox(20, 146, 156, &KeyStateSelct);
    DDrawBox(20, 45, 165, &KeyRewind);

    DrawGUIButton(20, 173, 17, 181, 25, "+", 70, -2, -1); // + Rewind States
    DrawGUIButton(20, 184, 17, 192, 25, "-", 71, -2, -1); // - Rewind States
    DrawGUIButton(20, 173, 29, 181, 37, "+", 72, -2, -1); // + Second/Rewind
    DrawGUIButton(20, 184, 29, 192, 37, "-", 73, -2, -1); // - Second/Rewind
}

static u1 SpdslidSet(void const* const p1) // slider var
{
    return *(u1 const*)p1 * 2;
}

static char const* SpdslidText(void const* const p1) // slider var, text
{
    static char GUISpeedTextD1[4];
    u4 const al = *(u1 const*)p1; // currently emuspeed ranges from 0 to 58
    if (al >= 29) // this will turn it into '/30' to '30x'
    { // ff
        sprintf(GUISpeedTextD1, "%ux", al - 28);
    } else { // slomo
        sprintf(GUISpeedTextD1, "/%u", 30 - al);
    }
    return GUISpeedTextD1;
}

void DisplayGUISpeed(void)
{
    GUIDrawWindowBox(21, "SPEED OPTIONS");

    if (frameskip == 0) {
        GUIDisplayText(21, 6, 15, "MAX FRAME SKIP");
        GUIDisplayText(21, 40, 79, "+ EMU SPEED");
        GUIDisplayText(21, 40, 99, "- EMU SPEED");
        GUIDisplayTextY(21, 8, 164, "EMU SPEED:");
    } else {
        GUIDisplayText(21, 6, 15, "FRAME RATE");
        GUIDisplayText(21, 40, 79, "+ FRAME RATE");
        GUIDisplayText(21, 40, 99, "- FRAME RATE");
    }
    GUIDisplayText(21, 6, 26, "FASTFWD RATIO x");
    GUIDisplayText(21, 6, 37, "SLOWDWN RATIO /");

    GUIDisplayTextY(21, 8, 49, "SHORTCUTS:");
    GUIDisplayText(21, 40, 59, "FAST FORWARD");
    GUIDisplayText(21, 40, 69, "SLOW DOWN");
    GUIDisplayText(21, 40, 89, "RESET SPEED");
    GUIDisplayText(21, 40, 109, "PAUSE GAME");
    GUIDisplayText(21, 40, 119, "INCR FRAME");

    if (frameskip == 0) // Shortcut Boxes
    {
        DDrawBox(21, 10, 76, &KeyEmuSpeedUp);
        DDrawBox(21, 10, 96, &KeyEmuSpeedDown);
    } else {
        DDrawBox(21, 10, 76, &KeyFRateUp);
        DDrawBox(21, 10, 96, &KeyFRateDown);
    }
    DDrawBox(21, 10, 56, &KeyFastFrwrd);
    DDrawBox(21, 10, 66, &KeySlowDown);
    DDrawBox(21, 10, 86, &KeyResetSpeed);
    DDrawBox(21, 10, 106, &EMUPauseKey);
    DDrawBox(21, 10, 116, &INCRFrameKey);

    GUIDisplayCheckboxu(21, 11, 135, &FastFwdToggle, "TOGGLED FFWD/SLWDWN", 0);
    GUIDisplayCheckboxun(21, 11, 145, &frameskip, 0, "AUTO FRAME RATE", 0);

    char GUISpeedTextZ3[3];

    GUIDisplayBBox(21, 96, 24, 114, 31, 167); // FF Ratio Box
    sprintf(GUISpeedTextZ3, "%2u", FFRatio + 2);
    GUIDisplayTextG(21, 101, 26, GUISpeedTextZ3);

    GUIDisplayBBox(21, 96, 35, 114, 42, 167); // SD Ratio Box
    sprintf(GUISpeedTextZ3, "%2u", SDRatio + 2);
    GUIDisplayTextG(21, 101, 37, GUISpeedTextZ3);

    DrawGUIButton(21, 118, 24, 126, 32, "+", 74, -2, -1); // + Rewind States
    DrawGUIButton(21, 129, 24, 137, 32, "-", 75, -2, -1); // - Rewind States
    DrawGUIButton(21, 118, 35, 126, 43, "+", 76, -2, -1); // + Second/Rewind
    DrawGUIButton(21, 129, 35, 137, 43, "-", 77, -2, -1); // - Second/Rewind

    if (frameskip == 0) {
        GUIDrawSlider(21, 7, 116, 175, &EmuSpeed, SpdslidSet, SpdslidText);
    }

    GUIDisplayBBox(21, 96, 13, 114, 20, 167);
    static char GUISpeedTextX[] = "-";
    if (frameskip != 0) // Determine if AutoFR is enabled
    { // Non AFR FrameRate +/- Box
        GUISpeedTextX[0] = '0' + (frameskip - 1);
    } else { // AFR Max Frameskip +/- Box
        GUISpeedTextX[0] = '0' + maxskip;
    }
    GUIDisplayTextG(21, 107, 15, GUISpeedTextX);
    DrawGUIButton(21, 118, 13, 126, 21, "+", 78, -2, -1);
    DrawGUIButton(21, 129, 13, 137, 21, "-", 79, -2, -1);
}
