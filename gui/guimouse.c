/*
 * Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )
 *
 * http://www.zsnes.com
 * http://sourceforge.net/projects/zsnes
 * https://zsnes.bountysource.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdarg.h>
#include <string.h>

#include "../asm_call.h"
#include "../c_init.h"
#include "../c_intrf.h"
#include "../cfg.h"
#include "../cpu/dsp.h"
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
#include "c_guiwindp.h"
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
#include "../win/winlink.h"
#endif

u1 GUIcwinpress;
u1 lastmouseholded;

static u1 LastHoldEnable;
static u1 MouseMoveOkay;
static u1 ntscCurVar;
static u1 ntscLastVar[6];
static u1 ntscWhVar;
static u2 mousebuttonstat;
static u4 GUIBlankVar;
static u4 GUINumValue;
static u4 GUIdispmode;
static u4 GUIlastcpos;
static u4 GUIlastdispval;
static u4 GUIlastvpos;
static u4 GUIlastypos;

static char const guipresstext1[] = "ENTER THE KEY";
static char const guipresstext2[] = "OR BUTTON TO USE";

static void GUIProcVideo(void)
{
    GUICBHold = 0;
    newengen = 1;
    guiprevideo();
    u1 const prevvid = cvidmode;
    cvidmode = GUIcurrentvideocursloc;
#ifdef __MSDOS__
    ExitFromGUI = 1;
#endif
    initvideo();
#ifdef __MSDOS__
    if (videotroub == 1) {
        videotroub = 0;
        cvidmode = prevvid;
        initvideo();
        GUISetPal();
        guipostvideofail();
    } else
#endif
    {
        GUISetPal();
        guipostvideo();
        if (GUIkeydelay == 0) {
            videotroub = 0;
            cvidmode = prevvid;
            initvideo();
            GUISetPal();
            vidpastecopyscr();
        }
    }
    GUIkeydelay = 0;
}

static void GUINTSCReset(void)
{
    if (GUICBHold != 38) {
        NTSCBlend = 0;
        NTSCRef = 0;
        NTSCHue = 0;
        NTSCSat = 0;
        NTSCCont = 0;
        NTSCBright = 0;
        NTSCSharp = 0;
    }
    if (GUICBHold != 37) {
        NTSCGamma = 0;
        NTSCRes = 0;
        NTSCArt = 0;
        NTSCFringe = 0;
        NTSCBleed = 0;
        NTSCWarp = 0;
    }
    GUICBHold = 0;
#ifndef __MSDOS__
    NTSCFilterInit();
#endif
}

static void GUINTSCPreset(void)
{
    switch (GUICBHold) {
    case 81:
        NTSCPresetVar = 0;
        break;
    case 82:
        NTSCPresetVar = 1;
        break;
    case 83:
        NTSCPresetVar = 2;
        break;
    case 84:
        NTSCPresetVar = 3;
        break;
    }
#ifndef __MSDOS__
    NTSCFilterInit();
#endif
    NTSCPresetVar = 4;
    GUICBHold = 0;
}

#ifndef __MSDOS__
static void GUIProcCustomVideo(void)
{
    SetCustomXY();
    GUICBHold = 0;
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
    if (GUIWFVID[cvidmode] != 0) {
        cvidmode = PrevWinMode;
        initvideo();
    } else
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
    return p1 <= eax && eax <= p3 && p2 <= edx && edx <= p4;
}

static void GUIClickCButton(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u1* const p3)
{
    if (GUIClickArea(eax, edx, p1 + 1, p2 + 3, p1 + 6, p2 + 8))
        *p3 ^= 1;
}

static void GUIClickCButtonC(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u1* const p3)
{
    if (GUIClickArea(eax, edx, p1 + 1, p2 + 3, p1 + 6, p2 + 8)) {
        *p3 ^= 1;
        GUIccombviewloc = 0;
        GUIccombcursloc = 0;
        GUIccomblcursloc = 0;
    }
}

#ifdef __WIN32__
static void GUIClickCButtonf(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u1* const p3, void (*const p4)(void))
#else
static void GUIClickCButtonf(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u1* const p3)
#define GUIClickCButtonf(eax, edx, p1, p2, p3, p4) GUIClickCButtonf((eax), (edx), (p1), (p2), (p3))
#endif
{
    if (GUIClickArea(eax, edx, p1 + 1, p2 + 3, p1 + 6, p2 + 8)) {
        *p3 ^= 1;
#ifdef __WIN32__
        p4();
#endif
    }
}

static void GUIClickCButtonK(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u1* const p3, void (*const p4)(void))
{
    if (GUIClickArea(eax, edx, p1 + 1, p2 + 3, p1 + 6, p2 + 8)) {
        *p3 ^= 1;
        p4();
    }
}

static void GUIClickCButtonN(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u1* const p3, void (*const p4)(void))
{
    if (GUIClickArea(eax, edx, p1 + 1, p2 + 3, p1 + 6, p2 + 8)) {
#ifdef __WIN32__
        if (*p3 != 1)
            Keep4_3Ratio = 1;
#endif
        *p3 ^= 1;
#ifdef __WIN32__
        p4();
#endif
    }
}

static void GUIClickCButtonM(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u1* const p3)
{
    if (GUIClickArea(eax, edx, p1 + 1, p2 + 3, p1 + 6, p2 + 8)) {
        *p3 ^= 1;
        MultiTap = pl12s34 != 1 && (pl3contrl != 0 || pl4contrl != 0 || pl5contrl != 0);
    }
}

#ifdef __MSDOS__

static void GUIClickCButtonID(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u1* const p3)
{
    if (GUIClickArea(eax, edx, p1 + 1, p2 + 3, p1 + 6, p2 + 8)) {
        *p3 ^= 1 SetDevice();
    }
}

#endif

static bool GUIClickCButton5(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u1* const p3, u1 const p4)
{
    if (GUIClickArea(eax, edx, p1 + 1, p2 + 3, p1 + 6, p2 + 8)) {
        *p3 = *p3 == p4 ? *p3 ^ p4 : p4;
        return true;
    }
    return false;
}

static bool GUIClickCButton6(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u1* const p3, u1 const p4)
{
    if (GUIClickArea(eax, edx, p1 + 1, p2 + 3, p1 + 6, p2 + 8)) {
        if (*p3 == p4) {
            *p3 = 0;
        } else {
            *p3 = p4;
            memset(vidbufferofsb, 0, 288 * 128 * 4);
        }
        return true;
    }

    return false;
}

static void GUIClickCButtonfr(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u1* const p3)
{
    if (GUIClickArea(eax, edx, p1 + 1, p2 + 3, p1 + 6, p2 + 8)) {
        FPSOn = 0;
        *p3 = !*p3;
    }
}

static bool GUIClickCButtonL(s4 const eax, s4 const edx, s4 const p1, s4 const p2)
{
    if (GUIClickArea(eax, edx, p1 + 1, p2 + 3, p1 + 6, p2 + 8)) {
        showallext ^= 1;
        GUIcurrentfilewin = 0;
        GetLoadData();
        return true;
    }
    return false;
}

static void GUIClickCButtonI(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u1* const p3)
{
    if (GUIClickArea(eax, edx, p1 + 1, p2 + 3, p1 + 6, p2 + 8)) {
        *p3 ^= 1;
#ifndef __MSDOS__
        if (GUIBIFIL[cvidmode] != 0) {
#ifdef __WIN32__
            initDirectDraw();
#elif defined __OPENGL__
            initwinvideo();
#endif
        }
#endif
        Clear2xSaIBuffer();
    }
}

#ifdef __MSDOS__

static void GUIClickCButtonT(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u1* const p3, u1* const p4)
{
    if (GUIClickArea(eax, edx, p1 + 1, p2 + 3, p1 + 6, p2 + 8)) {
        *p3 ^= 1;
        *p4 = 0;
    }
}

#endif

static bool GUISlidebarPostImpl(s4 const eax, s4 const edx, u4 const p1, s4 const p2, s4 const p3, s4 const p4, u1 const p7, u4 const p8, u4* const p9, u4* const p10, u4 const* const p11, u4* const p12, u4 const p13, s4 const p14, s4 const p15, s4 const p16, s4 const p17, void (*const p23)(s4 eax, s4 edx)) // p1-p13: x1,y1,x2,y2,upjump,downjump,holdpos,scsize,view,cur,listsize, p14-p24: x1,y1,x2,y2,view,curs,num,.scru,.scrd,jumpto,sizeofscreen
{
    if (*p11 == 0)
        return false;

    if (GUIdispmode != 1) {
        if (GUIClickArea(eax, edx, p1, p2 - 7, p3, p2 - 1)) {
            *p12 = p13;
            GUICHold = p7;
            goto scrollup;
        }
        if (GUIClickArea(eax, edx, p1, p4 + 1, p3, p4 + 7)) {
            *p12 = p13;
            GUICHold = p7 + 1;
            goto scrolldown;
        }
    } else if (GUIClickArea(eax, edx, p1, p2, p3, p4)) // slidebar
    {
        *p12 = p13;
        // displacement = (GUIdispmode * pixeldisp. / (listsize-scsize))
        s8 const edxeax = (s8)(s4)(*p11 - p8) * (s8)(edx - GUIlastypos);
        u4 const ebx = GUIlastdispval;
        if (ebx != 0 && !(ebx & 0x80000000)) {
            u4 const eax = edxeax / ebx;
            *p9 = GUIlastvpos + eax;
            *p10 = GUIlastcpos + eax;
            if (*p9 & 0x8000000)
                *p9 = 0; // XXX probably should be 0x80000000
            if (*p10 & 0x8000000)
                *p10 = 0; // XXX probably should be 0x80000000
            u4 const eax_ = *p11;
            if (*p10 >= eax_ - 1)
                *p10 = eax_ - 1;
            if (*p9 >= eax_ - p8)
                *p9 = eax_ - p8;
        }
        return true;
    }

    if (p14 <= eax && eax <= p16) {
        if (edx == p15) { // Scroll Up
            *p10 = *p9;
        scrollup:
            if (GUIScrolTim1 != 0)
                goto donescrol;
            if (*p10 != 0) {
                --*p10;
                if (*p9 != 0)
                    --*p9;
            }
        } else if (edx == p17) { // Scroll Down
            if (*p11 > p8)
                *p10 = *p9 + p8 - 1;
        scrolldown:
            if (GUIScrolTim1 != 0)
                goto donescrol;
            if (*p11 - 1 > *p10) {
                ++*p10;
                if (*p11 <= p8)
                    goto donescrol;
                u4 const ebx = *p11 - p8;
                if (++*p9 >= ebx)
                    *p9 = ebx;
            }
        } else {
            return false;
        }

        GUIScrolTim1 = 1;
        if (GUIScrolTim2 >= 4)
            ++GUIScrolTim1;
        if (GUIScrolTim2 != 0) {
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

    if (GUIClickArea(eax, edx, p1, p2, p3, p4)) {
        if (p5[1] > (u4)(edx - p2)) {
            *p7 -= p6;
            *p8 -= p6;
            if (*p7 & 0x8000000) // XXX probably should be 0x80000000
            {
                *p7 = 0;
                *p8 = 0;
            }
        } else if (p5[2] < (u4)(edx - p2)) {
            *p7 += p6;
            *p8 += p6;
            u4 const ebx = *p9 - 1;
            if (*p8 >= ebx) {
                *p8 = ebx;
                *p7 = ebx - p6 + 1;
            }
            u4 const ebx_ = *p9 - p6;
            if (*p7 >= ebx_)
                *p7 = ebx_;
        } else {
            GUIlastypos = edx;
            GUIdispmode = 1;
            GUIHoldYlim = GUIwinposy[p10] + p2;
            GUIHoldYlimR = GUIwinposy[p10] + p4;
            GUIHoldXlimL = GUIwinposx[p10] + p1;
            GUIHoldXlimR = GUIwinposx[p10] + p3;
            GUIlastdispval = *p5;
            GUIlastcpos = *p8;
            GUIlastvpos = *p7;
            GUIHold = 3;
        }
        return true;
    }

    // upper arrow
    if (GUIClickArea(eax, edx, p1, p2 - 7, p3, p2 - 1)) {
        GUIHoldYlim = GUIwinposy[p10] + p2 - 7;
        GUIHoldYlimR = GUIwinposy[p10] + p2 - 1;
        GUIHoldXlimL = GUIwinposx[p10] + p1;
        GUIHoldXlimR = GUIwinposx[p10] + p3;
        GUIHold = 3;
        return true;
    }

    // lower arrow
    if (GUIClickArea(eax, edx, p1, p4 + 1, p3, p4 + 7)) {
        GUIHoldYlim = GUIwinposy[p10] + p4 + 1;
        GUIHoldYlimR = GUIwinposy[p10] + p4 + 7;
        GUIHoldXlimL = GUIwinposx[p10] + p1;
        GUIHoldXlimR = GUIwinposx[p10] + p3;
        GUIHold = 3;
        return true;
    }

    return false;
}

static void GUIPHoldbutton(s4 const eax, s4 const edx, s4 const p1, s4 const p2, s4 const p3, s4 const p4, u1 const p5)
{
    if (GUIClickArea(eax, edx, p1, p2, p3, p4)) {
        GUIHoldXlimL = p1;
        GUIHoldXlimR = p3;
        GUIHoldYlim = p2;
        GUIHoldYlimR = p4;
        GUICBHold2 = p5;
        GUIHold = 4;
    }
}

static void GUIPHoldbutton2(s4 const eax, s4 const edx, s4 const p1, s4 const p2, s4 const p3, s4 const p4, u1 const p5, u1* const p6, s1 const p7, u1 const p8)
{
    if (GUIClickArea(eax, edx, p1, p2, p3, p4) && GUIHold == 0) {
        GUIHoldXlimL = p1;
        GUIHoldXlimR = p3;
        GUIHoldYlim = p2;
        GUIHoldYlimR = p4;
        GUICBHold2 = p5;
        GUIHold = 4;
        if (*p6 != p8)
            *p6 += p7;
    }
}

static void GUITextBoxInputNach(s4 const eax, s4 const edx, s4 const p1, s4 const p2, s4 const p3, s4 const p4, u4 const p5, u4 const p6, void (*const p7)(void))
{
    if (GUIClickArea(eax, edx, p1, p2, p3, p4)) {
        p7();
        GUIInputBox = p5 + 1;
        GUIInputLimit = p6 - 1;
    }
}

static void GUIPButtonHole(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u1* const p3, u1 const p4)
{
    if (GUIClickArea(eax, edx, p1 + 1, p2 + 1, p1 + 7, p2 + 7))
        *p3 = p4;
}

static void GUIPButtonHoleS(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u1* const p3, u1 const p4)
{
    if (GUIClickArea(eax, edx, p1 + 1, p2 + 1, p1 + 7, p2 + 7)) {
        *p3 = p4;
#ifdef __MSDOS__
        asm_call(DOSClearScreen);
        if (cvidmode == 2 /* Mode Q */ || cvidmode == 5 /* Mode X */) {
            cbitmode = 1;
            asm_call(initvideo2);
            cbitmode = 0;
            GUISetPal();
        }
