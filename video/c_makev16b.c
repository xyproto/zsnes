#include <string.h>

#include "../asm_call.h"
#include "../c_vcache.h"
#include "../cpu/regs.h"
#include "../cpu/regsw.h"
#include "../endmem.h"
#include "../initc.h"
#include "../macros.h"
#include "../ui.h"
#include "../vcache.h"
#include "c_makev16b.h"
#include "c_makevid.h"
#include "makev16b.h"
#include "makev16t.h"
#include "makevid.h"
#include "mode716.h"
#include "mode716b.h"
#include "mode716e.h"
#include "newgfx.h"
#include "procvid.h"

static u2 draw16x1616b_yadd;
static u2 draw16x1616b_yflipadd;

static void blanker16b(void)
{
    // calculate current video offset
    memset(vidbuffer + curypos * 576 + 32, 0, 512);
}

static void setpalallgamma(void)
{
    u4 i = 0;
    do {
        u2 const dx = cgram[i];
        prevpal[i] = dx;

        u2 r = (dx & 0x1F) + gammalevel16b;
        if (r > 31)
            r = 31;
        r = r * vidbright / 15 << vesa2_rpos;

        u2 g = (dx >> 5 & 0x1F) + gammalevel16b;
        if (g > 31)
            g = 31;
        g = g * vidbright / 15 << vesa2_gpos;

        u2 b = (dx >> 10 & 0x1F) + gammalevel16b;
        if (b > 31)
            b = 31;
        b = b * vidbright / 15 << vesa2_bpos;

        u2 c = r + g + b;
        if (c == 0 && vidbright != 0)
            c |= 0x0020;
        pal16b[i] = pal16b[i] & 0xFFFF0000 | c;
        pal16bcl[i] = pal16bcl[i] & 0xFFFF0000 | c & vesa2_clbit;
        pal16bxcl[i] = pal16bxcl[i] & 0xFFFF0000 | (c ^ 0xFFFF) & vesa2_clbit;
    } while (++i != 256);
    prevbright = vidbright;
}

static void setpalette16bgamma(void)
{
    if (vidbright != prevbright) {
        setpalallgamma();
        return;
    }

    if (cgmod == 0)
        return;
    cgmod = 0;

    u4 i = 0;
    do {
        u2 const dx = cgram[i];
        if (prevpal[i] == dx)
            continue;
        prevpal[i] = dx;

        u2 r = (dx & 0x1F) + gammalevel16b;
        if (r > 31)
            r = 31;
        r = r * vidbright / 15 << vesa2_rpos;

        u2 g = (dx >> 5 & 0x1F) + gammalevel16b;
        if (g > 31)
            g = 31;
        g = g * vidbright / 15 << vesa2_gpos;

        u2 b = (dx >> 10 & 0x1F) + gammalevel16b;
        if (b > 31)
            b = 31;
        b = b * vidbright / 15 << vesa2_bpos;

        u2 c = r + g + b;
        if (c == 0 && vidbright != 0)
            c |= 0x0020;
        pal16b[i] = pal16b[i] & 0xFFFF0000 | c;
        pal16bcl[i] = pal16bcl[i] & 0xFFFF0000 | c & vesa2_clbit;
        pal16bxcl[i] = pal16bxcl[i] & 0xFFFF0000 | (c ^ 0xFFFF) & vesa2_clbit;
    } while (++i != 256);
}

// Set palette 16bit
static void setpalall(void)
{
    if (V8Mode == 1)
        doveg();
    u4 i = 0;
    do {
        u2 const dx = cgram[i];
        prevpal[i] = dx;
        u2 const r = (dx & 0x1F) * vidbright / 15 << vesa2_rpos;
        u2 const g = (dx >> 5 & 0x1F) * vidbright / 15 << vesa2_gpos;
        u2 const b = (dx >> 10 & 0x1F) * vidbright / 15 << vesa2_bpos;
        u2 c = r + g + b;
        if (c == 0 && vidbright != 0)
            c |= 0x0020;
        pal16b[i] = pal16b[i] & 0xFFFF0000 | c;
        pal16bcl[i] = pal16bcl[i] & 0xFFFF0000 | c & vesa2_clbit;
        pal16bxcl[i] = pal16bxcl[i] & 0xFFFF0000 | (c ^ 0xFFFF) & vesa2_clbit;
    } while (++i != 256);
    prevbright = vidbright;
    if (V8Mode == 1)
        dovegrest();
}

void setpalette16b(void)
{
    if (gammalevel16b != 0) {
        setpalette16bgamma();
        return;
    }
    if (V8Mode == 1)
        doveg();
    if (vidbright != prevbright) {
        setpalall();
        return;
    }
    if (cgmod != 0) {
        cgmod = 0;
        u4 i = 0;
        do {
            u2 const dx = cgram[i];
            if (prevpal[i] == dx)
                continue;
            prevpal[i] = dx;

            u2 const r = (dx & 0x1F) * vidbright / 15 << vesa2_rpos;
            u2 const g = (dx >> 5 & 0x1F) * vidbright / 15 << vesa2_gpos;
            u2 const b = (dx >> 10 & 0x1F) * vidbright / 15 << vesa2_bpos;
            u2 c = r + g + b;
            if (c == 0 && vidbright != 0)
                c |= 0x0020;
            pal16b[i] = pal16b[i] & 0xFFFF0000 | c;
            pal16bcl[i] = pal16bcl[i] & 0xFFFF0000 | c & vesa2_clbit;
            pal16bxcl[i] = pal16bxcl[i] & 0xFFFF0000 | (c ^ 0xFFFF) & vesa2_clbit;
        } while (++i != 256);
    }
    if (V8Mode == 1)
        dovegrest();
}

// Clear Backarea, 16-bit mode
void clearback16b(void)
{
    u2 c;
    if (scaddtype & 0x20 && !(scaddtype & 0x80)) {
        u2 const dx = cgram[0];
        c = 0;

        u2 r = (dx & 0x1F) + coladdr;
        if (r > 31)
            r = 31;
        c += r * vidbright << vesa2_rpos;

        u2 g = (dx >> 5 & 0x1F) + coladdg;
        if (g > 31)
            g = 31;
        c += g * vidbright << vesa2_gpos;

        u2 b = (dx >> 10 & 0x1F) + coladdb;
        if (b > 31)
            b = 31;
        c += b * vidbright << vesa2_bpos;
    } else {
        c = pal16b[0];
    }
    u4 eax = c * 0x00010001;
    u1* buf = curvidoffset;
    u4 n = 128;
    do
        *(u4*)buf = eax;
    while (buf += 4, --n != 0);
}

static void sprdrawpra16b(u4 const eax, u1 const cl, u1 const ch, u4 const ebx, u2* const edi, u4 const p1)
{
    if (eax == 0)
        return;
    if (sprpriodata[ebx - p1 + 16] & cl)
        return;
    edi[ebx - p1] = pal16b[(eax + ch) & 0xFF];
    sprpriodata[ebx - p1 + 16] |= cl;
}

static void sprdrawprb16b(u4 const eax, u1 const cl, u1 const ch, u4 const ebx, u2* const edi, u4 const p1)
{
    if (eax == 0)
        return;
    edi[ebx - p1] = pal16b[(eax + ch) & 0xFF];
}

static void sprdrawa16b(u1 const cl, u1 const ch, u4 const ebx, u1* const esi, u2* const edi, void (*const f)(u4 eax, u1 cl, u1 ch, u4 ebx, u2* edi, u4 p1))
{
    f(esi[0], cl, ch, ebx, edi, 8);
    f(esi[1], cl, ch, ebx, edi, 7);
    f(esi[2], cl, ch, ebx, edi, 6);
    f(esi[3], cl, ch, ebx, edi, 5);
    f(esi[4], cl, ch, ebx, edi, 4);
    f(esi[5], cl, ch, ebx, edi, 3);
    f(esi[6], cl, ch, ebx, edi, 2);
    f(esi[7], cl, ch, ebx, edi, 1);
}

static void sprdrawaf16b(u1 const cl, u1 const ch, u4 const ebx, u1* const esi, u2* const edi, void (*const f)(u4 eax, u1 cl, u1 ch, u4 ebx, u2* edi, u4 p1))
{
    f(esi[0], cl, ch, ebx, edi, 1);
    f(esi[1], cl, ch, ebx, edi, 2);
    f(esi[2], cl, ch, ebx, edi, 3);
    f(esi[3], cl, ch, ebx, edi, 4);
    f(esi[4], cl, ch, ebx, edi, 5);
    f(esi[5], cl, ch, ebx, edi, 6);
    f(esi[6], cl, ch, ebx, edi, 7);
    f(esi[7], cl, ch, ebx, edi, 8);
}

static void sprdrawpra2(u1 const dl, u4 const ebx, u4 const p1, u1 const p2)
{
    if (p2 == 0)
        return;
    sprpriodata[ebx - p1 + 16] |= dl;
}

