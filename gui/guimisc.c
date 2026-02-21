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

#include <string.h>

#include "../c_intrf.h"
#include "../cpu/dspproc.h"
#include "../cpu/execute.h"
#include "../cpu/regs.h"
#include "../cpu/spc700.h"
#include "../gblvars.h"
#include "../initc.h"
#include "../input.h"
#include "../macros.h"
#include "../video/procvidc.h"
#include "../zmovie.h"
#include "../zstate.h"
#include "c_gui.h"
#include "c_guiwindp.h"
#include "gui.h"
#include "guikeys.h"
#include "guimisc.h"
#include "guimouse.h"

#define ConfigureKey2(i, player)            \
    do {                                    \
        switch (i) {                        \
        case 0:                             \
            guicpressptr = &player##upk;    \
            break; /* Up     */             \
        case 1:                             \
            guicpressptr = &player##downk;  \
            break; /* Down   */             \
        case 2:                             \
            guicpressptr = &player##leftk;  \
            break; /* Left   */             \
        case 3:                             \
            guicpressptr = &player##rightk; \
            break; /* Right  */             \
        case 4:                             \
            guicpressptr = &player##startk; \
            break; /* Start  */             \
        case 5:                             \
            guicpressptr = &player##selk;   \
            break; /* Select */             \
        case 6:                             \
            guicpressptr = &player##Ak;     \
            break; /* A      */             \
        case 7:                             \
            guicpressptr = &player##Bk;     \
            break; /* B      */             \
        case 8:                             \
            guicpressptr = &player##Xk;     \
            break; /* X      */             \
        case 9:                             \
            guicpressptr = &player##Yk;     \
            break; /* Y      */             \
        case 10:                            \
            guicpressptr = &player##Lk;     \
            break; /* L      */             \
        case 11:                            \
            guicpressptr = &player##Rk;     \
            break; /* R      */             \
        }                                   \
    } while (0)

void SetAllKeys(void)
{
    static char const guipresstext4b[][21] = {
        "FOR UP              ",
        "FOR DOWN            ",
        "FOR LEFT            ",
        "FOR RIGHT           ",
        "FOR START           ",
        "FOR SELECT          ",
        "FOR A (RIGHT BUTTON)",
        "FOR B (DOWN BUTTON) ",
        "FOR X (TOP BUTTON)  ",
        "FOR Y (LEFT BUTTON) ",
        "FOR THE L BUTTON    ",
        "FOR THE R BUTTON    "
    };

    memset(pressed, 0, sizeof(pressed));

    GUICBHold = 0;

    u1 const* keycontrolval;
    switch (cplayernum) {
    default:
        keycontrolval = &pl1contrl;
        break;
    case 1:
        keycontrolval = &pl2contrl;
        break;
    case 2:
        keycontrolval = &pl3contrl;
        break;
    case 3:
        keycontrolval = &pl4contrl;
        break;
    case 4:
        keycontrolval = &pl5contrl;
        break;
    }

    // Check if controller is set
    if (*keycontrolval == 0)
        return; // XXX original compares dword instead of byte, former makes no sense
    u4 i = 0;
    do {
        u4* guicpressptr;
        switch (cplayernum) {
        case 0:
            ConfigureKey2(i, pl1);
            break;
        case 1:
            ConfigureKey2(i, pl2);
            break;
        case 2:
            ConfigureKey2(i, pl3);
            break;
        case 3:
            ConfigureKey2(i, pl4);
            break;
        case 4:
            ConfigureKey2(i, pl5);
            break;
        }
        guipresstestb(guicpressptr, guipresstext4b[i]);
    } while (++i != lengthof(guipresstext4b));
}

#undef ConfigureKey2

void CalibrateDev1(void)
{
    GUICBHold = 0;
}

void SetDevice(void)
{
    GUICBHold = 0;
    u4 const player = cplayernum;
    u4 const contrl = GUIcurrentinputcursloc;
    *GUIInputRefP[player] = contrl;
    {
        CalibXmin = 0;
        SetInputDevice(contrl, player);
    }
    UpdateDevices();
    MultiTap = SFXEnable != 1 && (pl3contrl != 0 || pl4contrl != 0 || pl5contrl != 0);
}

void GUIDoReset(void)
{
    Clear2xSaIBuffer();

    MovieStop();
    RestoreSystemVars();

    // reset the snes
    init65816();
    procexecloop();

    spcPCRam = SPCRAM + 0xFFC0;
    spcS = 0x1EF;
    spcRamDP = SPCRAM;
    spcA = 0;
    spcX = 0;
    spcY = 0;
    spcP = 0;
    spcNZ = 0;
    GUIQuit = 2;
    memset(&Voice0Status, 0, sizeof(Voice0Status));
}
