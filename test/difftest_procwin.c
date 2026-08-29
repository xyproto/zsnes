/* procwindowback16t (video/makev16t.asm) against video/c_procwin.c. It takes
 * no arguments and writes winon, numwin and the windowdata run list, so the
 * comparison covers those plus the whole of windowdata - a port that writes one
 * byte too few leaves the drawer on a stale run.
 *
 * The registers matter too: clearback16bts runs straight after and spills eax,
 * ebx, ecx and esi, so what this leaves in them is that one's input. edx and
 * edi are driven and checked to hold the port to touching neither, and every
 * register goes in with a non-zero upper half because the writes are partial.
 *
 * makedualwincol is external, so both sides call the same stub and the test
 * only checks the arguments and the call count. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;

/* The window registers the routine reads, and what it writes. windowdata is
   bigger than the six bytes ever written so an overrun shows up. */
#define WINDOWDATA_SZ 64u
u1 winon, numwin, windowdata[WINDOWDATA_SZ];
u1 wincolen, scaddset;
u1 winl1, winr1, winl2, winr2;

/* The shared stub: both sides reach the real symbol, so a divergence in the
   argument or the call count is visible. */
static u4 dual_hits;
static u4 dual_arg;
void makedualwincol(u1 const al)
{
    dual_hits++;
    dual_arg = al;
}

#include "../video/c_procwin.h"

void asm_procwindowback16t(void);

/* Driven into the registers on the way in, read back out afterwards. */
u4 in_ax, in_bx, in_cx, in_dx, in_si, in_di;
u4 out_ax, out_bx, out_cx, out_dx, out_si, out_di;

/* The original clobbers ebx and esi and does not save them - inside the
   pre-port mode 7 driver the enclosing routine had already pushed them, and
   the emulator reaches the label through calldl16t, which pushes them too. So
   the test has to as well, or the oracle corrupts its C caller. */
static void call_oracle(void)
{
    __asm__ volatile("pushl %%ebx\n\t"
                     "pushl %%esi\n\t"
                     "pushl %%edi\n\t"
                     "pushl %%ebp\n\t"
                     "movl in_ax, %%eax\n\t"
                     "movl in_bx, %%ebx\n\t"
                     "movl in_cx, %%ecx\n\t"
                     "movl in_dx, %%edx\n\t"
                     "movl in_si, %%esi\n\t"
                     "movl in_di, %%edi\n\t"
                     "call asm_procwindowback16t\n\t"
                     "movl %%eax, out_ax\n\t"
                     "movl %%ebx, out_bx\n\t"
                     "movl %%ecx, out_cx\n\t"
                     "movl %%edx, out_dx\n\t"
                     "movl %%esi, out_si\n\t"
                     "movl %%edi, out_di\n\t"
                     "popl %%ebp\n\t"
                     "popl %%edi\n\t"
                     "popl %%esi\n\t"
                     "popl %%ebx\n\t" ::
                         : "eax", "ecx", "edx", "cc", "memory");
}

static void call_port(void)
{
    pwregs r = { in_ax, in_bx, in_cx, in_si };

    c_procwindowback16t(&r);
    out_ax = r.ax;
    out_bx = r.bx;
    out_cx = r.cx;
    out_si = r.si;
    out_dx = in_dx; /* the port must not have anywhere to touch these */
    out_di = in_di;
}

typedef struct {
    u1 winon_, numwin_;
    u1 data[WINDOWDATA_SZ];
    u4 hits, arg;
    u4 ax, bx, cx, dx, si, di;
} snapshot;

static void run(void (*fn)(void), u1 const* init, snapshot* out)
{
    memcpy(windowdata, init, WINDOWDATA_SZ);
    winon = 0xAA; /* neither side's answer, so "wrote nothing" fails */
    numwin = 0xAA;
    dual_hits = dual_arg = 0;

    fn();

    out->winon_ = winon;
    out->numwin_ = numwin;
    memcpy(out->data, windowdata, WINDOWDATA_SZ);
    out->hits = dual_hits;
    out->arg = dual_arg;
    out->ax = out_ax;
    out->bx = out_bx;
    out->cx = out_cx;
    out->dx = out_dx;
    out->si = out_si;
    out->di = out_di;
}

int main(void)
{
    u1 init[WINDOWDATA_SZ];

    DT_MAIN(4242, 200000)
    {
        snapshot a, c;

        dt_fill(init, sizeof init);
        /* Uniform bytes reach the interesting branches rarely: the dual-window
           path needs two particular bits, the clear and off paths need two
           particular values of scaddset's high nibble, and the "covers the
           whole line" case needs the edges at their extremes. */
        wincolen = (u1)(dt_mod(2) ? (dt_u32() & 0x0Fu) : dt_u32());
        scaddset = (u1)(dt_mod(2) ? (dt_mod(4) << 4) : dt_u32());
        winl1 = (u1)(dt_mod(3) == 0 ? dt_mod(3) : dt_u32());
        winr1 = (u1)(dt_mod(3) == 0 ? 253u + dt_mod(3) : dt_u32());
        winl2 = (u1)(dt_mod(3) == 0 ? dt_mod(3) : dt_u32());
        winr2 = (u1)(dt_mod(3) == 0 ? 253u + dt_mod(3) : dt_u32());

        /* Non-zero upper halves, so a port that writes a whole register where
           the assembly wrote a byte or a word shows up. */
        in_ax = dt_u32();
        in_bx = dt_u32();
        in_cx = dt_u32();
        in_dx = dt_u32();
        in_si = dt_u32();
        in_di = dt_u32();

        run(call_oracle, init, &a);
        run(call_port, init, &c);

        DT_EQ("winon", a.winon_, c.winon_);
        DT_EQ("numwin", a.numwin_, c.numwin_);
        DT_MEM("windowdata", a.data, c.data, WINDOWDATA_SZ);
        DT_EQ("makedualwincol calls", a.hits, c.hits);
        DT_EQ("makedualwincol arg", a.arg, c.arg);
        DT_EQ("eax", a.ax, c.ax);
        DT_EQ("ebx", a.bx, c.bx);
        DT_EQ("ecx", a.cx, c.cx);
        DT_EQ("edx", a.dx, c.dx);
        DT_EQ("esi", a.si, c.si);
        DT_EQ("edi", a.di, c.di);
    }

    if (dt_fails) {
        printf("procwindowback: FAIL (%d/%ld iterations mismatched)\n",
            dt_fails, dt_iters);
        return 1;
    }
    printf("procwindowback: PASS (%ld iterations bit-identical to asm)\n",
        dt_iters);
    return 0;
}