static void sprdrawaf(u1 const dl, u4 const ebx, u1* const esi, void (*const f)(u1 dl, u4 ebx, u4 p1, u1 p2))
{
    f(dl, ebx, 1, esi[0]);
    f(dl, ebx, 2, esi[1]);
    f(dl, ebx, 3, esi[2]);
    f(dl, ebx, 4, esi[3]);
    f(dl, ebx, 5, esi[4]);
    f(dl, ebx, 6, esi[5]);
    f(dl, ebx, 7, esi[6]);
    f(dl, ebx, 8, esi[7]);
}

static void sprdrawa(u1 const dl, u4 const ebx, u1* const esi, void (*const f)(u1 dl, u4 ebx, u4 p1, u1 p2))
{
    f(dl, ebx, 8, esi[0]);
    f(dl, ebx, 7, esi[1]);
    f(dl, ebx, 6, esi[2]);
    f(dl, ebx, 5, esi[3]);
    f(dl, ebx, 4, esi[4]);
    f(dl, ebx, 3, esi[5]);
    f(dl, ebx, 2, esi[6]);
    f(dl, ebx, 1, esi[7]);
}

static void sprdrawpraw16b(u4 const eax, u1 const cl, u1 const ch, u4 const ebx, u2* const edi, u4 const p1)
{
    if (eax == 0)
        return;
    if (sprpriodata[ebx - p1 + 16] & cl)
        return;
    if (winspdata[ebx - p1 + 16] != 0)
        return;
    edi[ebx - p1] = pal16b[(eax + ch) & 0xFF];
    sprpriodata[ebx - p1 + 16] |= cl;
}

static void sprdrawprbw16b(u4 const eax, u1 const cl, u1 const ch, u4 const ebx, u2* const edi, u4 const p1)
{
    if (eax == 0)
        return;
    if (winspdata[ebx - p1 + 16] != 0)
        return;
    edi[ebx - p1] = pal16b[(eax + ch) & 0xFF];
}

static void drawspritesprio16bwinon(u1 cl, u4 const ebp)
{
    SpriteInfo const* esi = currentobjptr;
    u2* const edi = (u2*)curvidoffset;
    if (sprsingle == 1) {
        esi += cl - 1;
        do {
            u2 const ebx = esi->x;
            u1 const ch = esi->pal;
            u1* const esi_ = esi->obj;
            if (esi->status & 0x20) { // flip x
                sprdrawaf16b(cl, ch, ebx, esi_, edi, sprdrawprbw16b);
            } else {
                sprdrawa16b(cl, ch, ebx, esi_, edi, sprdrawprbw16b);
            }
        } while (--esi, --cl != 0);
    } else {
        csprprlft = cl;
        do {
            u2 const ebx = esi->x;
            u1 const ch = esi->pal;
            u4 const edx = esi->status & 0x03;
            u1* const esi_ = esi->obj;
            if (esi->status & 0x20) { // flip x
                if (edx == ebp) {
                    sprdrawaf16b(csprbit, ch, ebx, esi_, edi, sprdrawpraw16b);
                } else {
                    sprdrawaf(csprbit, ebx, esi_, sprdrawpra2);
                }
            } else {
                if (edx == ebp) {
                    sprdrawa16b(csprbit, ch, ebx, esi_, edi, sprdrawpraw16b);
                } else {
                    sprdrawa(csprbit, ebx, esi_, sprdrawpra2);
                }
            }
        } while (++esi, --csprprlft != 0);
        csprbit = ROL(csprbit, 1);
        if (csprbit == 1)
            memset(sprpriodata + 16, 0, 256);
    }
}

static void drawsprites16bprio(u1 cl, u4 const ebp)
{
    if (sprclprio[ebp] == 0)
        return;
    if (cwinenabm & 0x10 && winonsp != 0) {
        drawspritesprio16bwinon(cl, ebp);
    } else {
        SpriteInfo const* esi = currentobjptr;
        u2* const edi = (u2*)curvidoffset;
        if (sprsingle == 1) {
            esi += cl - 1;
            do {
                u2 const ebx = esi->x;
                u1 const ch = esi->pal;
                u1* const esi_ = esi->obj;
                if (esi->status & 0x20) { // flip x
                    sprdrawaf16b(cl, ch, ebx, esi_, edi, sprdrawprb16b);
                } else {
                    sprdrawa16b(cl, ch, ebx, esi_, edi, sprdrawprb16b);
                }
            } while (--esi, --cl != 0);
        } else {
            csprprlft = cl;
            do {
                u2 const ebx = esi->x;
                u1 const ch = esi->pal;
                u4 const edx = esi->status & 0x03;
                u1* const esi_ = esi->obj;
                if (esi->status & 0x20) { // flip x
                    if (edx == ebp) {
                        sprdrawaf16b(csprbit, ch, ebx, esi_, edi, sprdrawpra16b);
                    } else {
                        sprdrawaf(csprbit, ebx, esi_, sprdrawpra2);
                    }
                } else {
                    if (edx == ebp) {
                        sprdrawa16b(csprbit, ch, ebx, esi_, edi, sprdrawpra16b);
                    } else {
                        sprdrawa(csprbit, ebx, esi_, sprdrawpra2);
                    }
                }
            } while (++esi, --csprprlft != 0);
            csprbit = ROL(csprbit, 1);
            if (csprbit == 1)
                memset(sprpriodata + 16, 0, 256);
        }
    }
}

static void drawsprites16bwinon(u1 cl)
{
    SpriteInfo* esi = currentobjptr;
    u2* const edi = (u2*)curvidoffset;
    do {
        u2 const ebx = esi->x;
        u1 const ch = esi->pal;
        u1* const esi_ = esi->obj;
        if (esi->status & 0x20) { // flip x
            if (esi_[7] & 0x0F && winspdata[ebx - 8 - 16] == 0)
                edi[ebx - 8] = pal16b[(esi_[7] + ch) & 0xFF];
            if (esi_[6] & 0x0F && winspdata[ebx - 7 - 16] == 0)
                edi[ebx - 7] = pal16b[(esi_[6] + ch) & 0xFF];
            if (esi_[5] & 0x0F && winspdata[ebx - 6 - 16] == 0)
                edi[ebx - 6] = pal16b[(esi_[5] + ch) & 0xFF];
            if (esi_[4] & 0x0F && winspdata[ebx - 5 - 16] == 0)
                edi[ebx - 5] = pal16b[(esi_[4] + ch) & 0xFF];
            if (esi_[3] & 0x0F && winspdata[ebx - 4 - 16] == 0)
                edi[ebx - 4] = pal16b[(esi_[3] + ch) & 0xFF];
            if (esi_[2] & 0x0F && winspdata[ebx - 3 - 16] == 0)
                edi[ebx - 3] = pal16b[(esi_[2] + ch) & 0xFF];
            if (esi_[1] & 0x0F && winspdata[ebx - 2 - 16] == 0)
                edi[ebx - 2] = pal16b[(esi_[1] + ch) & 0xFF];
            if (esi_[0] & 0x0F && winspdata[ebx - 1 - 16] == 0)
                edi[ebx - 1] = pal16b[(esi_[0] + ch) & 0xFF];
        } else {
            if (esi_[0] & 0x0F && winspdata[ebx - 8 + 16] == 0)
                edi[ebx - 8] = pal16b[(esi_[0] + ch) & 0xFF];
            if (esi_[1] & 0x0F && winspdata[ebx - 7 + 16] == 0)
                edi[ebx - 7] = pal16b[(esi_[1] + ch) & 0xFF];
            if (esi_[2] & 0x0F && winspdata[ebx - 6 + 16] == 0)
                edi[ebx - 6] = pal16b[(esi_[2] + ch) & 0xFF];
            if (esi_[3] & 0x0F && winspdata[ebx - 5 + 16] == 0)
                edi[ebx - 5] = pal16b[(esi_[3] + ch) & 0xFF];
            if (esi_[4] & 0x0F && winspdata[ebx - 4 + 16] == 0)
                edi[ebx - 4] = pal16b[(esi_[4] + ch) & 0xFF];
            if (esi_[5] & 0x0F && winspdata[ebx - 3 + 16] == 0)
                edi[ebx - 3] = pal16b[(esi_[5] + ch) & 0xFF];
            if (esi_[6] & 0x0F && winspdata[ebx - 2 + 16] == 0)
                edi[ebx - 2] = pal16b[(esi_[6] + ch) & 0xFF];
            if (esi_[7] & 0x0F && winspdata[ebx - 1 + 16] == 0)
                edi[ebx - 1] = pal16b[(esi_[7] + ch) & 0xFF];
        }
    } while (++esi, --cl != 0);
    currentobjptr = esi;
}

