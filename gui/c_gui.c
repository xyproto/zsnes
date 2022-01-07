#include <string.h>

#include "../asm.h"
#include "../c_init.h"
#include "../c_intrf.h"
#include "../c_vcache.h"
#include "../cfg.h"
#include "../cpu/c_dspproc.h"
#include "../cpu/c_execute.h"
#include "../cpu/execute.h"
#include "../cpu/regs.h"
#include "../effects/burn.h"
#include "../effects/smoke.h"
#include "../effects/water.h"
#include "../endmem.h"
#include "../macros.h"
#include "../ui.h"
#include "../vcache.h"
#include "../video/c_mode716.h"
#include "../video/makevid.h"
#include "../video/procvid.h"
#include "../video/procvidc.h"
#include "../zmovie.h"
#include "../zstate.h"
#include "../ztimec.h"
#include "c_gui.h"
#include "c_guiwindp.h"
#include "gui.h"
#include "guicheat.h"
#include "guifuncs.h"
#include "guikeys.h"
#include "guimisc.h"
#include "guimouse.h"
#include "guitools.h"
#include "guiwindp.h"

#ifdef __MSDOS__
#include "../asm_call.h"
#include "../dos/vesa2.h"
#endif

#ifdef __OPENGL__
#include "../linux/c_sdlintrf.h"
#endif

#ifdef __WIN32__
#include "../win/winlink.h"
#endif

u1 MouseDis;
u1 MousePRClick;
u1 ShowTimer;
u2 GUICPC[256];
u4 MsgGiftLeft = 0;
u4 SantaPos = 272;

// bitmap 5x5 font; char - offset for ASCII2Font
u1 const GUIFontData[][5] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00 }, // empty space 0x0
    { 0x70, 0x98, 0xA8, 0xC8, 0x70 }, // 0 0x01
    { 0x20, 0x60, 0x20, 0x20, 0x70 }, // 1 0x02
    { 0x70, 0x88, 0x30, 0x40, 0xF8 }, // 2 0x03
    { 0x70, 0x88, 0x30, 0x88, 0x70 }, // 3 0x04
    { 0x50, 0x90, 0xF8, 0x10, 0x10 }, // 4 0x05
    { 0xF8, 0x80, 0xF0, 0x08, 0xF0 }, // 5 0x06
    { 0x70, 0x80, 0xF0, 0x88, 0x70 }, // 6 0x07
    { 0xF8, 0x08, 0x10, 0x10, 0x10 }, // 7 0x08
    { 0x70, 0x88, 0x70, 0x88, 0x70 }, // 8 0x09
    { 0x70, 0x88, 0x78, 0x08, 0x70 }, // 9 0x0A
    { 0x70, 0x88, 0xF8, 0x88, 0x88 }, // A 0x0B
    { 0xF0, 0x88, 0xF0, 0x88, 0xF0 }, // B 0x0C
    { 0x70, 0x88, 0x80, 0x88, 0x70 }, // C 0x0D
    { 0xF0, 0x88, 0x88, 0x88, 0xF0 }, // D 0x0E
    { 0xF8, 0x80, 0xF0, 0x80, 0xF8 }, // E 0x0F
    { 0xF8, 0x80, 0xF0, 0x80, 0x80 }, // F 0x10
    { 0x78, 0x80, 0x98, 0x88, 0x70 }, // G 0x11
    { 0x88, 0x88, 0xF8, 0x88, 0x88 }, // H 0x12
    { 0xF8, 0x20, 0x20, 0x20, 0xF8 }, // I 0x13
    { 0x78, 0x10, 0x10, 0x90, 0x60 }, // J 0x14
    { 0x90, 0xA0, 0xE0, 0x90, 0x88 }, // K 0x15
    { 0x80, 0x80, 0x80, 0x80, 0xF8 }, // L 0x16
    { 0xD8, 0xA8, 0xA8, 0xA8, 0x88 }, // M 0x17
    { 0xC8, 0xA8, 0xA8, 0xA8, 0x98 }, // N 0x18
    { 0x70, 0x88, 0x88, 0x88, 0x70 }, // O 0x19
    { 0xF0, 0x88, 0xF0, 0x80, 0x80 }, // P 0x1A
    { 0x70, 0x88, 0xA8, 0x90, 0x68 }, // Q 0x1B
    { 0xF0, 0x88, 0xF0, 0x90, 0x88 }, // R 0x1C
    { 0x78, 0x80, 0x70, 0x08, 0xF0 }, // S 0x1D
    { 0xF8, 0x20, 0x20, 0x20, 0x20 }, // T 0x1E
    { 0x88, 0x88, 0x88, 0x88, 0x70 }, // U 0x1F
    { 0x88, 0x88, 0x50, 0x50, 0x20 }, // V 0x20
    { 0x88, 0xA8, 0xA8, 0xA8, 0x50 }, // W 0x21
    { 0x88, 0x50, 0x20, 0x50, 0x88 }, // X 0x22
    { 0x88, 0x50, 0x20, 0x20, 0x20 }, // Y 0x23
    { 0xF8, 0x10, 0x20, 0x40, 0xF8 }, // Z 0x24
    { 0x00, 0x00, 0xF8, 0x00, 0x00 }, // - 0x25
    { 0x00, 0x00, 0x00, 0x00, 0xF8 }, // _ 0x26
    { 0x68, 0x90, 0x00, 0x00, 0x00 }, // ~ 0x27
    { 0x00, 0x00, 0x00, 0x00, 0x20 }, // . 0x28
    { 0x08, 0x10, 0x20, 0x40, 0x80 }, // / 0x29
    { 0x10, 0x20, 0x40, 0x20, 0x10 }, // < 0x2A
    { 0x40, 0x20, 0x10, 0x20, 0x40 }, // > 0x2B
    { 0x70, 0x40, 0x40, 0x40, 0x70 }, // [ 0x2C
    { 0x70, 0x10, 0x10, 0x10, 0x70 }, // ] 0x2D
    { 0x00, 0x20, 0x00, 0x20, 0x00 }, // : 0x2E
    { 0x60, 0x98, 0x70, 0x98, 0x68 }, // & 0x2F
    { 0x20, 0x20, 0xA8, 0x70, 0x20 }, // arrow down 0x30
    { 0x50, 0xF8, 0x50, 0xF8, 0x50 }, // # 0x31
    { 0x00, 0xF8, 0x00, 0xF8, 0x00 }, // = 0x32
    { 0x48, 0x90, 0x00, 0x00, 0x00 }, // " 0x33
    { 0x80, 0x40, 0x20, 0x10, 0x08 }, // \ 0x34
    { 0xA8, 0x70, 0xF8, 0x70, 0xA8 }, // * 0x35
    { 0x70, 0x88, 0x30, 0x00, 0x20 }, // ? 0x36
    { 0x88, 0x10, 0x20, 0x40, 0x88 }, // % 0x37
    { 0x20, 0x20, 0xF8, 0x20, 0x20 }, // + 0x38
    { 0x00, 0x00, 0x00, 0x20, 0x40 }, // , 0x39
    { 0x30, 0x40, 0x40, 0x40, 0x30 }, // ( 0x3A
    { 0x60, 0x10, 0x10, 0x10, 0x60 }, // ) 0x3B
    { 0x70, 0x98, 0xB8, 0x80, 0x70 }, // @ 0x3C
    { 0x20, 0x40, 0x00, 0x00, 0x00 }, // ' 0x3D
    { 0x20, 0x20, 0x20, 0x00, 0x20 }, // ! 0x3E
    { 0x78, 0xA0, 0x70, 0x28, 0xF0 }, // $ 0x3F
    { 0x00, 0x20, 0x00, 0x20, 0x40 }, // ; 0x40
    { 0x40, 0x20, 0x00, 0x00, 0x00 }, // ` 0x41
    { 0x20, 0x50, 0x00, 0x00, 0x00 }, // ^ 0x42
    { 0x30, 0x40, 0xC0, 0x40, 0x30 }, // { 0x43
    { 0x60, 0x10, 0x18, 0x10, 0x60 }, // } 0x44
    { 0x20, 0x20, 0x70, 0x70, 0xF8 }, // up 0x45
    { 0xF8, 0x70, 0x70, 0x20, 0x20 }, // down 0x46
    { 0x08, 0x38, 0xF8, 0x38, 0x08 }, // left 0x47
    { 0x80, 0xE0, 0xF8, 0xE0, 0x80 }, // right 0x48
    { 0x20, 0x60, 0xF8, 0x60, 0x20 }, // arrow left 0x49
    { 0x38, 0x20, 0x30, 0x08, 0xB0 }, // .5 0x4A
    { 0xFC, 0x84, 0xFC, 0x00, 0x00 }, // maximize (Win) 0x4B
    { 0x00, 0xFC, 0x00, 0x00, 0x00 }, // minimize (Win) 0x4C
    { 0xF8, 0x88, 0x88, 0x88, 0xF8 }, // maximize (SDL) 0x4D
    { 0x00, 0x00, 0x20, 0x50, 0x20 }, // shw fullstop 0x4E
    { 0x70, 0x40, 0x40, 0x40, 0x00 }, // shw left bracket 0x4F
    { 0x00, 0x10, 0x10, 0x10, 0x70 }, // shw right bracket 0x50
    { 0x00, 0x00, 0x00, 0x40, 0x20 }, // shw comma 0x51
    { 0x00, 0x20, 0x70, 0x20, 0x00 }, // shw mid-dot 0x52
    { 0xF8, 0x08, 0xF0, 0x20, 0xC0 }, // shw wo 0x53
    { 0x00, 0xF8, 0x50, 0x60, 0x40 }, // shw mini a 0x54
    { 0x00, 0x10, 0x20, 0xE0, 0x20 }, // shw mini i 0x55
    { 0x00, 0x20, 0xF8, 0x88, 0x30 }, // shw mini u 0x56
    { 0x00, 0x00, 0xF8, 0x20, 0xF8 }, // shw mini e 0x57
    { 0x00, 0x10, 0xF8, 0x30, 0xD0 }, // shw mini o 0x58
    { 0x00, 0x40, 0xF8, 0x50, 0x40 }, // shw mini ya 0x59
    { 0x00, 0x00, 0xF0, 0x10, 0xF8 }, // shw mini yu 0x5A
    { 0x00, 0xF8, 0x08, 0x78, 0xF8 }, // shw mini yo 0x5B
    { 0x00, 0xA8, 0xA8, 0x10, 0x60 }, // shw mini tsu 0x5C
    { 0x00, 0x80, 0x78, 0x00, 0x00 }, // shw prolong 0x5D
    { 0xF8, 0x28, 0x30, 0x20, 0xC0 }, // shw a 0x5E
    { 0x08, 0x30, 0xE0, 0x20, 0x20 }, // shw i 0x5F
    { 0x20, 0xF8, 0x88, 0x10, 0x60 }, // shw u 0x60
    { 0xF8, 0x20, 0x20, 0x20, 0xF8 }, // shw e 0x61
    { 0x10, 0xF8, 0x30, 0x50, 0x90 }, // shw o 0x62
    { 0x40, 0xF8, 0x48, 0x48, 0x98 }, // shw ka 0x63
    { 0x20, 0xF8, 0x20, 0xF8, 0x20 }, // shw ki 0x64
    { 0x40, 0x78, 0x88, 0x10, 0x60 }, // shw ku 0x65
    { 0x40, 0x78, 0x90, 0x10, 0x60 }, // shw ke 0x66 ^^
    { 0xF8, 0x08, 0x08, 0x08, 0xF8 }, // shw ko 0x67
    { 0x50, 0xF8, 0x50, 0x10, 0x60 }, // shw sa 0x68
    { 0x40, 0xA8, 0x48, 0x10, 0xE0 }, // shw shi 0x69
    { 0xF8, 0x08, 0x10, 0x30, 0xC8 }, // shw su 0x6A
    { 0x40, 0xF8, 0x50, 0x40, 0x38 }, // shw se 0x6B
    { 0x88, 0x48, 0x08, 0x10, 0x60 }, // shw so 0x6C
    { 0x40, 0x78, 0xC8, 0x30, 0x60 }, // shw ta 0x6D
    { 0xF8, 0x20, 0xF8, 0x20, 0x40 }, // shw chi 0x6E
    { 0xA8, 0xA8, 0x08, 0x10, 0x60 }, // shw tsu 0x6F
    { 0xF8, 0x00, 0xF8, 0x20, 0xC0 }, // shw te 0x70
    { 0x40, 0x40, 0x60, 0x50, 0x40 }, // shw to 0x71
    { 0x20, 0xF8, 0x20, 0x20, 0x40 }, // shw na 0x72
    { 0xF0, 0x00, 0x00, 0x00, 0xF8 }, // shw ni 0x73
    { 0xF8, 0x08, 0x28, 0x10, 0x68 }, // shw nu 0x74
    { 0x20, 0xF8, 0x08, 0x70, 0xA8 }, // shw ne 0x75
    { 0x08, 0x08, 0x08, 0x10, 0x60 }, // shw no 0x76
    { 0x50, 0x50, 0x50, 0x88, 0x88 }, // shw ha 0x77
    { 0x80, 0x98, 0xE0, 0x80, 0x78 }, // shw hi 0x78
    { 0xF8, 0x08, 0x08, 0x10, 0x60 }, // shw hu 0x79
    { 0x40, 0xA0, 0x90, 0x08, 0x00 }, // shw he 0x7A
    { 0x20, 0xF8, 0x70, 0xA8, 0x20 }, // shw ho 0x7B
    { 0xF8, 0x08, 0x90, 0x60, 0x20 }, // shw ma 0x7C
    { 0xF8, 0x00, 0xF8, 0x00, 0xF8 }, // shw mi 0x7D
    { 0x20, 0x40, 0x40, 0x90, 0xF8 }, // shw mu 0x7E
    { 0x08, 0x48, 0x30, 0x30, 0xC8 }, // shw me 0x7F
    { 0xF8, 0x20, 0xF8, 0x20, 0x38 }, // shw mo 0x80
    { 0x40, 0xFC, 0x48, 0x20, 0x20 }, // shw ya 0x81
    { 0xF0, 0x10, 0x10, 0x10, 0xF8 }, // shw yu 0x82
    { 0xF8, 0x08, 0xF8, 0x08, 0xF8 }, // shw yo 0x83
    { 0xF8, 0x00, 0xF8, 0x10, 0x60 }, // shw ra 0x84
    { 0x88, 0x88, 0x88, 0x10, 0x60 }, // shw ri 0x85
    { 0x60, 0x60, 0x68, 0x68, 0xB0 }, // shw ru 0x86
    { 0x80, 0x80, 0x88, 0x88, 0xF0 }, // shw re 0x87
    { 0xF8, 0x88, 0x88, 0x88, 0xF8 }, // shw ro 0x88
    { 0xF8, 0x88, 0x08, 0x10, 0x60 }, // shw wa 0x89
    { 0x80, 0x48, 0x08, 0x10, 0xE0 }, // shw n 0x8A
    { 0xA0, 0xA0, 0x00, 0x00, 0x00 }, // shw voiced 0x8B
    { 0x40, 0xA0, 0x40, 0x00, 0x00 } // shw halfvoiced 0x8C
};

