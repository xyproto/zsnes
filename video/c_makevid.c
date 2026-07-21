#include <string.h>

#include "../c_vcache.h"
#include "../cpu/regs.h"
#include "../endmem.h"
#include "../initc.h"
#include "../ui.h"
#include "../vcache.h"
#include "c_makevid.h"
#include "makevid.h"

// --- Dual-window mask engine (ported from video/makevid.asm) ----------------
//
// dualstartprocess() builds a 256-byte per-pixel mask for window 1 into
// dwinptrproc[], then combines window 2 into it with the layer's logic
// operator (OR/AND/XOR/XNOR). Mask bytes are always 0 or 1.
//
// The window edges are packed into winl1 as four bytes:
//   [0] = window 1 left   [1] = window 1 right
//   [2] = window 2 left   [3] = window 2 right
// The enable byte `al` selects the window "type": bit 0 for window 1 and
// bit 2 for window 2 (set = "outside", clear = "inside").
//
// The exact loop bounds — the do/while forms that touch one pixel past an
// edge, and the sides that overwrite rather than mask — are preserved
// verbatim from the assembly. `i` is a u1 so it wraps at 256 like the 8-bit
// index register the original used.

static void dualwinand(u1 const al)
{
    u1* const p = dwinptrproc;
    u1 const dl = winl1 >> 16; // window 2 left
    u1 const dh = winl1 >> 24; // window 2 right
    u1 i;

    if (al & 0x04) { // window 2 is the "outside" type
        if (dl >= dh) {
            return; // AND against all-ones: leave the mask unchanged
        }
        if (dl <= 1 && dh >= 254) {
            memset(p, 0, 256); // clipped
            return;
        }
        i = 0;
        do {
            p[i] &= 1;
        } while (++i < dl);
        do {
            p[i] = 0;
        } while (++i < dh);
        p[i] = 0;
        if (i != 255) {
            ++i;
            do {
                p[i] &= 1;
            } while (++i != 0);
        }
    } else { // window 2 is the "inside" type
        if (dl == 254 || dl >= dh) {
            memset(p, 0, 256); // clipped
            return;
        }
        i = 0;
        if (dl != 0) {
            do {
                p[i] = 0;
            } while (++i <= dl);
        }
        do {
            p[i] &= 1;
        } while (++i < dh);
        p[i] &= 1;
        if (dh != 255) {
            do {
                p[i] = 0;
            } while (++i != 0);
        }
    }
}

static void dualwinor(u1 const al)
{
    u1* const p = dwinptrproc;
    u1 const dl = winl1 >> 16;
    u1 const dh = winl1 >> 24;
    u1 i;

    if (al & 0x04) { // outside
        if (dl >= dh) {
            memset(p, 1, 256); // OR against all-ones region
            return;
        }
        if (dl <= 1 && dh >= 254) {
            return; // clipped: OR against nothing
        }
        i = 0;
        do {
            p[i] = 1;
        } while (++i < dl);
        i = dh;
        if (i != 255) {
            ++i;
            do {
                p[i] = 1;
            } while (++i != 0);
        }
    } else { // inside
        if (dl == 254 || dl >= dh) {
            return; // clipped: OR against nothing
        }
        i = 0;
        if (dl != 0) {
            i = dl + 1;
        }
        do {
            p[i] = 1;
        } while (++i < dh);
        p[i] = 1;
    }
}

static void dualwinxor(u1 const al)
{
    u1* const p = dwinptrproc;
    u1 const dl = winl1 >> 16;
    u1 const dh = winl1 >> 24;
    u1 i;
    int k;

    if (al & 0x04) { // outside
        if (dl >= dh) {
            for (k = 0; k < 256; k++)
                p[k] ^= 1; // XOR every pixel
            return;
        }
        if (dl <= 1 && dh >= 254) {
            return; // clipped
        }
        i = 0;
        do {
            p[i] ^= 1;
        } while (++i < dl);
        i = dh;
        if (i != 255) {
            ++i;
            do {
                p[i] ^= 1;
            } while (++i != 0);
        }
    } else { // inside
        if (dl == 254 || dl >= dh) {
            return; // clipped
        }
        i = 0;
        if (dl != 0) {
            i = dl + 1;
        }
        do {
            p[i] ^= 1;
        } while (++i < dh);
        p[i] ^= 1;
    }
}

