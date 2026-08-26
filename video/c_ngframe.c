/*
 * video/c_ngframe.c - StartDrawNewGfx16b and its screen clip, from
 * video/newgfx16.asm.
 *
 * The frame driver. Once per frame it clears the per-layer counters, then
 * walks a fixed running order - sub screen first if colour maths is on, then
 * the main screen - calling one of the sixteen background dispatchers, the
 * sprite pass or the mode 7 pass for each layer and priority in turn. Every
 * one of those is C already; what was left here is the order and the gates.
 *
 * It still runs on the caller's register block, because the colour-maths pass
 * it ends with reads two of them: `c_transp_halfsub` takes the caller's eax
 * and `c_transp_halfadd` the caller's edx, upper halves included. Nothing
 * here means anything by those - they are whatever the last cdecl call left -
 * but the screen clip writes both on its way through, so the block has to be
 * carried rather than invented.
 */
#include <stdint.h>
#include <string.h>

#include "../types.h"
#include "makevid.h" /* SpriteInfo */

enum { R_EDI,
    R_ESI,
    R_EBP,
    R_ESP,
    R_EBX,
    R_EDX,
    R_ECX,
    R_EAX };

extern u4 WindowRedraw, ngmsdraw, ngextbg; /* dwords where they are defined */
extern u1 scrndis;
extern u1 modeused[8], scadsng[256], winbg1enval[];
extern u4 nglogicval; /* a dword where it is defined (video/newgfx.c) */
extern u1 prdata[256], prdatb[256], prdatc[256];
extern u1 winbg1envals[], winbg1envalm[];
extern u1* vidbuffer;
extern u2 winlogicaval[256], resolutn;
extern u4 endlinet, scfbl, bgcmsung, mode0ads;
extern zreg CMainWinScr, CSubWinScr;
extern u4 ngwinen, ngwintable[32];
extern u4 bg1totng, bg2totng, bg3totng, bg4totng;
extern u4 bg1drwng, bg2drwng, bg3drwng, bg4drwng;
extern u4 UnusedBit[2], UnusedBitXor[2];
extern zreg sprtbng[256];
extern u4 sprtlng[64];
extern u1 sprlefttot[256];
extern u1* spritetablea;

void BuildWindow(u4 line, u4 which); /* video/c_makev16b.c */
void c_process_transparencies(zreg* r); /* video/c_ngtransp.c */

/* video/c_ngprocbg.c */
void c_procbg16b(u4 layer, void (*lineproc)(zreg*), void (*tileproc)(zreg*),
    u1 const* prdat, int main_, u4 mask, int kind);
void c_procspr16b(int main_, u4 mask, int modes);
void c_procmode7ng16b(int main_, u4 mask, int kind);

/* The sixteen dispatchers, video/c_ngbg.c. c_procbg16b hands each of them the
   pass's register file. */
void c_drawbg1line16b(zreg* r), c_drawbg2line16b(zreg* r);
void c_drawbg3line16b(zreg* r), c_drawbg4line16b(zreg* r);
void c_drawbg1tile16b(zreg* r), c_drawbg2tile16b(zreg* r);
void c_drawbg3tile16b(zreg* r), c_drawbg4tile16b(zreg* r);
void c_drawbg1linepr116b(zreg* r), c_drawbg2linepr116b(zreg* r);
void c_drawbg3linepr116b(zreg* r), c_drawbg4linepr116b(zreg* r);
void c_drawbg1tilepr116b(zreg* r), c_drawbg2tilepr116b(zreg* r);
void c_drawbg3tilepr116b(zreg* r), c_drawbg4tilepr116b(zreg* r);

/* The sub screen sits 75036 words past the main one. */
#define SUBOFF (75036u * 2u)

static u2 rdw(u1 const* const p)
{
    u2 v;

    memcpy(&v, p, 2);
    return v;
}

static void wrw(u1* const p, u2 const v) { memcpy(p, &v, 2); }

