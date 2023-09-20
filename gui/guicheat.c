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

#include "../cfg.h"
#include "../cpu/memtable.h"
#include "../init.h"
#include "../ui.h"
#include "c_gui.h"
#include "c_guiwindp.h"
#include "gui.h"
#include "guicheat.h"
#include "guikeys.h"
#include "guiwindp.h"

u1 CopyRamToggle;

static u1 FirstSearch;

static void set_cheat_data(u1* const c, u1 const toggle, u1 const value, u4 const address, u1 const pvalue)
{
    c[0] = toggle;
    c[1] = value;
    c[2] = address;
    c[3] = address >> 8;
    c[4] = address >> 16;
    c[5] = pvalue;
}

static void AddCheatCode(u4 const eax, u1 const bl)
{
    GUICBHold = 0;
    if (NumCheats == 255)
        return;

    u1* const edx = cheatdata + NumCheats++ * 28;
    { // transfer description
        u1* eax = edx;
        char const* ebx = CSDescDisplay;
        u4 ecx = 20;
        do {
            char const dl = *ebx++;
            eax[8] = dl;
            eax[8 + 18] = dl;
            eax[8 + 18 * 2] = dl;
        } while (++eax, --ecx != 0);
    }
    // toggle, value, address, pvalue, name(12)
    u1* const eax_ = wramdata + (eax - 0x7E0000);
    u1 const bh = *eax_;
    *eax_ = bl;
    set_cheat_data(edx, 0, bl, eax, bh);

    u1 const al = GUIpmenupos;
    CheckMenuItemHelp(7);
    GUIpmenupos = al;
    CheatOn = 1;
}

void AddCSCheatCode(void)
{
    if (CSInputDisplay[0] == '_')
        return;

    curaddrvalcs = curentryval;
    curvaluecs = CSCurValue;
    u4 ecx = CheatSrcByteSize + 1;
    if (CheatUpperByteOnly != 0) {
        ecx = 1;
        while (curvaluecs >= 0xFF) {
            curvaluecs >>= 8;
            ++curaddrvalcs;
        }
    }
    do {
        u4 eax = curaddrvalcs + 0x7E0000;
        u1 const bl = curvaluecs;
        // write bl at address eax
        AddCheatCode(eax, bl);

        curvaluecs >>= 8;
        ++curaddrvalcs;
    } while (--ecx != 0);
    CheatWinMode = 2;
}

#define SearchMacro(CMP)            \
    do {                            \
        u1 dl = 0xFE;               \
        do {                        \
            if (!(CMP))             \
                *eax &= dl;         \
            ++edi;                  \
            ++esi;                  \
            if (dl == 0x7F)         \
                ++eax;              \
            dl = dl << 1 | dl >> 7; \
        } while (--ecx != 0);       \
    } while (0)

#define SearchMacroB(L, R)           \
    {                                \
        switch (CheatCompareValue) { \
        case 0:                      \
            SearchMacro((L) > (R));  \
            break; /* Greater   */   \
        case 1:                      \
            SearchMacro((L) < (R));  \
            break; /* Less      */   \
        case 3:                      \
            SearchMacro((L) != (R)); \
            break; /* Not equal */   \
        default:                     \
            SearchMacro((L) == (R)); \
            break; /* Equal     */   \
        }                            \
    }

