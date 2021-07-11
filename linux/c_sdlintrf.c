#include <stdio.h>
#include <string.h>

#include "../c_intrf.h"
#include "../c_vcache.h"
#include "../cfg.h"
#include "../cpu/c_dspproc.h"
#include "../cpu/dspproc.h"
#include "../cpu/execute.h"
#include "../gui/c_gui.h"
#include "../input.h"
#include "../intrf.h"
#include "../link.h"
#include "../macros.h"
#include "../ui.h"
#include "../vcache.h"
#include "../video/c_2xsaiw.h"
#include "../video/newgfx16.h"
#include "c_sdlintrf.h"
#include "sdllink.h"

#ifdef __OPENGL__
#include "../video/procvidc.h"

u1 blinit;
#endif

void StartUp(void)
{
}

void SystemInit(void)
{
    // Be sure to set SBHDMA to a value other than 0 if 16bit sound exists
    SBHDMA = 1;
}

void PrintChar(char const c)
{
    putchar(c);
}

void PrintStr(char const* const s)
{
    fputs(s, stdout);
}

char WaitForKey(void)
{
    return getchar();
}

u1 Check_Key(void)
{
    return CurKeyPos != CurKeyReadPos ? 0xFF : 0;
}

char Get_Key(void)
{
    u4 const pos = CurKeyReadPos;
    while (CurKeyPos == pos) { } // XXX busy waiting
    if (KeyBuffer[pos] & 0x100) {
        KeyBuffer[pos] -= 0x100;
        return 0;
    } else {
        CurKeyReadPos = (pos + 1) % lengthof(KeyBuffer);
        return KeyBuffer[pos];
    }
}

void delay(u4 const n) { (void)n; /* Stub please fix */ }

void InitPreGame(void)
{
    pressed[1] = 2;
    Start60HZ();
    initwinvideo();

    if (V8Mode != GrayscaleMode)
        V8Mode ^= 1;

    AdjustFrequency();

    memset(vidbufferofsb, 0, 288 * 128 * 4);

    clearwin();
}

void SetupPreGame(void)
{
    pressed[1] = 2;
}

void DeInitPostGame(void)
{
    Stop60HZ();
}

void GUIInit(void)
{
    Start36HZ();
}

void GUIDeInit(void)
{
    Stop36HZ();
}

static void InitializeGfxStuff(void)
{
    static u1 const BitPosR = 11;
    static u1 const BitPosG = 5;
    static u1 const BitPosB = 0;
    static u1 const BitSizeR = 5;
    static u1 const BitSizeG = 6;
    static u1 const BitSizeB = 5;

    { // Process Red Stuff
        u1 al = BitPosR;
        u2 bx = 1U << al;
        if (BitSizeR == 6) {
            vesa2_usbit = bx;
            ++al;
        }
        vesa2_clbit |= bx;
        vesa2_rpos = al;
        --al;
        vesa2_rfull = al != 0xFF ? 0x1FU << al : 0x1FU >> 1;
        bx = 1U << (al + 5);
        vesa2_rtrcl = bx;
        vesa2_rtrcla = ~bx;
    }

    { // Process Green Stuff
        u1 al = BitPosG;
        u2 bx = 1U << al;
        if (BitSizeG == 6) {
            vesa2_usbit = bx;
            ++al;
        }
        vesa2_clbit |= bx;
        vesa2_gpos = al;
        --al;
        vesa2_gfull = al != 0xFF ? 0x1FU << al : 0x1FU >> 1;
        bx = 1U << (al + 5);
        vesa2_gtrcl = bx;
        vesa2_gtrcla = ~bx;
    }

    { // Process Blue Stuff
        u1 al = BitPosB;
        u2 bx = 1U << al;
        if (BitSizeB == 6) {
            vesa2_usbit = bx;
            ++al;
        }
        vesa2_clbit |= bx;
        vesa2_bpos = al;
        --al;
        vesa2_bfull = al != 0xFF ? 0x1FU << al : 0x1FU >> 1;
        bx = 1U << (al + 5);
        vesa2_btrcl = bx;
        vesa2_btrcla = ~bx;
    }

    vesa2_clbit ^= 0xFFFF;
    genfulladdtab();
    Init_2xSaIMMX(converta != 1 ? 565 : 555);
}