// Processes & Draws 4-bit sprites
void drawsprites16b(u1 cl, u4 const ebp)
{
    if (sprprifix == 1) {
        drawsprites16bprio(cl, ebp);
    } else if (cwinenabm & 0x10 && winonsp != 0) {
        drawsprites16bwinon(cl);
    } else {
        SpriteInfo* esi = currentobjptr;
        u2* const dst = (u2*)curvidoffset;
        do {
            u2 const ebx = esi->x & 0x00007FFF;
            u1 const ch = esi->pal;
            u1* const src = esi->obj;
            if (esi->status & 0x20) { // flip x
                if (src[7] & 0x0F)
                    dst[ebx - 8] = pal16b[(src[7] + ch) & 0xFF];
                if (src[6] & 0x0F)
                    dst[ebx - 7] = pal16b[(src[6] + ch) & 0xFF];
                if (src[5] & 0x0F)
                    dst[ebx - 6] = pal16b[(src[5] + ch) & 0xFF];
                if (src[4] & 0x0F)
                    dst[ebx - 5] = pal16b[(src[4] + ch) & 0xFF];
                if (src[3] & 0x0F)
                    dst[ebx - 4] = pal16b[(src[3] + ch) & 0xFF];
                if (src[2] & 0x0F)
                    dst[ebx - 3] = pal16b[(src[2] + ch) & 0xFF];
                if (src[1] & 0x0F)
                    dst[ebx - 2] = pal16b[(src[1] + ch) & 0xFF];
                if (src[0] & 0x0F)
                    dst[ebx - 1] = pal16b[(src[0] + ch) & 0xFF];
            } else {
                if (src[0] & 0x0F)
                    dst[ebx - 8] = pal16b[(src[0] + ch) & 0xFF];
                if (src[1] & 0x0F)
                    dst[ebx - 7] = pal16b[(src[1] + ch) & 0xFF];
                if (src[2] & 0x0F)
                    dst[ebx - 6] = pal16b[(src[2] + ch) & 0xFF];
                if (src[3] & 0x0F)
                    dst[ebx - 5] = pal16b[(src[3] + ch) & 0xFF];
                if (src[4] & 0x0F)
                    dst[ebx - 4] = pal16b[(src[4] + ch) & 0xFF];
                if (src[5] & 0x0F)
                    dst[ebx - 3] = pal16b[(src[5] + ch) & 0xFF];
                if (src[6] & 0x0F)
                    dst[ebx - 2] = pal16b[(src[6] + ch) & 0xFF];
                if (src[7] & 0x0F)
                    dst[ebx - 1] = pal16b[(src[7] + ch) & 0xFF];
            }
        } while (++esi, --cl != 0);
        currentobjptr = esi;
    }
}

void procspritesmain16b(u4 const ebp)
{
    if (scrndis & 0x10)
        return;
    if (!(scrnon & 0x10))
        return;
    if (winonsp == 0xFF)
        return;
    u1 const cl = cursprloc[curypos & 0x00FF];
    if (sprprifix == 0)
        cursprloc += 256;
    if (cl == 0)
        return;
    drawsprites16b(cl, ebp);
}

static void Draw16x1616bwin(u1 const dh, u1 const* const ebx, u1 const* const ebp, u2* const esi, u4 const p1, u4 const p2)
{
    u1 const al = ebx[p1];
    if (al == 0 || ebp[p2] != 0)
        return;
    esi[p2] = pal16b[(al + dh) & 0xFF];
}

static void draw16x1616bwinon(u2* esi, u2 const* edi)
{
    tileleft16b = 33;
    u1 dl = temp;
    u1 const* ebp = winptrref;
    do {
        u2 ax = *edi;
        u1 dh = ax >> 8;
        a16x16xinc ^= 1;
        if (a16x16xinc & 0x01) {
            if (dh & 0x40)
                ++ax;
        } else {
            if (!(dh & 0x40))
                ++ax;
            ++edi;
        }
        dh ^= curbgpr;
        if (!(dh & 0x20)) {
            ++drawn;
            ax += dh & 0x80 ? draw16x1616b_yflipadd : draw16x1616b_yadd;
            ax &= 0x03FF; // filter out tile #
            u1 const* ebx = tempcach + ax * 64;
            if (ebx >= bgofwptr)
                ebx -= bgsubby;
            ebx += dh & 0x80 ? yrevadder : yadder;
            u1 const dh_ = ((dh & 0x1C) << bshifter) + bgcoloradder; // process palette # (bits 10-12)
            if (dh & 0x40) { // reversed loop
                Draw16x1616bwin(dh_, ebx, ebp, esi, 7, 0);
                Draw16x1616bwin(dh_, ebx, ebp, esi, 6, 1);
                Draw16x1616bwin(dh_, ebx, ebp, esi, 5, 2);
                Draw16x1616bwin(dh_, ebx, ebp, esi, 4, 3);
                Draw16x1616bwin(dh_, ebx, ebp, esi, 3, 4);
                Draw16x1616bwin(dh_, ebx, ebp, esi, 2, 5);
                Draw16x1616bwin(dh_, ebx, ebp, esi, 1, 6);
                Draw16x1616bwin(dh_, ebx, ebp, esi, 0, 7);
            } else { // Start loop
                Draw16x1616bwin(dh_, ebx, ebp, esi, 0, 0);
                Draw16x1616bwin(dh_, ebx, ebp, esi, 1, 1);
                Draw16x1616bwin(dh_, ebx, ebp, esi, 2, 2);
                Draw16x1616bwin(dh_, ebx, ebp, esi, 3, 3);
                Draw16x1616bwin(dh_, ebx, ebp, esi, 4, 4);
                Draw16x1616bwin(dh_, ebx, ebp, esi, 5, 5);
                Draw16x1616bwin(dh_, ebx, ebp, esi, 6, 6);
                Draw16x1616bwin(dh_, ebx, ebp, esi, 7, 7);
            }
        }
        esi += 8;
        ebp += 8;
        if (!(a16x16xinc & 0x01))
            ++dl;
        if (dl == 0x20) {
            dl = 0;
            edi = temptile;
        }
    } while (--tileleft16b != 0);
    if (curmosaicsz != 1) {
        u4 edx = curmosaicsz << 8;
        asm volatile("push %%ebp;  call %P1;  pop %%ebp"
                     : "+d"(edx)
                     : "X"(domosaic16b)
                     : "cc", "memory", "eax", "ecx", "esi", "edi");
    }
}

// Processes & Draws 16x16 tiles in 2, 4, & 8 bit mode
static void Draw16x1616b(u1 const dh, u1 const* const ebx, u2* const esi, u4 const p1, u4 const p2)
{
    u1 const al = ebx[p1];
    if (al == 0)
        return;
    esi[p2] = pal16b[(al + dh) & 0xFF];
}

void draw16x1616b(u4 const eax, u4 const ecx, u2* const edx, u1* const ebx, u4 const eax_, u2 const* edi)
{
    drawn = 0;
    temp = eax >> 0;
    bshifter = eax >> 8;
    a16x16xinc = eax >> 16;
    a16x16yinc = eax >> 24;
    yadder = ecx;
    tempcach = ebx;
    yrevadder = 56 - ecx;
    temptile = edx;
    // set up y adders
    if (a16x16yinc & 0x01) {
        draw16x1616b_yadd = 16;
        draw16x1616b_yflipadd = 0;
    } else {
        draw16x1616b_yadd = 0;
        draw16x1616b_yflipadd = 16;
    }
    // esi = pointer to video buffer
    winptrref = cwinptr - eax_;
    u2* esi;
    if (curmosaicsz != 1) {
        memset(xtravbuf + 16, 0, 256 * sizeof(*xtravbuf));
        esi = xtravbuf + 16;
    } else {
        esi = (u2*)curvidoffset; // esi = [vidbuffer] + curypos * 288 + 16 - HOfs
    }
    esi -= eax_;
    temptile = edx;

    bgsubby = 262144;
    u1* ecx_ = vcache2b + 262144;
    bgofwptr = ecx_;
    if (tempcach >= ecx_) {
        bgsubby = 131072;
        ecx_ = vcache4b + 131072;
        bgofwptr = ecx_;
        if (tempcach >= ecx_) {
            ecx_ = vcache8b + 65536;
            bgofwptr = ecx_;
            bgsubby = 65536;
        }
    }

    if (curmosaicsz == 1 && winon != 0) {
        draw16x1616bwinon(esi, edi);
    } else {
        tileleft16b = 33;
        u1 dl = temp;
        do {
            u2 ax = *edi;
            u1 dh = ax >> 8;
            a16x16xinc ^= 1;
            if (a16x16xinc & 0x01) {
                if (dh & 0x40)
                    ++ax;
            } else {
                if (!(dh & 0x40))
                    ++ax;
                ++edi;
            }
            dh ^= curbgpr;
            if (!(dh & 0x20)) {
                ++drawn;
                ax += dh & 0x80 ? draw16x1616b_yflipadd : draw16x1616b_yadd;
                ax &= 0x03FF; // filter out tile #
                u1 const* ebx = tempcach + ax * 64;
                if (ebx >= bgofwptr)
                    ebx -= bgsubby;
                ebx += dh & 0x80 ? yrevadder : yadder;
                u1 const dh_ = ((dh & 0x1C) << bshifter) + bgcoloradder; // process palette # (bits 10-12)
                if (dh & 0x40) { // reversed loop
                    Draw16x1616b(dh_, ebx, esi, 7, 0);
                    Draw16x1616b(dh_, ebx, esi, 6, 1);
                    Draw16x1616b(dh_, ebx, esi, 5, 2);
                    Draw16x1616b(dh_, ebx, esi, 4, 3);
                    Draw16x1616b(dh_, ebx, esi, 3, 4);
                    Draw16x1616b(dh_, ebx, esi, 2, 5);
                    Draw16x1616b(dh_, ebx, esi, 1, 6);
                    Draw16x1616b(dh_, ebx, esi, 0, 7);
                } else { // Start loop
                    Draw16x1616b(dh_, ebx, esi, 0, 0);
                    Draw16x1616b(dh_, ebx, esi, 1, 1);
                    Draw16x1616b(dh_, ebx, esi, 2, 2);
                    Draw16x1616b(dh_, ebx, esi, 3, 3);
                    Draw16x1616b(dh_, ebx, esi, 4, 4);
                    Draw16x1616b(dh_, ebx, esi, 5, 5);
                    Draw16x1616b(dh_, ebx, esi, 6, 6);
                    Draw16x1616b(dh_, ebx, esi, 7, 7);
                }
            }
            esi += 8;
            if (!(a16x16xinc & 0x01))
                ++dl;
            if (dl == 0x20) {
                dl = 0;
                edi = temptile;
            }
        } while (--tileleft16b != 0);
        if (curmosaicsz != 1) {
            u4 edx = curmosaicsz << 8;
            asm volatile("push %%ebp;  call %P1;  pop %%ebp"
                         : "+d"(edx)
                         : "X"(domosaic16b)
                         : "cc", "memory", "eax", "ecx", "esi", "edi");
        }
    }
}

