#include <string.h>

#include "../asm_call.h"
#include "../c_vcache.h"
#include "../cpu/regs.h"
#include "../endmem.h"
#include "../initc.h"
#include "../ui.h"
#include "../vcache.h"
#include "c_makevid.h"
#include "makevid.h"

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
        u1 al_ = al;
        u1 cl_ = cl;
        asm volatile("call %P2"
                     : "+a"(al_), "+c"(cl_)
                     : "X"(dualstartprocess)
                     : "cc", "memory", "edx", "edi");
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
        asm_call(makedualwinsp);
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
            asm volatile("call %P0" ::"X"(cachetile2b16x16), "a"(eax)
                         : "cc", "memory", "ecx", "esi", "edi");
            break;
        case 2:
            asm volatile("call %P0" ::"X"(cachetile4b16x16), "a"(eax)
                         : "cc", "memory", "ecx", "esi", "edi");
            break;
        default:
            asm volatile("call %P0" ::"X"(cachetile8b16x16), "a"(eax)
                         : "cc", "memory", "ecx", "esi", "edi");
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
            asm volatile("call %P0" ::"X"(cachetile2b16x16), "a"(eax)
                         : "cc", "memory", "ecx", "esi", "edi");
            break;
        case 2:
            asm volatile("call %P0" ::"X"(cachetile4b16x16), "a"(eax)
                         : "cc", "memory", "ecx", "esi", "edi");
            break;
        default:
            asm volatile("call %P0" ::"X"(cachetile8b16x16), "a"(eax)
                         : "cc", "memory", "ecx", "esi", "edi");
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
            asm volatile("call %P0" ::"X"(cachetile2b), "a"(eax)
                         : "cc", "memory", "ecx", "esi", "edi");
            break;
        case 2:
            asm volatile("call %P0" ::"X"(cachetile4b), "a"(eax)
                         : "cc", "memory", "ecx", "esi", "edi");
            break;
        default:
            asm volatile("call %P0" ::"X"(cachetile8b), "a"(eax)
                         : "cc", "memory", "ecx", "esi", "edi");
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