u1 GUIFontData1[][5];

/* Window sizes and positions
 *                       LOAD STAT INPT  OPT  VID  SND  CHT  NET GMKEY GUIOP  ABT RSET  SRC STCN MOVE CMBO ADDO CHIP PATH SAVE SPED */
static u4 GUIwinposxo[] = { 0, 6, 65, 33, 42, 5, 34, 6, 64, 8, 5, 33, 56, 64, 56, 5, 3, 28, 48, 6, 28, 53 };
static u4 GUIwinposyo[] = { 0, 20, 70, 20, 20, 20, 20, 20, 30, 30, 20, 20, 60, 30, 60, 20, 20, 60, 60, 20, 30, 20 };
u4 GUIwinsizex[] = { 0, 244, 126, 205, 180, 245, 188, 244, 128, 240, 245, 190, 144, 128, 144, 246, 250, 200, 160, 244, 200, 150 };
u4 GUIwinsizey[] = { 0, 190, 68, 192, 190, 190, 188, 191, 40, 170, 150, 190, 42, 40, 42, 190, 190, 120, 100, 190, 168, 180 };
u1 GUIwinptr;

static u4 SantaNextT = 36 * 15;
u4 NumSnow;
u4 SnowTimer = 36 * 30;

u1 savecfgforce;

#ifdef __MSDOS__
static u1 SubPalTable[256]; // Corresponding Gray Scale Color
#endif

// The first byte is the number of fields on the right not including the seperators
static u1 MenuDat1[] = { 12, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 2, 0 };
static u1 MenuDat2[] = { 8, 3, 1, 1, 0, 1, 1, 1, 0, 2, 0 };
static u1 MenuDat3[] = { 10, 3, 0, 1, 1, 0, 1, 1, 1, 1, 1, 2, 0 };
static u1 MenuDat4[] = { 2, 3, 1, 2, 0 };
#ifndef __MSDOS__
static u1 MenuDat5[] = { 0, 2, 0, 0 };
#else
static u1 MenuDat5[] = { 1, 3, 2, 0 };
#endif
static u1 MenuDat6[] = { 6, 3, 1, 1, 1, 1, 0, 2, 0 };

static bool GUIPalConv;
static char* GUICMessage;
static u1 OkaySC;
static u2 PrevResoln;
static u2 TBVal;
static u2 TBVali;
static u2 TGVal;
static u2 TGVali;
static u2 TRVal;
static u2 TRVali;
static u4 GUICTimer;
static u4 SnowMover;

static char GUIGameMenuData[][14] = {
    { "\x1"
      "LOAD        " },
    { "\x1"
      "RUN  [ESC]  " },
    { "\x1"
      "RESET       " },
    { "\x0"
      "------------" },
    { "\x1"
      "SAVE STATE  " },
    { "\x1"
      "OPEN STATE  " },
    { "\x1"
      "PICK STATE  " },
    { "\x0"
      "------------" },
    { "\x1"
      "QUIT        " }
};

static char GUIConfigMenuData[][14] = {
    { "\x1"
      "INPUT       " },
    { "\x0"
      "------------" },
    { "\x1"
      "DEVICES     " },
    { "\x1"
      "CHIP CFG    " },
    { "\x0"
      "------------" },
    { "\x1"
      "OPTIONS     " },
    { "\x1"
      "VIDEO       " },
    { "\x1"
      "SOUND       " },
    { "\x1"
      "PATHS       " },
    { "\x1"
      "SAVES       " },
    { "\x1"
      "SPEED       " }
};

static char GUICheatMenuData[][14] = {
    { "\x1"
      "ADD CODE    " },
    { "\x1"
      "BROWSE      " },
    { "\x1"
      "SEARCH      " }
};

static char GUINetPlayMenuData[][14] = {
#ifndef __MSDOS__
    { "\x1"
      "INTERNET    " },
    { "\x0"
      "------------" }
#else
    { "\x1"
      "MODEM       " },
    { "\x1"
      "IPX         " }
#endif
};

static char GUIMiscMenuData[][14] = {
    { "\x1"
      "MISC KEYS   " },
    { "\x1"
      "GUI OPTS    " },
    { "\x1"
      "MOVIE OPT   " },
    { "\x1"
      "KEY COMB.   " },
    { "\x1"
      "SAVE CFG    " },
    { "\x0"
      "------------" },
    { "\x1"
      "ABOUT       " }
};