// Processes & Draws 8x8 tiles in 2, 4, & 8 bit mode
static void Draw8x816bmacro(u1 const dh, u1 const* const ebx, u2* const esi, u4 const p1)
{
    u1 const al = ebx[p1];
    if (al == 0)
        return;
    esi[p1] = pal16b[(al + dh) & 0xFF];
}

static void Draw8x816bflipmacro(u1 const dh, u1 const* const ebx, u2* const esi, u4 const p1)
{
    u1 const al = ebx[7 - p1];
    if (al == 0)
        return;
    esi[p1] = pal16b[(al + dh) & 0xFF];
}

// Processes & Draws 8x8 offset mode in Mode 2/4
static void initoffsetmode(u4 const ebp, u2 const* const edi)
{
    u4 ebx = 0x2000 << ebp;
    OMBGTestVal = ebx;
    u4 ecx = bg1scroly[ebp] + ebx;
    u4 edx = bg1scroly[2];
    if (edx != 0xFFFF)
        edx &= 0x01FF;
    u4 eax = (bg1ptr[2] + (edx >> 3 << 6)) & 0x0000FFFF;
    ebx = 0;
    ebx = *(u4*)&curypos; // XXX cast makes no sense, variable defined in .c
    ofsmcyps = ebx;
    eax = (eax + ((bg1scrolx[2] & 0x00F8) >> 3 << 1)) & 0x0000FFFF;
    if (bg1scroly[2] > 0xFFF7)
        eax = (eax + 0x0780) & 0x0000FFFF;
    eax += 0x40;
    ofsmcptr = vram + (eax & 0xFFFFFFC0);
    ofsmcptr2 = eax & 0x3F;
    ofsmady = bg1ptry[ebp];
    ofsmadx = bg1ptrx[ebp];
    eax = *(u4*)&bg1ptr[ebp]; // XXX strange cast
    ofsmtptr = eax;
    ofsmtptrs = eax;
    if (ecx & 0x0100)
        eax += bg1ptry[ebp];
    eax += (ecx * 8) & 0x07C0; // 0x1F * 0x40
    yposngom = yadder;
    flipyposngom = yrevadder;
    ecx = *(u4*)&bg1scrolx[ebp]; // XXX strange cast
    edx = bg1ptrx[ebp];
    if (ecx & 0x0100) {
        eax += edx;
        ofsmtptr += edx;
        edx = edx & 0xFFFF0000 | -edx & 0x0000FFFF;
    }
    edx = edx & 0xFFFF0000 | (edx - 64) & 0x0000FFFF;
    ecx = (ecx & 0x00F8) >> 2;
    bgtxadd = edx;
    ofsmtptr += ecx;
    ofsmmptr = ecx + (eax & 0xFFFF);
    bgsubby = bg1objptr[ebp] >> 5;
    ofsmmptr = (u1 const*)edi - vram;
    ofshvaladd = 0;
}

static void offsetmcachechk(u4 const eax)
{ // Cache check
    u4 const ecx = (eax + ngptrdat2) & 0x07FF;
    if (vidmemch4[ecx] == 0)
        return;
    asm volatile("call %P0" ::"X"(cachesingle4bng), "c"(ecx)
                 : "cc", "memory");
}

static u2* procoffsetmode(void)
{
    // TODO most/all of the upper halfword preservation probably is pointless
    ofsmmptr = ofsmmptr & 0xFFFF0000 | (ofsmmptr + 2) & 0x0000FFFF;
    ofsmtptr = ofsmtptr & 0xFFFF0000 | (ofsmtptr + 2) & 0x0000FFFF;
    u4 eax = flipyposngom;
    yadder = yposngom;
    yrevadder = eax;
    eax = eax & 0xFFFF0000 | ofsmmptr & 0x0000FFFF;
    if ((eax & 0x3F) == 0) {
        u4 const ebx = bgtxadd;
        eax = eax & 0xFFFF0000 | (eax + ebx) & 0x0000FFFF;
        ofsmmptr = ofsmmptr & 0xFFFF0000 | (ofsmmptr + ebx) & 0x0000FFFF;
        ofsmtptr = ofsmtptr & 0xFFFF0000 | (ofsmtptr + ebx) & 0x0000FFFF;
    }
    u1* edi = vram + eax;
    u4* const ebx_ = (u4*)(ofsmcptr + ofsmcptr2);
    eax = OMBGTestVal;
    if (*ebx_ & eax) {
        u4 const ebx = *ebx_ & 0x003FF + ofsmcyps;
        eax = eax & 0xFFFF0000 | ofsmtptr & 0x0000FFFF;
        if (ebx & 0x100)
            eax = eax & 0xFFFF0000 | (eax + ofsmady) & 0x0000FFFF;
        u4 const edx = (ebx & 0x07) << 3;
        eax = eax & 0xFFFF0000 | (eax + ((ebx & 0xF8) << 3)) & 0x0000FFFF;
        yadder = edx;
        yrevadder = edx ^ 0x38;
        edi = vram + eax;
    }
    ofshvaladd += 8;
    ofsmcptr2 = (ofsmcptr2 + 2) & 0x3F;
    if (ebx_[-16] & OMBGTestVal) {
        u4 eax = edi - vram;
        u4 const ebx = ebx_[-16] + ofshvaladd;
        eax = eax & 0xFFFF0000 | (eax + ofsmtptr - ofsmtptrs) & 0x0000FFFF;
        if (ebx & 0x100)
            eax = eax & 0xFFFF0000 | (eax + ofsmadx) & 0x0000FFFF;
        eax = eax & 0xFFFF0000 | (eax + ((ebx & 0xF8) >> 2)) & 0x0000FFFF;
        edi = vram + eax;
    }
    return (u2*)edi;
}

static void Draw8x816bwinmacro(u1 const dh, u1 const* const ebx, u1 const* const ebp, u2* const esi, u4 const p1)
{
    u1 const al = ebx[p1];
    if (al != 0 && ebp[p1] == 0)
        esi[p1] = pal16b[(al + dh) & 0xFF];
}

static void Draw8x816bwinflipmacro(u1 const dh, u1 const* const ebx, u1 const* const ebp, u2* const esi, u4 const p1)
{
    u1 const al = ebx[7 - p1];
    if (al != 0 && ebp[p1] == 0)
        esi[p1] = pal16b[(al + dh) & 0xFF];
}