static u4 rdd(u1 const* const p)
{
    u4 v;

    memcpy(&v, p, 4);
    return v;
}

static void wrd(u1* const p, u4 const v) { memcpy(p, &v, 4); }

/* One clipped pixel. The main pass masks the main screen and sets the bit on
   the sub; the sub pass only masks. */
static void clip_word(int const sub, u1* const p, u4 const ebx)
{
    wrw(p, (u2)(rdw(p) & (u2)ebx));
    if (!sub)
        wrw(p + SUBOFF, (u2)(rdw(p + SUBOFF) | (u2)ebx));
}

static void clip_dword(int const sub, u1* const p, u4 const ebx)
{
    wrd(p, rdd(p) & ebx);
    if (!sub)
        wrd(p + SUBOFF, rdd(p + SUBOFF) | ebx);
}

/* MainScreenClip falls straight into SubScreenClip in the assembly, so the
 * second is never called alone. Two bits of scadsng say whether the line is
 * clipped and which side survives; the sub pass reads the pair two bits
 * higher. Both set sends the whole line through the wide writer. */
static void screen_clip(zreg* const r, int const sub)
{
    u1* base = vidbuffer + 16u * 2u + 288u * 2u;
    u4 eax = r[R_EAX];
    u4 edx = r[R_EDX];
    u4 ecx = r[R_ECX];
    u4 const* edi = (u4 const*)(uintptr_t)r[R_EDI];
    u4 bx = 1;

    for (;;) {
        u1 al;
        u1* p = base;
        u4 ebx;

        eax = (eax & 0xFFFFFF00u) | scadsng[bx];
        if (sub)
            eax = (eax & 0xFFFFFF00u) | (u1)((u1)eax << 2);
        if (((u1)eax & 0xC0u) == 0)
            goto next;
        eax = (eax & 0xFFFFFF00u) | ((u1)eax & 0xC0u); /* and al,0C0h */
        al = (u1)eax;
        ebx = sub ? UnusedBitXor[0] : UnusedBit[0];
        if (al == 0xC0u)
            goto full;

        /* al is 40h or 80h here: inside the window, or outside it. */
        ngwinen = 0;
        if (winbg1enval[bx + 5u * 256u] & 0x0Au) {
            /* The high byte of the pair, where the layer windows read the
               low one. */
            nglogicval
                = (u1)((((u1 const*)winlogicaval)[bx * 2u + 1u] >> 2) & 3u);
            BuildWindow(bx, bx + 5u * 256u);
        }
        ebx = sub ? UnusedBitXor[0] : UnusedBit[0];
        edx = 256;
        if (ngwinen == 0) {
            if (al == 0x80u)
                goto next; /* outside a window that is not there */
            goto full;
        }

        /* The run list is walked without a bound, as the assembly walks it:
           a table whose runs never reach 256 pixels reads past the end. */
        edi = ngwintable;
        ecx = *edi++;
        if (ecx == 0)
            (*(u4*)(uintptr_t)edi)--; /* a zero run steals from the next */
        else
            ecx--;
        if (al == 0x80u) {
            if (ecx != 0)
                goto noclipping;
            ecx = *edi++;
        } else if (ecx == 0) {
            goto skipclipping;
        }
        /* startclippingb */
        if (ecx >= 256u)
            goto full;
    clipc:
        if (ecx == 0)
            goto skipclipping;
        for (;;) {
            clip_word(sub, p, ebx);
            p += 2;
            if (--edx == 0)
                goto next;
            if (--ecx == 0)
                break;
        }
    skipclipping:
        ecx = *edi++;
    noclipping:
        { /* sub edx,ecx sets the flags *and* keeps the result, so edx is
             left decremented even on the way out. */
            u4 const before = edx;

            edx -= ecx;
            if (before <= ecx)
                goto next;
        }
        p += ecx * 2u;
        ecx = *edi++;
        goto clipc;

    full:
        for (ecx = 128; ecx != 0; ecx--) {
            clip_dword(sub, p, ebx);
            p += 4;
        }

    next:
        bx++;
        base += 288u * 2u;
        if (resolutn < (u2)bx)
            break;
    }

    r[R_EAX] = eax;
    r[R_EBX] = bx;
    r[R_ECX] = ecx;
    r[R_EDX] = edx;
    r[R_ESI] = (zreg)(uintptr_t)base;
    r[R_EDI] = (zreg)(uintptr_t)edi;
}