void initvideo(void)
{
    static u4 firstvideo = 1;

    res640 = 1;
    res480 = 1;
    cbitmode = 1;
    vesa2_x = 512;
    vesa2_y = 480;
    vesa2_bits = 16;
    vesa2_rpos = 11;
    vesa2_gpos = 5;
    vesa2_bpos = 0;
    vesa2red10 = 0;
    vesa2_rposng = 11;
    vesa2_gposng = 5;
    vesa2_bposng = 0;
    vesa2_clbitng = 0x0000F7DE;
    vesa2_clbitng2[0] = 0xF7DEF7DE;
    vesa2_clbitng2[1] = 0xF7DEF7DE;
    vesa2_clbitng3 = 0x00007BEF;

    initwinvideo();

    if (GUIWFVID[cvidmode] != 0)
        PrevFSMode = cvidmode;
    else
        PrevWinMode = cvidmode;

    if (firstvideo != 1)
        InitializeGfxStuff();
    firstvideo = 0;

    InitializeGfxStuff();
}

void deinitvideo(void) { }

void DrawScreen(void)
{
    if (converta == 1) {
        UnusedBit[0] = 0x80008000;
        HalfTrans[0] = 0x7BDE7BDE;
        UnusedBitXor[0] = 0x7FFF7FFF;
        UnusedBit[1] = 0x80008000;
        HalfTrans[1] = 0x7BDE7BDE;
        UnusedBitXor[1] = 0x7FFF7FFF;
        HalfTransB[0] = 0x04210421;
        HalfTransB[1] = 0x04210421;
        HalfTransC[0] = 0x7BDE7BDE;
        HalfTransC[1] = 0x7BDE7BDE;
        ngrposng = 10;
        nggposng = 5;
        ngbposng = 0;
        ConvertToAFormat();
    }
    drawscreenwin();
#ifdef __OPENGL__
    if (blinit == 1) {
        initwinvideo();
        Clear2xSaIBuffer();
        blinit = 0;
    }
#endif
}

void vidpastecopyscr(void)
{
    u1* const buf = vidbuffer;
    u4 n = 224 * 288 - 288;
    u4 i = 224 * 288 - 1;
    do
        ((u2*)buf)[i] = GUICPC[buf[i]];
    while (--i, --n != 0);
    DrawScreen();
}

void UpdateDevices(void)
{ /* Stub please fix */
}

void JoyRead(void)
{
    UpdateVFrame();
}

#define SetDefaultKey2(player, k)           \
    player##upk = k[2], /* Up     */        \
        player##downk = k[3], /* Down   */  \
        player##leftk = k[4], /* Left   */  \
        player##rightk = k[5], /* Right  */ \
        player##startk = k[1], /* Start  */ \
        player##selk = k[0], /* Select */   \
        player##Ak = k[7], /* A      */     \
        player##Bk = k[10], /* B      */    \
        player##Xk = k[6], /* X      */     \
        player##Yk = k[9], /* Y      */     \
        player##Lk = k[8], /* L      */     \
        player##Rk = k[11] /* R      */

void SetInputDevice(u1 const device, u1 const player)
{
    // Sets keys according to input device selected
    static u1 const keys[][12] = {
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
#ifdef __UNIXSDL__
        { 54, 28, 90, 96, 92, 94, 31, 45, 32, 30, 44, 46 },
        { 56, 29, 36, 50, 49, 51, 98, 89, 91, 99, 95, 97 }
#else
        { 54, 28, 200, 208, 203, 205, 31, 45, 32, 30, 44, 46 },
        { 56, 29, 36, 50, 49, 51, 210, 199, 201, 211, 207, 209 }
#endif
    };

    u1 const* const k = device == 0 ? keys[0] : player != 1 ? keys[1]
                                                            : keys[2];

    switch (player) {
    case 0:
        SetDefaultKey2(pl1, k);
        break;
    case 1:
        SetDefaultKey2(pl2, k);
        break;
    case 2:
        SetDefaultKey2(pl3, k);
        break;
    case 3:
        SetDefaultKey2(pl4, k);
        break;
    case 4:
        SetDefaultKey2(pl5, k);
        break;
    }
}

#undef SetDefaultKey2