static void draw8x816bwinonoffset(u2* esi, u2 const* edi)
{
    tileleft16b = 33;
    drawn = 0;
    u1 const* ebp = winptrref;
    do {
        u2 const ax = *edi++;
        u1 const dh = ax >> 8 ^ curbgpr;
        if (!(dh & 0x20)) {
            ++drawn;
            u4 const eax = (ax & 0x03FF); // filter out tile #
            offsetmcachechk(eax);
            u1 const* ebx = tempcach + eax * 64;
            if (ebx >= bgofwptr)
                ebx -= bgsubby; // Clip
            ebx += dh & 0x80 ? yrevadder : yadder;

            u1 const dh_ = ((dh & 0x1C) << bshifter) + bgcoloradder; // process palette # (bits 10-12)
            if (!(dh & 0x40)) { // Begin Normal Loop
                // Start loop
                if (*(u4 const*)ebx != 0) {
                    Draw8x816bwinmacro(dh_, ebx, ebp, esi, 0);
                    Draw8x816bwinmacro(dh_, ebx, ebp, esi, 1);
                    Draw8x816bwinmacro(dh_, ebx, ebp, esi, 2);
                    Draw8x816bwinmacro(dh_, ebx, ebp, esi, 3);
                }
                if (*(u4 const*)(ebx + 4) != 0) {
                    Draw8x816bwinmacro(dh_, ebx, ebp, esi, 4);
                    Draw8x816bwinmacro(dh_, ebx, ebp, esi, 5);
                    Draw8x816bwinmacro(dh_, ebx, ebp, esi, 6);
                    Draw8x816bwinmacro(dh_, ebx, ebp, esi, 7);
                }
            } else { // reversed loop
                if (*(u4 const*)(ebx + 4) != 0) {
                    Draw8x816bwinflipmacro(dh_, ebx, ebp, esi, 0);
                    Draw8x816bwinflipmacro(dh_, ebx, ebp, esi, 1);
                    Draw8x816bwinflipmacro(dh_, ebx, ebp, esi, 2);
                    Draw8x816bwinflipmacro(dh_, ebx, ebp, esi, 3);
                }
                if (*(u4 const*)ebx != 0) {
                    Draw8x816bwinflipmacro(dh_, ebx, ebp, esi, 4);
                    Draw8x816bwinflipmacro(dh_, ebx, ebp, esi, 5);
                    Draw8x816bwinflipmacro(dh_, ebx, ebp, esi, 6);
                    Draw8x816bwinflipmacro(dh_, ebx, ebp, esi, 7);
                }
            }
        }
        edi = procoffsetmode();
        esi += 8;
        ebp += 8;
    } while (--tileleft16b != 0);
}

// Processes & Draws 8x8 tiles, offset mode
void draw8x816boffset(u4 const eax, u4 const ecx, u2* const edx, u1* const ebx, u4 const ebp, u4 const eax_, u2 const* edi)
{
    temp = eax;
    bshifter = eax >> 8;
    yadder = ecx;
    tempcach = ebx;
    yrevadder = 56 - ecx;
    initoffsetmode(ebp, edi);
    // esi = pointer to video buffer
    winptrref = cwinptr - eax_;
    u2* esi = (u2*)curvidoffset - eax_; // esi = [vidbuffer] + curypos * 288 + 16 - HOfs
    if (curmosaicsz != 1) {
        memset(xtravbuf + 16, 0, 256 * sizeof(*xtravbuf));
        esi = xtravbuf + 16 - eax_;
    }
    temptile = edx;
    bgsubby = 262144;
    u1* ecx_ = vcache2b + 262144;
    bgofwptr = ecx_;
    if (tempcach >= ecx_) {
        bgsubby = 131072;
        ecx_ = vcache4b + 131072;
        bgofwptr = ecx_;
        if (tempcach >= ecx_) {
            ecx_ = vcache8b + 65536;
            bgofwptr = ecx_;
            bgsubby = 65536;
        }
    }
    /* tile value : bit 15 = flipy, bit 14 = flipx, bit 13 = priority value
     *              bit 10-12 = palette, 0-9=tile# */
    if (curmosaicsz == 1 && winon != 0) {
        draw8x816bwinonoffset(esi, edi);
    } else {
        tileleft16b = 33;
        drawn = 0;
        do {
            u2 ax = *edi++;
            u1 const dh = ax >> 8 ^ curbgpr;
            if (!(dh & 0x20)) {
                ++drawn;
                ax &= 0x03FF; // filter out tile #
                offsetmcachechk(ax);
                u1 const* ebx = tempcach + ax * 64;
                if (ebx >= bgofwptr)
                    ebx -= bgsubby;
                ebx += dh & 0x80 ? yrevadder : yadder;
                u2 const dh_ = ((dh & 0x1C) << bshifter) + bgcoloradder; // process palette # (bits 10-12)
                if (dh & 0x40) { // reversed loop
                    if (*(u4 const*)(ebx + 4) != 0) {
                        Draw8x816bflipmacro(dh_, ebx, esi, 0);
                        Draw8x816bflipmacro(dh_, ebx, esi, 1);
                        Draw8x816bflipmacro(dh_, ebx, esi, 2);
                        Draw8x816bflipmacro(dh_, ebx, esi, 3);
                    }
                    if (*(u4 const*)ebx != 0) {
                        Draw8x816bflipmacro(dh_, ebx, esi, 4);
                        Draw8x816bflipmacro(dh_, ebx, esi, 5);
                        Draw8x816bflipmacro(dh_, ebx, esi, 6);
                        Draw8x816bflipmacro(dh_, ebx, esi, 7);
                    }
                } else { // Begin Normal Loop
                    // Start loop
                    if (*(u4 const*)ebx != 0) {
                        Draw8x816bmacro(dh_, ebx, esi, 0);
                        Draw8x816bmacro(dh_, ebx, esi, 1);
                        Draw8x816bmacro(dh_, ebx, esi, 2);
                        Draw8x816bmacro(dh_, ebx, esi, 3);
                    }
                    if (*(u4 const*)(ebx + 4) != 0) {
                        Draw8x816bmacro(dh_, ebx, esi, 4);
                        Draw8x816bmacro(dh_, ebx, esi, 5);
                        Draw8x816bmacro(dh_, ebx, esi, 6);
                        Draw8x816bmacro(dh_, ebx, esi, 7);
                    }
                }
            }
            edi = procoffsetmode();
        } while (esi += 8, --tileleft16b != 0);
        if (drawn != 0 && curmosaicsz != 1) {
            u4 edx = curmosaicsz << 8;
            asm volatile("push %%ebp;  call %P1;  pop %%ebp"
                         : "+d"(edx)
                         : "X"(domosaic16b)
                         : "cc", "memory", "eax", "ecx", "esi", "edi");
        }
    }
}

static void draw8x816bwinon(u2* esi, u2 const* edi)
{
    tileleft16b = 33;
    drawn = 0;
    u1 dl = temp;
    u1 const* ebp = winptrref;
    do {
        u2 const ax = *edi++;
        u1 const dh = ax >> 8 ^ curbgpr;
        if (!(dh & 0x20)) {
            ++drawn;
            u1 const* ebx = tempcach + (ax & 0x03FF) * 64; // filter out tile #
            if (ebx >= bgofwptr)
                ebx -= bgsubby;
            ebx += dh & 0x80 ? yrevadder : yadder;

            u1 const dh_ = ((dh & 0x1C) << bshifter) + bgcoloradder; // process palette # (bits 10-12)
            if (!(dh & 0x40)) { // Begin Normal Loop
                // Start loop
                if (*(u4 const*)ebx != 0) {
                    Draw8x816bwinmacro(dh_, ebx, ebp, esi, 0);
                    Draw8x816bwinmacro(dh_, ebx, ebp, esi, 1);
                    Draw8x816bwinmacro(dh_, ebx, ebp, esi, 2);
                    Draw8x816bwinmacro(dh_, ebx, ebp, esi, 3);
                }
                if (*(u4 const*)(ebx + 4) != 0) {
                    Draw8x816bwinmacro(dh_, ebx, ebp, esi, 4);
                    Draw8x816bwinmacro(dh_, ebx, ebp, esi, 5);
                    Draw8x816bwinmacro(dh_, ebx, ebp, esi, 6);
                    Draw8x816bwinmacro(dh_, ebx, ebp, esi, 7);
                }
            } else { // reversed loop
                if (*(u4 const*)(ebx + 4) != 0) {
                    Draw8x816bwinflipmacro(dh_, ebx, ebp, esi, 0);
                    Draw8x816bwinflipmacro(dh_, ebx, ebp, esi, 1);
                    Draw8x816bwinflipmacro(dh_, ebx, ebp, esi, 2);
                    Draw8x816bwinflipmacro(dh_, ebx, ebp, esi, 3);
                }
                if (*(u4 const*)ebx != 0) {
                    Draw8x816bwinflipmacro(dh_, ebx, ebp, esi, 4);
                    Draw8x816bwinflipmacro(dh_, ebx, ebp, esi, 5);
                    Draw8x816bwinflipmacro(dh_, ebx, ebp, esi, 6);
                    Draw8x816bwinflipmacro(dh_, ebx, ebp, esi, 7);
                }
            }
        }
        esi += 8;
        ebp += 8;
        if (++dl == 0x20)
            edi = temptile;
    } while (--tileleft16b != 0);

    if (drawn != 0 && curmosaicsz != 1) {
        u4 edx = curmosaicsz << 8;
        asm volatile("push %%ebp;  call %P1;  pop %%ebp"
                     : "+d"(edx)
                     : "X"(domosaic16b)
                     : "cc", "memory", "eax", "ecx", "esi", "edi");
    }
}