#endif
    }
}

static void GUIPButtonHoleLoad(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u1* const p3, u1 const p4)
{
    if (GUIClickArea(eax, edx, p1 + 1, p2 + 1, p1 + 7, p2 + 7)) {
        *p3 = p4;
        GetLoadData();
    }
}

static bool GUIWinControl(s4 const eax, s4 const edx, s4 const p1, s4 const p2, s4 const p3, s4 const p4, u4* const p5, u4 const* const p6, u4 const* const p7, s4 const p8, u4 const p9, u4* const p10, u4 const p11, u4 const p12, u4 const p13) // x,y,x2,y2,currentwin,vpos,#entries,starty,y/entry,cpos,winval,win#,dclicktick#
{
    if (GUIClickArea(eax, edx, p1, p2, p3, p4) && *p7 != 0) {
        *p5 = p11;
        s4 const eax = (u4)(edx - p8) / (u4)p9 + *p6;
        if (eax <= (s4)*p7 - 1)
            *p10 = eax;
        if (GUIHold == 0) {
            if (GUIDClickTL != 0 && GUIDClCWin == p11 && GUIDClCEntry == eax) {
                GUIDClickTL = 0;
                if (p12 == 1) {
                    GUILoadData();
                } else {
                    CheatCodeToggle();
                }
                return true;
            }
            GUIDClickTL = p13;
            GUIDClCWin = p11;
            GUIDClCEntry = eax;
        }
        GUIHoldYlim = GUIwinposy[p12] + p2 - 1;
        GUIHoldYlimR = GUIwinposy[p12] + p4 + 1;
        GUIHoldXlimL = GUIwinposx[p12] + p1;
        GUIHoldXlimR = GUIwinposx[p12] + p3;
        GUIHold = 3;
        return true;
    }
    return false;
}

static bool GUIWinControl2(s4 const eax, s4 const edx, s4 const p1, s4 const p2, s4 const p3, s4 const p4, u4* const p5, s4 const p6, s4 const* const p7, s4 const p8, s4 const p9, s4* const p10, u4 const p11, u4 const p12, u4 const p13) // Key Combination Editor Exclusive
{
    if (GUIClickArea(eax, edx, p1, p2, p3, p4) && *p7 != 0) {
        GUIccomblcursloc = 0xFFFFFFFF;
        *p5 = p11;
        s4 const eax = (edx - p8) / p9 + p6;
        if (eax <= *p7 - 1)
            *p10 = eax;

        if (GUIHold == 0) {
            if (GUIDClickTL != 0 && GUIDClCWin == p11 && GUIDClCEntry == eax) {
                GUIDClickTL = 0;
                u4 const eax = p12;
                if (eax == 1) {
                    GUILoadData();
                } else {
                    CheatCodeToggle();
                }
                return true;
            }
            GUIDClickTL = p13;
            GUIDClCWin = p11;
            GUIDClCEntry = eax;
        }

        GUIHoldYlim = GUIwinposy[p12] + p2 - 1;
        GUIHoldYlimR = GUIwinposy[p12] + p4 + 1;
        GUIHoldXlimL = GUIwinposx[p12] + p1;
        GUIHoldXlimR = GUIwinposx[p12] + p3;
        GUIHold = 3;
        return true;
    }

    return false;
}

static bool DGOptnsProcBox(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u4* const p3, u1* const keycontrolval)
{
    if (GUIClickArea(eax, edx, p1, p2, p1 + 19, p2 + 6)) {
        u4 const ebx = guipresstest();
        if (ebx != 0x01 && ebx != 0x3B) {
            *p3 = ebx;
#ifndef __MSDOS__
            if (keycontrolval != 0)
                *keycontrolval = 1;
#endif
        } else {
            *p3 = 0;
        }
        return true;
    }

    return false;
}

static bool DGOptnsProcBoxc(s4 const eax, s4 const edx, s4 const p1, s4 const p2, s4 const p3, s4 const p4, u4* const p5)
{
    if (GUIClickArea(eax, edx, p1, p2, p3, p4)) {
        u4 const ebx = guipresstest();
        switch (ebx) {
        case 1: // ESC
            pressed[1] = 2;
            /* FALLTHROUGH */
        case 0x3B: // Cancel
            *p5 = 0;
            break;

        default:
            *p5 = ebx;
            break;
        }
        return true;
    }

    return false;
}

static void GUIPTabClick(s4 const eax, s4 const edx, s4 const p1, s4 const p2, u4 const p3, u4* const p4, ...) // minX, maxX, value, var, vars to zero
{
    if (GUIClickArea(eax, edx, p1 + 1, 11, p2 - 1, 22)) {
        GUIInputBox = 0;
        p4[0] = p3;
        va_list ap;
        va_start(ap, p4);
        for (;;) {
            u4* const p = va_arg(ap, u4*);
            if (!p)
                break;
            *p = 0;
        }
        GUIFreshInputSelect = 1;
    }
}

static void GUIBoxVar(s4 const eax, s4 const edx, s4 const p1, s4 const p2, s4 const p3, s4 const p4, u4* const p5, u4 const p6)
{
    if (GUIClickArea(eax, edx, p1, p2, p3, p4))
        *p5 = p6;
}

static void ComboBoxProc(s4 const eax, s4 const edx, s4 const p1, s4 const p2, s4 const p3, s4 const p4, u1 const p5)
{
    if (GUIClickArea(eax, edx, p1, p2, p3, p4) && GUINumCombo != 42) {
        GUIComboData[GUINumCombo++] = p5;
    }
}

static void ComboBoxProcD(s4 const eax, s4 const edx, s4 const p1, s4 const p2, s4 const p3, s4 const p4)
{
    if (GUIClickArea(eax, edx, p1, p2, p3, p4) && GUINumCombo != 0)
        --GUINumCombo;
}

static void DisplayGUIConfirmClick_skipscrol(s4 const eax, s4 const edx)
{
    if (GUIClickCButtonL(eax, edx, 10, 187))
        return;
    if (GUIWinControl(eax, edx, 5, 27, 144, 26 + 15 * 7, &GUIcurrentfilewin, (u4 const*)&GUIcurrentviewloc, (u4 const*)&GUIfileentries, 27, 7, (u4*)&GUIcurrentcursloc, 0, 1, 30))
        return; // XXX ugly casts
    if (GUIWinControl(eax, edx, 160, 27, 228, 26 + 15 * 7, &GUIcurrentfilewin, (u4 const*)&GUIcurrentdirviewloc, (u4 const*)&GUIdirentries, 27, 7, (u4*)&GUIcurrentdircursloc, 1, 1, 30))
        return; // XXX ugly casts
    GUIPHoldbutton(eax, edx, 186, 165, 228, 176, 1);
    GUIPButtonHoleLoad(eax, edx, 9, 163, &GUIloadfntype, 0);
    GUIPButtonHoleLoad(eax, edx, 9, 171, &GUIloadfntype, 1);
#ifdef __MSDOS__
    GUIPButtonHoleLoad(eax, edx, 9, 179, &GUIloadfntype, 2);
#endif
    if (GUIClickCButton5(eax, edx, 144, 177, &ForceROMTiming, 1))
        return;
    if (GUIClickCButton5(eax, edx, 144, 187, &ForceROMTiming, 2))
        return;
    if (GUIClickCButton5(eax, edx, 184, 177, &ForceHiLoROM, 1))
        return;
    if (GUIClickCButton5(eax, edx, 184, 187, &ForceHiLoROM, 2))
        return;
}

static void DisplayGUIConfirmClick(s4 const eax, s4 const edx)
{
    // offset 0 = (ysize-(yend-ystart+1)), offset 1 = starty, offset 2 = endy
    // SlideBar Implementation
    if (GUISlidebarImpl(eax, edx, 146, 33, 153, 33 + 93, GUILStA, 15, (u4*)&GUIcurrentviewloc, (u4*)&GUIcurrentcursloc, (u4 const*)&GUIfileentries, 1))
        return; // XXX ugly casts
    if (GUISlidebarImpl(eax, edx, 230, 33, 237, 33 + 93, GUILStB, 15, (u4*)&GUIcurrentdirviewloc, (u4*)&GUIcurrentdircursloc, (u4 const*)&GUIdirentries, 1))
        return; // XXX ugly casts
    GUIScrolTim1 = 0;
    GUIScrolTim2 = 6;
    DisplayGUIConfirmClick_skipscrol(eax, edx);
}

static void DisplayGUIConfirmClick2(s4 const eax, s4 const edx)
{
    if (GUIfileentries > 1) {
        if (GUISlidebarPostImpl(eax, edx, 146, 33, 153, 33 + 93, 1, 15, (u4*)&GUIcurrentviewloc, (u4*)&GUIcurrentcursloc, (u4 const*)&GUIfileentries, &GUIcurrentfilewin, 0, 5, 26, 144, 27 + 15 * 7, DisplayGUIConfirmClick_skipscrol))
            return; // XXX ugly casts
    }
    if (GUIdirentries > 1) {
        if (GUISlidebarPostImpl(eax, edx, 230, 33, 237, 33 + 93, 3, 15, (u4*)&GUIcurrentdirviewloc, (u4*)&GUIcurrentdircursloc, (u4 const*)&GUIdirentries, &GUIcurrentfilewin, 1, 160, 26, 228, 27 + 15 * 7, DisplayGUIConfirmClick_skipscrol))
            return; // XXX ugly casts
    }
    DisplayGUIConfirmClick(eax, edx);
}