void CheatCodeSearchProcess(void)
{
    if (CheatSrcSearchType == 1) { // Comparative
        CSInputDisplay[0] = '_';
        CSInputDisplay[1] = '\0';
        u1* eax = vidbuffer + 129600 + 65536 * 2;
        u1 const* esi = vidbuffer + 129600;
        u1 const* edi = wramdata;
        u4 ecx = 65536 * 2;
        switch (CheatSrcByteSize) {
        default:
            ecx -= 0;
            SearchMacroB(*(u1 const*)edi, *(u1 const*)esi);
            break;
        case 1:
            ecx -= 1;
            SearchMacroB(*(u2 const*)edi, *(u2 const*)esi);
            break;
        case 2:
            ecx -= 2;
            SearchMacroB(*(u4 const*)edi & 0x00FFFFFF, *(u2 const*)esi & 0x00FFFFFF);
            break;
        case 3:
            ecx -= 3;
            SearchMacroB(*(u4 const*)edi, *(u4 const*)esi);
            break;
        }
        CopyRamToggle = 1;
    } else if (CSInputDisplay[0] != '\0' && CSInputDisplay[0] != '_' && CSOverValue != 1) {
        CSInputDisplay[0] = '_';
        CSInputDisplay[1] = '\0';
        // Process Cheat Search
        u1 const ebx = CheatSrcByteSize;
        u4 ecx = 65536 * 2 - ebx;
        u4 const edx = SrcMask[ebx];
        u1* edi = vidbuffer + 129600 + 65536 * 2;
        u1 const* esi = wramdata;
        u1 bl = 0xFE;
        u1 bh = 0x01;
        do {
            u4 const eax = *(u4 const*)esi & edx; // XXX unaligned
            if (eax == CSCurValue) {
                if (!(edi[16384] & bh))
                    goto failedfind;
            } else if ((eax + 1) & edx == CSCurValue) {
                if (FirstSearch == 1) {
                    edi[16384] &= bl;
                } else {
                    if (edi[16384] & bh)
                        goto failedfind;
                }
            } else {
            failedfind:
                *edi &= bl;
            }
            if (bl == 0x7F)
                ++edi;
            bl = bl << 1 | bl >> 7;
            bh = bh << 1 | bh >> 7;
        } while (++esi, --ecx != 0);
        CopyRamToggle = 1;
        CheatSearchStatus = 1;
    }
}

void CheatCodeSearchInit(void)
{
    CSInputDisplay[0] = '_';
    CSInputDisplay[1] = '\0';
    CheatWinMode = 1;
    CheatSearchStatus = 0;
    FirstSearch = 1;
    // copy 128k ram
    memcpy(vidbuffer + 129600, wramdata, 131072);
    // fill searched buffer with 0xFF
    memset(vidbuffer + 129600 + 65536 * 2, 0xFF, 32768 * 4);
    if (CheatSrcSearchType == 1)
        CheatSearchStatus = 1;
    CheatCompareValue = 0;
    u1 val;
    switch (CheatSrcByteSize) {
    case 1:
        val = 0x7F;
        break;
    case 2:
        val = 0x3F;
        break;
    case 3:
        val = 0x1F;
        break;
    default:
        return;
    }
    vidbuffer[129600 + 65536 * 2 + 16383] &= val;
}

static void DisableCheatCode(u1* const esi)
{
    esi[0] |= 0x04;
    if (esi[0] & 0x01) {
        u4 const ecx = *(u4 const*)(esi + 2) & 0x00FFFFFF; // XXX unaligned
        romdata[ecx] = esi[5];
    } else if (!(esi[0] & 0x80) && !(esi[-28] & 0x80)) {
        memw8(esi[4], *(u2 const*)(esi + 2), esi[5]);
    }
}

void DisableCheatsOnLoad(void)
{
    // Disable all codes
    u1* esi = cheatdata;
    for (u4 ecx = NumCheats; ecx != 0; esi += 28, --ecx) {
        if (esi[0] & 0x04)
            continue;
        DisableCheatCode(esi);
    }
}

static void EnableCheatCode(u1* const esi)
{
    esi[0] &= 0xFB;
    if (esi[0] & 0x01) {
        u1 const al = esi[1];
        u4 const ecx = *(u4 const*)(esi + 2) & 0x00FFFFFF; // XXX unaligned
        u1* const esi = romdata;
        u1 const bl = esi[ecx];
        esi[ecx] = al;
        esi[5] = bl;
    } else {
        u1 const al = esi[1];
        u2 const cx = *(u2 const*)(esi + 2);
        u1 const bl = esi[4];
        esi[5] = memr8(bl, cx);
        if (!(esi[0] & 0x80) && !(esi[-28] & 0x80))
            memw8(bl, cx, al);
    }
}

void EnableCheatsOnLoad(void)
{
    // Enable all ON toggled cheat codes
    u1* esi = cheatdata;
    for (u4 ecx = NumCheats; ecx != 0; esi += 28, --ecx) {
        if (esi[0] & 0x04)
            continue;
        EnableCheatCode(esi);
    }
}

void CheatCodeRemove(void)
{
    GUICBHold = 0;
    if (NumCheats == 0)
        return;

    u1* const esi = cheatdata + GUIcurrentcheatcursloc * 28;
    DisableCheatCode(esi);
    memmove(esi, esi + 28, (255 - GUIcurrentcheatcursloc) * 18); // XXX 18? Probably should be 28

    u4 const eax = GUIcurrentcheatcursloc;
    if (--NumCheats != 0 && eax == NumCheats) {
        GUIcurrentcheatcursloc = eax - 1;
        GUIcurrentcheatviewloc = eax - 12;
        if (GUIcurrentcheatviewloc & 0x80000000)
            GUIcurrentcheatviewloc = 0;
    }

    if (NumCheats == 0)
        CheatOn = 0;
}

