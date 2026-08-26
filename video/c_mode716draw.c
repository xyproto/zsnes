/*
 * video/c_mode716draw.c - drawmode7win16b and drawmode7ngextbg16b, ported from
 * video/mode716.asm, together with the four Mode7*Sub wrappers from
 * video/mode716.mac.
 *
 * This is the top of the Mode 7 renderer: pick the pixel writer from the
 * layer's main/sub and colour-add settings, set the window state up, then run
 * one of the four scanline walks in video/c_mode716proc.c.
 *
 * Which walk runs is decided in Mode7Startup: a window on the layer takes the
 * ngw one, adders of a whole tile or more take ProcessB, everything else the
 * plain one. The MainSub wrappers always take the two-writer ngw2 walk.
 *
 * Reached by call from video/newgfx16.mac with ebx = the scanline, esi = the
 * video pointer and ebp = the palette. The mosaic tail stays in assembly: it
 * is a tail-jump into domosaicng16b with dh carrying curmosaicsz.
 */
#include <stdint.h>

#include "../types.h"
#include "c_mode716write.h"

zreg M7DrawAX; /* ax and dx are the scanline's y and x coordinates */
zreg M7DrawDX;
zreg M7DrawBX;
zreg M7DrawSI;
zreg M7DrawDI;
zreg M7DrawBP;
u4 M7DrawMosaic; /* non-zero: the thunk tail-jumps to domosaicng16b */

extern u1 scrndis; /* cpu/regs.inc */
extern u1 BGMS1[];
extern u1 scadtng[256], FillSubScr[256];
extern u4 ngwinen; /* video/c_makev16b.c */
extern u1 *CMainWinScr, *CSubWinScr; /* video/c_newgfx16data.c */
extern u1 *cwinptr, *winptrref; /* video/makevid.c */
extern u1* curvidoffset;
extern u1 curmosaicsz;
extern u4 mmode7xadder, mmode7yadder; /* video/c_mode716data.c */
extern u4 mmode7xpos, mmode7ypos, mmode7xrpos, mmode7yrpos;
extern u4 mode7xpos, mode7ypos, mode7xrpos, mode7yrpos;
extern u4 mode7xadder, mode7yadder;
extern u1* vram;

/* The parts already ported; each reads its own seam block. */
extern zreg M7BWBX; /* video/c_mode716bw.c */
void c_ProcessMode7BuildWindow(void);
extern zreg M7StartAX, M7StartDX, M7StartSI, M7StartDI; /* c_mode716start.c */
void c_Mode7Startup16b(void);
extern zreg M7PAX, M7PBX, M7PCX, M7PDX, M7PSI, M7PDI, M7PBP; /* c_mode716proc.c */
extern u4 M7PWriter, M7PWriter2;
void c_Mode7Process(void);
void c_Mode7ProcessB(void);
void c_Mode7Processngw16b(void);
void c_Mode7Processngw216b(void);

/* Mode7MidRoutines: the ngw walks read the unprefixed copies. The assembly
   shuttles each through ebx, so it comes out holding the last one. */
static u4 mid_routines(void)
{
    mode7xrpos = mmode7xrpos;
    mode7yrpos = mmode7yrpos;
    mode7xadder = mmode7xadder;
    mode7yadder = mmode7yadder;
    mode7xpos = mmode7xpos;
    mode7ypos = mmode7ypos;
    return mmode7ypos;
}

/* An adder of a whole tile or more needs the big-step walk. The compares are
   signed, and 0x7F0 rather than 0x800 - the assembly's margin, not a slip.

   Mode7ProcessB turns out to be a strict generalisation of Mode7Process for
   any adder a Mode 7 matrix can produce: below 0x800 its step offsets come out
   zero and its 0xF8 off-tile test degenerates to the 0x08 one. So choosing the
   big-step walk too often is invisible, and only choosing it too rarely shows
   up - difftest_m7draw.c pins exactly that asymmetry. */
static int big_step(void)
{
    return (s4)mmode7xadder > 0x7F0 || (s4)mmode7xadder < -0x7F0
        || (s4)mmode7yadder > 0x7F0 || (s4)mmode7yadder < -0x7F0;
}

/* The four Mode7*Sub wrappers. `mainsub` picks WinEnCheckMS and the two-writer
   walk; `ext` picks Mode7NoTranspPreStartUp2, which skips the sub-screen
   offset the EXTBG pass does not want. */
static void mode7_sub(u4 const bx, enum m7_writer const w,
    enum m7_writer const w2, int const mainsub, int const ext)
{
    u1* esi;

    /* WinEnCheck / WinEnCheckMS, then ProcessBuildWindow. */
    if (mainsub) {
        extern u1 winbg1enval[];
        esi = winbg1enval;
    } else {
        esi = CMainWinScr;
        if ((FillSubScr[bx] & 1u) && !(BGMS1[bx * 2] & 1u)) {
            esi = CSubWinScr;
        }
    }
    ngwinen = 0;
    if (esi[bx] & 0x0Au) {
        M7BWBX = bx;
        c_ProcessMode7BuildWindow();
    }

