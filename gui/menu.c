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

#include "../asm.h"
#include "../asm_call.h"
#include "../c_intrf.h"
#include "../c_vcache.h"
#include "../cfg.h"
#include "../cpu/c_65816d.h"
#include "../cpu/c_execute.h"
#include "../cpu/c_memory.h"
#include "../cpu/execute.h"
#include "../cpu/memory.h"
#include "../cpu/memtable.h"
#include "../cpu/regs.h"
#include "../cpu/spc700.h"
#include "../debugger.h"
#include "../endmem.h"
#include "../init.h"
#include "../initc.h"
#include "../ui.h"
#include "../vcache.h"
#include "../video/procvid.h"
#include "../video/procvidc.h"
#include "../zip/zpng.h"
#include "../zstate.h"
#include "gui.h"
#include "guifuncs.h"
#include "menu.h"

#if !defined NO_PNG && defined __MSDOS__
#include "../dos/dosintrf.h"
#endif

u1 NoInputRead;
u1 SPCSave;
u1 keyonsn;
u1 nextmenupopup;

static u1 PrevMenuPos;
static u1 MenuNoExit;
static u4 MenuDisplace;
static u4 MenuDisplace16;
static u1 menu16btrans;
static u4 menucloc;
static u2 allred;

static char const menudrawbox_string[] = "MISC OPTIONS";
static char const menudrawbox_stringa[] = "SAVE SNAPSHOT";
static char const menudrawbox_stringb[] = "SHOW FPS";
static char const menudrawbox_stringc[] = "HIDE FPS";
static char const menudrawbox_stringd[] = "SAVE SPC DATA";
static char const menudrawbox_stringe[] = "SOUND BUFFER DUMP";
static char const menudrawbox_stringf[] = "SNAPSHOT/INCR FRM";
static char const menudrawbox_stringg[] = "INCR FRAME ONLY";
static char const menudrawbox_stringh[] = "MOVE THIS WINDOW";
static char menudrawbox_stringi[] = "IMAGE FORMAT: ---";

static void GUIBufferData(void)
{
    // copy to spritetable
    u4 const n =
#ifdef __MSDOS__
        cbitmode != 1 ? 64000 :
#endif
                      129536;
    memcpy(spritetablea + 4 * 384, vidbuffer + 4 * 384, n);
    memset(sprlefttot, 0, sizeof(sprlefttot));
    memset(sprleftpr, 0, sizeof(sprleftpr));
    memset(sprleftpr1, 0, sizeof(sprleftpr1));
    memset(sprleftpr2, 0, sizeof(sprleftpr2));
    memset(sprleftpr3, 0, sizeof(sprleftpr3));
}

static void GUIUnBuffer(void)
{
    // copy from spritetable
    u4 const n =
#ifdef __MSDOS__
        cbitmode != 1 ? 64000 :
#endif
                      129536;
    memcpy(vidbuffer + 4 * 384, spritetablea + 4 * 384, n);
}

static void menudrawcursor16b(void)
{
    // draw a small red box
    u2* buf = (u2*)vidbuffer + MenuDisplace16 / 2 + menucloc + 41 + 34 * 288;
    u2 const c = allred;
    u4 h = 9;
    do {
        u4 w = 148;
        do
            *buf++ = c;
        while (--w != 0);
        buf += 288 - 148;
    } while (--h != 0);
}

