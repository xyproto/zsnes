/* Differential test: the clearback16b* cluster in video/makev16t.asm against
 * the C port in video/c_mv16bclr.c.
 *
 * Nine assembly routines with one entry point, so one comparison covers all of
 * them. Both sides are real assembly - the oracle from the pre-port revision
 * and the current file from the working tree - so the seam thunk is tested too.
 *
 * Besides the line buffer and the registers, the cluster leaves state behind:
 * numwin is counted down to zero, DoTransp says whether the line ended up
 * fully transparent, and prevrgbcol/prevrgbpal are the colour cache. All four
 * are reseeded before each run and compared after.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;

extern u1 DoTransp, winon, scaddset, bgmode, numwin, vidbright;
extern u1 coladdr, coladdg, coladdb, colnull;
extern u1 windowdata[];
extern u2 scrnon, prevrgbpal;
extern u4 prevrgbcol, vesa2_rpos, vesa2_gpos, vesa2_bpos;
extern u1 *curvidoffset, *cwinptr;

#define VIDSZ 1024u
#define MASKSZ 256u
static u1 vidbuf[VIDSZ];
static u1 maskbuf[MASKSZ];

void asm_clearback16bts(void);

/* The port's side. Its trampoline is gone - video/c_mv16tline.c spills into
   the CLB seam at the call site now - so the test drives the C body through
   that seam while the oracle still goes through dt_call with registers. */
extern u4 CLBAX, CLBBX, CLBCX, CLBDX, CLBSI, CLBDI;
void c_clearback16bts(void);

u4 rg_eax, rg_ebx, rg_ecx, rg_edx, rg_esi, rg_edi, rg_ebp, rg_fn;
__asm__(".pushsection .text\n"
        ".globl dt_call\n"
        "dt_call:\n"
        "  pushl %ebx\n  pushl %esi\n  pushl %edi\n  pushl %ebp\n"
        "  movl rg_eax, %eax\n"
        "  movl rg_ebx, %ebx\n"
        "  movl rg_ecx, %ecx\n"
        "  movl rg_edx, %edx\n"
        "  movl rg_esi, %esi\n"
        "  movl rg_edi, %edi\n"
        "  movl rg_ebp, %ebp\n"
        "  call *rg_fn\n"
        "  movl %eax, rg_eax\n"
        "  movl %ebx, rg_ebx\n"
        "  movl %ecx, rg_ecx\n"
        "  movl %edx, rg_edx\n"
        "  movl %esi, rg_esi\n"
        "  movl %edi, rg_edi\n"
        "  movl %ebp, rg_ebp\n"
        "  popl %ebp\n  popl %edi\n  popl %esi\n  popl %ebx\n"
        "  ret\n"
        ".popsection\n");
void dt_call(void);

typedef struct {
    u4 ax, bx, cx, dx, si, di, bp;
    u4 prevcol;
    u2 prevpal;
    u1 transp, nwin;
    u1 vid[VIDSZ];
} snapshot;

typedef struct {
    u4 reg[7];
    u4 prevcol;
    u2 prevpal;
    u1 transp, nwin;
    u1 vid[VIDSZ];
} seed;

/* Same shape as run(), but through the seam the C body reads. It never took
   ebp, so that one is passed straight through. */
static void run_port(seed const* const s, snapshot* const out)
{
    memcpy(vidbuf, s->vid, VIDSZ);
    prevrgbcol = s->prevcol;
    prevrgbpal = s->prevpal;
    DoTransp = s->transp;
    numwin = s->nwin;
    CLBAX = s->reg[0];
    CLBBX = s->reg[1];
    CLBCX = s->reg[2];
    CLBDX = s->reg[3];
    CLBSI = s->reg[4];
    CLBDI = s->reg[5];

    c_clearback16bts();

    out->ax = CLBAX;
    out->bx = CLBBX;
    out->cx = CLBCX;
    out->dx = CLBDX;
    out->si = CLBSI;
    out->di = CLBDI;
    out->bp = s->reg[6];
    out->prevcol = prevrgbcol;
    out->prevpal = prevrgbpal;
    out->transp = DoTransp;
    out->nwin = numwin;
    memcpy(out->vid, vidbuf, VIDSZ);
}

static void run(void (*fn)(void), seed const* const s, snapshot* const out)
{
    memcpy(vidbuf, s->vid, VIDSZ);
    prevrgbcol = s->prevcol;
    prevrgbpal = s->prevpal;
    DoTransp = s->transp;
    numwin = s->nwin;
    rg_eax = s->reg[0];
    rg_ebx = s->reg[1];
    rg_ecx = s->reg[2];
    rg_edx = s->reg[3];
    rg_esi = s->reg[4];
    rg_edi = s->reg[5];
    rg_ebp = s->reg[6];
    rg_fn = (u4)(uintptr_t)fn;
    dt_call();

    out->ax = rg_eax;
    out->bx = rg_ebx;
    out->cx = rg_ecx;
    out->dx = rg_edx;
    out->si = rg_esi;
    out->di = rg_edi;
    out->bp = rg_ebp;
    out->prevcol = prevrgbcol;
    out->prevpal = prevrgbpal;
    out->transp = DoTransp;
    out->nwin = numwin;
    memcpy(out->vid, vidbuf, VIDSZ);
}