static void dualwinxnor(u1 const al)
{
    u1* const p = dwinptrproc;
    u1 const dl = winl1 >> 16;
    u1 const dh = winl1 >> 24;
    u1 i;
    int k;

    // Same region walk as XOR, but every path (including the clipped ones)
    // falls through to a final full-buffer XOR, which inverts the result.
    if (al & 0x04) { // outside
        if (dl >= dh) {
            for (k = 0; k < 256; k++)
                p[k] ^= 1;
        } else if (dl <= 1 && dh >= 254) {
            // clipped: straight to the final inversion
        } else {
            i = 0;
            do {
                p[i] ^= 1;
            } while (++i < dl);
            i = dh;
            if (i != 255) {
                ++i;
                do {
                    p[i] ^= 1;
                } while (++i != 0);
            }
        }
    } else { // inside
        if (dl == 254 || dl >= dh) {
            // clipped: straight to the final inversion
        } else {
            i = 0;
            if (dl != 0) {
                i = dl + 1;
            }
            do {
                p[i] ^= 1;
            } while (++i < dh);
            p[i] ^= 1;
        }
    }

    for (k = 0; k < 256; k++)
        p[k] ^= 1; // xnor's trailing inversion
}

static void dualstartprocess(u1 const al, u1 const cl)
{
    u1* const p = dwinptrproc;
    u1 const dl = winl1; // window 1 left
    u1 const dh = winl1 >> 8; // window 1 right
    u1 i;

    if (al & 0x01) { // window 1 is the "outside" type
        if (dl >= dh) {
            memset(p, 1, 256);
        } else if (dl <= 1 && dh >= 254) {
            memset(p, 0, 256); // clipped
        } else {
            i = 0;
            do {
                p[i] = 1;
            } while (++i < dl);
            do {
                p[i] = 0;
            } while (++i < dh);
            p[i] = 0;
            if (i != 255) {
                ++i;
                do {
                    p[i] = 1;
                } while (++i != 0);
            }
        }
    } else { // window 1 is the "inside" type
        if (dl == 254 || dl >= dh) {
            memset(p, 0, 256); // clipped
        } else {
            i = 0;
            if (dl != 0) {
                do {
                    p[i] = 0;
                } while (++i <= dl);
            }
            do {
                p[i] = 1;
            } while (++i < dh);
            p[i] = 1;
            if (dh != 255) {
                do {
                    p[i] = 0;
                } while (++i != 0);
            }
        }
    }

    switch (cl) {
    case 0:
        dualwinor(al);
        break;
    case 2:
        dualwinxor(al);
        break;
    case 3:
        dualwinxnor(al);
        break;
    default:
        dualwinand(al);
        break; // cl == 1
    }
}

// Colour-window variant of makedualwin, called from the procwindowback macro
// (video/vidmacro.mac). Uses the colour window logic (winlogicb bits 2-3) and
// checks both the sprite and background caches before rebuilding the mask.
void makedualwincol(u1 const al)
{
    u1 const cl = (winlogicb >> 2) & 0x03;
    winon = 1;

    if (cl == dualwinsp && al == pwinspenab && winl1 == pwinsptype) { // sprite data matches
        cwinptr = winspdata + 16;
        winon = winonstype;
        return;
    }
    if (cl == dualwinbg && al == pwinbgenab && winl1 == pwinbgtype) { // bg data matches
        cwinptr = winbgdata + 16;
        winon = winonbtype;
        return;
    }

    dualwinbg = cl;
    pwinbgenab = al;
    pwinbgtype = winl1;
    dwinptrproc = winbgdata + 16;
    cwinptr = winbgdata + 16;
    winon = 1;
    winonbtype = 1;
    dualstartprocess(al, cl);
}