static void menudrawbox16b(void)
{
    if (menu16btrans == 0) { // draw shadow behind box
        menu16btrans = 1;
        u2* buf = (u2*)vidbuffer + MenuDisplace16 / 2 + 50 + 30 * 288;
        u4 h = 95;
        do {
            u4 w = 150;
            do {
                *buf = (*buf & vesa2_clbit) >> 1;
                ++buf;
            } while (--w != 0);
            buf += 288 - 150;
        } while (--h != 0);
    }

    allred = 0x1F << vesa2_rpos;

    // draw a small blue box with a white border
    u2* buf = (u2*)vidbuffer + MenuDisplace16 / 2 + 40 + 20 * 288;
    u2 c = 0x12 << vesa2_bpos;
    u2 const dc = 0x01 << vesa2_gpos | 0x01 << vesa2_rpos;
    u4 step = 5;
    u4 h = 95;
    do {
        u4 w = 150;
        do
            *buf++ = c;
        while (--w != 0);
        buf += 288 - 150;
        if (--step == 0) {
            c += dc;
            step = 5;
        }
    } while (--h != 0);

    // Draw lines
    drawhline16b((u2*)vidbuffer + MenuDisplace16 / 2 + 40 + 20 * 288, 150, 0xFFFF);
    drawvline16b((u2*)vidbuffer + MenuDisplace16 / 2 + 40 + 20 * 288, 95, 0xFFFF);
    drawhline16b((u2*)vidbuffer + MenuDisplace16 / 2 + 40 + 114 * 288, 150, 0xFFFF);
    drawhline16b((u2*)vidbuffer + MenuDisplace16 / 2 + 40 + 32 * 288, 150, 0xFFFF);
    drawvline16b((u2*)vidbuffer + MenuDisplace16 / 2 + 189 + 20 * 288, 95, 0xFFFF);
    menudrawcursor16b();

    OutputGraphicString16b((u2*)vidbuffer + MenuDisplace16 / 2 + 45 + 23 * 288, menudrawbox_string);
    OutputGraphicString16b((u2*)vidbuffer + MenuDisplace16 / 2 + 45 + 35 * 288, menudrawbox_stringa);
    OutputGraphicString16b((u2*)vidbuffer + MenuDisplace16 / 2 + 45 + 45 * 288, FPSOn & 1 ? menudrawbox_stringc : menudrawbox_stringb);
    OutputGraphicString16b((u2*)vidbuffer + MenuDisplace16 / 2 + 45 + 55 * 288, menudrawbox_stringd);
    OutputGraphicString16b((u2*)vidbuffer + MenuDisplace16 / 2 + 45 + 65 * 288, menudrawbox_stringe);
    OutputGraphicString16b((u2*)vidbuffer + MenuDisplace16 / 2 + 45 + 75 * 288, menudrawbox_stringf);
    OutputGraphicString16b((u2*)vidbuffer + MenuDisplace16 / 2 + 45 + 85 * 288, menudrawbox_stringg);
    OutputGraphicString16b((u2*)vidbuffer + MenuDisplace16 / 2 + 45 + 95 * 288, menudrawbox_stringh);
    OutputGraphicString16b((u2*)vidbuffer + MenuDisplace16 / 2 + 45 + 105 * 288, menudrawbox_stringi);
    copyvid();
}

#ifdef __MSDOS__
static void menudrawcursor8b(void)
{
    if (cbitmode != 1) // XXX always true due to caller
    {
        // draw a small red box
        u1* buf = vidbuffer + MenuDisplace + menucloc + 41 + 34 * 288;
        u4 h = 9;
        do {
            memset(buf, 160, 148);
            buf += 288;
        } while (--h != 0);
    } else {
        menudrawcursor16b();
    }
}
#endif

static void menudrawbox8b(void)
{
#ifdef __MSDOS__
    if (cbitmode != 1) {
        // draw a small blue box with a white border
        u1* buf = vidbuffer + MenuDisplace + 40 + 20 * 288;
        u4 h = 95;
        do {
            memset(buf, 144, 150);
            buf += 288;
        } while (--h != 0);

        // Draw lines
        drawhline(vidbuffer + MenuDisplace + 40 + 20 * 288, 150, 128);
        drawvline(vidbuffer + MenuDisplace + 40 + 20 * 288, 95, 128);
        drawhline(vidbuffer + MenuDisplace + 40 + 114 * 288, 150, 128);
        drawhline(vidbuffer + MenuDisplace + 40 + 32 * 288, 150, 128);
        drawvline(vidbuffer + MenuDisplace + 189 + 20 * 288, 95, 128);
        menudrawcursor8b();

        OutputGraphicString(vidbuffer + MenuDisplace + 45 + 23 * 288, menudrawbox_string);
        OutputGraphicString(vidbuffer + MenuDisplace + 45 + 35 * 288, menudrawbox_stringa);
        OutputGraphicString(vidbuffer + MenuDisplace + 45 + 45 * 288, FPSOn & 1 ? menudrawbox_stringc : menudrawbox_stringb);
        OutputGraphicString(vidbuffer + MenuDisplace + 45 + 55 * 288, menudrawbox_stringd);
        OutputGraphicString(vidbuffer + MenuDisplace + 45 + 65 * 288, menudrawbox_stringe);
        OutputGraphicString(vidbuffer + MenuDisplace + 45 + 75 * 288, menudrawbox_stringf);
        OutputGraphicString(vidbuffer + MenuDisplace + 45 + 85 * 288, menudrawbox_stringg);
        OutputGraphicString(vidbuffer + MenuDisplace + 45 + 95 * 288, menudrawbox_stringh);
        OutputGraphicString(vidbuffer + MenuDisplace + 45 + 105 * 288, menudrawbox_stringi);
        copyvid();
    } else
#endif
    {
        menudrawbox16b();
    }
}