u2 SnowData[] = {
    161, 251, 115, 211, 249, 87, 128, 101, 232, 176, 51, 180, 108, 193, 224, 112, 254, 159, 102, 238,
    223, 123, 218, 42, 173, 160, 143, 170, 64, 1, 174, 29, 34, 187, 194, 199, 40, 89, 232, 32,
    7, 195, 141, 67, 216, 48, 234, 1, 243, 116, 164, 182, 146, 136, 66, 70, 36, 43, 98, 208,
    63, 240, 216, 253, 147, 36, 33, 253, 98, 80, 228, 156, 73, 82, 85, 1, 97, 72, 187, 239,
    18, 196, 127, 182, 22, 22, 101, 25, 124, 145, 240, 213, 186, 22, 7, 161, 30, 98, 90, 197,
    22, 205, 32, 150, 59, 133, 49, 140, 10, 128, 142, 185, 176, 142, 220, 195, 100, 102, 105, 194,
    43, 139, 184, 153, 1, 95, 176, 169, 192, 201, 233, 243, 73, 65, 188, 14, 194, 39, 251, 140,
    239, 181, 142, 160, 242, 248, 82, 49, 9, 157, 233, 162, 254, 121, 112, 6, 118, 24, 56, 121,
    74, 209, 1, 223, 145, 6, 75, 73, 18, 168, 194, 168, 58, 39, 222, 170, 214, 75, 45, 218,
    39, 197, 242, 98, 22, 90, 255, 5, 144, 244, 252, 55, 98, 18, 135, 101, 27, 85, 215, 207,
    183, 28, 201, 142, 45, 122, 145, 159, 41, 243, 109, 29, 117, 203, 7, 234, 231, 214, 131, 133,
    217, 8, 74, 207, 130, 77, 21, 229, 167, 78, 218, 109, 142, 58, 134, 238, 29, 182, 178, 14,
    144, 129, 196, 219, 60, 128, 30, 105, 57, 53, 76, 122, 242, 208, 101, 241, 246, 99, 248, 67,
    137, 244, 70, 51, 202, 94, 164, 125, 115, 72, 61, 72, 129, 169, 155, 122, 91, 154, 160, 83,
    41, 102, 223, 218, 140, 40, 132, 16, 223, 92, 50, 230, 168, 47, 126, 117, 242, 136, 1, 245,
    171, 0, 36, 98, 73, 69, 14, 229, 66, 177, 108, 92, 39, 250, 243, 161, 111, 85, 211, 99,
    52, 98, 121, 188, 128, 201, 90, 205, 223, 92, 177, 19, 87, 18, 75, 54, 6, 81, 235, 137,
    247, 66, 211, 129, 247, 39, 119, 206, 116, 250, 113, 231, 190, 196, 53, 51, 34, 114, 39, 22,
    192, 33, 249, 151, 26, 22, 139, 97, 171, 238, 182, 88, 22, 176, 157, 255, 178, 199, 138, 98,
    140, 36, 112, 90, 25, 245, 134, 64, 48, 190, 165, 113, 24, 195, 84, 70, 175, 9, 179, 69,
    13, 26, 167, 237, 163, 159, 185, 128, 109, 114, 86, 74, 188, 103, 141, 48, 188, 203, 205, 191,
    215, 193, 224, 4, 153, 36, 108, 3, 172, 235, 56, 251, 211, 115, 173, 216, 240, 33, 78, 150,
    133, 64, 51, 103, 56, 26, 165, 222, 70, 148, 115, 119, 246, 229, 181, 63, 109, 49, 228, 108,
    126, 10, 170, 48, 87, 42, 193, 24, 28, 255, 176, 176, 209, 181, 97, 93, 61, 241, 201, 137,
    129, 97, 24, 159, 168, 215, 61, 113, 104, 143, 168, 7, 196, 216, 149, 239, 110, 65, 75, 143,
    238, 0, 37, 19, 8, 56, 65, 234, 228, 72, 42, 5, 226, 95, 243, 51, 55, 231, 114, 90,
    160, 141, 171, 108, 218, 252, 154, 64, 175, 142, 214, 211, 180, 129, 217, 118, 33, 130, 213, 2,
    73, 145, 93, 21, 162, 141, 97, 225, 112, 253, 49, 43, 113, 208, 131, 104, 31, 51, 192, 37,
    117, 186, 16, 45, 61, 114, 220, 6, 89, 163, 197, 203, 142, 80, 89, 115, 190, 190, 228, 15,
    166, 145, 59, 139, 120, 79, 104, 252, 246, 73, 113, 144, 224, 65, 204, 155, 221, 85, 31, 99,
    48, 253, 94, 159, 215, 31, 123, 204, 248, 153, 31, 210, 174, 178, 54, 146, 152, 88, 56, 92,
    197, 35, 124, 104, 211, 118, 1, 207, 108, 68, 123, 161, 107, 69, 143, 13, 79, 170, 130, 193,
    214, 153, 219, 247, 227, 2, 170, 208, 248, 139, 118, 241, 247, 183, 18, 135, 246, 126, 201, 46,
    70, 234, 171, 72, 18, 135, 236, 216, 32, 178, 148, 231, 161, 15, 6, 254, 34, 181, 5, 71,
    2, 219, 71, 87, 252, 16, 202, 190, 180, 83, 99, 209, 75, 134, 78, 84, 114, 32, 171, 246,
    125, 11, 57, 200, 102, 29, 176, 26, 205, 151, 152, 108, 100, 146, 117, 95, 71, 77, 158, 207,
    60, 192, 50, 135, 223, 237, 231, 53, 27, 195, 170, 146, 155, 160, 92, 224, 247, 187, 14, 50,
    203, 5, 153, 42, 17, 75, 109, 14, 78, 160, 236, 114, 131, 105, 189, 209, 233, 135, 221, 207,
    226, 119, 104, 10, 178, 107, 77, 160, 233, 179, 120, 227, 133, 241, 32, 223, 63, 247, 66, 157,
    140, 81, 118, 81, 63, 193, 173, 228, 214, 78, 124, 123, 222, 149, 9, 242, 0, 128, 194, 110
};

u1 SnowVelDist[] = {
    57, 92, 100, 19, 100, 184, 238, 225, 55, 240, 255, 221, 215, 105, 226, 153, 164, 41, 22, 93,
    176, 203, 155, 199, 244, 52, 233, 219, 110, 227, 229, 227, 152, 240, 83, 248, 226, 31, 163, 22,
    28, 156, 18, 10, 248, 67, 123, 167, 25, 138, 90, 10, 79, 107, 208, 229, 248, 233, 185, 10,
    167, 21, 19, 178, 132, 154, 81, 70, 20, 71, 95, 147, 72, 27, 91, 189, 13, 189, 102, 84,
    195, 123, 251, 93, 68, 36, 178, 59, 107, 99, 104, 191, 76, 110, 44, 206, 123, 46, 98, 112,
    26, 50, 1, 35, 150, 17, 242, 208, 69, 23, 202, 197, 59, 80, 136, 124, 40, 89, 11, 40,
    1, 136, 90, 72, 198, 83, 2, 174, 174, 4, 28, 205, 135, 35, 194, 54, 22, 40, 4, 132,
    191, 88, 163, 66, 204, 230, 35, 111, 9, 177, 254, 174, 163, 68, 5, 88, 111, 235, 58, 236,
    4, 248, 172, 154, 101, 164, 43, 223, 10, 13, 210, 125, 146, 73, 192, 57, 117, 152, 128, 36,
    106, 21, 253, 113, 110, 133, 244, 4, 150, 32, 76, 71, 22, 106, 210, 244, 46, 128, 27, 215,
    231, 112, 177, 196, 198, 120, 196, 57, 234, 74, 235, 108, 64, 181, 209, 188, 177, 63, 197, 200,
    126, 164, 136, 163, 48, 62, 225, 223, 212, 201, 195, 121, 90, 7, 10, 196, 88, 53, 39, 249,
    147, 98, 65, 253, 246, 3, 152, 125, 242, 105, 44, 129, 94, 232, 13, 4, 86, 220, 194, 67,
    186, 210, 171, 197, 64, 138, 89, 78, 58, 150, 52, 79, 138, 201, 244, 111, 106, 181, 192, 69,
    234, 253, 239, 113, 98, 37, 209, 151, 60, 47, 241, 235, 185, 52, 173, 94, 172, 182, 47, 150,
    80, 118, 10, 58, 161, 237, 10, 64, 238, 198, 14, 74, 132, 250, 234, 63, 169, 86, 158, 170,
    76, 168, 124, 133, 28, 203, 246, 140, 228, 77, 50, 53, 115, 113, 157, 218, 90, 192, 28, 209,
    72, 117, 156, 101, 226, 99, 11, 245, 69, 59, 17, 175, 164, 59, 8, 166, 163, 185, 10, 60,
    100, 19, 26, 38, 114, 232, 180, 115, 238, 184, 88, 103, 178, 67, 212, 21, 87, 64, 85, 1,
    62, 87, 155, 62, 21, 96, 205, 195, 131, 97, 191, 252, 218, 209, 179, 201, 12, 2, 234, 110,
    162, 14, 145, 170, 156, 105, 85, 132, 132, 60, 239, 14, 80, 129, 225, 144, 149, 244, 188, 8,
    13, 168, 181, 168, 30, 142, 24, 110, 26, 172, 231, 182, 50, 214, 66, 193, 100, 45, 132, 144,
    205, 190, 16, 133, 45, 250, 83, 183, 140, 229, 117, 226, 68, 59, 163, 96, 235, 227, 25, 155,
    209, 105, 41, 214, 30, 107, 2, 85, 180, 23, 241, 39, 113, 63, 75, 44, 107, 142, 93, 29,
    62, 240, 235, 152, 147, 52, 54, 146, 109, 112, 139, 162, 238, 198, 201, 8, 141, 115, 112, 106,
    4, 99, 25, 155, 111, 161, 114, 253, 75, 100, 28, 59, 101, 150, 2, 122, 228, 6, 12, 59,
    249, 181, 67, 136, 227, 227, 199, 46, 75, 203, 50, 25, 50, 61, 62, 22, 238, 124, 218, 134,
    243, 21, 243, 222, 94, 138, 161, 234, 133, 23, 138, 45, 4, 226, 154, 227, 8, 84, 105, 126,
    200, 127, 240, 144, 124, 197, 102, 144, 53, 29, 94, 231, 108, 175, 136, 37, 44, 183, 178, 95,
    41, 196, 214, 12, 42, 221, 106, 225, 151, 32, 53, 130, 24, 211, 88, 14, 135, 18, 90, 219,
    177, 129, 90, 217, 162, 181, 199, 133, 116, 56, 36, 100, 230, 91, 220, 83, 41, 65, 20, 64,
    177, 197, 249, 24, 242, 62, 26, 234, 92, 44, 167, 153, 243, 94, 179, 163, 103, 29, 220, 199,
    128, 94, 236, 152, 53, 32, 77, 78, 228, 89, 124, 85, 87, 50, 197, 116, 179, 105, 236, 139,
    102, 17, 159, 66, 176, 27, 205, 36, 113, 80, 60, 6, 61, 174, 254, 174, 246, 72, 154, 31,
    97, 40, 10, 8, 114, 203, 238, 26, 89, 51, 134, 110, 118, 176, 87, 32, 192, 210, 146, 207,
    88, 45, 156, 179, 61, 224, 87, 107, 107, 1, 252, 187, 203, 100, 169, 211, 205, 105, 12, 231,
    137, 176, 166, 37, 192, 241, 169, 84, 32, 85, 112, 168, 154, 7, 247, 146, 183, 225, 246, 173,
    57, 103, 110, 236, 113, 118, 203, 200, 22, 87, 251, 7, 138, 37, 12, 84, 221, 171, 51, 209,
    242, 37, 89, 73, 151, 162, 139, 189, 131, 209, 221, 96, 107, 144, 175, 79, 199, 123, 98, 138,
    226, 86, 221, 254, 72, 14, 126, 180, 200, 171, 85, 94, 120, 124, 196, 225, 150, 57, 219, 158
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
    ++SnowMover;
    if (GUIScrolTim1 != 0)
        --GUIScrolTim1;
    if (GUIDClickTL != 0)
        --GUIDClickTL;
    if (GUIkeydelay != 0)
        --GUIkeydelay;
    if (GUICTimer != 0)
        --GUICTimer;
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
    GUIGameMenuData[1][0] = v;
    GUIGameMenuData[2][0] = v;
    GUIGameMenuData[4][0] = v;
    GUIGameMenuData[5][0] = v;
    GUIGameMenuData[6][0] = v;
    GUICheatMenuData[0][0] = v;
    GUICheatMenuData[1][0] = v;
    GUICheatMenuData[2][0] = v;
    GUIMiscMenuData[2][0] = v;
}