void CheatCodeFix(void)
{
    GUICBHold = 0;
    if (NumCheats == 0)
        return;

    u1* const esi = cheatdata + GUIcurrentcheatcursloc * 28;
    DisableCheatCode(esi);
    esi[3] ^= 0x80;
    EnableCheatCodeNoPrevMod(esi);
}

void CheatCodeToggle(void)
{
    GUICBHold = 0;
    if (NumCheats == 0)
        return;

    u1* const esi = cheatdata + GUIcurrentcheatcursloc * 28;
    if (esi[0] & 0x04) {
        EnableCheatCodeNoPrevMod(esi);
    } else {
        DisableCheatCode(esi);
    }
}

void EnableCheatCodeNoPrevMod(u1* const esi)
{
    // code is at esi
    esi[0] &= 0xFB;
    if (esi[0] & 0x01) {
        u1 const al = esi[1];
        u4 const ecx = *(u4 const*)(esi + 2) & 0x00FFFFFF; // XXX unaligned
        u1* const esi = romdata;
        u1 const bl = esi[ecx];
        esi[ecx] = al;
        esi[5] = bl;
    } else if (!(esi[0] & 0x80) && !(esi[-28] & 0x80)) {
        memw8(esi[4], *(u2 const*)(esi + 2), esi[1]);
    }
}

static void decodepar(u1 const guicheatvalrep)
{
    // convert code to number format
    u4 ecx = 8;
    char* esi = GUICheatTextZ1;
    do {
        char const al = *esi;
        *esi++ = al < 'A' ? al - '0' : al - 'A' + 10;
    } while (--ecx != 0);

    // get address
    u1 const bl = (u1)GUICheatTextZ1[0] << 4 | (u1)GUICheatTextZ1[1] << 0;
    u2 const cx = (u1)GUICheatTextZ1[2] << 12 | (u1)GUICheatTextZ1[3] << 8 | (u1)GUICheatTextZ1[4] << 4 | (u1)GUICheatTextZ1[5] << 0;
    u1 const al = (u1)GUICheatTextZ1[6] << 4 | (u1)GUICheatTextZ1[7] << 0;

    // store into cheatdata
    u1* const edx = cheatdata + NumCheats * 28;
    set_cheat_data(edx, guicheatvalrep, al, bl << 16 | cx, memr8(bl, cx));

    if (!(edx[0] & 0x80) && !(edx[-28] & 0x80))
        memw8(bl, cx, al);

    CheatOn = 1;
    ++NumCheats;
    memset(GUICheatTextZ1, '\0', 4);
    memset(GUICheatTextZ2, '\0', 4);
    GUICheatPosA = 0;
    GUICheatPosB = 0;
    GUIcurrentcheatwin = 1;
    u4 const eax = NumCheats;
    GUIcurrentcheatcursloc = eax - 1;
    GUIcurrentcheatviewloc = eax - 12;
    if (GUIcurrentcheatviewloc & 0x80000000)
        GUIcurrentcheatviewloc = 0;
}

