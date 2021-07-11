#include "c_newgfx16.h"
#include "../cpu/regs.h"
#include "../endmem.h"
#include "../ui.h"
#include "../vcache.h"
#include "makev16b.h"
#include "newgfx.h"
#include "newgfx16.h"
#include "procvid.h"

static u2 prevpal2[] = {
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
    0xF00F,
};

static void setpalallng(void)
{
    palchanged = 1;
    cgmod = 0;
    cpalptrng = (cpalptrng + 1024) & (255 * 1024);
    u2* pal = (u2*)(vbufdptr + cpalptrng);
    u4 i = 0;
    do {
        u2 const dx = cgram[i];
        prevpal2[i] = dx;

        u2 c = 0;

        u4 r = (dx & 0x1F) + gammalevel16b;
        if (r > 31)
            r = 31;
        c += r * vidbright / 15 << ngrposng;

        u4 g = (dx >> 5 & 0x1F) + gammalevel16b;
        if (g > 31)
            g = 31;
        c += g * vidbright / 15 << nggposng;

        u4 b = (dx >> 10 & 0x1F) + gammalevel16b;
        if (b > 31)
            b = 31;
        c += b * vidbright / 15 << ngbposng;

        pal[0] = c; // standard
        pal[256] = c | UnusedBit[0]; // standard
    } while (++pal, ++i != 256);
    prevbright = vidbright;
}

void setpalette16bng(void)
{
    if (V8Mode == 1)
        doveg();
    if (vidbright != prevbright) {
        setpalallng();
    } else if (cgmod != 0) {
        cgmod = 0;
        u2* edi = (u2*)(vbufdptr + cpalptrng);
        cpalptrng = (cpalptrng + 1024) & (255 * 1024);
        u2* pal = (u2*)(vbufdptr + cpalptrng);
        u4 i = 0;
        do {
            u2 const dx = cgram[i];
            if (prevpal2[i] == dx) {
                pal[0] = edi[0];
                pal[256] = edi[256];
            } else {
                prevpal2[i] = dx;

                if (i != 0)
                    palchanged = 1;

                u2 c = 0;

                u2 r = (dx & 0x1F) + gammalevel16b;
                if (r > 31)
                    r = 31;
                c += r * vidbright / 15 << ngrposng;

                u2 g = (dx >> 5 & 0x1F) + gammalevel16b;
                if (g > 31)
                    g = 31;
                c += g * vidbright / 15 << nggposng;

                u2 b = (dx >> 10 & 0x1F) + gammalevel16b;
                if (b > 31)
                    b = 31;
                c += b * vidbright / 15 << ngbposng;

                pal[0] = c; // standard
                pal[256] = c | UnusedBit[0]; // standard
            }
        } while (++edi, ++pal, ++i != 256);
    }
    if (V8Mode == 1)
        dovegrest();
}

void Gendcolortable(void)
{
    // Generate Direct Color Table
    u4 ecx = 0;
    do {
        u4 const edx = ((ecx & 0x07) >> 0) * vidbright / 15 << 13 | ((ecx & 0x38) >> 3) * vidbright / 15 << 8 | ((ecx & 0xC0) >> 6) * vidbright / 15 << 3;
        dcolortab[0][ecx] = edx;
        dcolortab[1][ecx] = edx | UnusedBit[0];
    } while (++ecx != 256);
}

void BackAreaFill(u4 const eax)
{
    u1* buf = vidbuffer + 16 * 2 + eax * 576 + BackAreaAdd;

    if (winbgbackenval[eax] != 0 && BackAreaFillCol != BackAreaUnFillCol) { // Construct Window in buf
        buf -= 2;
        u4* edi = ngwintable;
        u4 eax = 256;
        for (;;) {
            {
                u4 edx = *edi++;
                if (edx != 0) {
                    --edx;
                    u4 const ebx = BackAreaUnFillCol;
                    for (;;) {
                        *(u4*)buf = ebx;
                        *(u4*)(buf + 4) = ebx;
                        buf += 8;
                        if (eax < 4)
                            return;
                        eax -= 4;
                        if (edx < 4)
                            break;
                        edx -= 4;
                    }
                    edx -= 4;
                    eax -= edx + 1;
                    buf += edx * 2 + 2;
                }
            }

            {
                u4 edx = *edi++ - 1;
                u4 const ebx = BackAreaFillCol;
                for (;;) {
                    *(u4*)buf = ebx;
                    *(u4*)(buf + 4) = ebx;
                    buf += 8;
                    if (eax < 4)
                        return;
                    eax -= 4;
                    if (edx < 4)
                        break;
                    edx -= 4;
                }
                edx -= 4;
                eax -= edx + 1;
                buf += edx * 2 + 2;
            }
        }
    } else {
        u4 const ebx = BackAreaUnFillCol;
        u4 eax = 64;
        do {
            *(u4*)buf = ebx;
            *(u4*)(buf + 4) = ebx;
            buf += 8;
        } while (--eax != 0);
    }
}
