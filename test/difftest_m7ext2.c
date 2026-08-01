/* Differential test: drawmode7ngextbg216b in video/mode716.asm against the C
 * port in video/c_mode716ext2.c.
 *
 * The routine is a 256-pixel loop over a priority plane one buffer on, so the
 * test compares the two 512-byte windows it can write (main and sub screen),
 * the plane itself, and the registers the caller gets back - the assembly
 * leaves eax, ecx, esi and the low half of edx behind, and a thunk that forgets
 * one of them is exactly the mistake this has to catch.
 *
 * The oracle (_m7ext2.o, built by mkm7ext2.sh from the pre-port revision) is
 * driven through asm_m7ext2, which sets the registers up from the same seam
 * block the ported side reads. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;

#define VBUF 75036u
#define SUB_OFF (VBUF * 2u)
#define PRIO_OFF (VBUF * 8u)
#define WINDOW 512u
#define VSZ (PRIO_OFF + WINDOW)

u1 scrndis;
u1 mode7hr[256];
u1 BGMS1[2048];
u1 FillSubScr[256];
u1 scadtng[256];
u4 UnusedBitXor[2];
u1* curvidoffset;

/* The seam block lives in video/c_mode716calc.c, which this test does not
   link; both sides reach it through these. */
u4 M7SeamA, M7SeamB, M7SeamC, M7SeamD, M7SeamSI, M7SeamBP;

void asm_m7ext2(void); /* _m7ext2.o */
void c_drawmode7ngextbg216b(void);

/* Both sides render into the same buffer - esi comes back as a raw pointer, so
   they have to be handed the same base. Only the three 512-byte windows the
   routine can touch are refreshed between runs, not the whole 600K. */
static u1 vbuf[VSZ];
static u2 pal[512];

typedef struct {
    u4 a, b, c, d, si, bp;
    u1 main[WINDOW], sub[WINDOW], prio[WINDOW];
} snapshot;

static void run(int const asm_side, u4 const bx, u4 const d, u1 const* const prio,
    u1 const* const painted, snapshot* const out)
{
    u1* const v = vbuf;

    memcpy(v + PRIO_OFF, prio, WINDOW);
    memcpy(v, painted, WINDOW);
    memcpy(v + SUB_OFF, painted, WINDOW);

    curvidoffset = v;
    M7SeamA = 0xAA000000u | bx;
    M7SeamB = bx;
    M7SeamC = 0xCC000000u | bx;
    M7SeamD = d;
    /* esi is not an input - every path reloads it - but a thunk that fails to
       write it back is still visible, so give it a distinctive value. */
    M7SeamSI = 0x51000000u | bx;
    M7SeamBP = (u4)(uintptr_t)pal;

    if (asm_side) {
        asm_m7ext2();
    } else {
        c_drawmode7ngextbg216b();
    }

    out->a = M7SeamA;
    out->b = M7SeamB;
    out->c = M7SeamC;
    out->d = M7SeamD;
    out->si = M7SeamSI;
    out->bp = M7SeamBP;
    memcpy(out->main, v, WINDOW);
    memcpy(out->sub, v + SUB_OFF, WINDOW);
    memcpy(out->prio, v + PRIO_OFF, WINDOW);
}

int main(void)
{
    DT_MAIN(20260801, 200000)
    {
        u4 const bx = dt_mod(256);
        u4 const d = dt_u32();
        u1 seed[WINDOW];

        scrndis = (u1)(dt_mod(16) == 0 ? 1 : 0); /* the early-out, occasionally */
        UnusedBitXor[0] = dt_u32();
        UnusedBitXor[1] = dt_u32();
        for (u4 i = 0; i < 512; i++) {
            pal[i] = (u2)dt_u32();
        }
        /* Only these three bytes steer the dispatch; drive each independently
           so all five pixel writers come up. */
        BGMS1[bx * 2] = (u1)dt_u32();
        BGMS1[bx * 2 + 1] = (u1)dt_u32();
        FillSubScr[bx] = (u1)dt_u32();
        scadtng[bx] = (u1)dt_u32();
        mode7hr[bx] = (u1)dt_mod(2);

        /* The priority plane is read one byte per pixel at stride 2. Bit 7
           gates the write, so make it a coin flip rather than 1-in-256. */
        for (u4 i = 0; i < WINDOW; i++) {
            seed[i] = (u1)dt_u32();
        }
        for (u4 i = 0; i < 256; i++) {
            seed[i * 2] = (u1)((seed[i * 2] & 0x7Fu) | (dt_mod(2) ? 0x80u : 0u));
        }
        u1 painted[WINDOW];
        dt_fill(painted, WINDOW);

        snapshot x, y;
        run(1, bx, d, seed, painted, &x);
        run(0, bx, d, seed, painted, &y);

        DT_EQ("eax", x.a, y.a);
        DT_EQ("ebx", x.b, y.b);
        DT_EQ("ecx", x.c, y.c);
        DT_EQ("edx", x.d, y.d);
        DT_EQ("esi", x.si, y.si);
        DT_EQ("ebp", x.bp, y.bp);
        DT_MEM("main screen", x.main, y.main, WINDOW);
        DT_MEM("sub screen", x.sub, y.sub, WINDOW);
        DT_MEM("priority plane", x.prio, y.prio, WINDOW);
        if (dt_bad && DT_SHOW()) {
            printf("  ^ bx=%x edx=%x scrndis=%u BGMS1=%02x%02x fill=%02x scad=%02x\n",
                bx, d, scrndis, BGMS1[bx * 2 + 1], BGMS1[bx * 2],
                FillSubScr[bx], scadtng[bx]);
        }
    }
    DT_DONE("mode 7 EXTBG second pass");
}