static void drawpixel16b8x8winon(u1 const dh, u1 const* const ebx, u2* const esi, u1 const* const ebp, u4 const p1, u4 const p2, u4 const p3)
{
    u1 const al = ebx[p1];
    if (al != 0 && ebp[p3] == 0)
        esi[p2] = pal16b[(al + dh) & 0xFF];
}

static void draw16x816winonb(u2* esi, u2 const* edi)
{
    tileleft16b = 33;
    drawn = 0;
    u1 const* ebp = winptrref;
    u1 dl = temp;
    do {
        u2 const ax = *edi++;
        u1 const dh = (ax >> 8) ^ curbgpr;
        if (!(dh & 0x20)) {
            ++drawn;
            u1 const* ebx = tempcach + (ax & 0x03FF) * 64; // filter out tile #
            if (ebx >= bgofwptr)
                ebx -= bgsubby; // Clip
            ebx += dh & 0x80 ? yrevadder : yadder;

            u1 const dh_ = ((dh & 0x1C) << bshifter) + bgcoloradder; // process palette # (bits 10-12)
            if (!(dh & 0x40)) { // Begin Normal Loop
                // Start loop
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 1, 0, 0);
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 3, 1, 1);
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 5, 2, 2);
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 7, 3, 3);
                ebx += 64;
                // Start loop
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 1, 4, 4);
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 3, 5, 5);
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 5, 6, 6);
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 7, 7, 7);
            } else { // reversed loop
                // Start loop
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 0, 7, 0);
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 2, 6, 1);
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 4, 5, 2);
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 6, 4, 3);
                ebx += 64;
                // Start loop
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 0, 3, 4);
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 2, 2, 5);
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 4, 1, 6);
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 6, 0, 7);
            }
        }
        esi += 8;
        ebp += 8;
        if (++dl == 0x20)
            edi = temptile;
    } while (--tileleft16b != 0);

    if (drawn != 0 && curmosaicsz != 1) {
        u4 edx = curmosaicsz << 8;
        asm volatile("push %%ebp;  call %P1;  pop %%ebp"
                     : "+d"(edx)
                     : "X"(domosaic16b)
                     : "cc", "memory", "eax", "ecx", "esi", "edi");
    }
}

static void draw16x816bwinon(u2* esi, u2 const* edi)
{
    if (res512switch != 0) {
        draw16x816winonb(esi, edi);
        return;
    }

    tileleft16b = 33;
    drawn = 0;
    u1 const* ebp = winptrref;
    u1 dl = temp;
    do {
        u2 const ax = *edi++;
        u1 const dh = ax >> 8 ^ curbgpr;
        if (!(dh & 0x20)) {
            ++drawn;
            u1 const* ebx = tempcach + (ax & 0x03FF) * 64; // filter out tile #
            if (ebx >= bgofwptr)
                ebx -= bgsubby; // Clip
            ebx += dh & 0x80 ? yrevadder : yadder;

            u1 const dh_ = ((dh & 0x1C) << bshifter) + bgcoloradder; // process palette # (bits 10-12)
            if (!(dh & 0x40)) { // Begin Normal Loop
                // Start loop
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 0, 0, 0);
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 2, 1, 1);
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 4, 2, 2);
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 6, 3, 3);
                ebx += 64;
                // Start loop
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 0, 4, 4);
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 2, 5, 5);
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 4, 6, 6);
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 6, 7, 7);
            } else { // reversed loop
                // Start loop
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 1, 7, 0);
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 3, 6, 1);
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 5, 5, 2);
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 7, 4, 3);
                ebx += 64;
                // Start loop
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 1, 3, 4);
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 3, 2, 5);
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 5, 1, 6);
                drawpixel16b8x8winon(dh_, ebx, esi, ebp, 7, 0, 7);
            }
        }
        esi += 8;
        ebp += 8;
        if (++dl == 0x20)
            edi = temptile;
    } while (--tileleft16b != 0);

    if (drawn != 0 && curmosaicsz != 1) {
        u4 edx = curmosaicsz << 8;
        asm volatile("push %%ebp;  call %P1;  pop %%ebp"
                     : "+d"(edx)
                     : "X"(domosaic16b)
                     : "cc", "memory", "eax", "ecx", "esi", "edi");
    }
}

static void drawpixel16b8x8(u1 const dh, u1 const* const ebx, u2* const esi, u4 const p1, u4 const p2)
{
    u1 const al = ebx[p1];
    if (al != 0)
        esi[p2] = pal16b[(al + dh) & 0xFF];
}

static void draw16x816b(u2* esi, u2 const* edi)
{
    tileleft16b = 33;
    drawn = 0;
    u1 dl = temp;
    do {
        u2 const ax = *edi++;
        u1 const dh = ax >> 8 ^ curbgpr;
        if (!(dh & 0x20)) {
            ++drawn;
            u1 const* ebx = tempcach + (ax & 0x03FF) * 64; // Filter out tile #
            if (ebx >= bgofwptr)
                ebx -= bgsubby; // Clip
            ebx += dh & 0x80 ? yrevadder : yadder;

            u1 const dh_ = (dh & 0x1C << bshifter) + bgcoloradder; // process palette # (bits 10-12)
            if (!(dh & 0x40)) { // Begin Normal Loop
                // Start loop
                drawpixel16b8x8(dh_, ebx, esi, 1, 0);
                drawpixel16b8x8(dh_, ebx, esi, 3, 1);
                drawpixel16b8x8(dh_, ebx, esi, 5, 2);
                drawpixel16b8x8(dh_, ebx, esi, 7, 3);
                ebx += 64;
                // Start loop
                drawpixel16b8x8(dh_, ebx, esi, 1, 4);
                drawpixel16b8x8(dh_, ebx, esi, 3, 5);
                drawpixel16b8x8(dh_, ebx, esi, 5, 6);
                drawpixel16b8x8(dh_, ebx, esi, 7, 7);
            } else { // reversed loop
                // Start loop
                drawpixel16b8x8(dh_, ebx, esi, 0, 7);
                drawpixel16b8x8(dh_, ebx, esi, 2, 6);
                drawpixel16b8x8(dh_, ebx, esi, 4, 5);
                drawpixel16b8x8(dh_, ebx, esi, 6, 4);
                ebx += 64;
                // Start loop
                drawpixel16b8x8(dh_, ebx, esi, 0, 3);
                drawpixel16b8x8(dh_, ebx, esi, 2, 2);
                drawpixel16b8x8(dh_, ebx, esi, 4, 1);
                drawpixel16b8x8(dh_, ebx, esi, 6, 0);
            }
        }
        esi += 8;
        if (++dl == 0x20)
            edi = temptile;
    } while (--tileleft16b != 0);

    if (drawn != 0 && curmosaicsz != 1) {
        u4 edx = curmosaicsz << 8;
        asm volatile("push %%ebp;  call %P1;  pop %%ebp"
                     : "+d"(edx)
                     : "X"(domosaic16b)
                     : "cc", "memory", "eax", "ecx", "esi", "edi");
    }
}