/*****************************
 * Mouse Stuff
 *****************************/

u4 Init_Mouse(void)
{
    return 1;
}

u4 Get_MouseData(void)
{
    u4 const x = GetMouseX();
    u4 const y = GetMouseY();
    u4 const buttons = GetMouseButton();
    return y << 24 | x << 16 | buttons;
}

void Set_MouseXMax(u4 const min, u4 const max)
{
    (void)min;
    (void)max; /* Stub please fix */
}
void Set_MouseYMax(u4 const min, u4 const max)
{
    (void)min;
    (void)max; /* Stub please fix */
}

void Set_MousePosition(u4 const x, u4 const y)
{
    (void)x;
    (void)y; /* Stub please fix */
}

u4 Get_MousePositionDisplacement(void)
{
    u4 const x = GetMouseMoveX();
    u4 const y = GetMouseMoveY();
    return y << 16 | x;
}

void MouseWindow(void)
{
    MouseButton |= 0x02;
    T36HZEnabled = 1;
    GetMouseButton();
    MouseButton &= 0xFD;
}

void StopSound(void)
{
    Start36HZ();
    JoyRead();
}

void StartSound(void)
{
    Start60HZ();
    JoyRead();
}

void Check60hz(void)
{
    CheckTimers();
    sem_sleep();
}

char const GUIVideoModeNames[][18] = {
    "256x224       R W", //  0
    "256x224       R F", //  1
    "512x448      DR W", //  2
    "512x448      DR F", //  3
    "640x480      DR F", //  4
#ifdef __OPENGL__
    "256x224     O R W", //  5
    "512x448     ODR W", //  6
    "640x480     ODS F", //  7
    "640x480     ODS W", //  8
    "640x560     ODR W", //  9
    "768x672     ODR W", // 10
    "800x600     ODS F", // 11
    "800x600     ODS W", // 12
    "896x784     ODR W", // 13
    "1024x768    ODS F", // 14
    "1024x768    ODS W", // 15
    "1024x896    ODR W", // 16
    "1280x960    ODS F", // 17
    "1280x1024   ODS F", // 18
    "1600x1200   ODS F", // 19
    "VARIABLE    ODR W", // 20
    "VARIABLE    ODS W", // 21
    "CUSTOM      OD  F" // 22
#endif
};

u4 const NumVideoModes = lengthof(GUIVideoModeNames);

#define _ 0
#define X 1
/* Video Mode Feature Availability (X = Available, _ = Not Available)
 * Left side starts with Video Mode 0
 *                 0                   1                   2
 *                 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 */
u1 GUIBIFIL[] = { _, _, _, _, _, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X };
u1 GUIDSIZE[] = { _, _, X, X, X, _, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X };
u1 GUIHQ2X[] = { _, _, X, X, X, _, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X };
u1 GUIHQ3X[] = { _, _, X, X, X, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _ };
u1 GUIHQ4X[] = { _, _, X, X, X, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _ };
u1 GUII2VID[] = { _, _, X, X, X, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _ };
u1 GUIKEEP43[] = { _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, X, _, _, X, X };
u1 GUIM7VID[] = { _, _, X, X, X, _, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X };
u1 GUINTVID[] = { _, _, X, X, X, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _ };
u1 GUIRESIZE[] = { _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, X, X, _ };
u1 GUIWFVID[] = { _, X, _, X, X, _, _, X, _, _, _, X, _, _, X, _, _, X, X, X, _, _, X };
#undef X
#undef _

char const GUIInputNames[][17] = {
    "NONE            ",
    "KEYBOARD/GAMEPAD",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                "
};

u4 const NumInputDevices;