/* One background layer's pass. The mode-0 palette block is set for every
   layer whether or not it is used, as the assembly does. */
static void bg_pass(u4 const layer, void (*const lineproc)(zreg*),
    void (*const tileproc)(zreg*), u1 const* const prdat, int const main_,
    u4 const mask, int const kind, u4 const ads)
{
    mode0ads = ads;
    c_procbg16b(layer, lineproc, tileproc, prdat, main_, mask, kind);
}

/* Sprites in modes 0 and 1 only, or in 2..7 only, or unconditionally. The
   modeused test is a dword read of a byte table: the first covers modes 0..3
   and the second 4..7. */
static int sprites_01(void) { return (rdd(modeused) & 0x00000101u) != 0; }

static int sprites_27(void)
{
    return (rdd(modeused) & 0x01010000u) != 0
        || (rdd(modeused + 4) & 0x01010101u) != 0;
}

static void clear_counters(void)
{
    bg1totng = bg2totng = bg3totng = bg4totng = 0;
    bg1drwng = bg2drwng = bg3drwng = bg4drwng = 0;
}

/* The sub screen, then the main one. The two differ in the bgcmsung bits they
   test - the sub screen looks at the high nibble pair only, the main screen at
   both - and in which window table CMainWinScr points at. */
static void sub_screen(zreg* const eax)
{
    CMainWinScr = (zreg)(uintptr_t)winbg1envals;
    CSubWinScr = (zreg)(uintptr_t)winbg1envals;

    if (!(scrndis & 8u) && (bgcmsung & 0x800u))
        bg_pass(3, c_drawbg4line16b, c_drawbg4tile16b, 0, 0, 8u, 0, 0x60606060u);
    if (!(scrndis & 4u) && (bgcmsung & 0x400u))
        bg_pass(2, c_drawbg3line16b, c_drawbg3tile16b, 0, 0, 4u, 1, 0x40404040u);
    if (!(scrndis & 0x10u) && sprites_01() && (bgcmsung & 0x1000u))
        c_procspr16b(0, 0x10u, 0);

    if (!(scrndis & 8u) && (bgcmsung & 0x800u)) {
        *eax = bg4totng;
        if (*eax != bg4drwng)
            bg_pass(3, c_drawbg4linepr116b, c_drawbg4tilepr116b, prdata, 0, 8u, 2,
                0x60606060u);
    }
    if (!(scrndis & 4u) && (bgcmsung & 0x400u)) {
        *eax = bg3totng;
        if (*eax != bg3drwng)
            bg_pass(2, c_drawbg3linepr116b, c_drawbg3tilepr116b, prdatc, 0, 4u, 3,
                0x40404040u);
    }
    if (!(scrndis & 0x10u) && sprites_01() && (bgcmsung & 0x1000u))
        c_procspr16b(0, 0x10u, 0);

    if (!(scrndis & 2u) && (bgcmsung & 0x200u))
        bg_pass(1, c_drawbg2line16b, c_drawbg2tile16b, 0, 0, 2u, 0, 0x20202020u);
    if (ngextbg != 0 && (bgcmsung & 0x300u))
        c_procmode7ng16b(0, 3u, 1);
    if (!(scrndis & 0x10u) && sprites_27() && (bgcmsung & 0x1000u))
        c_procspr16b(0, 0x10u, 1);

    if (!(scrndis & 1u) && (bgcmsung & 0x100u))
        bg_pass(0, c_drawbg1line16b, c_drawbg1tile16b, 0, 0, 1u, 0, 0);
    if (modeused[7] != 0 && (bgcmsung & 0x300u))
        c_procmode7ng16b(0, 1u, 0);
    if (!(scrndis & 0x10u) && (bgcmsung & 0x1000u))
        c_procspr16b(0, 0x10u, 2);

    if (!(scrndis & 2u) && (bgcmsung & 0x200u)) {
        *eax = bg2totng;
        if (*eax != bg2drwng)
            bg_pass(1, c_drawbg2linepr116b, c_drawbg2tilepr116b, prdata, 0, 2u, 2,
                0x20202020u);
    }
    if (ngextbg != 0 && (bgcmsung & 0x300u))
        c_procmode7ng16b(0, 2u, 2);
    if (!(scrndis & 0x10u) && sprites_27() && (bgcmsung & 0x1000u))
        c_procspr16b(0, 0x10u, 1);

    if (!(scrndis & 1u) && (bgcmsung & 0x100u)) {
        *eax = bg1totng;
        if (*eax != bg1drwng)
            bg_pass(0, c_drawbg1linepr116b, c_drawbg1tilepr116b, prdatb, 0, 1u, 2,
                0);
    }
    if (!(scrndis & 0x10u) && (bgcmsung & 0x1000u))
        c_procspr16b(0, 0x10u, 2);

    if (!(scrndis & 4u) && modeused[1] != 0 && (bgcmsung & 0x400u)) {
        *eax = bg3totng;
        if (*eax != bg3drwng)
            bg_pass(2, c_drawbg3linepr116b, c_drawbg3tilepr116b, prdatc, 0, 4u, 4,
                0x40404040u);
    }
    clear_counters();
}