static void ProcessSnowVelocity(void)
{
    if (MsgGiftLeft != 0)
        --MsgGiftLeft;

    if (NumSnow == 200) {
        if (SantaNextT != 0) {
            --SantaNextT;
        } else if (--SantaPos == 0) {
            SantaPos = 272;
            SantaNextT = 36 * 60;
        }
    } else {
        if (--SnowTimer == 0) {
            ++NumSnow;
            SnowTimer = 18;
        }
    }

    u4 i = 0;
    u4 n = NumSnow;
    while (n-- != 0) {
        SnowData[i * 2] += SnowVelDist[i * 2] + 4 * (u1)(100 - MusicRelVol);
        SnowData[i * 2 + 1] += SnowVelDist[i * 2 + 1] + 256;
        if (SnowData[i * 2 + 1] <= 0x200)
            SnowVelDist[i * 2] |= 8;
        ++i;
    }
}

static void DrawSnow(void)
{
    static u1 const SantaData[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0,
        1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1,
        1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1,
        1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1
    };

    if (OkaySC != 0) {
        if (MsgGiftLeft != 0) {
            GUIOuttextwin(20, 210, "A GIFT TO YOU IN THE OPTIONS!", 228);
        }
        u1* dst = vidbuffer + SantaPos + 60 * 288;
        u1 const* src = SantaData;
        u4 h = 8;
        do {
            u4 w = 16;
            do {
                if (*src != 0)
                    *dst = 0;
                ++dst;
                ++src;
            } while (--w != 0);
            dst += 288 - 16;
        } while (--h != 0);
    }

    u1* const dst = vidbuffer;
    u4 i = 0;
    do {
        u4 const eax = (SnowData[i * 2 + 1] >> 8) * 288 + (SnowData[i * 2] >> 8) + 16;
        if ((SnowVelDist[i * 2] & 8) != 0)
            dst[eax] = 228 + (SnowVelDist[i * 2] & 0x03);
    } while (++i != 200);
    // Change Snow Displacement Values
    for (; SnowMover != 0; --SnowMover)
        ProcessSnowVelocity();
}

static void guipostvideoloop(void)
{
    do {
        GUIUnBuffer();
        DisplayBoxes();
        DisplayMenu();
        GUIBox3D(43, 90, 213, 163);
        GUIOuttextShadowed(55, 95, "VIDEO MODE CHANGED.");
        GUIOuttextShadowed(55, 150, "  PRESS SPACEBAR.");
        vidpastecopyscr();
        // Wait for all mouse and input data to be 0

        if (GUIkeydelay == 0)
            break;

        // This is to make all ports not register space bar from being pressed earlier
        pressed[0x2C] = 0;

        JoyRead();
    } while (pressed[0x39] == 0);
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
    while (*end != '\0' && *end != '$')
        ++end;
    char(*guipostvidptr)[26] = guipostvidmsg3b;
    for (;; ++src, ++guipostvidptr) {
        u4 n = end - src;
        for (;;) {
            if (n == 0)
                goto notext;
            if (n < lengthof(*guipostvidptr))
                break;
            while (src[--n] != ' ') { } // XXX potential buffer underrun
        }
        char* dst = *guipostvidptr;
        do {
            char c = *src++;
            if (c == '$')
                c = '\0';
            *dst++ = c;
        } while (--n != 0);
        *dst = '\0';
        if (*src == '\0' || *src == '$')
            break; // XXX buffer overrun if there are more than 3 lines of text
    }
notext:

    memset(pressed, 0, 256); // XXX maybe should be sizeof(pressed)
    GUIUnBuffer();
    DisplayBoxes();
    DisplayMenu();
    GUIBox3D(43, 90, 213, 163);
    GUIOuttextShadowed(55, 95, "VIDEO MODE CHANGE FAILED.");
    GUIOuttextShadowed(55, 107, "UNABLE TO INIT VESA2:");
    GUIOuttextShadowed(55, 118, guipostvidmsg3b[0]);
    GUIOuttextShadowed(55, 128, guipostvidmsg3b[1]);
    GUIOuttextShadowed(55, 138, guipostvidmsg3b[2]);
    GUIOuttextShadowed(55, 151, "PRESS ANY KEY");
    vidpastecopyscr();
    GUIUnBuffer();
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
    if (GUIwinactiv[id] != 1) {
        GUIwinorder[GUIwinptr++] = id;
        GUIwinactiv[id] = 1;
        if (savewinpos == 0) {
            GUIwinposx[id] = GUIwinposxo[id];
            GUIwinposy[id] = GUIwinposyo[id];
        }
    } else {
        u4 i = 0;
        // look for match
        while (GUIwinorder[i] != id)
            ++i;
        for (; ++i != GUIwinptr;) {
            GUIwinorder[i - 1] = GUIwinorder[i];
        }
        GUIpclicked = 0;
        GUIwinorder[i - 1] = id;
    }
}

static void GUICheckMenuItem(u4 const id, u4 const row)
{
    if (GUIcrowpos == row)
        CheckMenuItemHelp(id);
}

// Defines which menu item calls what window number
void GUITryMenuItem(void)
{
    if (GUIcmenupos == 1) {
        if (GUIcrowpos < 10) {
            loadquickfname(GUIcrowpos);
            return;
        }
        if (GUIcrowpos == 11) {
            prevlfreeze ^= 1;
            memcpy(GUIPrevMenuData + 347, prevlfreeze != 0 ? " ON " : " OFF", 4);
        }
        if (GUIcrowpos == 12 && prevlfreeze == 0) {
            memset(prevloadiname, ' ', sizeof(prevloadiname));
            memset(prevloaddnamel, '\0', sizeof(prevloaddnamel));
            memset(prevloadfnamel, '\0', sizeof(prevloadfnamel));
            GUIQuickLoadUpdate();
            return;
        }
    }
    if (GUIcmenupos == 2) {
        GUICheckMenuItem(1, 0); // Load
        if (GUIcrowpos == 0) {
            GUIcurrentfilewin = 0;
            GetLoadData();
            return;
        }
        if (romloadskip == 0) {
            if (GUIcrowpos == 1) { // Run
                GUIQuit = 2;
                return;
            }
            GUICheckMenuItem(12, 2); // Reset
            if (GUIcrowpos == 2) {
                GUICResetPos = 1;
            }
            if (GUIcrowpos == 4) {
                GUIStatesText5 = 0;
                GUICStatePos = 1;
            }
            if (GUIcrowpos == 5) {
                GUIStatesText5 = 1;
                GUICStatePos = 1;
            }
            GUICheckMenuItem(14, 4); // Save State
            GUICheckMenuItem(14, 5); // Load State
            GUICheckMenuItem(2, 6); // Select State
        }
        if (GUIcrowpos == 8)
            GUIQuit = 1;
    }
    if (GUIcmenupos == 3) {
        // The number on the left is the window to open
        // the number on the right is where in the drop down box we are
        GUICheckMenuItem(3, 0); // Input #1-5
        GUICheckMenuItem(17, 2); // Devices
        GUICheckMenuItem(18, 3); // Chip Config
        GUICheckMenuItem(4, 5); // Options
        if (GUIcrowpos == 6) // Video
        {
            // set Video cursor location
            u4 const v = cvidmode;
            u4 const n = NumVideoModes;
            GUIcurrentvideocursloc = v;
            GUIcurrentvideoviewloc = n <= 20 ? 0 : n - 20 < v ? n - 20
                                                              : v;
            CheckMenuItemHelp(5);
        }
        GUICheckMenuItem(6, 7); // Sound
        GUICheckMenuItem(19, 8); // Paths
        GUICheckMenuItem(20, 9); // Saves
        GUICheckMenuItem(21, 10); // Speed
    }
    if (romloadskip == 0 && GUIcmenupos == 4) {
        GUICheckMenuItem(7, 0);
        GUICheckMenuItem(7, 1);
        GUICheckMenuItem(13, 2);
        if (GUIcrowpos == 0)
            GUIcurrentcheatwin = 1;
        if (GUIcrowpos == 1)
            GUIcurrentcheatwin = 0;
    }
    if (GUIcmenupos == 6) {
        GUICheckMenuItem(9, 0);
        GUICheckMenuItem(10, 1);
        if (romloadskip == 0) {
            GUICheckMenuItem(15, 2);
            if (GUIcrowpos == 2)
                MovieRecordWinVal = 0;
        }
        GUICheckMenuItem(16, 3); // Save Config
        if (GUIcrowpos == 4) {
            savecfgforce = 1;
            GUISaveVars();
            savecfgforce = 0;

            Makemode7Table();
            GUICMessage = "CONFIGURATION FILES SAVED.";
            GUICTimer = 50;
        }
        GUICheckMenuItem(11, 6);
    }
}