    /* Mode7NoTranspPreStartUp(2). */
    winptrref = cwinptr;
    esi = curvidoffset;
    if (!ext && (FillSubScr[bx] & 1u) && !(BGMS1[bx * 2] & 1u)) {
        esi += M7_BUF * 2u;
    }

    M7StartAX = M7DrawAX;
    M7StartDX = M7DrawDX;
    M7StartSI = (zreg)(uintptr_t)esi;
    M7StartDI = M7DrawDI;
    c_Mode7Startup16b();

    M7PAX = M7StartAX;
    M7PBX = 0;
    M7PCX = 0;
    M7PDX = 0;
    M7PSI = M7StartSI;
    M7PDI = M7StartDI;
    M7PBP = M7DrawBP;
    M7PWriter = (u4)w;
    M7PWriter2 = (u4)w2;

    if (mainsub) {
        M7PBX = mid_routines();
        M7PDI = (zreg)(uintptr_t)vram;
        c_Mode7Processngw216b();
    } else if ((u1)ngwinen == 1) { /* a byte compare, as in the assembly */
        M7PBX = mid_routines();
        M7PDI = (zreg)(uintptr_t)vram;
        c_Mode7Processngw16b();
    } else if (big_step()) {
        M7PDI = (zreg)(uintptr_t)vram;
        c_Mode7ProcessB();
    } else {
        M7PDI = (zreg)(uintptr_t)vram;
        c_Mode7Process();
    }
    M7DrawMosaic = curmosaicsz != 1;
}

/* CheckTransparency: a layer that is only a sub-screen, or a line with no
   sub-screen, skips the transparent variants. */
static int transparent(u4 const bx, u1 const mask)
{
    return (BGMS1[bx * 2] & mask) && (FillSubScr[bx] & 1u);
}

/* The dispatch both draw routines share below the transparency test, laid out
   as the assembly's label chain: the two window tables decide whether one walk
   covers the line or the paired main/sub one does. `t` is the colour-add
   (transparent) side. */
static void dispatch(u4 const bx, int const t, int const ext)
{
    enum m7_writer const plain = ext ? M7W_EXTBG : M7W_NORMAL;
    enum m7_writer const tr = ext ? M7W_EXTBG_T : M7W_NORMAL_T;
    enum m7_writer const both_t = ext ? M7W_EXTBG_MST : M7W_NORMAL_MST;
    enum m7_writer const both = ext ? M7W_EXTBG_MS : M7W_NORMAL_MS;
    enum m7_writer const sub = ext ? M7W_EXTBG_S : M7W_NORMAL_S;

    if (!t) {
        mode7_sub(bx, plain, plain, 0, ext);
        return;
    }
    if (scadtng[bx] & 1u) {
        if (!(BGMS1[bx * 2 + 1] & 1u)) {
            mode7_sub(bx, tr, tr, 0, ext);
        } else if (CMainWinScr[bx] != 0) {
            if (CSubWinScr[bx] != 0) {
                mode7_sub(bx, both_t, both_t, 0, ext);
            } else {
                mode7_sub(bx, both_t, sub, 1, ext);
            }
        } else if (CSubWinScr[bx] != 0) {
            mode7_sub(bx, both_t, tr, 1, ext);
        } else {
            mode7_sub(bx, both_t, both_t, 0, ext);
        }
        return;
    }
    if (!(BGMS1[bx * 2 + 1] & 1u)) {
        mode7_sub(bx, plain, plain, 0, ext);
    } else if (ngwinen == 0) {
        mode7_sub(bx, both, both, 0, ext);
    } else if (CMainWinScr[bx] != 0) {
        if (CSubWinScr[bx] != 0) {
            mode7_sub(bx, both, both, 0, ext);
        } else {
            mode7_sub(bx, both, sub, 1, ext);
        }
    } else if (CSubWinScr[bx] != 0) {
        mode7_sub(bx, both, plain, 1, ext);
    } else {
        mode7_sub(bx, both, both, 0, ext);
    }
}

void c_drawmode7win16b(void)
{
    u4 const bx = M7DrawBX;

    M7DrawMosaic = 0;
    if (scrndis & 1u) {
        return;
    }
    dispatch(bx, transparent(bx, 0x01u), 0);
}

void c_drawmode7ngextbg16b(void)
{
    u4 const bx = M7DrawBX;
    u1* p;

    M7DrawMosaic = 0;
    if (scrndis & 1u) {
        return;
    }
    /* The EXTBG pass owns the priority plane, so clear it first. */
    curmosaicsz = 1;
    p = curvidoffset;
    for (u4 i = 0; i < 256u; i++, p += 2) {
        p[M7_BUF * 8] = 0;
    }
    dispatch(bx, transparent(bx, 0x01u) || transparent(bx, 0x02u), 1);
}