static void makedualwin(u1 const al, Layer const ebp)
{
    u1 const cl = winlogica >> (u1)(ebp * 2) & 0x03;
    u4 const ebx = winl1;
    winon = 1;

    if (cl == dualwinsp && al == pwinspenab && ebx == pwinsptype) { // data matches previous sprite data
        cwinptr = winspdata + 16;
        winon = winonstype;
        return;
    } else if (cl == dualwinbg && al == pwinbgenab && ebx == pwinbgtype) { // data matches previous data
        cwinptr = winbgdata + 16;
        winon = winonbtype;
        return;
    } else {
        dualwinbg = cl;
        pwinbgenab = al;
        pwinbgtype = ebx;
        dwinptrproc = winbgdata + 16;
        cwinptr = winbgdata + 16;
        winon = 1;
        winonbtype = 1;
        dualstartprocess(al, cl);
    }
}

void makewindow(u1 al, Layer const ebp)
{
    // upon entry, al = win enable bits
    if (disableeffects == 1)
        return;

    switch (al & 0x0A) {
    case 0x00:
        return;
    case 0x0A:
        makedualwin(al, ebp);
        return;
    }

    winon = 1;
    u4 const ebx = winl1;

    if (al == pwinspenab && ebx == pwinsptype) { // data matches previous sprite data
        cwinptr = winspdata + 16;
        winon = winonstype;
    } else if (al == pwinbgenab && ebx == pwinbgtype) { // data matches previous data
        cwinptr = winbgdata + 16;
        winon = winonbtype;
    } else {
        pwinbgenab = al;
        pwinbgtype = ebx;

        u1 dl;
        u1 dh;
        if (al & 0x02) {
            dl = winl1;
            dh = winl1 >> 8;
        } else {
            dl = winl1 >> 16;
            dh = winl1 >> 24;
            al >>= 2;
        }

        if (al & 0x01) { // outside
            if (dl >= dh) {
                winon = 0xFF;
                winonbtype = 0xFF;
                cwinptr = winbgdata + 16;
                return;
            }
            if (dl <= 1 && dh >= 254)
                goto clipped;
            u1* const edi = winbgdata + 16;
            u1 eax = 0;
            // start drawing 1's from 0 to left
            do
                edi[eax] = 1;
            while (++eax != dl);
            do
                edi[eax] = 0;
            while (++eax != dh);
            edi[eax] = 0;
            // start drawing 1's from right to 255
            while (++eax != 0)
                edi[eax] = 1;
        } else {
            if (dl == 254 || dl >= dh)
                goto clipped;
            u1* const edi = winbgdata + 16;
            u1 eax = 0;
            // start drawing 1's from 0 to left
            while (eax != dl)
                edi[eax++] = 0;
            do
                edi[eax] = 1;
            while (++eax != dh);
            edi[eax] = 1;
            if (eax != 255) { // start drawing 1's from right to 255
                do
                    edi[eax] = 0;
                while (++eax != 0);
            }
        }
        winon = 1;
        winonbtype = 1;
        cwinptr = winbgdata + 16;
        return;

    clipped:
        winon = 0;
        winonbtype = 0;
        return;
    }
}

static void makedualwinsp(u1 const al)
{
    u1 const cl = winlogicb & 0x03;

    if (cl == dualwinsp && al == pwinspenab && winl1 == pwinsptype) { // data matches previous data
        cwinptr = winspdata + 16;
        winonsp = winonstype;
        return;
    }

    dualwinsp = cl;
    pwinspenab = al;
    pwinsptype = winl1;
    dwinptrproc = winspdata + 16;
    cwinptr = winspdata + 16;
    winonsp = 1;
    winonstype = 1;

    dualstartprocess(al, cl);
}