void GUIProcStates(void)
{
    GUIwinactiv[14] = 0;
    GUIwinorder[--GUIwinptr] = 0;
    u1 const h = GUICBHold;
    GUICBHold = 0;
    if (h != 10)
        return;
    if (GUIStatesText5 != 1) {
        statesaver();
    } else {
        loadstate2();
    }
}

void GUIProcReset(void)
{
    if (GUICBHold == 2) {
        GUIReset = 1;
        if (MovieProcessing == 2) // Recording
        {
            ResetDuringMovie();
        } else {
            GUIDoReset();
        }
    }
    GUICBHold = 0;
    GUIwinactiv[12] = 0;
    GUIwinorder[--GUIwinptr] = 0;
}

void GUIUnBuffer(void)
{
    // copy from spritetable
    u1* const dst = vidbuffer;
    memcpy(dst, spritetablea + 288 * 8, 65536);
    memset(dst + 65536, 0x01, 288 * 8);
}

static void GUIBufferData(void)
{
    // copy to spritetable
    u4 const n =
#ifdef __MSDOS__
        cbitmode != 1 ? 65536 :
#endif
                      131072;
    u1* src = vidbuffer;
    if (PrevResoln != 224)
        src += 288 * 8;
    memcpy(spritetablea + 288 * 8, src, n);
    memset(sprlefttot, 0, sizeof(sprlefttot));
    memset(sprleftpr, 0, sizeof(sprleftpr));
    memset(sprleftpr1, 0, sizeof(sprleftpr1));
    memset(sprleftpr2, 0, sizeof(sprleftpr2));
    memset(sprleftpr3, 0, sizeof(sprleftpr3));
}

static void InitGUI(void)
{
#ifdef __MSDOS__
    asm_call(DOSClearScreen);
#endif
    Clear2xSaIBuffer();
    GUISetPal();
    GUIBufferData();
}

static void DecPalVal(void)
{
    TRVal -= TRVali;
    TGVal -= TGVali;
    TBVal -= TBVali;
    if (TRVal & 0x8000)
        TRVal = 0;
    if (TGVal & 0x8000)
        TGVal = 0;
    if (TBVal & 0x8000)
        TBVal = 0;
}

static void GUIPal16b(u4 const idx, u4 const r, u4 const g, u4 const b)
{
    GUICPC[idx] = r >> 1 << 11 | g << 5 | b >> 1;
}