static void main_screen(zreg* const eax)
{
    CMainWinScr = (zreg)(uintptr_t)winbg1envalm;
    CSubWinScr = (zreg)(uintptr_t)winbg1envals;

    if (!(scrndis & 8u) && (bgcmsung & 0x808u))
        bg_pass(3, c_drawbg4line16b, c_drawbg4tile16b, 0, 1, 8u, 0, 0x60606060u);
    if (!(scrndis & 4u) && (bgcmsung & 0x404u))
        bg_pass(2, c_drawbg3line16b, c_drawbg3tile16b, 0, 1, 4u, 1, 0x40404040u);
    if (!(scrndis & 0x10u) && sprites_01() && (bgcmsung & 0x1010u))
        c_procspr16b(1, 0x10u, 0);

    if (!(scrndis & 8u) && (bgcmsung & 0x808u)) {
        *eax = bg4totng;
        if (*eax != bg4drwng)
            bg_pass(3, c_drawbg4linepr116b, c_drawbg4tilepr116b, prdata, 1, 8u, 2,
                0x60606060u);
    }
    if (!(scrndis & 4u) && (bgcmsung & 0x404u)) {
        *eax = bg3totng;
        if (*eax != bg3drwng)
            bg_pass(2, c_drawbg3linepr116b, c_drawbg3tilepr116b, prdatc, 1, 4u, 3,
                0x40404040u);
    }
    if (!(scrndis & 0x10u) && sprites_01() && (bgcmsung & 0x1010u))
        c_procspr16b(1, 0x10u, 0);

    if (!(scrndis & 2u) && (bgcmsung & 0x202u))
        bg_pass(1, c_drawbg2line16b, c_drawbg2tile16b, 0, 1, 2u, 0, 0x20202020u);
    if (ngextbg != 0 && (bgcmsung & 0x303u))
        c_procmode7ng16b(1, 3u, 1);
    if (!(scrndis & 0x10u) && sprites_27() && (bgcmsung & 0x1010u))
        c_procspr16b(1, 0x10u, 1);

    if (!(scrndis & 1u) && (bgcmsung & 0x101u))
        bg_pass(0, c_drawbg1line16b, c_drawbg1tile16b, 0, 1, 1u, 0, 0);
    /* 101h here where the sub screen's mode 7 pass tests 300h - the two are
       not each other's mirror, and neither is a typo to tidy up. */
    if (modeused[7] != 0 && (bgcmsung & 0x101u))
        c_procmode7ng16b(1, 1u, 0);
    if (!(scrndis & 0x10u) && (bgcmsung & 0x1010u))
        c_procspr16b(1, 0x10u, 2);

    /* The one place the two halves differ in order: the main screen's second
       EXTBG pass comes before BG2's priority 1, the sub screen's after. */
    if (ngextbg != 0 && (bgcmsung & 0x303u))
        c_procmode7ng16b(1, 2u, 2);

    if (!(scrndis & 2u) && (bgcmsung & 0x202u)) {
        *eax = bg2totng;
        if (*eax != bg2drwng)
            bg_pass(1, c_drawbg2linepr116b, c_drawbg2tilepr116b, prdata, 1, 2u, 2,
                0x20202020u);
    }
    if (!(scrndis & 0x10u) && sprites_27() && (bgcmsung & 0x1010u))
        c_procspr16b(1, 0x10u, 1);

    if (!(scrndis & 1u) && (bgcmsung & 0x101u)) {
        *eax = bg1totng;
        if (*eax != bg1drwng)
            bg_pass(0, c_drawbg1linepr116b, c_drawbg1tilepr116b, prdatb, 1, 1u, 2,
                0);
    }
    if (!(scrndis & 0x10u) && (bgcmsung & 0x1010u))
        c_procspr16b(1, 0x10u, 2);

    if (!(scrndis & 4u) && modeused[1] != 0 && (bgcmsung & 0x404u)) {
        *eax = bg3totng;
        if (*eax != bg3drwng)
            bg_pass(2, c_drawbg3linepr116b, c_drawbg3tilepr116b, prdatc, 1, 4u, 4,
                0x40404040u);
    }
}