void makewindowsp(void)
{
    winonsp = 0;

    if (!(winenabm & 0x10) && !(winenabs & 0x10))
        return;
    // upon entry, al = win enable bits
    if (disableeffects == 1)
        return;

    u1 al = winen[LAYER_OBJ];
    switch (al & 0x0A) {
    case 0x00:
        return;
    case 0x0A:
        makedualwinsp(al);
        return;
    }

    winonsp = 1;

    // check if data matches previous data
    if (al == pwinspenab && winl1 == pwinsptype) {
        cwinptr = winspdata + 16;
        winonsp = winonstype;
    } else {
        pwinspenab = al;
        pwinsptype = winl1;

        u1 dl;
        u1 dh;
        if (al & 0x02) {
            dl = winl1;
            dh = winl1 >> 8;
        } else {
            dl = winl1 >> 16;
            dh = winl1 >> 24;
            al >>= 2;
        }

        if (al & 0x01) { // outside
            if (dl >= dh) {
                winonsp = 0xFF;
                winonstype = 0xFF;
                cwinptr = winspdata + 16;
                return;
            }
            if (dl <= 1 && dh >= 254)
                goto clipped;
            u1* const edi = winspdata + 16;
            u1 eax = 0;
            // start drawing 1's from 0 to left
            do
                edi[eax] = 1;
            while (++eax != dl);
            do
                edi[eax] = 0;
            while (++eax != dh);
            edi[eax] = 0;
            // start drawing 1's from right to 255
            while (++eax != 0)
                edi[eax] = 1;
        } else {
            if (dl == 254 || dl >= dh)
                goto clipped;
            u1* const edi = winspdata + 16;
            u1 eax = 0;
            // start drawing 1's from 0 to left
            while (eax != dl)
                edi[eax++] = 0;
            do
                edi[eax] = 1;
            while (++eax != dh);
            edi[eax] = 1;
            if (eax != 255) { // start drawing 1's from right to 255
                do
                    edi[eax] = 0;
                while (++eax != 0);
            }
        }
        winonsp = 1;
        winonstype = 1;
        cwinptr = winspdata + 16;
        return;

    clipped:
        winonsp = 0;
        winonstype = 0;
        return;
    }
}

static void fillwithnothing(u1* const edi)
{
    memset(edi, 0, sizeof(*cachebg));
}

// Processes & Draws 16x16 tiles in 2, 4, & 8 bit mode
static void proc16x16(u2 const ax, u2 const dx, u1* const edi, u4 const layer, u1 const curcolor, u4 const bgptrb)
{
    a16x16yinc = (ax & 0x08) != 0;

    u4 const eax = ax >> 4 & 0x3F;
    if (edi[eax] == 0) {
        edi[eax] = 1;
        switch (curcolor) {
        case 1:
            cachetile2b16x16(eax);
            break;
        case 2:
            cachetile4b16x16(eax);
            break;
        default:
            cachetile8b16x16(eax);
            break;
        }
    }

    if (dx & 0x0200) // tilexa
    {
        if (eax & 0x20) // tileya
        { // bgptrd/bgptrc
            bgptrx1 = bgptrd;
            bgptrx2 = bgptrc;
        } else { // bgptrb/bgptra
            bgptrx1 = bgptrb;
            bgptrx2 = bgptr;
        }
    } else {
        if (eax & 0x20) // tileya
        { // bgptrc/bgptrd
            bgptrx1 = bgptrc;
            bgptrx2 = bgptrd;
        } else { // bgptra/bgptrb
            bgptrx1 = bgptr;
            bgptrx2 = bgptrb;
        }
    }

    // ax = # of rows down
    yadder = (ax & 0x07) * 8;
    // set up edi to point to tile data
    u2* edi_ = (u2*)vram + (eax & 0x1F) * 32;
    temptile = edi_; // XXX cast
    edi_ = (u2*)((u1*)edi_ + (bgptrx1 & 0x0000FFFF));
    // dx = # of columns right
    // cx = bgxlim
    a16x16xinc = (dx & 0x08) != 0;
    u4 const edx = dx >> 4 & 0x1F;
    temp = edx;
    edi_ += edx;

    u4 const esi_ = dx & 0x07;
    u1* const ebx_ = tempcach;
    u2* const edx_ = (u2*)((u1*)temptile + (bgptrx2 & 0x0000FFFF)); // XXX casts
    u4 const ecx_ = yadder;
    u4 const eax_ = a16x16yinc << 24 | a16x16xinc << 16 | bshifter << 8 | temp;
    // fill up tempbuffer with pointer #s that point to cached video mem
    // to calculate pointer, get first byte
    bg1vbufloc[layer] = esi_; // esi = pointer to video buffer
    bg1tdatloc[layer] = edi_; // edi = pointer to tile data
    bg1tdabloc[layer] = edx_; // edx = secondary tile pointer
    bg1cachloc[layer] = ebx_; // ebx = cached memory
    bg1yaddval[layer] = ecx_; // ecx = y adder
    bg1xposloc[layer] = eax_; // al  = current x position
}