static void DisplayGUIChoseSaveClick(s4 const eax, s4 const edx)
{
    GUIPHoldbutton2(eax, edx, 94, 59, 102, 67, 80, (u1*)&GUIChoseSlotTextX[0], 1, '9'); // XXX ugly cast
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
    if (MovieProcessing < 4 || 6 < MovieProcessing) {
        GUIPTabClick(eax, edx, 0, 57, 1, GUIMovieTabs, &GUIDumpingTab[0], (u4*)0);
    }

    if (MovieProcessing < 1 || 3 < MovieProcessing) {
        GUIPTabClick(eax, edx, 58, 110, 1, GUIDumpingTab, &GUIMovieTabs[0], (u4*)0);
    }

    GUIPButtonHole(eax, edx, 8, 39, (u1*)&CMovieExt, 'v'); // Radio buttons // XXX ugly cast
    GUIPButtonHole(eax, edx, 28, 39, (u1*)&CMovieExt, '1'); // XXX ugly cast
    GUIPButtonHole(eax, edx, 48, 39, (u1*)&CMovieExt, '2'); // XXX ugly cast
    GUIPButtonHole(eax, edx, 68, 39, (u1*)&CMovieExt, '3'); // XXX ugly cast
    GUIPButtonHole(eax, edx, 88, 39, (u1*)&CMovieExt, '4'); // XXX ugly cast
    GUIPButtonHole(eax, edx, 108, 39, (u1*)&CMovieExt, '5'); // XXX ugly cast
    GUIPButtonHole(eax, edx, 128, 39, (u1*)&CMovieExt, '6'); // XXX ugly cast
    GUIPButtonHole(eax, edx, 148, 39, (u1*)&CMovieExt, '7'); // XXX ugly cast
    GUIPButtonHole(eax, edx, 168, 39, (u1*)&CMovieExt, '8'); // XXX ugly cast
    GUIPButtonHole(eax, edx, 188, 39, (u1*)&CMovieExt, '9'); // XXX ugly cast

    if (GUIMovieTabs[0] == 1) {
        if (MovieRecordWinVal != 0) // Overwrite Window
        {
            GUIPHoldbutton(eax, edx, 17, 65, 59, 76, 19);
            GUIPHoldbutton(eax, edx, 70, 65, 112, 76, 20);
            return;
        }

        // Main Window
        GUIPHoldbutton(eax, edx, 7, 80, 49, 91, 16); // Buttons
        GUIPHoldbutton(eax, edx, 55, 80, 97, 91, 17);
        GUIPHoldbutton(eax, edx, 103, 80, 145, 91, 18);
        GUIPHoldbutton(eax, edx, 151, 80, 193, 91, 32);
        GUIPHoldbutton(eax, edx, 7, 108, 50, 119, 29);
        GUIPHoldbutton(eax, edx, 85, 108, 138, 119, 30);
        GUIPHoldbutton(eax, edx, 173, 108, 203, 119, 31);

        GUIPButtonHole(eax, edx, 8, 64, &MovieStartMethod, 0); // Start From
        GUIPButtonHole(eax, edx, 43, 64, &MovieStartMethod, 1);
        GUIPButtonHole(eax, edx, 89, 64, &MovieStartMethod, 2);
        GUIPButtonHole(eax, edx, 135, 64, &MovieStartMethod, 3);

        DGOptnsProcBox(eax, edx, 58, 110, &KeyInsrtChap, 0); // Keyboard Shortcut Boxes
        DGOptnsProcBox(eax, edx, 146, 110, &KeyPrevChap, 0);
        DGOptnsProcBox(eax, edx, 210, 110, &KeyNextChap, 0);

        DGOptnsProcBox(eax, edx, 135, 124, &KeyRTRCycle, 0);

        GUIPButtonHole(eax, edx, 8, 133, &MZTForceRTR, 0);
        GUIPButtonHole(eax, edx, 8, 143, &MZTForceRTR, 1);
        GUIPButtonHole(eax, edx, 8, 153, &MZTForceRTR, 2);

        GUIClickCButton(eax, edx, 8, 163, &MovieDisplayFrame); // Checkbox
    }

    if (GUIDumpingTab[0] == 1) {
        GUIPHoldbutton(eax, edx, 165, 178, 200, 189, 34);
        GUIPHoldbutton(eax, edx, 206, 178, 235, 189, 35);

        GUIPButtonHole(eax, edx, 8, 64, &MovieVideoMode, 0); // Movie Options
        GUIPButtonHole(eax, edx, 8, 74, &MovieVideoMode, 1);
        GUIPButtonHole(eax, edx, 8, 84, &MovieVideoMode, 2);
        GUIPButtonHole(eax, edx, 8, 94, &MovieVideoMode, 3);
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

#define GUIInputSetIndKey(p1, keycontrolval)                                            \
    do {                                                                                \
        /* Check if controller is set */                                                \
        if (*(u4 const*)(keycontrolval) == 0)                                           \
            return; /* XXX cast makes no sense */                                       \
                                                                                        \
        DGOptnsProcBox(eax, edx, 45, 102, &p1##upk, (keycontrolval)); /* Up */          \
        DGOptnsProcBox(eax, edx, 45, 112, &p1##downk, (keycontrolval)); /* Down */      \
        DGOptnsProcBox(eax, edx, 45, 122, &p1##leftk, (keycontrolval)); /* Left */      \
        DGOptnsProcBox(eax, edx, 45, 132, &p1##rightk, (keycontrolval)); /* Right */    \
        DGOptnsProcBox(eax, edx, 45, 142, &p1##startk, (keycontrolval)); /* Start */    \
        DGOptnsProcBox(eax, edx, 45, 152, &p1##selk, (keycontrolval)); /* Select */     \
        DGOptnsProcBox(eax, edx, 85, 102, &p1##Ak, (keycontrolval)); /* A */            \
        DGOptnsProcBox(eax, edx, 85, 112, &p1##Bk, (keycontrolval)); /* B */            \
        DGOptnsProcBox(eax, edx, 85, 122, &p1##Xk, (keycontrolval)); /* X */            \
        DGOptnsProcBox(eax, edx, 85, 132, &p1##Yk, (keycontrolval)); /* Y */            \
        DGOptnsProcBox(eax, edx, 85, 142, &p1##Lk, (keycontrolval)); /* L */            \
        DGOptnsProcBox(eax, edx, 85, 152, &p1##Rk, (keycontrolval)); /* R */            \
                                                                                        \
        DGOptnsProcBox(eax, edx, 125, 102, &p1##Xtk, (keycontrolval)); /* X Turbo */    \
        DGOptnsProcBox(eax, edx, 125, 112, &p1##Ytk, (keycontrolval)); /* Y Turbo */    \
        DGOptnsProcBox(eax, edx, 125, 122, &p1##Ltk, (keycontrolval)); /* L Turbo */    \
        DGOptnsProcBox(eax, edx, 165, 102, &p1##Atk, (keycontrolval)); /* A Turbo */    \
        DGOptnsProcBox(eax, edx, 165, 112, &p1##Btk, (keycontrolval)); /* B Turbo */    \
        DGOptnsProcBox(eax, edx, 165, 122, &p1##Rtk, (keycontrolval)); /* R Turbo */    \
                                                                                        \
        DGOptnsProcBox(eax, edx, 125, 142, &p1##ULk, (keycontrolval)); /* Up-Left */    \
        DGOptnsProcBox(eax, edx, 125, 152, &p1##DLk, (keycontrolval)); /* Down-Left */  \
        DGOptnsProcBox(eax, edx, 165, 142, &p1##URk, (keycontrolval)); /* Up-Right */   \
        DGOptnsProcBox(eax, edx, 165, 152, &p1##DRk, (keycontrolval)); /* Down-Right */ \
    } while (0)

static void DisplayGUIInputClick_skipscrol(s4 const eax, s4 const edx)
{
    // x,y,x2,y2,currentwin,vpos,#entries,starty,y/entry,cpos,winval,win#,dclicktick#
    GUIWinControl(eax, edx, 5, 36, 107, 34 + 5 * 8, &GUIBlankVar, &GUIcurrentinputviewloc, &GUINumValue, 35, 8, &GUIcurrentinputcursloc, 4, 3, 0);

    GUIPTabClick(eax, edx, 0, 21, 1, GUIInputTabs, (u4*)0);
    GUIPTabClick(eax, edx, 22, 43, 2, GUIInputTabs, (u4*)0);
    GUIPTabClick(eax, edx, 44, 65, 3, GUIInputTabs, (u4*)0);
    GUIPTabClick(eax, edx, 66, 87, 4, GUIInputTabs, (u4*)0);
    GUIPTabClick(eax, edx, 88, 109, 5, GUIInputTabs, (u4*)0);

    GUIPHoldbutton(eax, edx, 123, 34, 153, 45, 14); // Buttons
    GUIPHoldbutton(eax, edx, 123, 50, 177, 61, 40);
#ifdef __MSDOS__
    GUIPHoldbutton(eax, edx, 123, 66, 183, 77, 15);
#endif

    switch (cplayernum) {
    case 0:
        GUIInputSetIndKey(pl1, &pl1contrl);
        break;
    case 1:
        GUIInputSetIndKey(pl2, &pl2contrl);
        break;
    case 2:
        GUIInputSetIndKey(pl3, &pl3contrl);
        break;
    case 3:
        GUIInputSetIndKey(pl4, &pl4contrl);
        break;
    case 4:
        GUIInputSetIndKey(pl5, &pl5contrl);
        break;
    }

#ifdef __MSDOS__
    switch (cplayernum) {
    case 0:
        GUIClickCButtonID(eax, edx, 5, 190, &pl1p209);
        break;
    case 1:
        GUIClickCButtonID(eax, edx, 5, 190, &pl2p209);
        break;
    case 1:
        GUIClickCButtonID(eax, edx, 5, 190, &pl3p209);
        break;
    case 1:
        GUIClickCButtonID(eax, edx, 5, 190, &pl4p209);
        break;
    case 1:
        GUIClickCButtonID(eax, edx, 5, 190, &pl5p209);
        break;
    }
    GUIClickCButton(eax, edx, 105, 160, &SidewinderFix);
#endif

    GUIClickCButton(eax, edx, 5, 160, &GameSpecificInput);
    GUIClickCButton(eax, edx, 5, 170, &AllowUDLR);
    GUIClickCButton(eax, edx, 105, 170, &Turbo30hz);
    GUIClickCButtonM(eax, edx, 5, 180, &pl12s34);
}

static void DisplayGUIInputClick(s4 const eax, s4 const edx)
{
    // SlideBar Implementation
    GUINumValue = NumInputDevices;
    if (GUISlidebarImpl(eax, edx, 109, 42, 116, 69, GUIIStA, 5, &GUIcurrentinputviewloc, &GUIcurrentinputcursloc, &GUINumValue, 3))
        return;
    DisplayGUIInputClick_skipscrol(eax, edx);
}

static void DisplayGUIInputClick2(s4 const eax, s4 const edx)
{
    GUINumValue = NumInputDevices;
    if (GUISlidebarPostImpl(eax, edx, 109, 42, 116, 69, 9, 5, &GUIcurrentinputviewloc, &GUIcurrentinputcursloc, &GUINumValue, &GUIBlankVar, 1, 5, 35, 107, 35 + 5 * 8, DisplayGUIInputClick_skipscrol))
        return;
    DisplayGUIInputClick(eax, edx);
}

static void DisplayGUIOptionClick(s4 const eax, s4 const edx)
{
    GUIPTabClick(eax, edx, 0, 39, 1, GUIOptionTabs, (s4*)0);
    GUIPTabClick(eax, edx, 40, 74, 2, GUIOptionTabs, (s4*)0);

    if (GUIOptionTabs[0] == 1) { // Basic
        if (ShowMMXSupport == 1)
            GUIClickCButton(eax, edx, 11, 31, &MMXSupport);
        GUIClickCButton(eax, edx, 11, 41, &Show224Lines);
        GUIClickCButton(eax, edx, 11, 71, &newengen);
        GUIClickCButton(eax, edx, 11, 81, &bgfixer);
        GUIClickCButton(eax, edx, 11, 111, &AutoPatch);
        GUIClickCButton(eax, edx, 11, 121, &DisplayInfo);
        GUIClickCButton(eax, edx, 11, 131, &RomInfo);
#ifdef __WIN32__
        GUIClickCButton(eax, edx, 11, 161, &PauseFocusChange);
        GUIClickCButton(eax, edx, 11, 171, &HighPriority);
        CheckPriority();
#endif
        GUIClickCButton(eax, edx, 11, 181, &DisableScreenSaver);
#ifdef __WIN32__
        CheckScreenSaver();
#endif
    }

    if (GUIOptionTabs[0] == 2) {
        GUIClickCButton(eax, edx, 11, 31, &FPSAtStart);
        GUIClickCButton(eax, edx, 11, 41, &TimerEnable);
        GUIClickCButton(eax, edx, 89, 41, &TwelveHourClock);
        GUIClickCButton(eax, edx, 11, 51, &ClockBox);
        GUIClickCButton(eax, edx, 11, 81, &SmallMsgText);
        GUIClickCButton(eax, edx, 11, 91, &GUIEnableTransp);
        GUIPButtonHole(eax, edx, 11, 121, &ScreenShotFormat, 0);
#ifndef NO_PNG
        GUIPButtonHole(eax, edx, 11, 131, &ScreenShotFormat, 1);
#endif
    }
}

static void DisplayGUIVideoClick_notmodestab(s4 const eax, s4 const edx)
{
    if (GUIVideoTabs[0] == 2) // Filters tab
    {
        Clear2xSaIBuffer();

#ifdef __MSDOS__
        if (smallscreenon != 1)
#endif
        {
#ifdef __MSDOS__
            if (scanlines != 1)
#endif
            {
#ifndef __MSDOS__
                // Bilinear
                if (GUIBIFIL[cvidmode] != 0) {
                    if (GUIClickArea(eax, edx, 18 + 1, 35 + 3, 18 + 6, 35 + 8))
                        NTSCFilter = 0;
                    GUIClickCButtonI(eax, edx, 18, 35, &BilinearFilter);
                } else
#endif
                {
                    // Interpolations
#ifdef __MSDOS__
                    if (GUIEAVID[cvidmode] != 0 || GUII2VID[cvidmode] != 0)
#elif defined __WIN32__
                    if (GUIDSIZE[cvidmode] != 0)
#else
                    if (GUII2VID[cvidmode] != 0)
#endif
                    {
                        if (GUIClickArea(eax, edx, 18 + 1, 35 + 3, 18 + 6, 35 + 8)) {
                            hqFilter = 0;
                            NTSCFilter = 0;
                            En2xSaI = 0;
                        }
                        GUIClickCButton(eax, edx, 18, 35, &antienab);
                    }
                }

                // NTSC filter
                if (GUINTVID[cvidmode] != 0) {
                    if (GUIClickArea(eax, edx, 128 + 1, 35 + 3, 128 + 6, 35 + 8)) {
                        En2xSaI = 0;
                        hqFilter = 0;
                        scanlines = 0;
                        antienab = 0;
                    }
#ifdef __OPENGL__
                    if (GUIClickArea(eax, edx, 128 + 1, 35 + 3, 128 + 6, 35 + 8))
                        BilinearFilter = 0;
#endif
#ifndef __MSDOS__
                    GUIClickCButtonN(eax, edx, 128, 35, &NTSCFilter, NTSCFilterInit);
#endif
                }

                // Kreed 2x filters
#ifdef __MSDOS__
                if (GUI2xVID[cvidmode] != 0)
#else
                if (GUIDSIZE[cvidmode] != 0)
#endif
                {
                    if (GUIClickArea(eax, edx, 18 + 1, 45 + 3, 18 + 6, 45 + 8)) {
                        hqFilter = 0;
                        scanlines = 0;
                        antienab = 0;
                        NTSCFilter = 0;
                    }
                    if (GUIClickArea(eax, edx, 128 + 1, 45 + 3, 128 + 6, 45 + 8)) {
                        hqFilter = 0;
                        scanlines = 0;
                        antienab = 0;
                        NTSCFilter = 0;
                    }
                    if (GUIClickArea(eax, edx, 18 + 1, 55 + 3, 18 + 6, 55 + 8)) {
                        hqFilter = 0;
                        scanlines = 0;
                        antienab = 0;
                        NTSCFilter = 0;
                    }
                    GUIClickCButton6(eax, edx, 18, 45, &En2xSaI, 1);
                    GUIClickCButton6(eax, edx, 128, 45, &En2xSaI, 2);
                    GUIClickCButton6(eax, edx, 18, 55, &En2xSaI, 3);
                }

                u1 const bl = cvidmode; // Hq*x filters
#ifdef __MSDOS__
                if (GUIHQ2X[bl] != 0) {
                    if (GUIClickArea(eax, edx, 128 + 1, 55 + 3, 128 + 6, 55 + 8)) {
                        En2xSaI = 0;
                        scanlines = 0;
                        antienab = 0;
                        NTSCFilter = 0;
                    }
                    GUIClickCButton(eax, edx, 128, 55, &hqFilter);
                }
#else
                if (GUIHQ4X[bl] != 0) {
                    GUIPButtonHole(eax, edx, 188, 68, &hqFilterlevel, 4);
                    goto radiobuttonhq3x;
                }
                if (GUIHQ3X[bl] != 0) {
                radiobuttonhq3x:
                    GUIPButtonHole(eax, edx, 158, 68, &hqFilterlevel, 3);
                    goto radiobuttonhq2x;
                }
                if (GUIHQ2X[bl] != 0) {
                radiobuttonhq2x:
                    GUIPButtonHole(eax, edx, 128, 68, &hqFilterlevel, 2);
                    if (GUIClickArea(eax, edx, 128 + 1, 55 + 3, 128 + 6, 55 + 8)) {
                        En2xSaI = 0;
                        scanlines = 0;
                        antienab = 0;
                        NTSCFilter = 0;
                    }
                    GUIClickCButton(eax, edx, 128, 55, &hqFilter);
                }
#endif
            }

            GUIClickCButton(eax, edx, 18, 115, &GrayscaleMode); // Grayscale

            // Hires Mode7
            if (GUIM7VID[cvidmode] != 0)
                GUIClickCButton5(eax, edx, 128, 115, &Mode7HiRes16b, 1);

#ifdef __MSDOS__
            // Triple buffs/vsyncs
            if (GUITBVID[cvidmode] != 0)
                GUIClickCButtonT(eax, edx, 128, 145, &Triplebufen, &vsyncon);
            GUIClickCButtonT(eax, edx, 18, 145, &vsyncon, &Triplebufen);
#endif

#ifdef __WIN32__
            // Triple buffs/vsyncs
            if (GUIWFVID[cvidmode] != 0) {
                GUIClickCButtonf(eax, edx, 128, 145, &TripleBufferWin, initDirectDraw);
            }
            GUIClickCButtonf(eax, edx, 18, 145, &vsyncon, initDirectDraw);
#endif

#ifdef __OPENGL__
            if (GUIBIFIL[cvidmode] != 0)
                GUIClickCButtonI(eax, edx, 18, 145, &vsyncon);
#endif

            // Keep 4:3 Ratio
            if (GUIKEEP43[cvidmode] != 0)
                GUIClickCButtonK(eax, edx, 18, 175, &Keep4_3Ratio, initwinvideo);

#ifndef __MSDOS__
            // GL Scanlines
            if (GUIBIFIL[cvidmode] != 0) {
                // Update mouse location
                s4 const eax = GUImouseposx - GUIwinposx[5];
                s4 const edx = GUImouseposy - GUIwinposy[5];

                if (GUIClickArea(eax, edx, 23, 88, 23 + 100, 92)) {
                    sl_intensity = eax - 23;
                    GUIHold = 8; // Lock mouse to bar when clicked
                    GUIHoldYlim = GUIwinposy[5] + 90;
                    s4 const eax = GUIwinposx[5] + 23;
                    GUIHoldXlimL = eax;
                    GUIHoldXlimR = eax + 100;
                }
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
                    if (GUIClickArea(eax, edx, 168 + 1, 87 + 3, 168 + 38, 87 + 8)) {
                        En2xSaI = 0;
                        hqFilter = 0;
                        NTSCFilter = 0;
                    }
                    GUIPButtonHoleS(eax, edx, 18, 87, &scanlines, 0);
                    GUIPButtonHoleS(eax, edx, 168, 87, &scanlines, 1);
                }

#ifdef __MSDOS__
                if (ScreenScale != 1)
#endif
                {
#ifdef __MSDOS__
                    if (GUIHSVID[cvidmode] != 0)
#else
                    if (GUIDSIZE[cvidmode] != 0)
#endif
                    {
                        if (GUIClickArea(eax, edx, 68 + 1, 87 + 3, 68 + 38, 87 + 8)) {
                            En2xSaI = 0;
                            hqFilter = 0;
                            NTSCFilter = 0;
                        }
                        if (GUIClickArea(eax, edx, 118 + 1, 87 + 3, 118 + 38, 87 + 8)) {
                            En2xSaI = 0;
                            hqFilter = 0;
                            NTSCFilter = 0;
                        }
                        GUIPButtonHoleS(eax, edx, 68, 87, &scanlines, 2);
                        GUIPButtonHoleS(eax, edx, 118, 87, &scanlines, 3);
                    }
                }
            }
        }
    }

    if (GUIVntscTab[0] == 1) // NTSC Tab
    {
        // Update mouse location
        s4 const eax = GUImouseposx - GUIwinposx[5];
        s4 const edx = GUImouseposy - GUIwinposy[5];

        GUIClickCButton(eax, edx, 5, 25, &NTSCBlend); // Checkboxes
        GUIClickCButton(eax, edx, 135, 25, &NTSCRef);

        GUIPHoldbutton(eax, edx, 8, 166, 67, 177, 81);
        GUIPHoldbutton(eax, edx, 72, 166, 119, 177, 82);
        GUIPHoldbutton(eax, edx, 124, 166, 147, 177, 83);
        GUIPHoldbutton(eax, edx, 152, 166, 217, 177, 84);
        GUIPHoldbutton(eax, edx, 102, 186, 137, 197, 37); // button
        GUIPHoldbutton(eax, edx, 148, 186, 207, 197, 39); // button

        // Sliders
        if (8 <= eax && eax <= 8 + 200) // X-Range for click-area
        {
            s1 const al = eax - 108;
            if (54 <= edx && edx <= 58) // Y-Range for click-area
            { // Hue
                NTSCHue = al;
                ntscCurVar = al;
                ntscWhVar = 0;
                GUIHoldYlim = GUIwinposy[5] + 56;
            } else if (74 <= edx && edx <= 78) { // Saturation
                NTSCSat = al;
                ntscCurVar = al;
                ntscWhVar = 1;
                GUIHoldYlim = GUIwinposy[5] + 76;
            } else if (94 <= edx && edx <= 98) { // Contrast
                NTSCCont = al;
                ntscCurVar = al;
                ntscWhVar = 2;
                GUIHoldYlim = GUIwinposy[5] + 96;
            } else if (114 <= edx && edx <= 118) { // Brightness
                NTSCBright = al;
                ntscCurVar = al;
                ntscWhVar = 3;
                GUIHoldYlim = GUIwinposy[5] + 116;
            } else if (134 <= edx && edx <= 138) {
                NTSCSharp = al;
                ntscCurVar = al;
                ntscWhVar = 4;
                GUIHoldYlim = GUIwinposy[5] + 136;
            } else {
                goto nomovebar;
            }

            s4 const eax = GUIwinposx[5] + 8;
            GUIHoldXlimL = eax;
            GUIHoldXlimR = eax + 200;
            GUIHold = 7; // Lock mouse to bar when clicked
        nomovebar:;
        }

#ifndef __MSDOS__
        if (NTSCRef != 0) {
            NTSCPresetVar = 4;
            NTSCFilterInit();
        }
#endif
    }

    if (GUIVntscTab[0] == 2) // NTSC Tab
    {
        // Update mouse location
        s4 const eax = GUImouseposx - GUIwinposx[5];
        s4 const edx = GUImouseposy - GUIwinposy[5];

        GUIPHoldbutton(eax, edx, 102, 186, 137, 197, 38); // button
        GUIPHoldbutton(eax, edx, 148, 186, 207, 197, 39); // button

        // Sliders
        if (8 <= eax && eax <= 8 + 200) // X-Range for click-area
        {
            s1 const al = eax - 108;
            if (44 <= edx && edx <= 48) // Y-Range for click-area
            { // Gamma
                NTSCGamma = al;
                ntscCurVar = al;
                ntscWhVar = 0;
                GUIHoldYlim = GUIwinposy[5] + 46;
            } else if (64 <= edx && edx <= 68) { // Resolution
                NTSCRes = al;
                ntscCurVar = al;
                ntscWhVar = 1;
                GUIHoldYlim = GUIwinposy[5] + 66;
            } else if (84 <= edx && edx <= 88) { // Artifacts
                NTSCArt = al;
                ntscCurVar = al;
                ntscWhVar = 2;
                GUIHoldYlim = GUIwinposy[5] + 86;
            } else if (104 <= edx && edx <= 108) { // Fringing
                NTSCFringe = al;
                ntscCurVar = al;
                ntscWhVar = 3;
                GUIHoldYlim = GUIwinposy[5] + 106;
            } else if (124 <= edx && edx <= 128) { // Bleed
                NTSCBleed = al;
                ntscCurVar = al;
                ntscWhVar = 4;
                GUIHoldYlim = GUIwinposy[5] + 126;
            } else if (144 <= edx && edx <= 148) { // Hue warping
                NTSCWarp = al;
                ntscCurVar = al;
                ntscWhVar = 5;
                GUIHoldYlim = GUIwinposy[5] + 146;
            } else {
                goto nomovebar2;
            }

            s4 const eax = GUIwinposx[5] + 8;
            GUIHoldXlimL = eax;
            GUIHoldXlimR = eax + 200;
            GUIHold = 7; // Lock mouse to bar when clicked
        nomovebar2:;
        }

#ifndef __MSDOS__
        if (NTSCRef != 0) {
            NTSCPresetVar = 4;
            NTSCFilterInit();
        }
#endif
    }
}

static void DisplayGUIVideoClick_skipscrol(s4 const eax, s4 const edx)
{
    if (GUIWinControl(eax, edx, 5, 27, 115, 27 + 20 * 8, &GUIBlankVar, &GUIcurrentvideoviewloc, &GUINumValue, 27, 8, &GUIcurrentvideocursloc, 2, 5, 0))
        return;

    GUIPHoldbutton(eax, edx, 130, 31, 166, 41, 4);

#ifndef __MSDOS__
    GUIPHoldbutton(eax, edx, 182, 116, 218, 126, 12);

    GUITextBoxInputNach(eax, edx, 130, 130, 178, 140, 0, 5, SetCustomXY);
    GUITextBoxInputNach(eax, edx, 191, 130, 239, 140, 1, 5, SetCustomXY);
#endif

    DisplayGUIVideoClick_notmodestab(eax, edx);
}

static void DisplayGUIVideoClick(s4 const eax, s4 const edx)
{
    GUIPTabClick(eax, edx, 0, 39, 1, GUIVideoTabs, GUIVntscTab, (s4*)0);
    GUIPTabClick(eax, edx, 40, 91, 2, GUIVideoTabs, GUIVntscTab, (s4*)0);
    if (NTSCFilter == 1) {
        GUIPTabClick(eax, edx, 92, 125, 1, GUIVntscTab, GUIVideoTabs, (s4*)0);
        GUIPTabClick(eax, edx, 126, 184, 2, GUIVntscTab, GUIVideoTabs, (s4*)0);
    }

    if (GUIVideoTabs[0] == 1) { // SlideBar Implementation
        GUINumValue = NumVideoModes;
        if (GUISlidebarImpl(eax, edx, 117, 33, 124, 182, GUIVStA, 20, &GUIcurrentvideoviewloc, &GUIcurrentvideocursloc, &GUINumValue, 5))
            return;
        DisplayGUIVideoClick_skipscrol(eax, edx);
    } else {
        DisplayGUIVideoClick_notmodestab(eax, edx);
    }
}

static void DisplayGUIVideoClick2(s4 const eax, s4 const edx)
{
    if (GUIVideoTabs[0] == 1) // modes
    {
        GUINumValue = NumVideoModes;
        if (GUISlidebarPostImpl(eax, edx, 117, 33, 124, 182, 5, 20, &GUIcurrentvideoviewloc, &GUIcurrentvideocursloc, &GUINumValue, &GUIBlankVar, 1, 5, 27, 115, 27 + 20 * 8, DisplayGUIVideoClick_skipscrol))
            return;
    }
    DisplayGUIVideoClick(eax, edx);
}

static void DisplayGUISoundClick(void)
{
    s4 const eax = GUImouseposx - GUIwinposx[6];
    s4 const edx = GUImouseposy - GUIwinposy[6];

    GUIClickCButton(eax, edx, 11, 21, &SPCDisable);
    GUIClickCButtonf(eax, edx, 11, 31, &soundon, reInitSound);
    GUIClickCButton(eax, edx, 11, 41, &StereoSound);
    GUIClickCButton(eax, edx, 11, 51, &RevStereo);
    GUIClickCButton(eax, edx, 11, 61, &Surround);
#ifdef __MSDOS__
    GUIClickCButton(eax, edx, 11, 71, &Force8b);
#endif
#ifdef __WIN32__
    GUIClickCButton(eax, edx, 11, 71, &PrimaryBuffer);
#endif

    GUIPButtonHole(eax, edx, 11, 157, &SoundInterpType, 0);
    GUIPButtonHole(eax, edx, 11, 167, &SoundInterpType, 1);
    GUIPButtonHole(eax, edx, 11, 177, &SoundInterpType, 2);
    if (MMXSupport != 0)
        GUIPButtonHole(eax, edx, 11, 187, &SoundInterpType, 3);

    GUIPButtonHole(eax, edx, 111, 157, &LowPassFilterType, 0);
    GUIPButtonHole(eax, edx, 111, 167, &LowPassFilterType, 1);
    GUIPButtonHole(eax, edx, 111, 177, &LowPassFilterType, 2);
    if (MMXSupport != 0)
        GUIPButtonHole(eax, edx, 111, 187, &LowPassFilterType, 3);

    if (GUIClickArea(eax, edx, 15, 101, 69, 109)) {
        static u1 const sampratenext[] = { 1, 4, 5, 6, 2, 3, 0, 0 };
        SoundQuality = sampratenext[SoundQuality];
    }

    if (GUIClickArea(eax, edx, 15, 129, 115, 133)) {
        MusicRelVol = eax - 15;
        GUIHold = 5;
        GUIHoldYlim = GUIwinposy[6] + 131;
        u4 const vol = MusicRelVol * 128 / 100;
        MusicVol = vol < 127 ? vol : 127;
        asm volatile("call %P0" ::"X"(WDSPReg0C), "a"(DSPMem[0x0C])
                     : "cc", "memory");
        asm volatile("call %P0" ::"X"(WDSPReg1C), "a"(DSPMem[0x1C])
                     : "cc", "memory");

        s4 const eax = GUIwinposx[6] + 15;
        GUIHoldXlimL = eax;
        GUIHoldXlimR = eax + 100;
    }
}

static void DisplayGUICheatClick_skipscrol(s4 const eax, s4 const edx)
{
    if (GUIcurrentcheatwin == 0) {
        GUIWinControl(eax, edx, 5, 23, 229, 21 + 12 * 7, &GUIBlankVar, &GUIcurrentcheatviewloc, &NumCheats, 22, 7, &GUIcurrentcheatcursloc, 3, 7, 30);
    }

    GUIPHoldbutton(eax, edx, 5, 113, 47, 124, 5); // Buttons
    GUIPHoldbutton(eax, edx, 52, 113, 94, 124, 6);
    GUIPHoldbutton(eax, edx, 99, 113, 141, 124, 7);
    GUIPHoldbutton(eax, edx, 146, 113, 188, 124, 8);
    GUIPHoldbutton(eax, edx, 212, 134, 236, 145, 9);
    GUIPHoldbutton(eax, edx, 193, 113, 235, 124, 33);
    GUIClickCButton5(eax, edx, 11, 186, &AutoLoadCht, 1); // Checkbox
}

static void DisplayGUICheatClick(s4 const eax, s4 const edx)
{
    GUIBoxVar(eax, edx, 5, 20, 181, 110, &GUIcurrentcheatwin, 0); // Main Box
    GUIBoxVar(eax, edx, 82, 129, 173, 136, &GUIcurrentcheatwin, 1); // Enter Code Box
    GUIBoxVar(eax, edx, 82, 140, 196, 147, &GUIcurrentcheatwin, 2); // Description Box
    // SlideBar Implementation
    if (GUISlidebarImpl(eax, edx, 231, 28, 238, 100, GUICStA, 12, &GUIcurrentcheatviewloc, &GUIcurrentcheatcursloc, &NumCheats, 7))
        return;
    DisplayGUICheatClick_skipscrol(eax, edx);
}

static void DisplayGUICheatClick2(s4 const eax, s4 const edx)
{
    if (GUISlidebarPostImpl(eax, edx, 231, 28, 238, 100, 7, 12, &GUIcurrentcheatviewloc, &GUIcurrentcheatcursloc, &NumCheats, &GUIBlankVar, 1, 5, 22, 229, 22 + 12 * 7, DisplayGUICheatClick_skipscrol))
        return;
    DisplayGUICheatClick(eax, edx);
}

static void DisplayNetOptnsClick(void) { }

static void DisplayGameOptnsClick(s4 const eax, s4 const edx)
{
    // Shortcut Boxes
    DGOptnsProcBox(eax, edx, 27, 23, &KeyBGDisble0, 0);
    DGOptnsProcBox(eax, edx, 27 + 45, 23, &KeyBGDisble1, 0);
    DGOptnsProcBox(eax, edx, 27 + 45 * 2, 23, &KeyBGDisble2, 0);
    DGOptnsProcBox(eax, edx, 27 + 45 * 3, 23, &KeyBGDisble3, 0);
    DGOptnsProcBox(eax, edx, 27 + 45 * 4, 23, &KeySprDisble, 0);
    DGOptnsProcBox(eax, edx, 27, 41, &KeyDisableSC0, 0);
    DGOptnsProcBox(eax, edx, 27 + 45, 41, &KeyDisableSC1, 0);
    DGOptnsProcBox(eax, edx, 27 + 45 * 2, 41, &KeyDisableSC2, 0);
    DGOptnsProcBox(eax, edx, 27 + 45 * 3, 41, &KeyDisableSC3, 0);
    DGOptnsProcBox(eax, edx, 34 + 45 * 4, 41, &KeyVolUp, 0);
    DGOptnsProcBox(eax, edx, 27, 50, &KeyDisableSC4, 0);
    DGOptnsProcBox(eax, edx, 27 + 45, 50, &KeyDisableSC5, 0);
    DGOptnsProcBox(eax, edx, 27 + 45 * 2, 50, &KeyDisableSC6, 0);
    DGOptnsProcBox(eax, edx, 27 + 45 * 3, 50, &KeyDisableSC7, 0);
    DGOptnsProcBox(eax, edx, 34 + 45 * 4, 50, &KeyVolDown, 0);

    DGOptnsProcBox(eax, edx, 8 + 25, 70, &KeyQuickLoad, 0);
    DGOptnsProcBox(eax, edx, 8 + 57 + 26, 70, &KeyQuickRst, 0);
    DGOptnsProcBox(eax, edx, 8 + 114 + 20, 70, &KeyQuickExit, 0);
    DGOptnsProcBox(eax, edx, 8 + 114 + 78, 70, &KeyQuickClock, 0);
    DGOptnsProcBox(eax, edx, 8 + 25, 80, &KeyQuickChat, 0);
    DGOptnsProcBox(eax, edx, 8 + 57 + 45, 80, &KeyQuickSnapShot, 0);
    DGOptnsProcBox(eax, edx, 8 + 108 + 70, 80, &KeyQuickSaveSPC, 0);

    DGOptnsProcBox(eax, edx, 78, 100, &KeyUsePlayer1234, 0);
    DGOptnsProcBox(eax, edx, 78, 110, &KeyResetAll, 0);
    DGOptnsProcBox(eax, edx, 78, 120, &KeyDisplayFPS, 0);
#ifndef __MSDOS__
    DGOptnsProcBox(eax, edx, 78, 130, &KeyDisplayBatt, 0);
#endif

    DGOptnsProcBox(eax, edx, 191, 100, &KeyNewGfxSwt, 0);
    DGOptnsProcBox(eax, edx, 191, 110, &KeyWinDisble, 0);
    DGOptnsProcBox(eax, edx, 191, 120, &KeyOffsetMSw, 0);
    DGOptnsProcBox(eax, edx, 191, 130, &KeyIncreaseGamma, 0);
    DGOptnsProcBox(eax, edx, 191, 140, &KeyDecreaseGamma, 0);
}

static void DisplayGUIOptnsClick(void)
{
    s4 const eax = GUImouseposx - GUIwinposx[10];
    s4 const edx = GUImouseposy - GUIwinposy[10];

    GUIClickCButton(eax, edx, 12, 23, &GUIRClick); // Checkboxes
    GUIClickCButton(eax, edx, 12, 33, &lhguimouse);
    GUIClickCButton(eax, edx, 12, 43, &mouseshad);
    GUIClickCButton(eax, edx, 12, 53, &mousewrap);

    GUIClickCButton(eax, edx, 129, 23, &esctomenu);
    GUIClickCButton(eax, edx, 129, 33, &JoyPad1Move);
    GUIClickCButtonI(eax, edx, 129, 43, &FilteredGUI);
    GUIClickCButton(eax, edx, 129, 53, &newfont);
    GUIClickCButton(eax, edx, 129, 63, &savewinpos);

#ifdef __WIN32__
    GUIClickCButton(eax, edx, 12, 63, &TrapMouseCursor);
    GUIClickCButton(eax, edx, 12, 73, &MouseWheel);
    GUIClickCButton(eax, edx, 12, 168, &AlwaysOnTop);
    GUIClickCButton(eax, edx, 12, 178, &SaveMainWindowPos);
    GUIClickCButton(eax, edx, 12, 188, &AllowMultipleInst);
#endif

    GUIPButtonHole(eax, edx, 72, 88, &GUIEffect, 0); // Radio Buttons
    GUIPButtonHole(eax, edx, 122, 88, &GUIEffect, 1);
    GUIPButtonHole(eax, edx, 182, 88, &GUIEffect, 4);
    GUIPButtonHole(eax, edx, 72, 98, &GUIEffect, 5);
    GUIPButtonHole(eax, edx, 122, 98, &GUIEffect, 2);
    GUIPButtonHole(eax, edx, 182, 98, &GUIEffect, 3);

    GUIPButtonHole(eax, edx, 48, 108, &CurPalSelect, 0);
    GUIPButtonHole(eax, edx, 88, 108, &CurPalSelect, 1);
    GUIPButtonHole(eax, edx, 133, 108, &CurPalSelect, 2);

    // Sliders
    u1* TRVal2;
    u1* TGVal2;
    u1* TBVal2;
    switch (CurPalSelect) {
    default:
        TRVal2 = &GUIRAdd;
        TGVal2 = &GUIGAdd;
        TBVal2 = &GUIBAdd;
        break;
    case 1:
        TRVal2 = &GUITRAdd;
        TGVal2 = &GUITGAdd;
        TBVal2 = &GUITBAdd;
        break;
    case 2:
        TRVal2 = &GUIWRAdd;
        TGVal2 = &GUIWGAdd;
        TBVal2 = &GUIWBAdd;
        break;
    }

    bool changed = false;
    if (25 <= eax && eax <= 25 + 127) {
        u1 const al = (u4)(eax - 25) / 4;
        if (122 <= edx && edx <= 126) {
            if (*TRVal2 != al) {
                *TRVal2 = al;
                changed = true;
            }
            GUIHold = 2;
            GUIHoldYlim = GUIwinposy[10] + 124;
        } else if (134 <= edx && edx <= 138) {
            if (*TGVal2 != al) {
                *TGVal2 = al;
                changed = true;
            }
            GUIHold = 2;
            GUIHoldYlim = GUIwinposy[10] + 136;
        } else if (146 <= edx && edx <= 150) {
            if (*TBVal2 != al) {
                *TBVal2 = al;
                changed = true;
            }
            GUIHold = 2;
            GUIHoldYlim = GUIwinposy[10] + 148;
        }
    }

    if (changed)
        GUISetPal();

    if (GUIHold == 2) {
        GUIHoldXlimL = GUIwinposx[10] + 25;
        GUIHoldXlimR = GUIwinposx[10] + 25 + 127;
    }
}

static void DisplayGUIAboutClick(s4 const eax, s4 const edx)
{
#ifndef __MSDOS__
    if (EEgg != 1) {
        GUIPHoldbutton(eax, edx, 90, 22, 175, 32, 65);
        GUIPHoldbutton(eax, edx, 90, 33, 175, 43, 66);
    }
#endif
}

static void DisplayGUIComboClick_skipscrol(s4 const eax, s4 const edx)
{
    // x,y,x2,y2,currentwin,vpos,#entries,starty,y/entry,cpos,winval,win#,dclicktick#
    if (GUIWinControl2(eax, edx, 10, 23, 190, 22 + 8 * 7, &GUIBlankVar, GUIccombviewloc, (s4 const*)&NumCombo, 23, 7, (s4*)&GUIccombcursloc, 6, 16, 0))
        return;

    GUIPButtonHole(eax, edx, 158, 82, &GUIComboPNum, 0);
    GUIPButtonHole(eax, edx, 176, 82, &GUIComboPNum, 1);
    GUIPButtonHole(eax, edx, 194, 82, &GUIComboPNum, 2);
    GUIPButtonHole(eax, edx, 212, 82, &GUIComboPNum, 3);
    GUIPButtonHole(eax, edx, 230, 82, &GUIComboPNum, 4);
    GUIClickCButton(eax, edx, 163, 88, &GUIComboLHorz);
    GUIClickCButtonC(eax, edx, 163, 96, &GUIComboGameSpec);
    GUIPHoldbutton(eax, edx, 202, 20, 246, 31, 60);
    GUIPHoldbutton(eax, edx, 202, 35, 246, 46, 61);
    GUIPHoldbutton(eax, edx, 202, 50, 246, 61, 62);
    GUIPHoldbutton(eax, edx, 202, 65, 246, 76, 63);
    if (DGOptnsProcBoxc(eax, edx, 10, 91, 32, 99, &GUIComboKey))
        return;

    ComboBoxProc(eax, edx, 75, 150, 85, 157, 1);
    ComboBoxProc(eax, edx, 89, 150, 99, 157, 2);
    ComboBoxProc(eax, edx, 103, 150, 113, 157, 3);
    ComboBoxProc(eax, edx, 117, 150, 127, 157, 4);
    ComboBoxProc(eax, edx, 131, 150, 141, 157, 5);
    ComboBoxProc(eax, edx, 145, 150, 155, 157, 6);
    ComboBoxProc(eax, edx, 159, 150, 169, 157, 7);
    ComboBoxProc(eax, edx, 173, 150, 183, 157, 8);
    ComboBoxProc(eax, edx, 187, 150, 197, 157, 9);
    ComboBoxProc(eax, edx, 201, 150, 211, 157, 10);
    ComboBoxProc(eax, edx, 215, 150, 227, 157, 11);
    ComboBoxProc(eax, edx, 231, 150, 243, 157, 12);
    ComboBoxProc(eax, edx, 75, 160, 85, 167, 13);
    ComboBoxProc(eax, edx, 89, 160, 99, 167, 14);
    ComboBoxProc(eax, edx, 103, 160, 113, 167, 15);
    ComboBoxProc(eax, edx, 117, 160, 127, 167, 16);
    ComboBoxProc(eax, edx, 131, 160, 141, 167, 17);
    ComboBoxProc(eax, edx, 145, 160, 155, 167, 18);
    ComboBoxProc(eax, edx, 159, 160, 169, 167, 19);
    ComboBoxProc(eax, edx, 173, 160, 183, 167, 20);
    ComboBoxProc(eax, edx, 187, 160, 197, 167, 21);
    ComboBoxProc(eax, edx, 201, 160, 211, 167, 22);
    ComboBoxProc(eax, edx, 215, 160, 227, 167, 23);
    ComboBoxProc(eax, edx, 231, 160, 243, 167, 24);
    ComboBoxProc(eax, edx, 75, 170, 85, 177, 25);
    ComboBoxProc(eax, edx, 89, 170, 99, 177, 26);
    ComboBoxProc(eax, edx, 103, 170, 113, 177, 27);
    ComboBoxProc(eax, edx, 117, 170, 127, 177, 28);
    ComboBoxProc(eax, edx, 131, 170, 141, 177, 29);
    ComboBoxProc(eax, edx, 145, 170, 155, 177, 30);
    ComboBoxProc(eax, edx, 159, 170, 169, 177, 31);
    ComboBoxProc(eax, edx, 173, 170, 183, 177, 32);
    ComboBoxProc(eax, edx, 187, 170, 197, 177, 33);
    ComboBoxProc(eax, edx, 201, 170, 211, 177, 34);
    ComboBoxProc(eax, edx, 215, 170, 227, 177, 35);
    ComboBoxProc(eax, edx, 231, 170, 243, 177, 36);

    ComboBoxProc(eax, edx, 10, 189, 20, 196, 37);
    ComboBoxProc(eax, edx, 24, 189, 34, 196, 38);
    ComboBoxProc(eax, edx, 38, 189, 48, 196, 39);
    ComboBoxProc(eax, edx, 52, 189, 62, 196, 40);
    ComboBoxProc(eax, edx, 66, 189, 76, 196, 41);
    ComboBoxProc(eax, edx, 80, 189, 90, 196, 42);
    ComboBoxProc(eax, edx, 107, 189, 117, 196, 43);
    ComboBoxProc(eax, edx, 121, 189, 131, 196, 44);
    ComboBoxProc(eax, edx, 135, 189, 145, 196, 45);
    ComboBoxProc(eax, edx, 149, 189, 159, 196, 46);
    ComboBoxProc(eax, edx, 163, 189, 173, 196, 47);
    ComboBoxProc(eax, edx, 177, 189, 187, 196, 48);
    ComboBoxProcD(eax, edx, 204, 189, 218, 196);

    NumCombo = GUIComboGameSpec == 0 ? NumComboGlob : NumComboLocl;
}

static void DisplayGUIComboClick(s4 const eax, s4 const edx)
{
    // SlideBar Implementation
    // x1,y1,x2,y2,GUI?StA,ScrnSize,ViewLoc,CursLoc,Entries,win#
    if (GUISlidebarImpl(eax, edx, 192, 28, 199, 72, GUICSStC, 8, &GUIccombviewloc, &GUIccombviewloc, &NumCombo, 16))
        return;
    DisplayGUIComboClick_skipscrol(eax, edx);
}

static void DisplayGUIComboClick2(s4 const eax, s4 const edx)
{
    // x1,y1,x2,y2,upjump,downjump,holdpos,scsize,view,cur,listsize
    // x1,y1,x2,y2,view,curs,num,.scru,.scrd,jumpto,sizeofscreen
    if (GUISlidebarPostImpl(eax, edx, 192, 28, 199, 72, 13, 8, &GUIccombviewloc, &GUIccombcursloc, &NumCombo, &GUIBlankVar, 1, 10, 22, 190, 23 + 8 * 7, DisplayGUIComboClick_skipscrol))
        return;
    DisplayGUIComboClick(eax, edx);
}

static void DisplayGUIResetClick(s4 const eax, s4 const edx)
{
    GUIPHoldbutton(eax, edx, 20, 30, 56, 41, 2);
    GUIPHoldbutton(eax, edx, 80, 30, 116, 41, 3);
}

static void DisplayGUIStatesClick(s4 const eax, s4 const edx)
{
    GUIPHoldbutton(eax, edx, 20, 30, 56, 41, 10);
    GUIPHoldbutton(eax, edx, 80, 30, 116, 41, 11);
}

static void DisplayGUICheatSearchClick_skipscrol(s4 const eax, s4 const edx)
{
    // x,y,x2,y2,currentwin,vpos,#entries,starty,y/entry,cpos,winval,win#,dclicktick#
    if (GUIWinControl(eax, edx, 5, 23, 171, 21 + 12 * 7, &GUIBlankVar, &GUIcurrentchtsrcviewloc, &NumCheatSrc, 22, 7, &GUIcurrentchtsrccursloc, 5, 13, 30))
        return;

    GUIPHoldbutton(eax, edx, 70, 140, 130, 152, 54); // Return/Add Buttons
    GUIPHoldbutton(eax, edx, 140, 140, 180, 152, 55);
}

static void DisplayGUICheatSearchClick_view(s4 const eax, s4 const edx)
{
    // View Cheat
    // x1,y1,x2,y2,GUI?StA,ScrnSize,ViewLoc,CursLoc,Entries,win#
    if (GUISlidebarImpl(eax, edx, 173, 28, 180, 100, GUICSStA, 12, &GUIcurrentchtsrcviewloc, &GUIcurrentchtsrccursloc, &NumCheatSrc, 13))
        return;
    DisplayGUICheatSearchClick_skipscrol(eax, edx);
}

static void DisplayGUICheatSearchClick(s4 const eax, s4 const edx)
{
    switch (CheatWinMode) {
    case 1: // Exact Value
        GUIPHoldbutton(eax, edx, 30, 140, 80, 152, 51);
        if (CheatSearchStatus != 1) { // Return and Re-Search
            if (CheatSrcSearchType == 1) { // Select Comparison
                GUIPButtonHole(eax, edx, 11, 33, &CheatCompareValue, 0);
                GUIPButtonHole(eax, edx, 11, 43, &CheatCompareValue, 1);
                GUIPButtonHole(eax, edx, 11, 53, &CheatCompareValue, 2);
                GUIPButtonHole(eax, edx, 11, 63, &CheatCompareValue, 3);
            }
            // Restart/View/Search Buttons
            GUIPHoldbutton(eax, edx, 120, 140, 170, 152, 53);
        }
        GUIPHoldbutton(eax, edx, 10, 140, 60, 152, 51);
        GUIPHoldbutton(eax, edx, 70, 140, 110, 152, 52);
        break;

    case 2:
        DisplayGUICheatSearchClick_view(eax, edx);
        break;

    case 3: // Add Cheat
        GUIPHoldbutton(eax, edx, 60, 155, 120, 167, 56); // Return/Add Buttons
        GUIPHoldbutton(eax, edx, 130, 155, 160, 167, 57);
        GUIClickCButton(eax, edx, 8, 139, &CheatUpperByteOnly); // Checkbox
        GUIBoxVar(eax, edx, 10, 30, 80, 37, &CurCStextpos, 0); // Input Boxes
        GUIBoxVar(eax, edx, 10, 55, 126, 62, &CurCStextpos, 1);
        break;

    default:
        GUIPHoldbutton(eax, edx, 95, 140, 140, 152, 50); //  Main Menu
        GUIPButtonHole(eax, edx, 11, 28, &CheatSrcByteSize, 0);
        GUIPButtonHole(eax, edx, 11, 38, &CheatSrcByteSize, 1);
        GUIPButtonHole(eax, edx, 11, 48, &CheatSrcByteSize, 2);
        GUIPButtonHole(eax, edx, 11, 58, &CheatSrcByteSize, 3);
        GUIPButtonHole(eax, edx, 11, 73, &CheatSrcByteBase, 0);
        GUIPButtonHole(eax, edx, 11, 83, &CheatSrcByteBase, 1);
        GUIPButtonHole(eax, edx, 11, 113, &CheatSrcSearchType, 0);
        GUIPButtonHole(eax, edx, 11, 123, &CheatSrcSearchType, 1);
        break;
    }
}

static void DisplayGUICheatSearchClick2(s4 const eax, s4 const edx)
{
    if (CheatWinMode == 2) { // Preview Box
        // x1,y1,x2,y2,upjump,downjump,holdpos,scsize,view,cur,listsize
        // x1,y1,x2,y2,view,curs,num,.scru,.scrd,jumpto,sizeofscreen
        if (GUISlidebarPostImpl(eax, edx, 173, 28, 180, 100, 11, 12, &GUIcurrentchtsrcviewloc, &GUIcurrentchtsrccursloc, &NumCheatSrc, &GUIBlankVar, 1, 5, 22, 171, 22 + 12 * 7, DisplayGUICheatSearchClick_skipscrol))
            return;
        DisplayGUICheatSearchClick_view(eax, edx);
    } else {
        DisplayGUICheatSearchClick(eax, edx);
    }
}

static void DisplayGUIAddOnClick(s4 const eax, s4 const edx)
{
    GUIPButtonHole(eax, edx, 9, 23, &device1, 0);
    GUIPButtonHole(eax, edx, 9, 33, &device1, 1);

    GUIPButtonHole(eax, edx, 100, 23, &device2, 0);
    GUIPButtonHole(eax, edx, 100, 33, &device2, 1);
    GUIPButtonHole(eax, edx, 100, 43, &device2, 2);
    GUIPButtonHole(eax, edx, 100, 53, &device2, 3);
    GUIPButtonHole(eax, edx, 100, 63, &device2, 4);

    DGOptnsProcBox(eax, edx, 74, 76, &KeyExtraEnab1, 0);
    DGOptnsProcBox(eax, edx, 165, 76, &KeyExtraEnab2, 0);

    GUIClickCButton(eax, edx, 9, 83, &mouse1lh);
    GUIClickCButton(eax, edx, 100, 83, &mouse2lh);

    DGOptnsProcBox(eax, edx, 74, 108, &SSAutoFire, 0);
    DGOptnsProcBox(eax, edx, 165, 108, &SSPause, 0);
}

static void DisplayGUIChipClick(s4 const eax, s4 const edx)
{
    GUIClickCButton(eax, edx, 9, 43, &nssdip1);
    GUIClickCButton(eax, edx, 59, 43, &nssdip2);
    GUIClickCButton(eax, edx, 109, 43, &nssdip3);
    GUIClickCButton(eax, edx, 9, 53, &nssdip4);
    GUIClickCButton(eax, edx, 59, 53, &nssdip5);
    GUIClickCButton(eax, edx, 109, 53, &nssdip6);
}

#ifdef __MSDOS__
#define PATH_LENGTH 256
#else
#define PATH_LENGTH 1024
#endif

static void DisplayGUIPathsClick(s4 const eax, s4 const edx)
{
    GUIPTabClick(eax, edx, 0, 51, 1, GUIPathTabs, (u4*)0);
    GUIPTabClick(eax, edx, 52, 86, 2, GUIPathTabs, (u4*)0);
    GUIPTabClick(eax, edx, 87, 157, 3, GUIPathTabs, (u4*)0);

    if (GUIPathTabs[0] == 1) { // General
        GUITextBoxInputNach(eax, edx, 8, 41, 237, 51, 0, PATH_LENGTH, init_save_paths); // SRAMPath
        GUITextBoxInputNach(eax, edx, 8, 76, 237, 86, 1, PATH_LENGTH, init_save_paths); // SStatePath
        GUITextBoxInputNach(eax, edx, 8, 111, 237, 121, 2, PATH_LENGTH, init_save_paths); // MoviePath
        GUITextBoxInputNach(eax, edx, 8, 146, 237, 156, 3, PATH_LENGTH, init_save_paths); // IPSPath

        GUIPButtonHole(eax, edx, 8, 178, &RelPathBase, 0);
        GUIPButtonHole(eax, edx, 88, 178, &RelPathBase, 1);
    }

    if (GUIPathTabs[0] == 2) { // More paths
        GUITextBoxInputNach(eax, edx, 8, 41, 237, 51, 0, PATH_LENGTH, init_save_paths); // SnapPath
        GUITextBoxInputNach(eax, edx, 8, 76, 237, 86, 1, PATH_LENGTH, init_save_paths); // SPCPath
        GUITextBoxInputNach(eax, edx, 8, 111, 237, 121, 2, PATH_LENGTH, init_save_paths); // CHTPath
        GUITextBoxInputNach(eax, edx, 8, 146, 237, 156, 3, PATH_LENGTH, init_save_paths); // ComboPath
        GUITextBoxInputNach(eax, edx, 8, 181, 237, 191, 4, PATH_LENGTH, init_save_paths); // INPPath
    }

    if (GUIPathTabs[0] == 3) { // BIOS+Carts
        GUITextBoxInputNach(eax, edx, 8, 41, 237, 51, 0, PATH_LENGTH, init_save_paths); // BSXPath
        GUITextBoxInputNach(eax, edx, 8, 76, 237, 86, 1, PATH_LENGTH, init_save_paths); // STPath
        GUITextBoxInputNach(eax, edx, 8, 111, 237, 121, 2, PATH_LENGTH, init_save_paths); // GNextPath
        GUITextBoxInputNach(eax, edx, 8, 146, 237, 156, 3, PATH_LENGTH, init_save_paths); // SGPath
    }
}

static void DisplayGUISaveClick(s4 const eax, s4 const edx)
{
    GUIClickCButton(eax, edx, 11, 38, &nosaveSRAM);
    GUIClickCButton(eax, edx, 11, 48, &SRAMSave5Sec);
    GUIClickCButton(eax, edx, 11, 58, &SRAMState);
    GUIClickCButton(eax, edx, 11, 68, &LatestSave);
    GUIClickCButton(eax, edx, 11, 78, &AutoIncSaveSlot);
    GUIClickCButton(eax, edx, 11, 88, &AutoState);
    GUIClickCButton(eax, edx, 11, 98, &PauseLoad);
    GUIClickCButton(eax, edx, 11, 108, &PauseRewind);

    GUIPHoldbutton2(eax, edx, 173, 17, 181, 24, 70, &RewindStates, 1, 99);
    GUIPHoldbutton2(eax, edx, 184, 17, 192, 24, 71, &RewindStates, -1, 0);
    GUIPHoldbutton2(eax, edx, 173, 29, 181, 36, 72, &RewindFrames, 1, 99);
    GUIPHoldbutton2(eax, edx, 184, 29, 192, 36, 73, &RewindFrames, -1, 1);

    DGOptnsProcBox(eax, edx, 27, 130, &KeyStateSlc0, 0);
    DGOptnsProcBox(eax, edx, 27 + 45, 130, &KeyStateSlc1, 0);
    DGOptnsProcBox(eax, edx, 27 + 45 * 2, 130, &KeyStateSlc2, 0);
    DGOptnsProcBox(eax, edx, 27 + 45 * 3, 130, &KeyStateSlc3, 0);
    DGOptnsProcBox(eax, edx, 27, 139, &KeyStateSlc4, 0);
    DGOptnsProcBox(eax, edx, 27 + 45, 139, &KeyStateSlc5, 0);
    DGOptnsProcBox(eax, edx, 27 + 45 * 2, 139, &KeyStateSlc6, 0);
    DGOptnsProcBox(eax, edx, 27 + 45 * 3, 139, &KeyStateSlc7, 0);
    DGOptnsProcBox(eax, edx, 27, 148, &KeyStateSlc8, 0);
    DGOptnsProcBox(eax, edx, 27 + 45, 148, &KeyStateSlc9, 0);
    DGOptnsProcBox(eax, edx, 27 + 45 * 2, 148, &KeyIncStateSlot, 0);
    DGOptnsProcBox(eax, edx, 27 + 45 * 3, 148, &KeyDecStateSlot, 0);
    DGOptnsProcBox(eax, edx, 8 + 25, 157, &KeySaveState, 0);
    DGOptnsProcBox(eax, edx, 8 + 57 + 25, 157, &KeyLoadState, 0);
    DGOptnsProcBox(eax, edx, 8 + 114 + 25, 157, &KeyStateSelct, 0);
    DGOptnsProcBox(eax, edx, 8 + 38, 166, &KeyRewind, 0);
}

static void DisplayGUISpeedClick(void)
{
    // This updates the mouse location, effectively 'grabbing' the cursor.
    s4 const eax = GUImouseposx - GUIwinposx[21];
    s4 const edx = GUImouseposy - GUIwinposy[21];

    GUIClickCButton(eax, edx, 11, 135, &FastFwdToggle); // Checkboxes
    GUIClickCButtonfr(eax, edx, 11, 145, &frameskip);

    if (frameskip == 0) {
        DGOptnsProcBox(eax, edx, 12, 78, &KeyEmuSpeedUp, 0);
        DGOptnsProcBox(eax, edx, 12, 98, &KeyEmuSpeedDown, 0);
    } else {
        DGOptnsProcBox(eax, edx, 12, 78, &KeyFRateUp, 0);
        DGOptnsProcBox(eax, edx, 12, 98, &KeyFRateDown, 0);
    }

    DGOptnsProcBox(eax, edx, 12, 58, &KeyFastFrwrd, 0); // Shortcut Boxes
    DGOptnsProcBox(eax, edx, 12, 68, &KeySlowDown, 0);
    DGOptnsProcBox(eax, edx, 12, 88, &KeyResetSpeed, 0);
    DGOptnsProcBox(eax, edx, 12, 108, &EMUPauseKey, 0);
    DGOptnsProcBox(eax, edx, 12, 118, &INCRFrameKey, 0);

    GUIPHoldbutton2(eax, edx, 118, 24, 126, 32, 74, &FFRatio, 1, 28);
    GUIPHoldbutton2(eax, edx, 129, 24, 137, 32, 75, &FFRatio, -1, 0);
    GUIPHoldbutton2(eax, edx, 118, 35, 126, 43, 76, &SDRatio, 1, 28);
    GUIPHoldbutton2(eax, edx, 129, 35, 137, 43, 77, &SDRatio, -1, 0);
    if (frameskip != 0) {
        GUIPHoldbutton2(eax, edx, 118, 13, 126, 21, 78, &frameskip, 1, 10);
        GUIPHoldbutton2(eax, edx, 129, 13, 137, 21, 79, &frameskip, -1, 1);
    } else {
        GUIPHoldbutton2(eax, edx, 118, 13, 126, 21, 78, &maxskip, 1, 9);
        GUIPHoldbutton2(eax, edx, 129, 13, 137, 21, 79, &maxskip, -1, 0);

        // Speed Slider
        if (7 <= eax && eax <= 7 + 116 && // X-Range for click-area
            173 <= edx && edx <= 177) // Y-Range for click-area
        {
            EmuSpeed = (u4)(eax - 7) / 2;
            GUIHold = 6; // Lock mouse to bar when clicked
            GUIHoldYlim = GUIwinposy[21] + 175;
            s4 const eax = GUIwinposx[21] + 7;
            GUIHoldXlimL = eax;
            GUIHoldXlimR = eax + 116;
        }
    }
}

static void GUIWindowMove(void)
{
    u1 const id = GUIwinorder[GUIwinptr - 1];
    u4 const rx = GUImouseposx - GUIwinposx[id];
    u4 const ry = GUImouseposy - GUIwinposy[id];
    switch (id) {
    case 3:
        DisplayGUIInputClick2(rx, ry);
        break;
    case 5:
        DisplayGUIVideoClick2(rx, ry);
        break;
    case 7:
        DisplayGUICheatClick2(rx, ry);
        break;
    case 13:
        DisplayGUICheatSearchClick2(rx, ry);
        break;
    case 16:
        DisplayGUIComboClick2(rx, ry);
        break;
    default:
        DisplayGUIConfirmClick2(rx, ry);
        break;
    }
}

static void GUIWinClicked(u4 const i, u4 const id)
{
    u4 const rx = GUImouseposx - GUIwinposx[id];
    u4 const ry = GUImouseposy - GUIwinposy[id];
    s4 const esi = rx - GUIwinsizex[id] + 10;
    if (0 <= esi && esi < 10 && 0 < ry && ry < 10) {
        GUIwinorder[i] = 0;
        GUIwinactiv[id] = 0;
        GUIInputBox = 0;
        --GUIwinptr;
        init_save_paths();
        SetMovieForcedLength();
#ifndef __MSDOS__
        SetCustomXY();
#endif
    } else if (ry < 10) {
        GUIHold = 1;
        GUIHoldxm = (short)GUIwinposx[id];
        GUIHoldym = (short)GUIwinposy[id];
        GUIHoldx = GUImouseposx;
        GUIHoldy = GUImouseposy;
    } else {
        GUIInputBox = 0;
        switch (id) {
        case 1:
            DisplayGUIConfirmClick(rx, ry);
            break;
        case 2:
            DisplayGUIChoseSaveClick(rx, ry);
            break;
        case 3:
            DisplayGUIInputClick(rx, ry);
            break;
        case 4:
            DisplayGUIOptionClick(rx, ry);
            break;
        case 5:
            DisplayGUIVideoClick(rx, ry);
            break;
        case 6:
            DisplayGUISoundClick();
            break;
        case 7:
            DisplayGUICheatClick(rx, ry);
            break;
        case 8:
            DisplayNetOptnsClick();
            break;
        case 9:
            DisplayGameOptnsClick(rx, ry);
            break;
        case 10:
            DisplayGUIOptnsClick();
            break;
        case 11:
            DisplayGUIAboutClick(rx, ry);
            break;
        case 12:
            DisplayGUIResetClick(rx, ry);
            break;
        case 13:
            DisplayGUICheatSearchClick(rx, ry);
            break;
        case 14:
            DisplayGUIStatesClick(rx, ry);
            break;
        case 15:
            DisplayGUIMovieClick(rx, ry);
            break;
        case 16:
            DisplayGUIComboClick(rx, ry);
            break;
        case 17:
            DisplayGUIAddOnClick(rx, ry);
            break;
        case 18:
            DisplayGUIChipClick(rx, ry);
            break;
        case 19:
            DisplayGUIPathsClick(rx, ry);
            break;
        case 20:
            DisplayGUISaveClick(rx, ry);
            break;
        case 21:
            DisplayGUISpeedClick();
            break;
        default:
            break;
        }
    }
}

static void ProcessMouseButtons(void)
{
    static u1 GUIOnMenuItm;

    u2 const x = GUImouseposx;
    u2 const y = GUImouseposy;

    if (MouseMoveOkay == 1) {
        GUIOnMenuItm = 0;
        if (GUIMenuL < x && x < GUIMenuR && 18 < y && y < GUIMenuD) {
            // Mouse Menu
            GUIOnMenuItm = 2;
            u4 const row = (y - 18) / 10;
            if (GUICYLocPtr[row + 1] != 0) {
                GUIcrowpos = row;
                GUIOnMenuItm = 1;
            }
        }
    }

    // Check if mouse is clicked on menu
    u2 const buttons = mousebuttonstat;
    if (GUIRClick == 1) {
        if (buttons & 0x02) {
            if (romloadskip != 0 || MousePRClick != 0)
                goto norclick2;
            GUIQuit = 2;
        }
        MousePRClick = 0;
    norclick2:;
    }
    if (buttons & 0x01) {
        if (GUIpclicked != 1 && GUIOnMenuItm != 2) {
            if (GUIOnMenuItm == 1) {
                GUITryMenuItem();
            } else if (y <= 15) {
                if (3 <= y && y <= 14) {
                    if (233 <= x && x <= 242) {
                        GUIcwinpress =
#ifndef __UNIXSDL__
                            y > 8 ? 3 :
#endif
                                  1;
                        goto noclick;
                    } else if (244 <= x && x <= 253) {
                        GUIcwinpress = 2;
                        goto noclick;
                    }
                }

                if (3 <= y && y <= 13) {
                    if (4 <= x && x <= 12) {
                        GUIcmenupos = 1;
                        GUIcrowpos = 0;
                        goto noclick;
                    } else if (17 <= x && x <= 47) {
                        GUIcmenupos = 2;
                        GUIcrowpos = 0;
                        goto noclick;
                    } else if (52 <= x && x <= 94) {
                        GUIcmenupos = 3;
                        GUIcrowpos = 0;
                        goto noclick;
                    } else if (99 <= x && x <= 135) {
                        GUIcmenupos = 4;
                        GUIcrowpos = 0;
                        goto noclick;
                    } else if (140 <= x && x <= 188) {
                        GUIcmenupos = 5;
                        GUIcrowpos = 0;
                        goto noclick;
                    } else if (193 <= x && x <= 223) {
                        GUIcmenupos = 6;
                        GUIcrowpos = 0;
                        goto noclick;
                    } else if (224 <= x) {
                        goto noclick;
                    }
                }

#ifndef __MSDOS__
                GUIpclicked = 1;
                GUIHold = 255;
                GUIHoldYlim = y;
                GUIHoldXlimL = x;
                MouseWindow();
#endif
            } else if (GUIcmenupos == 0) {
                GUIpclicked = 1;
                u4 i = GUIwinptr;
                if (i != 0) {
                    u1 const id = GUIwinorder[--i];
                    if (GUIwinposx[id] < x && x < GUIwinposx[id] + GUIwinsizex[id] && GUIwinposy[id] < y && y < GUIwinposy[id] + GUIwinsizey[id] + 10) {
                        GUIWinClicked(i, id);
                        return;
                    }
                    while (i != 0) {
                        u1 const id = GUIwinorder[--i];
                        if (x <= GUIwinposx[id] || GUIwinposx[id] + GUIwinsizex[id] <= x)
                            continue;
                        if (y <= GUIwinposy[id] || GUIwinposy[id] + GUIwinsizey[id] + 10 <= y)
                            continue;
                        // Shift all following windows downwards by 1
                        while (++i != GUIwinptr)
                            GUIwinorder[i - 1] = GUIwinorder[i];
                        GUIwinorder[i - 1] = id;
                        GUIpclicked = 0;
                        return;
                    }
                }
                if (SantaPos != 272 && ShowTimer == 0) {
                    MsgGiftLeft = 36 * 4;
                    ShowTimer = 1;
                }
                return;
            } else {
                GUIpmenupos = GUIcmenupos;
                GUIcmenupos = 0;
            }
        }

    noclick:;
        GUIpclicked = 1;
        switch (GUIHold) {
        case 2: { // Colour Slide Bar Hold
            GUImouseposy = GUIHoldYlim;
            u4 const minx = GUIHoldXlimL;
            if (x < minx)
                GUImouseposx = minx;
            u4 const maxx = GUIHoldXlimR;
            if (x > maxx)
                GUImouseposx = maxx;
            lastmouseholded = 1;
            DisplayGUIOptnsClick();
            return;
        }

        case 3: { // Box Hold
            u4 const miny = GUIHoldYlim;
            if (y <= miny)
                GUImouseposy = miny;
            u4 const minx = GUIHoldXlimL;
            if (x <= minx)
                GUImouseposx = minx;
            u4 const maxy = GUIHoldYlimR;
            if (y >= maxy)
                GUImouseposy = maxy;
            u4 const maxx = GUIHoldXlimR;
            if (x >= maxx)
                GUImouseposx = maxx;
            lastmouseholded = 1;
            GUIWindowMove();
            return;
        }

        case 4: {
            u1 const id = GUIwinorder[GUIwinptr - 1];
            u4 const rx = x - GUIwinposx[id];
            u4 const ry = y - GUIwinposy[id];
            GUICBHold = GUIHoldXlimL <= rx && rx <= GUIHoldXlimR && GUIHoldYlim <= ry && ry <= GUIHoldYlimR ? GUICBHold2 : 0;
            return;
        }

        case 5: { // Sound Slide Bar Hold
            GUImouseposy = GUIHoldYlim;
            u4 const minx = GUIHoldXlimL;
            if (x < minx)
                GUImouseposx = minx;
            u4 const maxx = GUIHoldXlimR;
            if (x > maxx)
                GUImouseposx = maxx;
            lastmouseholded = 1;
            DisplayGUISoundClick();
            return;
        }

        case 6: { // Speed Slide Bar Hold
            GUImouseposy = GUIHoldYlim;
            u4 const minx = GUIHoldXlimL;
            if (x < minx)
                GUImouseposx = minx;
            u4 const maxx = GUIHoldXlimR;
            if (x > maxx)
                GUImouseposx = maxx;
            lastmouseholded = 1;
            DisplayGUISpeedClick();
            return;
        }

        case 7:
        case 8: { // Video Slide Bar Hold
            GUImouseposy = GUIHoldYlim;
            u4 const minx = GUIHoldXlimL;
            if (x < minx)
                GUImouseposx = minx;
            u4 const maxx = GUIHoldXlimR;
            if (x > maxx)
                GUImouseposx = maxx;
            lastmouseholded = 1;
            DisplayGUIVideoClick(0, 0); // XXX eax and edx do not seem to hold sensible values here
            return;
        }

        case 1: {
        hold:
            if (GUImouseposy < 16) {
                if (mousewrap & 1) {
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

        case 255: {
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
    GUICHold = 0;
    GUIpclicked = 0;
    GUIHold = 0;

#if defined __UNIXSDL__ || defined __WIN32__
    if (GUIcwinpress == 1) {
        GUIcwinpress = 0;
#ifdef __UNIXSDL__
        if (3 <= y && y <= 13)
#else
        if (3 <= y && y <= 7)
#endif
        {
            if (233 <= x && x <= 242) {
                SwitchFullScreen();
                return;
            }
        }
    }
#endif

#ifndef __MSDOS__
    if (GUIcwinpress == 2) {
        GUIcwinpress = 0;
        if (3 <= y && y <= 13 && 44 <= x && x <= 253) {
            GUIQuit = 1;
            return;
        }
    }

    if (GUIcwinpress == 3) {
        GUIcwinpress = 0;
        if (9 <= y && y <= 13 && 233 <= x && x <= 242) {
#ifdef __WIN32__
            MinimizeWindow();
#endif
            return;
        }
    }
#endif

    // ButtonProcess
    switch (GUICBHold) {
    case 1:
        GUILoadData();
        return;
    case 2:
        GUIProcReset();
        return;
    case 3:
        GUIProcReset();
        return;
    case 4:
        GUIProcVideo();
        return; // set video mode
#ifndef __MSDOS__
    case 12:
        GUIProcCustomVideo();
        return; // set custom video mode
#endif
    case 37:
    case 38:
    case 39:
        GUINTSCReset();
        return; // reset ntsc options
    case 81:
    case 82:
    case 83:
    case 84:
        GUINTSCPreset();
        return; // ntsc preset
    case 10:
    case 11:
        GUIProcStates();
        return;
    case 5:
        CheatCodeRemove();
        return;
    case 6:
        CheatCodeToggle();
        return;
    case 7:
        CheatCodeSave();
        return;
    case 8:
        CheatCodeLoad();
        return;
    case 9:
        ProcessCheatCode();
        return;
    case 33:
        CheatCodeFix();
        return;
    case 14:
        SetDevice();
        return;
    case 15:
        CalibrateDev1();
        return;

    case 16:
        GUICBHold = 0;
        MoviePlay();
        return; // movie replay
    case 17:
        GUICBHold = 0;
        MovieRecord();
        return; // movie record
    case 18:
        GUICBHold = 0;
        MovieStop();
        return; // movie stop
    case 19:
        GUICBHold = 0;
        MovieRecord();
        return; // overwrite zmv ? yes
    case 20:
        GUICBHold = 0;
        SkipMovie();
        return; // overwrite zmv ? no
    case 29:
        GUICBHold = 0;
        MovieInsertChapter();
        return; // insert chapter

    case 30:
        GUICBHold = 0;
        GUIQuit = 2;
        MovieSeekBehind();
        return; // back to previous chapter
    case 31:
        GUICBHold = 0;
        GUIQuit = 2;
        MovieSeekAhead();
        return; // jump to next chapter
    case 32:
        GUICBHold = 0;
        GUIQuit = 2;
        MovieAppend();
        return; // append movie

    case 34: // dump raw
    {
        GUICBHold = 0;
        GUIQuit = 2;
        SetMovieForcedLength();
        MovieDumpRaw();
        if (MovieVideoMode >= 2 && mencoderExists == 0) {
            guimencodermsg();
        }
        if (MovieAudio != 0 && MovieAudioCompress != 0 && lameExists == 0) {
            guilamemsg();
        }
        return;
    }

    case 35:
        GUICBHold = 0;
        GUIQuit = 2;
        MovieStop();
        return; // stop dump
    case 40:
        SetAllKeys();
        return;

    case 50:
        CheatCodeSearchInit();
        break;

    case 60:
        GUIComboTextH[0] = '\0';
        GUINumCombo = 0;
        GUIComboKey = 0;
        break;

    case 61:
        if (NumCombo != 50)
            ComboAdder();
        break;
    case 62:
        if (NumCombo != 0)
            ComboReplace();
        break;
    case 63:
        if (NumCombo != 0)
            ComboRemoval();
        break;

    case 51:
        CheatWinMode = 0;
        CheatSearchStatus = 0;
        break;

    case 52:
        CheatWinMode = 2;
        break;
    case 53:
        CheatCodeSearchProcess();
        break;
    case 54:
        CheatWinMode = 1;
        break;

    case 55:
        if (NumCheatSrc != 0) {
            CheatWinMode = 3;
            CurCStextpos = 0;
            CSInputDisplay[0] = '_';
            CSInputDisplay[1] = '\0';
            CSDescDisplay[0] = '\0';
        }
        break;

    case 56:
        CheatWinMode = 2;
        break;
    case 57:
        AddCSCheatCode();
        break;

#ifndef __MSDOS__
    case 65:
        ZsnesPage();
        break;
    case 66:
        DocsPage();
        break;
#endif
    }
    GUICBHold = 0;
}

void ProcessMouse(void)
{
    // Process holds
    if (LastHoldEnable != GUIHold) {
        switch (GUIHold) {
        case 0:
            if (LastHoldEnable == 7) // ntsc sliders
            {
                u4 const idx = ntscWhVar;
                u1 const cur = ntscCurVar;
                if (ntscLastVar[idx] != cur) {
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
            Set_MouseXMax(0, 255);
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
            Set_MouseYMax(GUIHoldYlim, GUIHoldYlimR);
            break;
        }
        LastHoldEnable = GUIHold;
    }
    MouseMoveOkay = 0;
    u4 buttons = Get_MouseData() & 0xFFFF;
    if (lhguimouse == 1)
        buttons = SwapMouseButtons(buttons);
    mousebuttonstat = buttons;
    if (lastmouseholded != 0 && !(buttons & 0x01)) {
        lastmouseholded = 0;
        Set_MousePosition(GUImouseposx, GUImouseposy);
    }
    if (mousewrap == 1) {
        u4 const delta = Get_MousePositionDisplacement();

        u2 x = GUImouseposx + delta;
        while (x & 0x8000)
            x += 256;
        while (x > 255)
            x -= 256;
        GUImouseposx = x;

        u2 y = GUImouseposy + (delta >> 16);
        while (y & 0x8000)
            y += 224;
        while (y > 223)
            y -= GUIHold == 1 ? 224 - 16 : 224;
        GUImouseposy = y;

        if (delta != 0)
            MouseMoveOkay = 1;
    } else {
        u4 const data = Get_MouseData();
        u2 x = data >> 16 & 0xFF;
        u2 y = data >> 24;
        if (GUImouseposx != x || GUImouseposy != y)
            MouseMoveOkay = 1;
        if (x & 0x8000)
            x = 0;
        if (x > 255)
            x = 255;
        GUImouseposx = x;
        if (y & 0x8000)
            y = 0;
        if (y > 223)
            y = 100;
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
    do
        JoyRead();
    while (!(key = GetAnyPressedKey()));
    for (u1* i = pressed; i != endof(pressed); ++i)
        if (*i != 0)
            *i = 2;
    while (Check_Key() != 0)
        Get_Key();
    return key - pressed;
}

void guipresstestb(u4* const guicpressptr, char const* const guipressptr)
{
    GUIUnBuffer();
    DisplayBoxes();
    DisplayMenu();
    GUIBox3D(65, 80, 194, 126);
    GUIOuttextShadowed(70, 85, guipresstext1);
    GUIOuttextShadowed(70, 95, guipresstext2);
    GUIOuttextShadowed(70, 105, guipressptr);
    GUIOuttextShadowed(70, 115, "(ESC TO SKIP)");
    vidpastecopyscr();
    delay(8192);
    do
        JoyRead();
    while (GetAnyPressedKey());

    u1* key;
    do
        JoyRead();
    while (!(key = GetAnyPressedKey()));
    u4 const key_id = key - pressed;
    while (Check_Key() != 0)
        Get_Key();
    if (key_id != 1 && key_id != 0x3B)
        *guicpressptr = key_id;
}

void DrawMouse(void)
{
    static u1 const GUIMousePtr[] = {
        50, 47, 45, 43, 40, 0, 0, 0,
        53, 52, 46, 42, 0, 0, 0, 0,
        55, 54, 54, 44, 0, 0, 0, 0,
        57, 57, 56, 52, 45, 0, 0, 0,
        59, 0, 0, 55, 50, 45, 0, 0,
        0, 0, 0, 0, 55, 50, 45, 0,
        0, 0, 0, 0, 0, 55, 50, 47,
        0, 0, 0, 0, 0, 0, 52, 0
    };

    u1* dst = vidbuffer + 16 + GUImouseposx + GUImouseposy * 288;
    u1 const* src = GUIMousePtr;
    u4 y = 8;
    do {
        u4 x = 8;
        do {
            u1 const al = *src++;
            if (al == 0)
                continue;

            if (mouseshad & 1) {
                u1* const px = &dst[288 * 10 + 8];
                if (*px < 32) {
                    *px = 96;
                } else {
                    u1* const px = &dst[288 * 4 + 3];
                    u1 const c = *px;
                    if (32 <= c && c <= 63) {
                        *px = (c - 32U) / 2 + 32U;
                    } else if ((c & 0xF0) == 64) {
                        *px = c + 16;
                    } else {
                        u1* const px = &dst[288 * 7 + 5];
                        u1 const c = *px;
                        if (148 <= c && c <= 167) {
                            *px = c + 20;
                        } else if (189 <= c && c <= 220) {
                            *px = (c - 189U) / 2 + 189U;
                        }
                    }
                }
            }
            *dst = al + 88;
        } while (++dst, --x != 0);
        dst += 288 - 8;
    } while (--y != 0);
}