static void draw16x816(u4 const eax, u4 const ecx, u2* const edx, u1* const ebx, u4 const eax_, u2 const* edi)
{
    hirestiledat[curypos & 0xFF] = 1;
    temp = eax;
    bshifter = eax >> 8;
    yadder = ecx;
    tempcach = ebx;
    yrevadder = 56 - ecx;
    // esi = pointer to video buffer
    winptrref = cwinptr - eax_;
    u2* esi = (u2*)curvidoffset - eax_; // esi = [vidbuffer] + curypos * 288 + 16 - HOfs
    if (curmosaicsz != 1) { // Mosaic
        memset(xtravbuf + 16, 0, 256 * sizeof(*xtravbuf));
        esi = xtravbuf + 16 - eax_;
    }
    temptile = edx;

    bgsubby = 262144;
    bgofwptr = vcache2b + 262144;
    if (tempcach >= bgofwptr) {
        bgsubby = 131072;
        bgofwptr = vcache4b + 131072;
        if (tempcach >= bgofwptr) {
            bgofwptr = vcache8b + 65536;
            bgsubby = 65536;
        }
    }

    /* tile value : bit 15 = flipy, bit 14 = flipx, bit 13 = priority value
     *              bit 10-12 = palette, 0-9=tile# */
    if (curmosaicsz == 1 && winon != 0) { // No mosaic
        draw16x816bwinon(esi, edi);
        return;
    }
    if (res512switch != 0) {
        draw16x816b(esi, edi);
        return;
    }

    tileleft16b = 33;
    drawn = 0;
    u1 dl = temp;
    do {
        u2 const ax = *edi++;
        u1 const dh = ax >> 8 ^ curbgpr;
        if (!(dh & 0x20)) {
            ++drawn;
            u4 const eax = (ax & 0x03FF) * 64; // filter out tile #
            u1* ebx = tempcach + eax;
            if (ebx >= bgofwptr)
                ebx -= bgsubby; // Clip
            ebx += dh & 0x80 ? yrevadder : yadder;

            u1 const dh_ = ((dh & 0x1C) << bshifter) + bgcoloradder; // process palette # (bits 10-12)
            if (!(dh & 0x40)) { // Begin Normal Loop
                // Start loop
                drawpixel16b8x8(dh_, ebx, esi, 0, 0);
                drawpixel16b8x8(dh_, ebx, esi, 2, 1);
                drawpixel16b8x8(dh_, ebx, esi, 4, 2);
                drawpixel16b8x8(dh_, ebx, esi, 6, 3);
                ebx += 64;
                // Start loop
                drawpixel16b8x8(dh_, ebx, esi, 0, 4);
                drawpixel16b8x8(dh_, ebx, esi, 2, 5);
                drawpixel16b8x8(dh_, ebx, esi, 4, 6);
                drawpixel16b8x8(dh_, ebx, esi, 6, 7);
            } else { // reversed loop
                // Start loop
                drawpixel16b8x8(dh_, ebx, esi, 1, 7);
                drawpixel16b8x8(dh_, ebx, esi, 3, 6);
                drawpixel16b8x8(dh_, ebx, esi, 5, 5);
                drawpixel16b8x8(dh_, ebx, esi, 7, 4);
                ebx += 64;
                // Start loop
                drawpixel16b8x8(dh_, ebx, esi, 1, 3);
                drawpixel16b8x8(dh_, ebx, esi, 3, 2);
                drawpixel16b8x8(dh_, ebx, esi, 5, 1);
                drawpixel16b8x8(dh_, ebx, esi, 7, 0);
            }
        }
        esi += 8;
        if (++dl == 0x20)
            edi = temptile;
    } while (--tileleft16b != 0);

    if (drawn != 0) {
        u1 const dh = curmosaicsz;
        if (dh != 1) {
            u4 edx = dh << 8;
            asm volatile("push %%ebp;  call %P1;  pop %%ebp"
                         : "+d"(edx)
                         : "X"(domosaic16b)
                         : "cc", "memory", "eax", "ecx", "esi", "edi");
        }
    }
}

void draw8x816b(u4 eax, u4 ecx, u2* edx, u1* ebx, u4 const layer, u4 eax_, u2 const* edi)
{
    if (osm2dis != 1 && bgmode == 2) {
        draw8x816boffset(eax, ecx, edx, ebx, layer, eax_, edi);
        return;
    }
    if (bgmode == 5) {
        draw16x816(eax, ecx, edx, ebx, eax_, edi);
        return;
    }
    temp = eax;
    bshifter = eax >> 8;
    yadder = ecx;
    tempcach = ebx;
    yrevadder = 56 - ecx;
    // esi = pointer to video buffer
    winptrref = cwinptr - eax_;
    u2* esi = (u2*)curvidoffset - eax_; // esi = [vidbuffer] + curypos * 288 + 16 - HOfs
    if (curmosaicsz != 1) {
        memset(xtravbuf + 16, 0, 256 * sizeof(*xtravbuf));
        esi = xtravbuf + 16 - eax_;
    }
    temptile = edx;
    bgsubby = 262144;
    u1* ecx_ = vcache2b + 262144;
    bgofwptr = ecx_;
    if (tempcach >= ecx_) {
        bgsubby = 131072;
        ecx_ = vcache4b + 131072;
        bgofwptr = ecx_;
        if (tempcach >= ecx_) {
            ecx_ = vcache8b + 65536;
            bgofwptr = ecx_;
            bgsubby = 65536;
        }
    }
    /* tile value : bit 15 = flipy, bit 14 = flipx, bit 13 = priority value
     *              bit 10-12 = palette, 0-9=tile# */
    if (curmosaicsz == 1 && winon != 0) {
        draw8x816bwinon(esi, edi);
        return;
    }
    tileleft16b = 33;
    drawn = 0;
    u1 dl = temp;
    do {
        u2 ax = *edi++;
        u1 const dh = ax >> 8 ^ curbgpr;
        if (!(dh & 0x20)) {
            ++drawn;
            ax &= 0x03FF; // filter out tile #
            u1 const* ebx = tempcach + ax * 64;
            if (ebx >= bgofwptr)
                ebx -= bgsubby;
            ebx += dh & 0x80 ? yrevadder : yadder;
            u1 const dh_ = ((dh & 0x1C) << bshifter) + bgcoloradder; // process palette # (bits 10-12)
            if (dh & 0x40) { // reversed loop
                if (*(u4 const*)(ebx + 4) != 0) {
                    Draw8x816bflipmacro(dh_, ebx, esi, 0);
                    Draw8x816bflipmacro(dh_, ebx, esi, 1);
                    Draw8x816bflipmacro(dh_, ebx, esi, 2);
                    Draw8x816bflipmacro(dh_, ebx, esi, 3);
                }
                if (*(u4 const*)ebx != 0) {
                    Draw8x816bflipmacro(dh_, ebx, esi, 4);
                    Draw8x816bflipmacro(dh_, ebx, esi, 5);
                    Draw8x816bflipmacro(dh_, ebx, esi, 6);
                    Draw8x816bflipmacro(dh_, ebx, esi, 7);
                }
            } else { // Begin Normal Loop
                // Start loop
                if (*(u4 const*)ebx != 0) {
                    Draw8x816bmacro(dh_, ebx, esi, 0);
                    Draw8x816bmacro(dh_, ebx, esi, 1);
                    Draw8x816bmacro(dh_, ebx, esi, 2);
                    Draw8x816bmacro(dh_, ebx, esi, 3);
                }
                if (*(u4 const*)(ebx + 4) != 0) {
                    Draw8x816bmacro(dh_, ebx, esi, 4);
                    Draw8x816bmacro(dh_, ebx, esi, 5);
                    Draw8x816bmacro(dh_, ebx, esi, 6);
                    Draw8x816bmacro(dh_, ebx, esi, 7);
                }
            }
        }
        if (++dl == 0x20)
            edi = temptile;
    } while (esi += 8, --tileleft16b != 0);
    if (drawn != 0 && curmosaicsz != 1) {
        u4 edx = curmosaicsz << 8;
        asm volatile("push %%ebp;  call %P1;  pop %%ebp"
                     : "+d"(edx)
                     : "X"(domosaic16b)
                     : "cc", "memory", "eax", "ecx", "esi", "edi");
    }
}

static void drawbackgrndmain16b(Layer const layer)
{
    if (colormodeofs[layer] == 0)
        return;
    if (!(scrnon & curbgnum))
        return;
    if (alreadydrawn & curbgnum)
        return;
    if (scrndis & curbgnum)
        return;
    winon = 0;
    if (winenabm & curbgnum) {
        makewindow(winen[layer], layer);
        if (winon == 0xFF)
            return;
    }
    curmosaicsz = mosaicon & curbgnum ? mosaicsz + 1 : 1;
    bgcoloradder = bgmode == 0 ? layer * 0x20 : 0;
    u4 esi = bg1vbufloc[layer];
    u2* edi = bg1tdatloc[layer];
    u2* edx = bg1tdabloc[layer];
    u1* ebx = bg1cachloc[layer];
    u4 eax = bg1xposloc[layer];
    u4 ecx = bg1yaddval[layer];
    if (bgtilesz & curbgnum) {
        draw16x1616b(eax, ecx, edx, ebx, esi, edi);
    } else {
        draw8x816b(eax, ecx, edx, ebx, layer, esi, edi);
    }
    if (drawn == 33)
        alreadydrawn |= curbgnum;
}

static void priority216b(void)
{
    cwinenabm = winenabm;
    // do background 2
    curbgpr = 0x00;
    curbgnum = 0x02;
    drawbackgrndmain16b(LAYER_BG2);
    procspritesmain16b(0);
    // do background 1
    curbgnum = 0x01;
    drawbackgrndmain16b(LAYER_BG1);
    procspritesmain16b(1);
    // do background 2
    curbgpr = 0x20;
    curbgnum = 0x02;
    drawbackgrndmain16b(LAYER_BG2);
    procspritesmain16b(2);
    // do background 1
    curbgnum = 0x01;
    drawbackgrndmain16b(LAYER_BG1);
    procspritesmain16b(3);
}

