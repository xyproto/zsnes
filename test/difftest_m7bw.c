/* ProcessBuildWindow (video/mode716.mac) against video/c_mode716bw.c - the one
 * port here that is deliberately *not* bit-identical. The assembly reached
 * BuildWindow by the register ABI that routine had before it became C, so its
 * push eax/push ebx were picked up as cdecl arguments and it got the caller's
 * eax where the scanline belonged; the port passes what was meant.
 *
 * So BuildWindow is stubbed and its arguments recorded: everything else must
 * match exactly, and that one argument must differ in exactly the documented
 * way. Either side changing fails this.
 *
 * Only the body moved to C; clearing ngwinen and the enable test stayed in the
 * macro, so the oracle is the whole macro and the C side is driven behind the
 * same gate. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;

u4 ngwinen, nglogicval;
u4 ngwintable[64];
u4* ngcwinptr;
u1 winlogicaval[1024];
static u1 winenable[1024]; /* what esi points at */

/* The stub. Both sides call it, so the arguments it saw are directly
   comparable. */
static u4 bw_hits, bw_a1, bw_a2, bw_logic, bw_fill0, bw_fill1;
void BuildWindow(u4 eax, u4 ebx)
{
    bw_hits++;
    bw_a1 = eax;
    bw_a2 = ebx;
    bw_logic = nglogicval;
    /* The table it leaves behind has to be independent of the arguments, or
       the one deliberate argument difference would cascade into every field
       the test compares. The arguments are compared directly instead. */
    ngwintable[0] = bw_fill0;
    ngwintable[1] = bw_fill1;
}

extern u4 M7BWBX; /* video/c_mode716bw.c */
extern u4 M7BWSI; /* _m7bw.o: what the macro loads into esi */

void asm_m7bw(void); /* _m7bw.o */
void c_ProcessMode7BuildWindow(void);

typedef struct {
    u4 winen, logic, tab0, tab1, cursor, hits, a1, a2, seen_logic;
    u4 bx, si;
} snapshot;

static void run(int const asm_side, u4 const ax, u4 const bx, snapshot* const out)
{
    ngwinen = 0xEEEEEEEEu;
    nglogicval = 0xAB00CD00u; /* only the low byte should ever change */
    ngwintable[0] = 0x11111111u;
    ngwintable[1] = 0x22222222u;
    ngcwinptr = 0;
    bw_hits = bw_a1 = bw_a2 = bw_logic = 0;

    M7BWBX = bx;
    M7BWSI = (u4)(uintptr_t)winenable;

    if (asm_side) {
        /* asm_m7bw also needs eax, which is exactly what leaks into the call. */
        extern u4 M7BWAX;
        M7BWAX = ax;
        asm_m7bw();
    } else {
        /* Stand in for the gate the macro still holds. */
        ngwinen = 0;
        if (winenable[bx] & 0x0Au) {
            c_ProcessMode7BuildWindow();
        }
    }

    out->winen = ngwinen;
    out->logic = nglogicval;
    out->tab0 = ngwintable[0];
    out->tab1 = ngwintable[1];
    out->cursor = ngcwinptr == ngwintable ? 1u : 0u;
    out->hits = bw_hits;
    out->a1 = bw_a1;
    out->a2 = bw_a2;
    out->seen_logic = bw_logic;
    out->bx = M7BWBX;
    out->si = M7BWSI;
}

int main(void)
{
    DT_MAIN(20260802, 200000)
    {
        u4 const bx = dt_mod(256);
        u4 const ax = dt_u32();
        snapshot x, y;

        /* Zero in slot 0 sends the decrement to slot 1; cover both. */
        bw_fill0 = dt_mod(2) ? 0u : dt_u32();
        bw_fill1 = dt_u32();

        for (u4 i = 0; i < 1024; i++) {
            winlogicaval[i] = (u1)dt_u32();
            /* Either of bits 1 and 3 gates the whole routine, so cover each
               on its own - with only one of them ever set, a mask that lost
               the other would go unnoticed. */
            u4 const g = dt_mod(3);
            winenable[i] = (u1)(g == 0 ? (dt_u32() & ~0x0Au)
                                       : ((dt_u32() & ~0x0Au) | (g == 1 ? 0x02u : 0x08u)));
        }

        run(1, ax, bx, &x);
        run(0, ax, bx, &y);

        DT_EQ("ngwinen", x.winen, y.winen);
        DT_EQ("nglogicval", x.logic, y.logic);
        DT_EQ("ngwintable[0]", x.tab0, y.tab0);
        DT_EQ("ngwintable[1]", x.tab1, y.tab1);
        DT_EQ("ngcwinptr", x.cursor, y.cursor);
        DT_EQ("BuildWindow calls", x.hits, y.hits);
        DT_EQ("BuildWindow arg1", x.a1, y.a1);
        DT_EQ("nglogicval at call", x.seen_logic, y.seen_logic);
        DT_EQ("ebx preserved", x.bx, y.bx);
        DT_EQ("esi preserved", x.si, y.si);

        /* The deliberate difference, pinned from both ends. */
        if (x.hits != 0) {
            DT_EQ("asm passed the stale eax", x.a2, ax);
            DT_EQ("the port passes the scanline", y.a2, bx);
        }

        if (dt_bad && DT_SHOW()) {
            printf("  ^ bx=%x eax=%x enable=%02x logic=%02x\n", bx, ax,
                winenable[bx], winlogicaval[bx * 2]);
        }
    }
    DT_DONE("mode 7 window builder");
}
