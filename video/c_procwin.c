/*
 * The colour-window setup the mode 7 line drivers run before each scanline,
 * ported from the procwindowback macro in video/vidmacro.mac (its only user
 * was procwindowback16t in video/makev16t.asm).
 *
 * It takes nothing and returns nothing: it reads the window registers and
 * leaves winon, numwin and the windowdata run list for the drawers. winon
 * selects what the drawer does - 0 none, 1 a run list, 2 nothing masked,
 * 3 the dual-window colour maths, 4 clear, 5 everything masked.
 *
 * It also has to report what it leaves in eax, ebx, ecx and esi. Both call
 * sites in video/c_mv16tline.c run it and then clearback16bts back to back,
 * and clearback16bts reads those same registers - so what this routine
 * incidentally leaves in them is the next one's input. pwregs carries them.
 *
 * The writes are partial-width on purpose: the assembly touches bl, al, cx and
 * esi, so the rest of each register has to survive.
 *
 * Verified against the assembly by test/difftest_procwin.c (make -C test
 * procwin), registers included.
 */

#include <stdint.h>

#include "../types.h"

extern u1 winon, numwin, windowdata[];
extern u1 wincolen, scaddset;
extern u1 winl1, winr1, winl2, winr2;

void makedualwincol(u1 al); /* video/c_makevid.c */

#include "c_procwin.h"

#define SETB(v, b) ((v) = ((v) & ~0xFFu) | (u1)(b))
#define SETW(v, w) ((v) = ((v) & ~0xFFFFu) | (u2)(w))

void c_procwindowback16t(pwregs* const r)
{
    u1 al, cl, ch, sel;

    winon = 0;
    /* Dead as far as the caller can tell - every path below rewrites bl
       before returning - but it is what the assembly does. */
    SETB(r->bx, wincolen & 0x0Au);

    /* Both colour windows enabled: the maths window is built elsewhere. */
    if ((wincolen & 0x0Au) == 0x0Au) {
        sel = scaddset & 0x30u;
        SETB(r->bx, sel);
        if (sel == 0x30u) {
            winon = 4;
            return;
        }
        if (sel == 0x00u)
            return;
        r->ax = wincolen; /* movzx, so the whole register */
        makedualwincol(wincolen);
        winon = 3;
        return;
    }

    SETB(r->bx, (scaddset >> 4) & 0x03u);
    switch ((u1)r->bx) {
    case 0:
        return;
    case 3:
        winon = 4;
        return;
    default:
        break;
    }
    if (!(wincolen & 0x0Au))
        return;

    /* Window 1 takes precedence, and its inside/outside bit is the one two
       places up - hence the shift rather than a second test. */
    al = wincolen & 0x05u;
    SETB(r->ax, al);
    r->si = (zreg)(uintptr_t)windowdata;
    winon = 1;
    numwin = 0;
    cl = winl2;
    ch = winr2;
    if (wincolen & 0x02u) {
        cl = winl1;
        ch = winr1;
        al = (u1)(al << 2);
        SETB(r->ax, al);
    }
    if (ch != 255)
        ch++;
    SETW(r->cx, cl | (u2)((u2)ch << 8));

    if (al & 0x04u) { /* mask inside the window */
        if (cl > ch) {
            winon = 5;
            return;
        }
        /* A window covering the whole line masks nothing at all. */
        if (ch >= 254 && cl <= 1) {
            winon = 2;
            return;
        }
        windowdata[0] = cl;
        windowdata[1] = 0x01;
        windowdata[2] = ch;
        windowdata[3] = 0xFF;
        numwin = 2;
        return;
    }

    /* mask outside it, so the run list has a middle section */
    if (cl >= ch) {
        winon = 2;
        return;
    }
    windowdata[0] = 0;
    windowdata[1] = 0x01;
    windowdata[2] = cl;
    windowdata[3] = 0xFF;
    windowdata[4] = ch;
    windowdata[5] = 0x01;
    numwin = 3;
}