static void procmode716bextbg(u2 const* const p1, u2 const* const p2, u1 const p3)
{
    u2 ax = curypos;
    if (mode7set & 0x02)
        ax = 255 - ax; // flip
    curmosaicsz = 1;
    if (mosaicon & p3) {
        u1 bl = mosaicsz;
        if (bl != 0) {
            ++bl;
            curmosaicsz = bl;
            ax = ax / bl * bl;
        }
    }
    m7starty = ax;
    u4 eax;
    u4 edx;
    asm volatile("push %%ebp;  call %P2;  pop %%ebp"
                 : "=a"(eax), "=d"(edx)
                 : "X"(drawmode716extbg), "a"(*p1), "d"(*p2)
                 : "cc", "memory", "ecx", "ebx", "esi", "edi");
}

static void procmode716bextbg2(u1 const p3)
{
    curmosaicsz = 1;
    if (mosaicon & p3) {
        u1 const bl = mosaicsz;
        if (bl != 0)
            curmosaicsz = bl + 1;
    }
    asm volatile("push %%ebp;  call %P0;  pop %%ebp" ::"X"(drawmode716extbg2)
                 : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
}

static void procmode716b(u2 const* const p1, u2 const* const p2, u1 const p3)
{
    u2 ax = curypos;
    if (mode7set & 0x02)
        ax = 255 - ax; // flip
    curmosaicsz = 1;
    if (mosaicon & p3) {
        u1 bl = mosaicsz;
        if (bl != 0) {
            ++bl;
            curmosaicsz = bl;
            ax = ax / bl * bl;
        }
    }
    m7starty = ax;
    u4 eax;
    u4 edx;
    asm volatile("call %P2"
                 : "=a"(eax), "=d"(edx)
                 : "X"(drawmode716b), "a"(*p1), "d"(*p2)
                 : "cc", "memory", "ecx", "ebx", "esi", "edi");
}

static void processmode716b(void)
{
    cwinenabm = winenabm;
    // get current sprite table
    currentobjptr = (SpriteInfo*)spritetablea + (curypos & 0x00FF) * 64;
    // setup priorities
    if (sprprifix != 0) {
        cursprloc = sprlefttot;
        asm_call(preparesprpr);
    }
    // calculate current video offset
    curvidoffset = vidbuffer + curypos * 576 + 32;
    // do sprite windowing
    makewindowsp();
    // set palette
    setpalette16b();
    // clear back area w/ back color
    clearback16b();

    extbgdone = 0;
    // mode 7 extbg
    if (interlval & 0x40 && !(scrndis & 0x02) && scrnon & 0x0202) { // do background 1
        winon = 0;
        if (!(winenabm & 0x01) || winenabs & 0x01 || (makewindow(winen[LAYER_BG1], LAYER_BG1 /* XXX not in original, but seems consistent, because winen[LAYER_BG1] is used */), winon != 0xFF)) {
            extbgdone = 1;
            procmode716bextbg(&bg1scroly_m7, &bg1scrolx_m7, 1);
        }
    }

    // do objects
    if (!(scrndis & 0x10) && scrnon & 0x1010 && winonsp != 0xFF) {
        u4 const ebx = curypos & 0x00FF;
        u1 const cl = sprprifix == 0 ? sprleftpr[ebx] : sprlefttot[ebx];
        if (cl != 0)
            drawsprites16b(cl, 0);
    }

    // display mode7
    if (!(interlval & 0x40) && !(scrndis & 0x01) && scrnon & 0x0101) { // do background 1
        winon = 0;
        if (!(winenabm & 0x01) || winenabs & 0x01 || (makewindow(winen[LAYER_BG1], LAYER_BG1 /* XXX not in original, but seems consistent, because winen[LAYER_BG1] is used */), winon != 0xFF)) {
            procmode716b(&bg1scroly_m7, &bg1scrolx_m7, 1);
        }
    }

    // do objects
    if (!(scrndis & 0x10) && scrnon & 0x1010 && winonsp != 0xFF) {
        u4 const ebx = curypos & 0x00FF;
        u1 const cl = sprprifix == 0 ? sprleftpr1[ebx] : sprlefttot[ebx];
        if (cl != 0)
            drawsprites16b(cl, 1);
    }

    if (interlval & 0x40 && !(scrndis & 0x01) && extbgdone == 0 && scrnon & 0x0101) { // do background 1
        winon = 0;
        if (!(winenabm & 0x02) || winenabs & 0x02 || (makewindow(winen[LAYER_BG1], LAYER_BG1 /* XXX not in original, but seems consistent, because winen[LAYER_BG1] is used */), winon != 0xFF)) {
            extbgdone = 1;
            procmode716bextbg(&bg1scroly_m7, &bg1scrolx_m7, 1);
        }
    }

    // mode 7 extbg
    if (interlval & 0x40 && extbgdone != 0 && !(scrndis & 0x01)) { // do background 1
        winon = 0;
        if (!(winenabm & 0x01) || winenabs & 0x01 || (makewindow(winen[LAYER_BG1], LAYER_BG1 /* XXX not in original, but seems consistent, because winen[LAYER_BG1] is used */), winon != 0xFF)) {
            procmode716bextbg2(1);
        }
    }

    // do objects
    if (!(scrndis & 0x10) && scrnon & 0x1010 && winonsp != 0xFF) {
        u4 const ebx = curypos & 0x00FF;
        u1 const cl = sprprifix == 0 ? sprleftpr2[ebx] : sprlefttot[ebx];
        if (cl != 0)
            drawsprites16b(cl, 2);
    }

    // do objects
    if (!(scrndis & 0x10) && scrnon & 0x1010 && winonsp != 0xFF) {
        u4 const ebx = curypos & 0x00FF;
        u1 const cl = sprprifix == 0 ? sprleftpr3[ebx] : sprlefttot[ebx];
        if (cl != 0)
            drawsprites16b(cl, 3);
    }
}

void drawline16b(void)
{
    cwinenabm = winenabs;

    bg3high2 = bgmode == 1 ? bg3highst : 0;
    if (curblank != 0)
        return;
    if (maxbr < vidbright)
        maxbr = vidbright;
    if (forceblnk != 0) {
        blanker16b();
        return;
    }
    alreadydrawn = 0;
    colormodeofs = colormodedef[bgmode];

    if (scrnon == 0x0117 && scaddset == 0x02 && scaddtype == 0x82)
        scrnon = 0x0116;

    if (scaddset & 0x02 || (scaddtype & 0x3F && (coladdr != 0 || coladdg != 0 || coladdb != 0 || colnull != 0))) {
        asm_call(drawline16t);
        return;
    }
    if (bgmode == 7) {
        processmode716b();
        return;
    }
    // calculate current video offset
    curvidoffset = vidbuffer + curypos * 576 + 32;
    // do sprite windowing
    makewindowsp();
    // set palette
    setpalette16b();
    // clear back area w/ back color
    clearback16b();
    // get current sprite table
    currentobjptr = (SpriteInfo*)spritetablea + (curypos & 0x00FF) * 64; // XXX ugly cast
    // setup priorities
    if (sprprifix != 0) {
        cursprloc = sprlefttot;
        asm_call(preparesprpr);
    } else {
        cursprloc = sprleftpr;
    }
    // process backgrounds
    // do background 2
    curbgnum = 0x02;
    procbackgrnd(0x01);
    // do background 1
    curbgnum = 0x01;
    procbackgrnd(0x00);
    // do background 4
    curbgnum = 0x08;
    procbackgrnd(0x03);
    // do background 3
    curbgnum = 0x04;
    procbackgrnd(0x02);

    if (bgmode > 1) {
        priority216b();
        return;
    }
    cwinenabm = winenabm;
    curbgpr = 0x00;
    // do background 4
    curbgnum = 0x08;
    drawbackgrndmain16b(LAYER_BG4);
    // do background 3
    curbgnum = 0x04;
    drawbackgrndmain16b(LAYER_BG3);
    procspritesmain16b(0);
    curbgpr = 0x20;
    // do background 4
    curbgnum = 0x08;
    drawbackgrndmain16b(LAYER_BG4);
    // do background 3
    if (bg3high2 != 1) {
        curbgnum = 0x04;
        drawbackgrndmain16b(LAYER_BG3);
    }
    procspritesmain16b(1);
    // do background 2
    curbgpr = 0x00;
    curbgnum = 0x02;
    drawbackgrndmain16b(LAYER_BG2);
    // do background 1
    curbgnum = 0x01;
    drawbackgrndmain16b(LAYER_BG1);
    procspritesmain16b(2);
    // do background 2
    curbgpr = 0x20;
    curbgnum = 0x02;
    drawbackgrndmain16b(LAYER_BG2);
    // do background 1
    curbgnum = 0x01;
    drawbackgrndmain16b(LAYER_BG1);
    procspritesmain16b(3);
    if (bg3high2 == 1) { // do background 3
        curbgpr = 0x20;
        curbgnum = 0x04;
        drawbackgrndmain16b(LAYER_BG3);
    }
}