void c_startdrawnewgfx16b(zreg* r);

void c_startdrawnewgfx16b(zreg* const r)
{
    zreg t[8]; /* the live registers; r keeps what the pops put back */
    zreg eax;
    u4 q;

    WindowRedraw = 1;
    endlinet -= 8;
    if (scfbl != 0)
        goto done;

    memcpy(t, r, sizeof t);

    /* Each of the 256 sprite table slots is 512 bytes on from the last. The
       assembly walked a second pointer over sprlefttot at the same time and
       never read it. */
    eax = (zreg)(uintptr_t)spritetablea;
    for (q = 0; q < 256u; q++) {
        sprtbng[q] = eax;
        eax += 64u * sizeof(SpriteInfo);
    }
    memset(sprtlng, 0, sizeof sprtlng);
    /* What the two loops leave behind. Nothing here reads them, but the colour
       maths pass at the end runs on this block and the clip only overwrites
       some of it. */
    eax = 0;
    t[R_EBX] = (zreg)(uintptr_t)(sprlefttot + 256);
    t[R_ECX] = 0;
    t[R_EDI] = (zreg)(uintptr_t)sprtlng + 256u;

    clear_counters();

    if (ngmsdraw != 0)
        sub_screen(&eax);
    main_screen(&eax);

    /* The clip runs on the live registers: edx has not been touched since
       entry - every call between here and the top preserves it - and the two
       passes run back to back, the second picking up where the first left
       ecx and edi. */
    t[R_EAX] = eax;
    screen_clip(t, 0);
    screen_clip(t, 1);
    c_process_transparencies(t);

done:
    /* pop ebp/edi/esi/edx put the rest back. */
    r[R_EAX] = 0;
    r[R_EBX] = 0;
    r[R_ECX] = 0;
}