static void decodegg(u1 const guicheatvalrep)
{
    /* Genie Hex:    D  F  4  7  0  9  1  5  6  B  C  8  A  2  3  E
     * Normal  Hex:  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
     *               4  6  D  E  2  7  8  3  B  5  C  9  A  0  F  1 */
    { // Convert code
        char* esi = GUICheatTextZ1;
        memmove(esi + 4, esi + 5, 4);
        // convert code to number format
        u4 ecx = 8;
        do {
            static u1 const GG2Norm[] = { 0x4, 0x6, 0xD, 0xE, 0x2, 0x7, 0x8, 0x3, 0xB, 0x5, 0xC, 0x9, 0xA, 0x0, 0xF, 0x1 };
            char const al = *esi;
            *esi++ = GG2Norm[al < 'A' ? al - '0' : al - 'A' + 10];
        } while (--ecx != 0);
    }

    char const* const esi = GUICheatTextZ1;
    u4 const ecx = (u1)esi[2] << 20 | (u1)esi[3] << 16 | (u1)esi[4] << 12 | (u1)esi[5] << 8 | (u1)esi[6] << 4 | (u1)esi[7] << 0;

    /*                        0123456789ABCDEF01234567
     * 24bit encoded address: ijklqrstopabcduvwxefghmn
     *                        abcdefghijklmnopqrstuvwx
     *                        >8  >12 >6<10 >6  <14 <10 */
    u4 const ebx = (ecx & 0x003C00) << 10 | // abcd
        (ecx & 0x00003C) << 14 | // efgh
        (ecx & 0xF00000) >> 8 | // ijkl
        (ecx & 0x000003) << 10 | // mn
        (ecx & 0x00C000) >> 6 | // op
        (ecx & 0x0F0000) >> 12 | // qrst
        (ecx & 0x0003C0) >> 6; // uvwx
    u1 const al = (u1)esi[0] << 1 | (u1)esi[1];

    // store into cheatdata
    u1* const edx = cheatdata + NumCheats * 28;
    set_cheat_data(edx, guicheatvalrep, al, ebx, memr8(ebx >> 16, ebx));

    if (!(edx[0] & 0x80) && !(edx[-28] & 0x80))
        memw8(ebx >> 16, ebx, al);

    CheatOn = 1;
    ++NumCheats;
    memset(GUICheatTextZ1, '\0', 4);
    memset(GUICheatTextZ2, '\0', 4);
    GUICheatPosA = 0;
    GUICheatPosB = 0;
    GUIcurrentcheatwin = 1;
    u4 const eax = NumCheats;
    GUIcurrentcheatcursloc = eax - 1;
    GUIcurrentcheatviewloc = eax - 12;
    if (GUIcurrentcheatviewloc & 0x80000000)
        GUIcurrentcheatviewloc = 0;
}

static void decodegf(u1 const guicheatvalrep)
{
    { // convert code to number format
        u4 ecx = 14;
        char* esi = GUICheatTextZ1;
        do {
            char const al = *esi;
            if (al != 'X')
                *esi = al < 'A' ? al - '0' : al - 'A' + 10;
        } while (++esi, --ecx != 0);
    }

    // get address
    u4 ecx = (u1)GUICheatTextZ1[0] << 16 | (u1)GUICheatTextZ1[1] << 12 | (u1)GUICheatTextZ1[2] << 8 | (u1)GUICheatTextZ1[3] << 4 | (u1)GUICheatTextZ1[4] << 0;

    u1* edx = cheatdata + NumCheats * 28;

    // Write data to memory
    if (GUICheatTextZ1[13] == 1) {
        if (ecx > 65535)
            goto quit;
        u1* const esi = sram;

        // get data
        if (GUICheatTextZ1[5] != 'X' && GUICheatTextZ1[6] != 'X') {
            u1 const bl = GUICheatTextZ1[5] << 4 | GUICheatTextZ1[6];
            u1 const al = esi[ecx];
            esi[ecx] = bl;
            set_cheat_data(edx, 2, bl, ecx, al);
            ++NumCheats;
            edx += 28;
        }

        ++ecx;
        if (GUICheatTextZ1[7] != 'X' && GUICheatTextZ1[8] != 'X' && NumCheats != 255 && ecx <= 65535) {
            u1 const bl = GUICheatTextZ1[7] << 4 | GUICheatTextZ1[8];
            u1 const al = esi[ecx + 1];
            esi[ecx + 1] = bl;
            set_cheat_data(edx, 2, bl, ecx, al);
            ++NumCheats;
            edx += 28;
        }

        ++ecx;
        if (GUICheatTextZ1[9] != 'X' && GUICheatTextZ1[10] != 'X' && NumCheats != 255 && ecx <= 65535) {
            u1 const bl = GUICheatTextZ1[9] << 4 | GUICheatTextZ1[10];
            u1 const al = esi[ecx + 2];
            esi[ecx + 2] = bl;
            set_cheat_data(edx, 2, bl, ecx, al);
            ++NumCheats;
        }
    } else {
        // get data
        if (GUICheatTextZ1[5] != 'X' && GUICheatTextZ1[6] != 'X') {
            u1 const bl = GUICheatTextZ1[5] << 4 | GUICheatTextZ1[6];
            u1* const esi = romdata;
            u1 const al = esi[ecx];
            esi[ecx] = bl;
            set_cheat_data(edx, guicheatvalrep | 1, bl, ecx, al);
            ++NumCheats;
            edx += 28;
        }

        ++ecx;
        if (GUICheatTextZ1[7] != 'X' && GUICheatTextZ1[8] != 'X' && NumCheats != 255) {
            u1 const bl = GUICheatTextZ1[7] << 4 | GUICheatTextZ1[8];
            u1* const esi = romdata;
            u1 const al = esi[ecx];
            esi[ecx] = bl;
            set_cheat_data(edx, 1, bl, ecx, al);
            ++NumCheats;
            edx += 28;
        }

        ++ecx;
        if (GUICheatTextZ1[9] != 'X' && GUICheatTextZ1[10] != 'X' && NumCheats != 255) {
            u1 const bl = GUICheatTextZ1[9] << 4 | GUICheatTextZ1[10];
            u1* const esi = romdata;
            u1 const al = esi[ecx];
            esi[ecx] = bl;
            set_cheat_data(edx, 1, bl, ecx, al);
            ++NumCheats;
        }
    }

quit:
    if (NumCheats != 0) {
        CheatOn = 1;
        GUIcurrentcheatwin = 1;
        u4 const eax = NumCheats;
        GUIcurrentcheatcursloc = eax - 1;
        GUIcurrentcheatviewloc = eax - 12;
        if (GUIcurrentcheatviewloc & 0x80000000)
            GUIcurrentcheatviewloc = 0;
    }
    memset(GUICheatTextZ1, 0, 4);
    memset(GUICheatTextZ2, 0, 4);
    GUICheatPosA = 0;
    GUICheatPosB = 0;
}