char const ScanCodeListing[] = {
    "---"
    "ESC"
    " 1 "
    " 2 "
    " 3 "
    " 4 "
    " 5 "
    " 6 " // 0x000
    " 7 "
    " 8 "
    " 9 "
    " 0 "
    " - "
    " = "
    "BKS"
    "TAB"
    " Q "
    " W "
    " E "
    " R "
    " T "
    " Y "
    " U "
    " I " // 0x010
    " O "
    " P "
    " [ "
    " ] "
    "RET"
    "LCT"
    " A "
    " S "
    " D "
    " F "
    " G "
    " H "
    " J "
    " K "
    " L "
    " : " // 0x020
    " \" "
    " ~ "
    "LSH"
    " \\ "
    " Z "
    " X "
    " C "
    " V "
    " B "
    " N "
    " M "
    " , "
    " . "
    " / "
    "RSH"
    " * " // 0x030
    "LAL"
    "SPC"
    "CAP"
    "F1 "
    "F2 "
    "F3 "
    "F4 "
    "F5 "
    "F6 "
    "F7 "
    "F8 "
    "F9 "
    "F10"
    "NUM"
    "SCR"
    "KP7" // 0x040
    "KP8"
    "KP9"
    "KP-"
    "KP4"
    "KP5"
    "KP6"
    "KP+"
    "KP1"
    "KP2"
    "KP3"
    "KP0"
    "KP."
    "   "
    "   "
    "OEM"
    "F11" // 0x050
    "F12"
    "HOM"
    "UP "
    "PGU"
    "LFT"
    "5DH"
    "RGT"
    "END"
    "DWN"
    "PGD"
    "INS"
    "DEL"
    "64H"
    "65H"
    "66H"
    "67H" // 0x060
    "68H"
    "69H"
    "6AH"
    "6BH"
    "6CH"
    "6DH"
    "6EH"
    "6FH"
    "70H"
    "71H"
    "72H"
    "73H"
    "74H"
    "75H"
    "76H"
    "77H" // 0x070
    "78H"
    "79H"
    "7AH"
    "7BH"
    "7CH"
    "7DH"
    "7EH"
    "7FH"
    // Keyboard continued (Direct Input)
    "80H"
    "81H"
    "82H"
    "83H"
    "84H"
    "85H"
    "86H"
    "87H" // 0x080
    "88H"
    "89H"
    "8AH"
    "8BH"
    "8CH"
    "8DH"
    "8EH"
    "8FH"
    "90H"
    "91H"
    "92H"
    "93H"
    "94H"
    "95H"
    "96H"
    "97H" // 0x090
    "98H"
    "99H"
    "9AH"
    "9BH"
    "9CH"
    "9DH"
    "9EH"
    "9FH"
    "A0H"
    "A1H"
    "A2H"
    "A3H"
    "A4H"
    "A5H"
    "A6H"
    "A7H" // 0x0A0
    "A8H"
    "A9H"
    "AAH"
    "ABH"
    "ACH"
    "ADH"
    "AEH"
    "AFH"
    "B0H"
    "B1H"
    "B2H"
    "B3H"
    "B4H"
    "B5H"
    "B6H"
    "B7H" // 0x0B0
    "B8H"
    "B9H"
    "BAH"
    "BBH"
    "BCH"
    "BDH"
    "BEH"
    "BFH"
    "C0H"
    "C1H"
    "C2H"
    "C3H"
    "C4H"
    "C5H"
    "C6H"
    "C7H" // 0x0C0
    "C8H"
    "C9H"
    "CAH"
    "CBH"
    "CCH"
    "CDH"
    "CEH"
    "CFH"
    "D0H"
    "D1H"
    "D2H"
    "D3H"
    "D4H"
    "D5H"
    "D6H"
    "D7H" // 0x0D0
    "D8H"
    "D9H"
    "DAH"
    "DBH"
    "DCH"
    "DDH"
    "DEH"
    "DFH"
    "E0H"
    "E1H"
    "E2H"
    "E3H"
    "E4H"
    "E5H"
    "E6H"
    "E7H" // 0x0E0
    "E8H"
    "E9H"
    "EAH"
    "EBH"
    "ECH"
    "EDH"
    "EEH"
    "EFH"
    "F0H"
    "F1H"
    "F2H"
    "F3H"
    "F4H"
    "F5H"
    "F6H"
    "F7H" // 0x0F0
    "F8H"
    "F9H"
    "FAH"
    "FBH"
    "FCH"
    "FDH"
    "FEH"
    "FFH"
    // Joystick Stuff
    "J00"
    "J01"
    "J02"
    "J03"
    "J04"
    "J05"
    "J06"
    "J07" // 0x100
    "J08"
    "J09"
    "J0A"
    "J0B"
    "J0C"
    "J0D"
    "J0E"
    "J0F"
    "J10"
    "J11"
    "J12"
    "J13"
    "J14"
    "J15"
    "J16"
    "J17" // 0x110
    "J18"
    "J19"
    "J1A"
    "J1B"
    "J1C"
    "J1D"
    "J1E"
    "J1F"
    "J20"
    "J21"
    "J22"
    "J23"
    "J24"
    "J25"
    "J26"
    "J27" // 0x120
    "J28"
    "J29"
    "J2A"
    "J2B"
    "J2C"
    "J2D"
    "J2E"
    "J2F"
    "J30"
    "J31"
    "J32"
    "J33"
    "J34"
    "J35"
    "J36"
    "J37" // 0x130
    "J38"
    "J39"
    "J3A"
    "J3B"
    "J3C"
    "J3D"
    "J3E"
    "J3F"
    "J40"
    "J41"
    "J42"
    "J43"
    "J44"
    "J45"
    "J46"
    "J47" // 0x140
    "J48"
    "J49"
    "J4A"
    "J4B"
    "J4C"
    "J4D"
    "J4E"
    "J4F"
    "J50"
    "J51"
    "J52"
    "J53"
    "J54"
    "J55"
    "J56"
    "J57" // 0x150
    "J58"
    "J59"
    "J5A"
    "J5B"
    "J5C"
    "J5D"
    "J5E"
    "J5F"
    "J60"
    "J61"
    "J62"
    "J63"
    "J64"
    "J65"
    "J66"
    "J67" // 0x160
    "J68"
    "J69"
    "J6A"
    "J6B"
    "J6C"
    "J6D"
    "J6E"
    "J6F"
    "J70"
    "J71"
    "J72"
    "J73"
    "J74"
    "J75"
    "J76"
    "J77" // 0x170
    "J78"
    "J79"
    "J7A"
    "J7B"
    "J7C"
    "J7D"
    "J7E"
    "J7F"
#ifdef __UNIXSDL__
    // Extra Joystick Stuff
    "J80"
    "J81"
    "J82"
    "J83"
    "J84"
    "J85"
    "J86"
    "J87" // 0x180
    "J88"
    "J89"
    "J8A"
    "J8B"
    "J8C"
    "J8D"
    "J8E"
    "J8F"
    "J90"
    "J91"
    "J92"
    "J93"
    "J94"
    "J95"
    "J96"
    "J97" // 0x190
    "J98"
    "J99"
    "J9A"
    "J9B"
    "J9C"
    "J9D"
    "J9E"
    "J9F"
    "JA0"
    "JA1"
    "JA2"
    "JA3"
    "JA4"
    "JA5"
    "JA6"
    "JA7" // 0x2A0
    "JA8"
    "JA9"
    "JAA"
    "JAB"
    "JAC"
    "JAD"
    "JAE"
    "JAF"
    "JB0"
    "JB1"
    "JB2"
    "JB3"
    "JB4"
    "JB5"
    "JB6"
    "JB7" // 0x2B0
    "JB8"
    "JB9"
    "JBA"
    "JBB"
    "JBC"
    "JBD"
    "JBE"
    "JBF"
#else
    // Extra Stuff (180h) (Parallel Port)
    "PPB"
    "PPY"
    "PSL"
    "PST"
    "PUP"
    "PDN"
    "PLT"
    "PRT" // 0x180
    "PPA"
    "PPX"
    "PPL"
    "PPR"
    "   "
    "   "
    "   "
    "   "
    "P2B"
    "P2Y"
    "P2S"
    "P2T"
    "P2U"
    "P2D"
    "P2L"
    "P2R" // 0x190
    "P2A"
    "P2X"
    "P2L"
    "P2R"
    "   "
    "   "
    "   "
    "   "
    "PPB"
    "PPY"
    "PSL"
    "PST"
    "PUP"
    "PDN"
    "PLT"
    "PRT" // 0x2A0
    "PPA"
    "PPX"
    "PPL"
    "PPR"
    "   "
    "   "
    "   "
    "   "
    "P2B"
    "P2Y"
    "P2S"
    "P2T"
    "P2U"
    "P2D"
    "P2L"
    "P2R" // 0x2B0
    "P2A"
    "P2X"
    "P2L"
    "P2R"
    "   "
    "   "
    "   "
    "   "
#endif
};