static void proc16x8(u2 const ax, u2 const dx, u1* const edi, u4 const layer, u1 const curcolor, u4 const bgptrb)
{
    // ax = # of rows down
    u4 const eax = ax >> 3 & 0x3F;
    if (edi[eax] == 0) {
        edi[eax] = 1;
        switch (curcolor) {
        case 1:
            cachetile2b16x16(eax);
            break;
        case 2:
            cachetile4b16x16(eax);
            break;
        default:
            cachetile8b16x16(eax);
            break;
        }
    }

    if (dx & 0x0100) {
        if (eax & 0x20) { // bgptrd/bgptrc
            bgptrx1 = bgptrd;
            bgptrx2 = bgptrc;
        } else { // bgptrb/bgptra
            bgptrx1 = bgptrb;
            bgptrx2 = bgptr;
        }
    } else {
        if (eax & 0x20) { // bgptrc/bgptrd
            bgptrx1 = bgptrc;
            bgptrx2 = bgptrd;
        } else { // bgptra/bgptrb
            bgptrx1 = bgptr;
            bgptrx2 = bgptrb;
        }
    }

    // set up edi & yadder to point to tile data
    yadder = (ax & 0x07) * 8;
    u2* edi_ = (u2*)vram + (eax & 0x1F) * 32;
    temptile = edi_;
    edi_ = (u2*)((u1*)edi_ + bgptrx1);
    // dx = # of columns right
    // cx = bgxlim
    u4 const edx = dx >> 3 & 0x1F;
    temp = edx;
    edi_ += edx;

    u4 const esi_ = dx & 0x07;
    u1* const ebx_ = tempcach;
    u2* const edx_ = (u2*)((u1*)temptile + (bgptrx2 & 0x0000FFFF));
    u4 const ecx_ = yadder;
    u4 const eax_ = bshifter << 8 | temp;
    // fill up tempbuffer with pointer #s that point to cached video mem
    // to calculate pointer, get first byte
    bg1vbufloc[layer] = esi_; // esi = pointer to video buffer
    bg1tdatloc[layer] = edi_; // edi = pointer to tile data
    bg1tdabloc[layer] = edx_; // edx = secondary tile pointer
    bg1cachloc[layer] = ebx_; // ebx = cached memory
    bg1yaddval[layer] = ecx_; // ecx = y adder
    bg1xposloc[layer] = eax_; // al  = current x position
}