static void saveimage(void)
{
    pressed[1] = 0;
    pressed[59] = 0;

#ifndef NO_PNG
    if (ScreenShotFormat == 1) {
        Grab_PNG_Data();
        return;
    }
#endif
#ifdef __MSDOS__
    if (cbitmode != 1) {
        Grab_BMP_Data_8();
    } else
#endif
    {
        Grab_BMP_Data();
    }
}

static void breakatsignb(void)
{
    keyonsn = 0;
#ifndef NO_DEBUGGER
    if (SPCSave == 1)
        debuggeron = 1;
#endif

    exiter = 1;
    u4 eax = xpc;
    u4 ebx = xpb;
    u1* esi = eax & 0x8000 ? snesmmap[ebx] : eax < 0x4300 || memtabler8[ebx] != regaccessbankr8 ? snesmap2[ebx]
                                                                                                : (u1*)dmadata - 0x4300; // XXX ugly cast
    initaddrl = esi;
    esi += eax; // add program counter to address
    u1* ebp = spcPCRam;
    u4 edx = curcyc /* cycles */ << 8 | xp /* flags */;
    eop** edi = Curtableaddr;
    UpdateDPage();
    // execute
    do {
        splitflags(edx);
        execute(&edx, &ebp, &esi, &edi);
        edx = joinflags(edx);
        edx = edx & 0xFFFF00FF | pdh << 8;

#ifndef NO_DEBUGGER
        if ((++numinst & 0xFF) == 0 && Check_Key() != 0 && Get_Key() == 27)
            break;
#endif
        if (SPCRAM[6] == 0x40)
            break;
    } while (keyonsn != 1);

    // copy back data
    spcPCRam = ebp;
    Curtableaddr = edi;
    xp = edx;
    curcyc = edx >> 8;
    xpc = esi - initaddrl; // subtract program counter by address
    exiter = 0;

#ifndef NO_DEBUGGER
    if (SPCSave == 1)
        debuggeron = 0;
#endif
}

#ifdef __MSDOS__
static inline void SetPal(u1 const i, u1 const r, u1 const g, u1 const b)
{
    outb(0x03C8, i);
    outb(0x03C9, r);
    outb(0x03C9, g);
    outb(0x03C9, b);
}
#endif