static void GUISetPal16(void)
{
    // set palette
    { // Fixed Color Scale = 0 .. 31
        GUIPal16b(0, 0, 0, 0);
        u4 i = 1;
        do
            GUIPal16b(i, i + GUIRAdd, i + GUIGAdd, i + GUIBAdd);
        while (++i != 32);
    }

    { // gray scale = 32 .. 63
        u4 i = 32;
        // XXX values too large: 64..126
        do
            GUIPal16b(i, i * 2, i * 2, i * 2);
        while (++i != 64);
    }

    { // shadow = 96 .. 127
        u4 i = 0;
        do {
            u1 const r = (i + GUIRAdd) * 3 / 4;
            u1 const g = (i + GUIGAdd) * 3 / 4;
            u1 const b = (i + GUIBAdd) * 3 / 4;
            GUIPal16b(96 + i, r, g, b);
        } while (++i != 32);
    }

    // 0,10,31
    TRVal = GUITRAdd;
    TGVal = GUITGAdd;
    TBVal = GUITBAdd;
    u2 const r = (TRVal + 1) / 8;
    TRVali = r;
    TRVal += r * 8;
    u2 const g = (TGVal + 1) / 8;
    TGVali = g;
    TGVal += g * 8;
    u2 const b = (TBVal + 1) / 8;
    TBVali = b;
    TBVal += b * 8;

    GUIPal16b(64, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(65, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(66, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(67, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(68, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(69, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(70, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(71, TRVal, TGVal, TBVal);

    GUIPal16b(72, 40, 0, 20);
    GUIPal16b(73, 34, 0, 21);

    GUIPal16b(80, 0, 10, 28);
    GUIPal16b(81, 0, 10, 27);
    GUIPal16b(82, 0, 10, 25);
    GUIPal16b(83, 0, 9, 24);
    GUIPal16b(84, 0, 8, 22);
    GUIPal16b(85, 0, 7, 20);
    GUIPal16b(86, 0, 6, 18);
    GUIPal16b(87, 0, 5, 15);
    GUIPal16b(88, 20, 0, 10);
    GUIPal16b(89, 17, 0, 10);

    { // Orange Scale
        u4 i = 0;
        do {
            ++i;
            GUIPal16b(127 + i, 63, i * 2, i);
        } while (i != 20);
    }

    // Blue scale = 148 .. 167
    TRVal = GUIWRAdd * 2;
    TGVal = GUIWGAdd * 2;
    TBVal = GUIWBAdd * 2;
    TRVali = 4;
    TGVali = 4;
    TBVali = 4;

    GUIPal16b(152, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(151, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(150, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(149, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(148, TRVal, TGVal, TBVal);

    TRVal = GUIWRAdd * 2;
    TGVal = GUIWGAdd * 2;
    TBVal = GUIWBAdd * 2;
    TRVali = 4;
    TGVali = 4;
    TBVali = 4;
    TRVal -= TRVal / 4;
    TGVal -= TGVal / 4;
    TBVal -= TBVal / 4;

    GUIPal16b(157, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(156, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(155, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(154, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(153, TRVal, TGVal, TBVal);

    TRVal = GUIWRAdd * 2;
    TGVal = GUIWGAdd * 2;
    TBVal = GUIWBAdd * 2;
    TRVali = 4;
    TGVali = 4;
    TBVali = 4;
    TRVal -= TRVal / 2;
    TGVal -= TGVal / 2;
    TBVal -= TBVal / 2;

    GUIPal16b(162, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(161, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(160, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(159, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(158, TRVal, TGVal, TBVal);

    GUIPal16b(163, 40, 40, 0);
    GUIPal16b(164, 30, 30, 0);
    GUIPal16b(165, 50, 0, 0);
    GUIPal16b(166, 35, 0, 0);
    GUIPal16b(167, 0, 0, 0);

    // Blue scale shadow
    TRVal = GUIWRAdd;
    TGVal = GUIWGAdd;
    TBVal = GUIWBAdd;
    TRVali = 2;
    TGVali = 2;
    TBVali = 2;

    GUIPal16b(172, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(171, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(170, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(169, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(168, TRVal, TGVal, TBVal);

    TRVal = GUIWRAdd;
    TGVal = GUIWGAdd;
    TBVal = GUIWBAdd;
    TRVali = 2;
    TGVali = 2;
    TBVali = 2;
    DecPalVal();
    DecPalVal();

    GUIPal16b(177, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(176, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(175, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(174, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(173, TRVal, TGVal, TBVal);

    TRVal = GUIWRAdd;
    TGVal = GUIWGAdd;
    TBVal = GUIWBAdd;
    TRVali = 2;
    TGVali = 2;
    TBVali = 2;
    DecPalVal();
    DecPalVal();
    DecPalVal();
    DecPalVal();

    GUIPal16b(182, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(181, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(180, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(179, TRVal, TGVal, TBVal);
    DecPalVal();
    GUIPal16b(178, TRVal, TGVal, TBVal);

    GUIPal16b(183, 20, 20, 0);
    GUIPal16b(184, 15, 15, 0);
    GUIPal16b(185, 25, 0, 0);
    GUIPal16b(186, 17, 0, 0);
    GUIPal16b(187, 0, 0, 0);

    { // gray scale2 = 189 .. 220
        u4 i = 0;
        do
            GUIPal16b(189 + i, i * 3 / 2, i * 3 / 2, i * 2);
        while (++i != 64); // XXX comment says till 220, but loop goes to 252
    }

    GUIPal16b(221, 0, 55, 0);
    GUIPal16b(222, 0, 45, 0);
    GUIPal16b(223, 0, 25, 0);

    GUIPal16b(224, 40, 0, 20);
    GUIPal16b(225, 32, 0, 15);

    GUIPal16b(226, 20, 0, 10);
    GUIPal16b(227, 16, 0, 7);

    GUIPal16b(228, 45, 45, 50);
    GUIPal16b(229, 40, 40, 45);
    GUIPal16b(230, 35, 35, 40);
    GUIPal16b(231, 30, 30, 35);

    GUIPal16b(232, 35, 15, 15);

    GUIPal16b(233, 50, 12, 60);
    GUIPal16b(234, 30, 14, 60);
    GUIPal16b(235, 12, 60, 25);
    GUIPal16b(236, 14, 42, 25);
    GUIPal16b(237, 60, 20, 25);
    GUIPal16b(238, 42, 20, 25);

    if (!GUIPalConv) {
        GUIPalConv = true;
        u1* buf = vidbuffer;
        u4 n = 288 * 240;
        u4 i = 0;
        do {
            u2 const c = ((u2 const*)buf)[i];
            u4 const r = c >> 11 & 0x1F;
            u4 const g = c >> 6 & 0x1F;
            u4 const b = c & 0x1F;
            static u1 const multab[] = {
                1, 1, 1, 2, 2, 3, 4, 4, 5, 6, 6, 7, 8, 8, 9, 10,
                10, 11, 12, 12, 13, 14, 14, 15, 16, 16, 17, 18, 18, 19, 20, 20,
                21, 22, 22, 23, 24, 24, 25, 26, 26, 27, 28, 28, 29, 30, 30, 31
            };
            buf[i] = multab[(r + g + b) / 2];
        } while (++i, --n != 0);
    }
}

#ifdef __MSDOS__
static void GUIconvpal(void)
{
    tempco0 = cgram[0];
    if ((scaddtype & 0xA0) == 0x20) {
        u2 const c = cgram[0];
        u4 r = (c & 0x1F) + coladdr;
        if (r >= 0x1F)
            r = 0x1F;
        u4 g = (c >> 5 & 0x1F) + coladdg;
        if (g >= 0x1F)
            g = 0x1F;
        u4 b = (c >> 10 & 0x1F) + coladdb;
        if (b >= 0x1F)
            b = 0x1F;
        cgram[0] = b << 10 | g << 5 | r;
    }
    u2 const* src = cgram;
    u2* dst = prevpal;
    u4 i = 0;
    do {
        u2 const c = *dst++ = *src++;
        u4 const curgsval = (c & 0x1F) * maxbr / 15 + (c >> 5 & 0x1F) * maxbr / 15 + (c >> 10 & 0x1F) * maxbr / 15;
        u4 bl = curgsval / 3;
        if (MessageOn != 0 && i == 128)
            bl = 31;
        if (bl == 0)
            bl = 1;
        SubPalTable[i] = bl;
    } while (++i != 256);
    prevbright = maxbr;
    cgram[0] = tempco0;
}

static void GUIRGB(u1 const r, u1 const g, u1 const b)
{
    outb(0x03C9, r);
    outb(0x03C9, g);
    outb(0x03C9, b);
}

static void GUIPal(u1 const idx, u1 const r, u1 const g, u1 const b)
{
    outb(0x03C8, idx);
    GUIRGB(r, g, b);
}
#endif

void GUISetPal(void)
{
#ifdef __MSDOS__
    if (cbitmode != 1) {
        // set palette
        { // Fixed Color Scale = 0 .. 31
            GUIPal(0, 0, 0, 0);

            outb(0x03C8, 1);
            u4 i = 0;
            do
                GUIRGB(i + GUIRAdd, i + GUIGAdd, i + GUIBAdd);
            while (++i != 32);
        }

        { // gray scale = 32 .. 63
            outb(0x03C8, 32);
            u4 i = 32;
            // XXX values too large: 64..126
            do
                GUIRGB(2 * i, 2 * i, 2 * i);
            while (++i != 64);
        }

        { // shadow = 96 .. 127
            u4 i = 0;
            outb(0x03C8, 96);
            do {
                u1 const r = (i + GUIRAdd) * 3 / 4;
                u1 const g = (i + GUIGAdd) * 3 / 4;
                u1 const b = (i + GUIBAdd) * 3 / 4;
                GUIRGB(r, g, b);
            } while (++i != 32);
        }

        { // 0, 10, 31
            TRVal = GUITRAdd;
            TGVal = GUITGAdd;
            TBVal = GUITBAdd;
            u2 const r = (TRVal + 1) / 8;
            TRVali = r;
            TRVal += r * 8;
            u2 const g = (TGVal + 1) / 8;
            TGVali = g;
            TGVal += g * 8;
            u2 const b = (TBVal + 1) / 8;
            TBVali = b;
            TBVal += b * 8;
        }

        GUIPal(64, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(65, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(66, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(67, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(68, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(69, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(70, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(71, TRVal, TGVal, TBVal);

        GUIPal(72, 40, 0, 20);
        GUIPal(73, 34, 0, 21);

        GUIPal(80, 0, 10, 28);
        GUIPal(81, 0, 10, 27);
        GUIPal(82, 0, 10, 25);
        GUIPal(83, 0, 9, 24);
        GUIPal(84, 0, 8, 22);
        GUIPal(85, 0, 7, 20);
        GUIPal(86, 0, 6, 18);
        GUIPal(87, 0, 5, 15);
        GUIPal(88, 20, 0, 10);
        GUIPal(89, 17, 0, 10);

        { // Orange Scale
            outb(0x03C8, 128);
            u4 i = 0;
            do {
                ++i;
                GUIRGB(63, i * 2, i);
            } while (i != 20);
        }

        // Blue scale = 148 .. 167
        TRVal = GUIWRAdd * 2;
        TGVal = GUIWGAdd * 2;
        TBVal = GUIWBAdd * 2;
        TRVali = 4;
        TGVali = 4;
        TBVali = 4;

        GUIPal(152, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(151, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(150, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(149, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(148, TRVal, TGVal, TBVal);

        TRVal = GUIWRAdd * 2;
        TGVal = GUIWGAdd * 2;
        TBVal = GUIWBAdd * 2;
        TRVali = 4;
        TGVali = 4;
        TBVali = 4;
        DecPalVal();
        DecPalVal();

        GUIPal(157, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(156, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(155, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(154, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(153, TRVal, TGVal, TBVal);

        TRVal = GUIWRAdd * 2;
        TGVal = GUIWGAdd * 2;
        TBVal = GUIWBAdd * 2;
        TRVali = 4;
        TGVali = 4;
        TBVali = 4;
        DecPalVal();
        DecPalVal();
        DecPalVal();
        DecPalVal();

        GUIPal(162, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(161, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(160, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(159, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(158, TRVal, TGVal, TBVal);

        GUIPal(163, 40, 40, 0);
        GUIPal(164, 30, 30, 0);
        GUIPal(165, 50, 0, 0);
        GUIPal(166, 35, 0, 0);
        GUIPal(167, 0, 0, 0);

        // Blue scale shadow
        TRVal = GUIWRAdd;
        TGVal = GUIWGAdd;
        TBVal = GUIWBAdd;
        TRVali = 2;
        TGVali = 2;
        TBVali = 2;

        GUIPal(172, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(171, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(170, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(169, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(168, TRVal, TGVal, TBVal);

        TRVal = GUIWRAdd;
        TGVal = GUIWGAdd;
        TBVal = GUIWBAdd;
        TRVali = 2;
        TGVali = 2;
        TBVali = 2;
        DecPalVal();
        DecPalVal();

        GUIPal(177, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(176, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(175, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(174, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(173, TRVal, TGVal, TBVal);

        TRVal = GUIWRAdd;
        TGVal = GUIWGAdd;
        TBVal = GUIWBAdd;
        TRVali = 2;
        TGVali = 2;
        TBVali = 2;
        DecPalVal();
        DecPalVal();
        DecPalVal();
        DecPalVal();

        GUIPal(182, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(181, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(180, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(179, TRVal, TGVal, TBVal);
        DecPalVal();
        GUIPal(178, TRVal, TGVal, TBVal);

        GUIPal(183, 20, 20, 0);
        GUIPal(184, 15, 15, 0);
        GUIPal(185, 25, 0, 0);
        GUIPal(186, 17, 0, 0);
        GUIPal(187, 0, 0, 0);

        { // gray scale2 = 189 .. 220
            outb(0x03C8, 189);
            u4 i = 0;
            do
                GUIRGB(i * 3 / 2, i * 3 / 2, i / 2);
            while (++i != 64); // XXX comment says till 220, but loop goes to 252
        }

        GUIPal(221, 0, 55, 0);
        GUIPal(222, 0, 45, 0);
        GUIPal(223, 0, 25, 0);

        GUIPal(224, 40, 0, 20);
        GUIPal(225, 32, 0, 15);

        GUIPal(226, 20, 0, 10);
        GUIPal(227, 16, 0, 7);

        GUIPal(228, 45, 45, 50);
        GUIPal(229, 40, 40, 45);
        GUIPal(230, 35, 35, 40);
        GUIPal(231, 30, 30, 35);

        GUIPal(232, 35, 15, 15);

        GUIPal(233, 50, 12, 60);
        GUIPal(234, 30, 14, 60);

        if (!GUIPalConv) {
            GUIPalConv = true;
            // Convert Image data to Gray Scale
            // Create Palette Table
            GUIconvpal();
            // Convert Current Image in Buffer
            u1* buf = vidbuffer;
            u4 n = 288 * 240;
            do
                *buf = SubPalTable[*buf];
            while (++buf, --n != 0);
        }
    } else
#endif
    {
        GUISetPal16();
    }
}

void convertnum(char* dst, u4 val)
{
    char buf[10];
    char* b = buf;
    do
        *b++ = '0' + val % 10;
    while ((val /= 10) != 0);
    do
        *dst++ = *--b;
    while (b != buf);
    *dst = '\0';
}

void converthex(char* dst, u4 val, u4 n)
{
    val <<= (4 - n) * 8;
    n *= 2;
    do {
        *dst++ = "0123456789ABCDEF"[val >> 28 & 0xF];
        val <<= 4;
    } while (--n != 0);
    *dst = '\0';
}

static void loadmenuopen(u4 const param1) // XXX better parameter name
{
    GUIpmenupos = GUIcmenupos;
    GUIcmenupos = 0;
    if (GUIwinactiv[param1] != 1) {
        GUIwinorder[GUIwinptr++] = param1;
        GUIwinactiv[param1] = 1;
        if (savewinpos == 0) {
            GUIwinposx[param1] = GUIwinposxo[param1];
            GUIwinposy[param1] = GUIwinposyo[param1];
        }
    } else {
        // look for match
        u4 i = 0;
        u1 bl; // XXX better variable name
        do
            bl = GUIwinorder[i++];
        while (bl != param1);
        for (; i != GUIwinptr; ++i)
            GUIwinorder[i - 1] = GUIwinorder[i];
        GUIpclicked = 0;
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
    GUIOuttext(x, y, text, 220);
}

static char const guiftimemsg8[] = "PRESS SPACEBAR TO PROCEED.";

static void guifirsttimemsg(void)
{
    memset(pressed, 0, 256); // XXX maybe should be sizeof(pressed)
    pressed[0x2C] = 0; // XXX redundant

    do {
        GUIBox3D(43, 75, 213, 163);
        GUIOuttextShadowed(51, 80, " ONE-TIME USER REMINDER : ");
        GUIOuttextShadowed(51, 95, "  PLEASE BE SURE TO READ  ");
        GUIOuttextShadowed(51, 103, "THE DOCUMENTATION INCLUDED");
        GUIOuttextShadowed(51, 111, " WITH ZSNES FOR IMPORTANT");
        GUIOuttextShadowed(51, 119, " INFORMATION AND ANSWERS");
        GUIOuttextShadowed(51, 127, "    TO COMMON PROBLEMS");
        GUIOuttextShadowed(51, 135, "      AND QUESTIONS.");
        GUIOuttextShadowed(51, 150, guiftimemsg8);
        vidpastecopyscr();
        GUIUnBuffer();
        DisplayBoxes();
        DisplayMenu();
        JoyRead();
    } while (pressed[0x39] == 0);
}

static void horizonfixmsg(void)
{
    memset(pressed, 0, 256); // XXX maybe should be sizeof(pressed)
    pressed[0x2C] = 0; // XXX redundant

    char const* const* const msg = horizon_get(GetTime());
    do {
        GUIBox3D(43, 75, 213, 163);
        GUIOuttextShadowed(51, 80, "     WELCOME TO ZSNES");
        GUIOuttextShadowed(51, 95, msg[0]);
        GUIOuttextShadowed(51, 103, msg[1]);
        GUIOuttextShadowed(51, 111, msg[2]);
        GUIOuttextShadowed(51, 119, msg[3]);
        GUIOuttextShadowed(51, 150, guiftimemsg8);
        vidpastecopyscr();
        GUIUnBuffer();
        DisplayBoxes();
        DisplayMenu();
        JoyRead();
    } while (pressed[0x39] == 0);
}

void StartGUI(void)
{
    static u1 MouseInitOkay = 0;

#ifdef __OPENGL__
    if (FilteredGUI == 0 && BilinearFilter == 1)
        blinit = 1;
#endif
    GUILoadPos = 0;
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

    memset(SpecialLine, 0, sizeof(SpecialLine));

    GUIOn = 1;
    GUIOn2 = 1;
    NumCombo = GUIComboGameSpec != 0 ? NumComboLocl : NumComboGlob;
#ifdef __MSDOS__
    asm_call(ResetTripleBuf);
#endif

    if (GUIwinposx[16] == 0) {
        GUIwinposx[16] = 3;
        GUIwinposy[16] = 22;
    }

    GUICTimer = 0;
    { // Initialize volume
        u4 vol = (u4)MusicRelVol * 128 / 100;
        if (vol > 127)
            vol = 127;
        MusicVol = vol;
    }
    CheatSearchStatus = 0;
    if (newgfx16b != 0)
        memset(vidbufferofsb, 0, 256 * 144 * 4);
    ShowTimer = 1;
    if ((GetDate() & 0xFFFF) == 0x0C25)
        OkaySC = 1;
    lastmouseholded = 1;
    if (GUIwinposx[15] == 0) { // Movie menu fix
        GUIwinposx[15] = 50;
        GUIwinposy[15] = 50;
    }
    PrevResoln = resolutn;
    resolutn = 224;

    GUIPalConv = false;
    MousePRClick = 1;

    if (MouseInitOkay != 1) {
        MouseInitOkay = 1;
        if (MouseDis != 1 && Init_Mouse() == 0)
            MouseDis = 1;
    }

    if (pressed[KeyQuickLoad] & 1) {
        GUIcmenupos = 0;
        loadmenuopen(1);
    }
    memset(pressed, 0, 256 + 128 + 32); // XXX 32 probably should be 64
    pressed[1] = 2;
    GUIescpress = 1;

    // set Video cursor location
    u4 eax = cvidmode;
    GUIcurrentvideocursloc = eax;
    u4 ebx = NumVideoModes;
    if (ebx > 20) {
        ebx -= 20;
        if (eax > ebx)
            eax = ebx;
        GUIcurrentvideoviewloc = eax;
    } else {
        GUIcurrentvideoviewloc = 0;
    }

    SaveSramData();
    GUIQuickLoadUpdate();

    LoadDetermine();

    if (AutoState != 0 && romloadskip == 0)
        SaveSecondState();

    GUIInit();
    memset(pressed, 0, 256); // XXX probably + 128 + 64 missing, maybe even completely redundant (has been zeroed above)

    if (GUIwinptr != 0) {
        GUIcmenupos = 0;
    } else if (esctomenu != 0) {
        GUIcmenupos = 2;
        GUIcrowpos = 0;
        GUICYLocPtr = MenuDat2;
        if (esctomenu != 1)
            GUIcmenupos = 0;
    }
    if (GUIwinactiv[1] != 0) {
        GUIcurrentfilewin = 0;
        GetLoadData();
    }
    GUIHold = 0;
    // clear 256 bytes from hirestiledat
    memset(hirestiledat, 0, sizeof(hirestiledat));
    curblank = 0;
    InitGUI();

    if (CheatWinMode != 0)
        LoadCheatSearchFile();

    GUIQuit = 0;
    while (GUIQuit != 2) {
        if (GUIQuit == 1) {
            GUIDeInit();

            resolutn = PrevResoln;
            endprog();
            return;
        }
        GUIQuit = 0;
        if (MouseDis != 1) {
            ProcessMouse();
            if (videotroub == 1)
                return;
        }
        GUIUnBuffer();
        if (GUIEffect == 1)
            DrawSnow();
        if (GUIEffect == 2)
            DrawWater();
        if (GUIEffect == 3)
            DrawWater();
        if (GUIEffect == 4)
            DrawBurn();
        if (GUIEffect == 5)
            DrawSmoke();

        DisplayBoxes();

        DisplayMenu();
        if (MouseDis != 1)
            DrawMouse();
        if (FirstTimeData == 0) {
            guifirsttimemsg();
            FirstTimeData = 1;
        }

        static bool showed_msg = false;
        if (!showed_msg && (GetDate() & 0xFFFF) == 0x0401) {
            showed_msg = true;
            horizonfixmsg();
        }

        if (GUICTimer != 0) {
            GUIOuttext(21, 211, GUICMessage, 50);
            GUIOuttext(20, 210, GUICMessage, 63);
        }
        vidpastecopyscr();
        GUIgetcurrentinput();
    }
    memset(spcBuffera, 0, 256 * 1024);
    GUIDeInit();
#ifdef __MSDOS__
    asm_call(DOSClearScreen);
    if (cbitmode == 0)
        dosmakepal();
#endif
    t1cc = 1;

    GUISaveVars();

    MousePRClick = 1;
    prevbright = 0;
    resolutn = PrevResoln;

    CheatOn = NumCheats != 0;

    if (CopyRamToggle == 1) {
        CopyRamToggle = 0;
        // copy 128k ram
        memcpy(vidbuffer + 129600, wramdata, 128 * 1024);
    }

    if (CheatWinMode == 2)
        CheatWinMode = 1;

    if (CheatWinMode != 0)
        SaveCheatSearchFile();

    memset(vidbuffer, 0, 288 * 120 * 4);

    memset(vidbufferofsb, 0, 256 * 144 * 4);

    AdjustFrequency();
    GUIOn = 0;
    GUIOn2 = 0;
    GUIReset = 0;
    continueprog();
}

void guimencodermsg(void)
{
    memset(pressed, 0, 256); // XXX maybe should be sizeof(pressed)
    pressed[0x2C] = 0; // XXX redundant

    do {
        GUIBox3D(43, 75, 213, 163);
        GUIOuttextShadowed(51, 95, " MENCODER IS MISSING: ");
        GUIOuttextShadowed(51, 133, "PRESS SPACE TO PROCEED");
        vidpastecopyscr();
        GUIUnBuffer();
        DisplayBoxes();
        DisplayMenu();
        JoyRead();
    } while (pressed[0x39] == 0);
}

void guilamemsg(void)
{
    memset(pressed, 0, 256); // XXX maybe should be sizeof(pressed)
    pressed[0x2C] = 0; // XXX redundant

    do {
        GUIBox3D(43, 75, 213, 163);
        GUIOuttextShadowed(51, 95, " LAME IS MISSING: ");
        GUIOuttextShadowed(51, 133, "PRESS SPACE TO PROCEED");
        vidpastecopyscr();
        GUIUnBuffer();
        DisplayBoxes();
        DisplayMenu();
        JoyRead();
    } while (pressed[0x39] == 0);
}

u1* GetAnyPressedKey(void)
{
    for (u1* i = pressed; i != endof(pressed); ++i)
        if (*i != 0)
            return i;
    return 0;
}

static u4 GetMouseButtons(void)
{
    if (MouseDis == 1)
        return 0;

    u4 buttons = Get_MouseData() & 0x0000FFFFU;
    if (lhguimouse == 1)
        buttons = SwapMouseButtons(buttons);
    return buttons;
}

void guiprevideo(void)
{
    memset(pressed, 0, 256); // XXX maybe should be sizeof(pressed)

    GUIUnBuffer();
    DisplayBoxes();
    DisplayMenu();
    GUIBox3D(43, 90, 213, 163);
    GUIOuttextShadowed(55, 95, "ZSNES WILL NOW ATTEMPT");
    GUIOuttextShadowed(55, 103, " TO CHANGE YOUR VIDEO");
    GUIOuttextShadowed(55, 111, " MODE.  IF THE CHANGE");
    GUIOuttextShadowed(55, 119, "IS UNSUCCESSFUL,  WAIT");
    GUIOuttextShadowed(55, 127, " 10 SECONDS AND VIDEO");
    GUIOuttextShadowed(55, 135, "MODE WILL BE RESTORED.");
    GUIOuttextShadowed(55, 150, "    PRESS ANY KEY.");
    vidpastecopyscr();
    pressed[0x2C] = 0; // XXX redundant
    for (;;) {
        JoyRead();

        u1* const key = GetAnyPressedKey();
        if (key) {
            *key = 0;
            return;
        }
        if (GetMouseButtons() & 0x01)
            return;
    }
}

void guicheaterror(void)
{
    memset(pressed, 0, sizeof(pressed));

    for (;;) {
        GUIUnBuffer();
        DisplayBoxes();
        DisplayMenu();
        GUIBox3D(75, 95, 192, 143);
        GUIOuttextShadowed(80, 100, "INVALID CODE!  YOU");
        GUIOuttextShadowed(80, 108, "MUST ENTER A VALID");
        GUIOuttextShadowed(80, 116, "GAME GENIE,PAR, OR");
        GUIOuttextShadowed(80, 124, "GOLD FINGER CODE.");
        GUIOuttextShadowed(80, 134, "PRESS ANY KEY.");
        vidpastecopyscr();
        JoyRead();

        if (GetAnyPressedKey())
            break;
        if (GetMouseButtons() & 0x01)
            break;
    }
    while (Check_Key() != 0)
        Get_Key();
    GUIcurrentcheatwin = 1;
    GUIpclicked = 1;
}

// Displays window when item is clicked
void DisplayBoxes(void)
{
    {
        u4 i = 0;
        while (GUIwinorder[i] != 0)
            ++i;
        cwindrawn = i - 1;
    }
    for (u1 const* i = GUIwinorder;; --cwindrawn, ++i) {
        switch (*i) {
        case 0:
            return;
        case 1:
            if (GUIReset != 1)
                DisplayGUILoad();
            break;
        case 2:
            DisplayGUIChoseSave();
            break;
        case 3:
            DisplayGUIInput();
            break;
        case 4:
            DisplayGUIOption();
            break;
        case 5:
            DisplayGUIVideo();
            break;
        case 6:
            DisplayGUISound();
            break;
        case 7:
            DisplayGUICheat();
            break;
        case 8:
            DisplayNetOptns();
            break;
        case 9:
            DisplayGameOptns();
            break;
        case 10:
            DisplayGUIOptns();
#ifdef __WIN32__
            CheckAlwaysOnTop();
#endif
            break;
        case 11:
            DisplayGUIAbout();
            break;
        case 12:
            DisplayGUIReset();
            break;
        case 13:
            DisplayGUISearch();
            break;
        case 14:
            DisplayGUIStates();
            break;
        case 15:
            DisplayGUIMovies();
            break;
        case 16:
            DisplayGUICombo();
            break;
        case 17:
            DisplayGUIAddOns();
            break;
        case 18:
            DisplayGUIChipConfig();
            break;
        case 19:
            DisplayGUIPaths();
            break;
        case 20:
            DisplayGUISave();
            break;
        case 21:
            DisplayGUISpeed();
            break;
        }
    }
}

static void GUIDMHelp(u4 const x1, u4 const x2, char const* const text, u4 const param4)
{
    bool const sel = GUIcmenupos == param4;
    GUIBox(x1, 3, x2, 3, sel ? 38 : 46);
    GUIBox(x1, 4, x2, 12, sel ? 40 : 42);
    GUIBox(x1, 13, x2, 13, sel ? 46 : 38);
    GUIBox(x1, 3, x1, 12, sel ? 40 : 44);
    GUIBox(x2, 4, x2, 13, sel ? 44 : 40);
    GUIOuttext(x1 + 5, 7, text, 44);
    GUIOuttext(x1 + 4, 6, text, 62);
}

static void GUIDMHelpB(u4 const x1, u4 const x2, char const* const text, u4 const param4)
{
    bool const sel = GUIcwinpress == param4;
    GUIBox(x1, 3, x2, 3, sel ? 38 : 46);
    GUIBox(x1, 4, x2, 13, sel ? 40 : 42);
    GUIBox(x1, 14, x2, 14, sel ? 46 : 38);
    GUIBox(x1, 3, x1, 13, sel ? 40 : 44);
    GUIBox(x2, 4, x2, 14, sel ? 44 : 40);
    GUIOuttext(x1 + 3, 7, text, 44);
    GUIOuttext(x1 + 2, 6, text, 62);
}

#ifdef __WIN32__
static void GUIDMHelpB2(u4 const x1, u4 const x2, char const* const text, u4 const param4)
{
    bool const sel = GUIcwinpress == param4;
    GUIBox(x1, 3, x2, 3, sel ? 38 : 46);
    GUIBox(x1, 4, x2, 6, sel ? 40 : 42);
    GUIBox(x1, 7, x2, 7, sel ? 46 : 38);
    GUIBox(x1, 3, x1, 6, sel ? 40 : 44);
    GUIBox(x2, 4, x2, 7, sel ? 44 : 40);
    GUIOuttext(x1 + 3, 5, text, 44);
    GUIOuttext(x1 + 2, 4, text, 62);
}

static void GUIDMHelpB3(u4 const x1, u4 const x2, char const* const text, u4 const param4)
{
    bool const sel = GUIcwinpress == param4;
    GUIBox(x1, 9, x2, 9, sel ? 38 : 46);
    GUIBox(x1, 10, x2, 12, sel ? 40 : 42);
    GUIBox(x1, 13, x2, 13, sel ? 46 : 38);
    GUIBox(x1, 9, x1, 12, sel ? 40 : 44);
    GUIBox(x2, 10, x2, 13, sel ? 44 : 40);
    GUIOuttext(x1 + 3, 11, text, 44);
    GUIOuttext(x1 + 2, 10, text, 62);
}
#endif

static void GUIMenuDisplay(u4 const n_cols, u4 n_rows, u1* dst, char const* text)
{
    u4 row = 0;
    do {
        u1 const al = *text;
        if (al != '\0') {
            ++text;
            if (GUIcrowpos != row) {
                GUIOutputString(dst + 289, text, al == 2 ? 42 : 44);
            }
            text = GUIOutputString(dst, text, al == 2 ? 57 : 63) + 1;
        } else {
            u1* d = dst + 4 * 288;
            u4 cols = n_cols;
            do {
                d[0] = 45;
                d[-289] = 40;
                d[289] = 42;
                ++d;
            } while (--cols != 0);
            text += 14;
        }
        dst += 10 * 288;
        ++row;
    } while (--n_rows != 0);
}

static void GUIDrawMenuM(u4 const x1, u4 const y1, u4 const p3, u4 const p4, char const* const text, u4 const p6, u4 const p7, u4 const p8, u4 const p9, u4 const p10)
{
    GUIShadow(p7, p8, p7 + 4 + p3 * 6, p8 + 3 + p4 * 10);
    GUIBox(x1, y1, x1 + 4 + p3 * 6, y1 + 3 + p4 * 10, 43);

    u1* dst = vidbuffer + GUIcrowpos * 2880 + x1 + 17 + 18 * 288;
    GUIDrawBox(dst, 6 * p3 + 3, 1, 73);
    GUIDrawBox(dst + 288, 6 * p3 + 3, 7, 72);
    GUIDrawBox(dst + 288 * 8, 6 * p3 + 3, 1, 73);

    GUIBox(x1 + p10, y1, x1 + 4 + p3 * 6, y1, 47);
    GUIBox(x1, y1, x1, p9, 45);
    GUIBox(x1, p9, x1 + 4 + p3 * 6, p9, 39);
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
    GUIBox(0, 1, 229, 1, 71);
    GUIBox(0, 2, 229, 2, 70);
    GUIBox(0, 3, 229, 3, 69);
    GUIBox(0, 4, 229, 4, 68);
    GUIBox(0, 5, 229, 5, 67);
    GUIBox(0, 6, 229, 6, 66);
    GUIBox(0, 7, 229, 7, 65);
    GUIBox(0, 8, 229, 8, 64);
    GUIBox(0, 9, 229, 9, 65);
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
    GUIShadow(238, 9, 247, 14);
    GUIShadow(238, 16, 247, 20);
    GUIShadow(249, 9, 257, 20);
#endif

#if defined __UNIXSDL__ || defined __WIN32__
#ifdef __UNIXSDL__
    GUIDMHelpB(233, 242, "\xF7", 1);
#endif
#ifdef __WIN32__
    GUIDMHelpB2(233, 242, "\xF9", 1);
    GUIDMHelpB3(233, 242, "\xF8", 3);
#endif
    GUIDMHelpB(244, 253, "x", 2);
#endif

    // Display upper-left box
    GUIDMHelp(4, 12, "", 1);
    char const* const down_arrow = "\x19";
    GUIOuttext(4 + 3, 7, down_arrow, 44);
    GUIOuttext(4 + 2, 6, down_arrow, 62);
    // Display boxes
    GUIDMHelp(17, 47, "GAME", 2);
    GUIDMHelp(52, 94, "CONFIG", 3);
    GUIDMHelp(99, 135, "CHEAT", 4);
    GUIDMHelp(140, 188, "NETPLAY", 5);
    GUIDMHelp(193, 223, "MISC", 6);

    GUIMenuL = 0;
    GUIMenuR = 0;
    GUIMenuD = 0;

    /* format : x pos, y pos, #charx, #chary, name, xpos+2, xpos+5,22,
     *          19+#chary*10, length of top menu box */
    if (GUIcmenupos == 1) {
        GUIDrawMenuM(4, 16, 30, 13, GUIPrevMenuData, 6, 9, 22, 149, 8); // 19+13*10
        GUICYLocPtr = MenuDat1;
    }
    if (GUIcmenupos == 2) {
        GUIDrawMenuM(17, 16, 10, 9, GUIGameMenuData[0], 19, 22, 22, 109, 30); // 19+9*10
        GUICYLocPtr = MenuDat2;
    }
    if (GUIcmenupos == 3) {
        GUIDrawMenuM(52, 16, 8, 11, GUIConfigMenuData[0], 54, 57, 22, 129, 42); // 19+11*10
        GUICYLocPtr = MenuDat3;
    }
    if (GUIcmenupos == 4) {
        GUIDrawMenuM(99, 16, 8, 3, GUICheatMenuData[0], 101, 104, 22, 49, 36); // 19+3*10
        GUICYLocPtr = MenuDat4;
    }
    if (GUIcmenupos == 5) {
#ifdef __MSDOS__
        GUIDrawMenuM(140, 16, 10, 2, GUINetPlayMenuData[0], 142, 145, 22, 39, 48); // 19+2*10
#else
        GUIDrawMenuM(140, 16, 10, 1, GUINetPlayMenuData[0], 142, 145, 22, 29, 48); // 19+1*10
#endif
        GUICYLocPtr = MenuDat5;
    }
    if (GUIcmenupos == 6) {
        GUIDrawMenuM(193, 16, 9, 7, GUIMiscMenuData[0], 195, 198, 22, 89, 30); // 19+5*10
        GUICYLocPtr = MenuDat6;
    }
}