// Processes & Draws 8x8 tiles in 2, 4, & 8 bit mode
static void proc8x8(u2 const ax, u2 const dx, u1* const edi, u4 const layer, u1 const curcolor, u4 const bgptrb)
{
    // ax = # of rows down
    u4 const eax = ax >> 3 & 0x3F;
    if (edi[eax] == 0) {
        edi[eax] = 1;
        switch (curcolor) {
        case 1:
            cachetile2b(eax);
            break;
        case 2:
            cachetile4b(eax);
            break;
        default:
            cachetile8b(eax);
            break;
        }
    }

    if (dx & 0x0100) // tilexa
    {
        if (eax & 0x20) // tileya
        { // bgptrd/bgptrc
            bgptrx1 = bgptrd;
            bgptrx2 = bgptrc;
        } else { // bgptrb/bgptra
            bgptrx1 = bgptrb;
            bgptrx2 = bgptr;
        }
    } else {
        if (eax & 0x20) // tileya
        { // bgptrc/bgptrd
            bgptrx1 = bgptrc;
            bgptrx2 = bgptrd;
        } else { // bgptra/bgptrb
            bgptrx1 = bgptr;
            bgptrx2 = bgptrb;
        }
    }

    // set up edi & yadder to point to tile data
    yadder = (ax & 0x07) * 8;
    u2* edi_ = (u2*)vram + (eax & 0x1F) * 32;
    temptile = edi_;
    edi_ = (u2*)((u1*)edi_ + bgptrx1);
    // dx = # of columns right
    // cx = bgxlim
    u4 const edx = dx >> 3 & 0x1F;
    temp = edx;
    edi_ += edx;

    u4 const esi_ = dx & 0x07;
    u1* const ebx_ = tempcach;
    u2* const edx_ = (u2*)((u1*)temptile + (bgptrx2 & 0x0000FFFF));
    u4 const ecx_ = yadder;
    u4 const eax_ = bshifter << 8 | temp;
    // fill up tempbuffer with pointer #s that point to cached video mem
    // to calculate pointer, get first byte
    bg1vbufloc[layer] = esi_; // esi = pointer to video buffer
    bg1tdatloc[layer] = edi_; // edi = pointer to tile data
    bg1tdabloc[layer] = edx_; // edx = secondary tile pointer
    bg1cachloc[layer] = ebx_; // ebx = cached memory
    bg1yaddval[layer] = ecx_; // ecx = y adder
    bg1xposloc[layer] = eax_; // al  = current x position
}

void procbackgrnd(u4 const layer)
{
    u1 const mode = colormodeofs[layer];
    if (mode == 0)
        return;
    u1 const al = curbgnum;
    if (scrndis & al)
        return;
    if (!(scrnon & (al * 0x0101)))
        return;
    u1* const edi = cachebg[layer];
    if (mode != curcolbg[layer]) { // clear cache
        curcolbg[layer] = mode;
        curbgofs[layer] = bg1ptr[layer];
        fillwithnothing(edi);
    }

    u4 eax = bg1objptr[layer];
    u1* edx;
    switch (mode) // decide on mode
    {
    case 1: // 2 bit
        bshifter = 0;
        edx = vcache2b;
        eax *= 4;
        break;

    case 2: // 4 bit
        bshifter = 2;
        edx = vcache4b;
        eax *= 2;
        break;

    default:
        bshifter = 6;
        edx = vcache8b;
        break;
    }
    tempcach = edx + eax;

    curtileptr = bg1objptr[layer];
    u2 const ax = bg1ptr[layer];
    bgptr = bgptr & 0xFFFF0000 | ax;
    if (ax != curbgofs[layer]) { // clear cache
        curbgofs[layer] = ax;
        fillwithnothing(edi);
    }
    u4 const bgptrb = bg1ptrb[layer];
    bgptrc = bgptrc & 0xFFFF0000 | bg1ptrc[layer];
    bgptrd = bgptrd & 0xFFFF0000 | bg1ptrd[layer];

    u2 y = curypos;
    curmosaicsz = 1;
    if (mosaicon & curbgnum) {
        u1 bl = mosaicsz;
        if (bl != 0) {
            ++bl;
            curmosaicsz = bl;
            y = y / bl * bl;
            y += MosaicYAdder[mosaicsz];
        }
    }

    y += bg1scroly[layer];
    u2 const x = bg1scrolx[layer];
    if (bgtilesz & curbgnum)
        proc16x16(y, x, edi, layer, mode, bgptrb);
    else if (bgmode == 5)
        proc16x8(y, x, edi, layer, mode, bgptrb);
    else
        proc8x8(y, x, edi, layer, mode, bgptrb);
}