void showmenu(void)
{
    for (;;) {
#ifdef __MSDOS__
        if (cbitmode != 1) {
            u1* buf = vidbuffer + 100000;
            outb(0x03C7, 0);
            *buf++ = 12;
            u4 n = 768;
            do
                *buf++ = inb(0x03C9) << 2;
            while (--n != 0);

            // set palette of colors 128,144, and 160 to white, blue, and red
            SetPal(128, 63, 63, 63);
            SetPal(144, 0, 0, 50);
            SetPal(160, 45, 0, 0);
        }
#endif

        ForceNonTransp = 1;
        NoInputRead = 0;
        if (SSKeyPressed == 1) {
            SSKeyPressed = 0;
            saveimage();
        } else if (SPCKeyPressed == 1) {
            goto savespckey;
        } else if (pressed[14] & 1) {
            saveimage();
        } else {
            menucloc = 0;
            if (nextmenupopup != 0) {
                pressed[0x1C] = 0;
                switch (PrevMenuPos) {
                default:
                    menucloc = 40 * 288;
                    break;
                case 1:
                    menucloc = 50 * 288;
                    break;
                case 2:
                    menucloc = 60 * 288;
                    break;
                }
            }
            if (PrevMenuPos == 3)
                menucloc = 70 * 288;

            char const* fmt = " BMP";
#ifndef NO_PNG
            if (ScreenShotFormat != 0) {
#ifdef __MSDOS__
                if (GUI16VID[cvidmode] != 1) {
                    ScreenShotFormat = 0;
                } else
#endif
                {
                    fmt = " PNG";
                }
            }
#endif
            memcpy(menudrawbox_stringi + 13, fmt, 4);

            nextmenupopup = 0;
            menu16btrans = 0;
            pressed[1] = 0;
            pressed[59] = 0;
            curblank = 0;
            GUIBufferData();
            // Draw box
            menudrawbox8b();
            menudrawbox8b(); // XXX twice?
            if (newengen != 0)
                GUIOn = 1;
            copyvid();
            StopSound();
            for (;;) {
                // GUIUnBuffer();
                menudrawbox8b();
                copyvid();

                JoyRead();
                if (Check_Key() == 0)
                    continue;
                u1 const key = Get_Key();
                if (key == 0) {
                    u1 const ext = Get_Key();
                    if (ext == 72) {
                        if (menucloc == 0)
                            menucloc += 80 * 288;
                        menucloc -= 10 * 288;
                        menudrawbox8b();
                    } else if (ext == 80) {
                        if (menucloc == 70 * 288)
                            menucloc -= 80 * 288;
                        menucloc += 10 * 288;
                        menudrawbox8b();
                        copyvid();
                    }
                } else if (key == 27)
                    goto exitloop;
                else if (key == 13)
                    break;
            }
            GUIUnBuffer();
            copyvid();
            if (menucloc == 0)
                saveimage();
            if (menucloc == 40 * 288) {
                saveimage();
                ExecExitOkay = 0;
                nextmenupopup = 3;
                NoInputRead = 1;
                t1cc = 0;
                PrevMenuPos = 0;
            }
            if (menucloc == 50 * 288) {
                ExecExitOkay = 0;
                nextmenupopup = 3;
                NoInputRead = 1;
                t1cc = 0;
                PrevMenuPos = 1;
            }
            if (menucloc == 70 * 288) {
#ifdef __MSDOS__
                if (cbitmode != 0)
#endif
                {
                    ScreenShotFormat ^= 1;
                    MenuNoExit = 1;
                    ExecExitOkay = 0;
                    nextmenupopup = 1;
                    NoInputRead = 1;
                    t1cc = 0;
                    PrevMenuPos = 3;
                }
            }
            if (menucloc == 60 * 288) {
                MenuNoExit = 1;
                ExecExitOkay = 0;
                nextmenupopup = 1;
                NoInputRead = 1;
                t1cc = 0;
                PrevMenuPos = 2;
                if (MenuDisplace != 0) {
                    MenuDisplace = 0;
                    MenuDisplace16 = 0;
                } else {
                    MenuDisplace = 90 * 288;
                    MenuDisplace16 = 90 * 288 * 2;
                }
            }
            if (menucloc == 10 * 288) {
                if (frameskip != 0) {
                    Msgptr = "NEED AUTO FRAMERATE ON";
                    MessageOn = MsgCount;
                } else {
                    FPSOn ^= 1;
                }
            }
            if (menucloc == 20 * 288) {
            savespckey:
                if (spcon != 0) {
                    Msgptr = "SEARCHING FOR SONG START.";
                    MessageOn = MsgCount;
                    copyvid();
                    SPCSave = 1;
                    breakatsignb();
                    SPCSave = 0;
                    savespcdata();

                    curblank = 0x40;
                    Msgptr = spcsaved;
                    MessageOn = MsgCount;
                } else {
                    Msgptr = "SOUND MUST BE ENABLED.";
                    MessageOn = MsgCount;
                }
            }
            if (menucloc == 30 * 288) {
                dumpsound();
                Msgptr = "BUFFER SAVED AS SOUNDDMP.RAW";
                MessageOn = MsgCount;
            }
            if (SPCKeyPressed == 1) {
                SPCKeyPressed = 0;
            } else {
            exitloop:
                GUIUnBuffer();
                copyvid();
#ifdef __MSDOS__
                if (cbitmode != 1) {
                    u1 const* buf = vidbuffer + 100000 + 1;
                    outb(0x03C8, 0);
                    u4 n = 768;
                    do
                        outb(0x03C9, *buf++ >> 2);
                    while (--n != 0);
                }
#endif
            }
        }
        u1* i = pressed;
        u4 n = 256; // XXX maybe should be lengthof(pressed)
        do {
            if (*i == 1)
                *i = 2;
            ++i;
        } while (--n != 0);
        StartSound();
        ForceNonTransp = 0;
        GUIOn = 0;
        Clear2xSaIBuffer();
        if (MenuNoExit != 1)
            break;
        MenuNoExit = 0;
    }
    continueprognokeys();
}