void ProcessCheatCode(void)
{
    GUICBHold = 0;
    if (NumCheats == 255)
        return;
    if (GUICheatPosA == 0)
        return;

    // make sure flashing cursor doesn't exist
    GUICheatTextZ2[GUICheatPosB] = '\0';
    { // transfer description
        u1* eax = cheatdata + NumCheats * 28 + 6;
        char const* ebx = GUICheatTextZ2;
        u4 ecx = 20;
        do {
            char const dl = *ebx++;
            eax[2] = dl;
            eax[18 + 2] = dl;
            eax[18 * 2 + 2] = dl;
        } while (++eax, --ecx != 0);
    }

    { // uppercase all codes if necessary
        char* eax = GUICheatTextZ1;
        u4 ecx = 14;
        do {
            char const bl = *eax;
            if ('a' <= bl && bl <= 'z')
                *eax = bl - 'a' + 'A';
        } while (++eax, --ecx != 0);
    }

    u1 guicheatvalrep = 0;
    if (GUICheatTextZ1[GUICheatPosA - 1] == 'R') {
        guicheatvalrep = 0x80;
        --GUICheatPosA;
    }

    // determine whether it is gamegenie, par, or GF
    switch (GUICheatPosA) {
    case 8: // PAR
    { // check if code is valid
        char const* eax = GUICheatTextZ1;
        u4 ecx = 8;
        do {
            char const bl = *eax++;
            if ('0' <= bl && bl <= '9')
                continue;
            if ('A' <= bl && bl <= 'F')
                continue;
            goto invalid;
        } while (--ecx != 0);
    }
        decodepar(guicheatvalrep);
        break;

    case 9: // GG
    { // check if code is valid
        char const* eax = GUICheatTextZ1;
        u4 ecx = 9;
        do {
            if (ecx == 5) {
                if (*eax != '-')
                    goto invalid;
                ++eax;
                --ecx;
            }
            char const bl = *eax++;
            if ('0' <= bl && bl <= '9')
                continue;
            if ('A' <= bl && bl <= 'F')
                continue;
            goto invalid;
        } while (--ecx != 0);
    }
        decodegg(guicheatvalrep);
        break;

    case 14: // GF
    { // check if code is valid
        char const* eax = GUICheatTextZ1;
        u4 ecx = 5;
        do {
            char const bl = *eax++;
            if ('0' <= bl && bl <= '9')
                continue;
            if ('A' <= bl && bl <= 'F')
                continue;
            goto invalid;
        } while (--ecx != 0);
    }
        {
            char const* eax = GUICheatTextZ1 + 5;
            u4 ecx = 6;
            do {
                char const bl = *eax++;
                if (bl == 'X')
                    continue;
                if ('0' <= bl && bl <= '9')
                    continue;
                if ('A' <= bl && bl <= 'F')
                    continue;
                goto invalid;
            } while (--ecx != 0);
            char const bl = GUICheatTextZ1[13];
            if (bl != '0' && bl != '1')
                goto invalid;
        }
        decodegf(guicheatvalrep);
        break;

    default:
    invalid:
        guicheaterror();
        break;
    }
}