int main(void)
{
    long cov[8];
    memset(cov, 0, sizeof cov);

    DT_MAIN(20260805, 60000)
    {
        snapshot x, y;
        seed s;
        u4 key;

        /* An 8-bit divide by 15 sits in the colour path, so the quotient has
           to fit a byte or the assembly traps. The PPU can only produce
           coladd* of 5 bits and vidbright of 4, which always does. */
        coladdr = (u1)dt_mod(32);
        coladdg = (u1)dt_mod(32);
        coladdb = (u1)dt_mod(32);
        colnull = (u1)dt_u32(); /* part of the dword read, shifted back out */
        vidbright = (u1)dt_mod(16);
        /* Mostly the real 5-6-5 positions; the rest stresses the shift count,
           which the hardware masks to five bits. */
        if (dt_mod(4)) {
            vesa2_rpos = 11;
            vesa2_gpos = 5;
            vesa2_bpos = 0;
        } else {
            vesa2_rpos = dt_mod(34);
            vesa2_gpos = dt_mod(34);
            vesa2_bpos = dt_mod(34);
        }
        if (dt_mod(8) == 0) {
            vesa2_rpos = 0; /* the one value that skips the colour entirely */
        }

        winon = (u1)(dt_mod(8) ? dt_mod(6) : dt_u32());
        scaddset = (u1)(dt_mod(4) ? dt_mod(4) << 4 : dt_u32());
        scrnon = (u2)(dt_mod(2) ? dt_mod(2) << 12 : dt_u32());
        bgmode = (u1)(dt_mod(2) ? 7 : dt_mod(8));

        key = (u4)coladdr << 8 | (u4)coladdg << 16 | (u4)coladdb << 24
            | vidbright;
        /* Half the time force a cache hit: it changes what is left in the top
           half of eax and so which side of the zero test the colour lands on. */
        s.prevcol = dt_mod(2) ? key : dt_u32();
        s.prevpal = (u2)(dt_mod(3) == 0 ? 0 : dt_u32());
        s.transp = (u1)dt_mod(2);
        s.nwin = (u1)dt_mod(10);

        /* Column/depth pairs. Sorted columns are the shape the window builder
           produces; random ones exercise the wrapping counters. */
        if (dt_mod(2)) {
            u1 col = 0;
            for (u4 i = 0; i < 16; i += 2) {
                col = (u1)(col + dt_mod(70));
                windowdata[i] = col;
                windowdata[i + 1] = (u1)(dt_mod(2) ? 1 : 0xFF);
            }
        } else {
            dt_fill(windowdata, 16);
        }

        /* The dual mask is one byte per pixel. Independent random bytes never
           produce a line that is uniformly masked, and that is the only case
           that separates the three loop heads from each other or shows whether
           DoTransp was preset - so generate runs, and sometimes one run. */
        if (dt_mod(4) == 0) {
            memset(maskbuf, (u1)dt_mod(2), MASKSZ);
        } else if (dt_mod(2)) {
            u1 v = (u1)dt_mod(2);
            u4 i = 0;

            while (i < MASKSZ) {
                u4 n = dt_mod(80) + 1;

                while (n-- != 0 && i < MASKSZ) {
                    maskbuf[i++] = v;
                }
                v ^= 1;
            }
        } else {
            for (u4 i = 0; i < MASKSZ; i++) {
                maskbuf[i] = (u1)(dt_mod(16) ? dt_mod(2) : dt_u32());
            }
        }
        cwinptr = maskbuf;

        dt_fill(s.vid, VIDSZ);
        curvidoffset = vidbuf + 128;
        for (u4 i = 0; i < 7; i++) {
            s.reg[i] = dt_u32();
        }

        run(asm_clearback16bts, &s, &x);
        run_port(&s, &y);

        cov[winon < 8 ? winon : 7]++;

        DT_EQ("eax", x.ax, y.ax);
        DT_EQ("ebx", x.bx, y.bx);
        DT_EQ("ecx", x.cx, y.cx);
        DT_EQ("edx", x.dx, y.dx);
        DT_EQ("esi", x.si, y.si);
        DT_EQ("edi", x.di, y.di);
        DT_EQ("ebp", x.bp, y.bp);
        DT_EQ("prevrgbcol", x.prevcol, y.prevcol);
        DT_EQ("prevrgbpal", x.prevpal, y.prevpal);
        DT_EQ("DoTransp", x.transp, y.transp);
        DT_EQ("numwin", x.nwin, y.nwin);
        DT_MEM("video buffer", x.vid, y.vid, VIDSZ);
        if (dt_bad && DT_SHOW()) {
            printf("  ^ winon=%u scaddset=%02x scrnon=%04x bgmode=%u "
                   "rpos=%u numwin=%u hit=%d\n",
                winon, scaddset, scrnon, bgmode, vesa2_rpos, s.nwin,
                s.prevcol == key);
        }
    }
    printf("  winon 0=%ld 1=%ld 2=%ld 3=%ld 4=%ld 5=%ld 6=%ld 7+=%ld\n",
        cov[0], cov[1], cov[2], cov[3], cov[4], cov[5], cov[6], cov[7]);
    DT_DONE("makev16t clearback16b* cluster");
}
