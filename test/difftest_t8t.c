/* Differential test: the six 8x8 colour-maths tile drawers in
 * video/makev16t.asm against the C port in video/c_mv16t8t.c.
 *
 * draw8x816t, draw8x8fulladd, draw8x816ts and their three winon twins sit
 * behind one entry point, so one comparison covers all six. The oracle is the
 * pre-port assembly; the port is video/c_mv16draw.c, whose dispatcher is C and
 * whose leaf still goes through the TT seam.
 *
 * Three exits leave the routine other than by returning, and all three are
 * caught here rather than stubbed blind: the bgmode 2 jump into
 * draw8x816toffset and the bgmode 5 jump into draw16x816t (rewritten in the
 * oracle by mkoracle's --stub-routine, since a call inside the file carries no
 * relocation; substituted for the port by linking c_mv16draw.c without
 * c_mv16leaf.c, which is why those two live in a file of their own), and the
 * mosaic tail-jump into domosaic16b. Each records the register state it was
 * reached with, because that contract is exactly what a seam can get wrong.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;

extern u1 temp, bshifter, curbgpr, bgcoloradder, curmosaicsz, winon, bgmode;
extern u1 osm2dis, scaddtype, coadder16;
extern u2 scrnon;
extern u4 pal16bcl[256], pal16bxcl[256];

/* Not in any of the emulator objects the oracle links, and a blanket stub
   would be far too small: the dword load reads one entry past the end. */
u2 fulladdtab[65537];
extern u1 drawn, tileleft16b;
extern u4 yadder, yrevadder, bgsubby;
extern u1 *tempcach, *bgofwptr, *cwinptr, *winptrref, *curvidoffset;
extern u2* temptile;
extern u4 pal16b[256];
extern u1 transpbuf[];
extern u1 xtravbuf[576];

/* Pointers, not arrays: a blanket stub would leave them null. */
u1 *vcache2b, *vcache4b, *vcache8b;

#define VIDSZ 2048u
#define TBSZ 1168u
#define WINSZ 1024u
#define MAPSZ 1024u
#define C2SZ (262144u + 256u)
#define C4SZ (131072u + 256u)
#define C8SZ (65536u + 256u)

static u1 vidbuf[VIDSZ];
static u1 winbuf[WINSZ];
static u1 mapbuf[MAPSZ];
static u1 cache2[C2SZ], cache4[C4SZ], cache8[C8SZ];

#include "../video/c_mv16draw.h"

void asm_draw8x816t(void);

/* The three ways out. `hit` counts them so a route that stops being taken is
   visible, and the registers are compared like any other output. */
static struct exits {
    u4 hit[3];
    u4 reg[3][7];
} ex;

u4 xr_eax, xr_ebx, xr_ecx, xr_edx, xr_esi, xr_edi, xr_ebp;
__asm__(".pushsection .text\n"
        ".globl domosaic16b\n"
        ".globl draw16x816t_stub\n"
        ".globl draw8x816toffset_stub\n"
        "domosaic16b:\n"
        "draw16x816t_stub:\n"
        "draw8x816toffset_stub:\n"
        "  movl %eax, xr_eax\n"
        "  movl %ebx, xr_ebx\n"
        "  movl %ecx, xr_ecx\n"
        "  movl %edx, xr_edx\n"
        "  movl %esi, xr_esi\n"
        "  movl %edi, xr_edi\n"
        "  movl %ebp, xr_ebp\n"
        "  call xr_note\n"
        "  ret\n"
        ".popsection\n");
void domosaic16b(void);
void draw16x816t_stub(void);
void draw8x816toffset_stub(void);

/* Which of the two shared-body exits ran is decided by the caller's state, not
   by the entry point, so the note is taken where that is still known. */
static int xr_which;
void xr_note(void);
void xr_note(void)
{
    ex.hit[xr_which]++;
    ex.reg[xr_which][0] = xr_eax;
    ex.reg[xr_which][1] = xr_ebx;
    ex.reg[xr_which][2] = xr_ecx;
    ex.reg[xr_which][3] = xr_edx;
    ex.reg[xr_which][4] = xr_esi;
    ex.reg[xr_which][5] = xr_edi;
    ex.reg[xr_which][6] = xr_ebp;
}

static void call_regs(m7regs* r, void (*fn)(void));

/* The port's stand-ins for the two routines c_mv16leaf.c defines. Both were
   reached by a jump, so the registers at the call are what the stub sees and
   what the dispatcher returns with - hence the round trip rather than just
   bumping a counter. */
u4 draw16x816t(m7regs* const r)
{
    call_regs(r, draw16x816t_stub);
    return 0;
}

u4 draw8x816toffset(m7regs* const r)
{
    call_regs(r, draw8x816toffset_stub);
    return 0;
}

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

static void call_regs(m7regs* const r, void (*const fn)(void))
{
    rg_eax = r->ax;
    rg_ebx = r->bx;
    rg_ecx = r->cx;
    rg_edx = r->dx;
    rg_esi = r->si;
    rg_edi = r->di;
    rg_ebp = r->bp;
    rg_fn = (u4)(uintptr_t)fn;
    dt_call();
    r->ax = rg_eax;
    r->bx = rg_ebx;
    r->cx = rg_ecx;
    r->dx = rg_edx;
    r->si = rg_esi;
    r->di = rg_edi;
    r->bp = rg_ebp;
}

typedef struct {
    u4 reg[7];
    u4 glob[8];
    u1 mem[3];
    struct exits ex;
    u1 vid[VIDSZ];
    u1 tb[TBSZ];
    u1 xv[576];
} snapshot;

static void setup(snapshot const* const in)
{
    memcpy(vidbuf, in->vid, VIDSZ);
    memcpy(transpbuf, in->tb, TBSZ);
    memcpy(xtravbuf, in->xv, 576);
    /* The routine writes all four of these; poison them first, or a port that
       fails to write one still agrees because the previous run left the right
       value behind. */
    temp = bshifter = coadder16 = 0xA5;
    curvidoffset = vidbuf + 64;
    cwinptr = winbuf + 64;
    memset(&ex, 0, sizeof ex);
    if (bgmode == 5) {
        xr_which = 1;
    } else if (osm2dis != 1 && bgmode == 2) {
        xr_which = 2;
    } else {
        xr_which = 0;
    }
}

static void finish(snapshot* const out)
{
    out->glob[0] = yadder;
    out->glob[1] = yrevadder;
    out->glob[2] = bgsubby;
    out->glob[3] = (u4)(uintptr_t)tempcach;
    out->glob[4] = (u4)(uintptr_t)bgofwptr;
    out->glob[5] = (u4)(uintptr_t)winptrref;
    out->glob[6] = (u4)(uintptr_t)temptile;
    out->glob[7] = coadder16;
    out->mem[0] = temp;
    out->mem[1] = bshifter;
    out->mem[2] = drawn;
    out->ex = ex;
    memcpy(out->vid, vidbuf, VIDSZ);
    memcpy(out->tb, transpbuf, TBSZ);
    memcpy(out->xv, xtravbuf, 576);
}

static void run_asm(snapshot const* const in, snapshot* const out)
{
    m7regs r;

    setup(in);
    r.ax = in->reg[0];
    r.bx = in->reg[1];
    r.cx = in->reg[2];
    r.dx = in->reg[3];
    r.si = in->reg[4];
    r.di = in->reg[5];
    r.bp = in->reg[6];
    call_regs(&r, asm_draw8x816t);
    out->reg[0] = r.ax;
    out->reg[1] = r.bx;
    out->reg[2] = r.cx;
    out->reg[3] = r.dx;
    out->reg[4] = r.si;
    out->reg[5] = r.di;
    out->reg[6] = r.bp;
    finish(out);
}

static void run_port(snapshot const* const in, snapshot* const out)
{
    m7regs r;

    setup(in);
    r.ax = in->reg[0];
    r.bx = in->reg[1];
    r.cx = in->reg[2];
    r.dx = in->reg[3];
    r.si = in->reg[4];
    r.di = in->reg[5];
    r.bp = in->reg[6];

    if (draw8x816t(&r) != 0) {
        /* The mosaic tail was a jump: run it with the registers the drawer
           ended on, which is what the stub records. */
        call_regs(&r, domosaic16b);
    }

    out->reg[0] = r.ax;
    out->reg[1] = r.bx;
    out->reg[2] = r.cx;
    out->reg[3] = r.dx;
    out->reg[4] = r.si;
    out->reg[5] = r.di;
    out->reg[6] = r.bp;
    finish(out);
}

int main(void)
{
    long cov[7];
    u1* const caches[3] = { cache2, cache4, cache8 };
    u4 const allocs[3] = { C2SZ, C4SZ, C8SZ };
    u4 const sizes[3] = { 262144u, 131072u, 65536u };

    memset(cov, 0, sizeof cov);
    for (u4 i = 0; i < 65537; i++) {
        fulladdtab[i] = (u2)(i * 5u + (i >> 3));
    }
    vcache2b = cache2;
    vcache4b = cache4;
    vcache8b = cache8;
    /* Real tile data is mostly transparent, and both group skips key off a
       zero dword - uniform random bytes produce one about once in 4 billion,
       so the skip would never be exercised. Seeded once: 448K per iteration
       would dominate the run, and DT_MAIN reseeds anyway. */
    srand(20260811);
    for (u4 i = 0; i < 3; i++) {
        u1* const p = caches[i];
        u4 const n = allocs[i];

        for (u4 j = 0; j + 4 <= n; j += 4) {
            for (u4 k = 0; k < 4; k++) {
                p[j + k] = (u1)(dt_mod(5) == 0 ? 0 : dt_u32());
            }
            if (dt_mod(3) == 0) {
                memset(p + j, 0, 4);
            }
        }
    }

    DT_MAIN(20260811, 30000)
    {
        snapshot in, x, y;
        u4 hofs, cache, off, size, shifter, column;
        int blank;
        u1* base;
        int route;

        /* bgmode 2 and 5 are the two redirects the thunk still owns; keep them
           rare so the body is what is mostly being compared. */
        bgmode = (u1)(dt_mod(8) == 0 ? (dt_mod(2) ? 2 : 5) : dt_mod(8));
        curbgpr = (u1)(dt_mod(2) ? 0x20u : 0);
        bgcoloradder = (u1)dt_u32();
        osm2dis = (u1)(dt_mod(3) ? dt_mod(2) : dt_u32());
        /* Bits 6 and 7 pick half add, full add or full sub. */
        scaddtype = (u1)(dt_mod(4) ? dt_mod(4) << 6 : dt_u32());
        scrnon = (u2)(dt_mod(3) ? dt_mod(2) << 12 : dt_u32());
        /* A mosaic size of 1 is what lets the windowed form be reached at all,
           and it is also what decides the mosaic tail. */
        curmosaicsz = (u1)(dt_mod(3) ? 1 : dt_mod(16) + 2);
        winon = (u1)(dt_mod(2) ? dt_mod(2) : dt_u32());
        /* A palette entry is 16 bits, and the whole of it indexes fulladdtab
           after the add - wider values read far outside the table. The clipped
           tables are left unclipped on purpose: the full-add path does not
           mask them, and only an unmasked value can show that. */
        for (u4 i = 0; i < 256; i++) {
            pal16b[i] = dt_u32() & 0xFFFFu;
            pal16bcl[i] = dt_u32() & 0xFFFFu;
            pal16bxcl[i] = dt_u32() & 0xFFFFu;
        }
        dt_fill(winbuf, WINSZ);
        for (u4 i = 0; i < WINSZ; i++) {
            /* Mostly-open windows, so the masked arm is not the only one. */
            winbuf[i] = (u1)(dt_mod(3) ? 0 : winbuf[i]);
        }
        /* Tile numbers must reach 0x3FF, and a zero tile word is the common
           "nothing here" case both group skips key off. */
        /* A line where every tile belongs to the other priority leaves drawn
           at zero, which is what decides whether the mosaic tail runs at all -
           33 independent coin flips never produce it. */
        blank = dt_mod(12) == 0;
        for (u4 i = 0; i < MAPSZ; i += 2) {
            u2 t = (u2)(dt_mod(4) == 0 ? 0 : dt_u32());

            if (blank) {
                t = (u2)((t & ~0x2000u) | (curbgpr & 0x20u ? 0u : 0x2000u));
            }
            *(u2*)(mapbuf + i) = t;
        }

        /* The horizontal offset biases three pointers, two of them by 2x, and
           transpbuf is only biased by 32 bytes. */
        hofs = dt_mod(17);

        /* Which cache the tile pointer lands in is the whole point of the
           prologue's compare chain, so drive all three - and put it near the
           end often enough that the clip fires. */
        cache = dt_mod(3);
        base = caches[cache];
        size = sizes[cache];
        off = dt_mod(2) ? size - 65536u + dt_mod(1024u) * 64u : dt_mod(size);
        off &= ~63u;
        if (off > size) {
            off = size;
        }
        /* Exactly at the end is the case that separates the clip test's >=
           from >, and every offset below it clips one tile later. */
        if (dt_mod(6) == 0) {
            off = size;
        }

        /* ah is the palette shifter: mostly small, but big enough to reach
           the 8-bit shift's own truncation. al is the starting column, which
           only wraps the map pointer if it can reach 0x20. */
        shifter = dt_mod(3) ? dt_mod(8) : dt_u32() & 0xFFu;
        column = dt_mod(2) ? dt_mod(0x22) : dt_u32() & 0xFFu;
        in.reg[0] = (dt_u32() & 0xFFFF0000u) | shifter << 8 | column;
        in.reg[1] = (u4)(uintptr_t)(base + off);
        in.reg[2] = dt_mod(8) * 8u;
        in.reg[3] = (u4)(uintptr_t)(mapbuf + dt_mod(64) * 2u);
        in.reg[4] = hofs;
        in.reg[5] = (u4)(uintptr_t)(mapbuf + dt_mod(64) * 2u);
        in.reg[6] = dt_u32();

        dt_fill(in.vid, VIDSZ);
        /* A transparent pixel underneath is what the half-add path branches
           on, and it is the common case on real hardware - uniform random
           bytes give a zero word about once in 65536. */
        dt_fill(in.tb, TBSZ);
        for (u4 i = 0; i < TBSZ; i += 2) {
            if (dt_mod(3) != 0) {
                in.tb[i] = 0;
                in.tb[i + 1] = 0;
            }
        }
        dt_fill(in.xv, 576);

        run_asm(&in, &x);
        run_port(&in, &y);

        if (bgmode == 5 || (osm2dis != 1 && bgmode == 2)) {
            route = 6;
        } else if (scaddtype & 0x80u) {
            route = 4;
        } else if (!(scaddtype & 0x40u) || (scrnon >> 8) == 0) {
            route = 2;
        } else {
            route = 0;
        }
        if (route != 6 && curmosaicsz == 1 && winon != 0) {
            route++;
        }
        cov[route]++;

        DT_EQ("eax", x.reg[0], y.reg[0]);
        DT_EQ("ebx", x.reg[1], y.reg[1]);
        DT_EQ("ecx", x.reg[2], y.reg[2]);
        DT_EQ("edx", x.reg[3], y.reg[3]);
        DT_EQ("esi", x.reg[4], y.reg[4]);
        DT_EQ("edi", x.reg[5], y.reg[5]);
        DT_EQ("ebp", x.reg[6], y.reg[6]);
        DT_MEM("scratch globals", x.glob, y.glob, sizeof x.glob);
        DT_MEM("temp/bshifter/drawn", x.mem, y.mem, sizeof x.mem);
        DT_MEM("exits", &x.ex, &y.ex, sizeof x.ex);
        DT_MEM("video buffer", x.vid, y.vid, VIDSZ);
        DT_MEM("transpbuf", x.tb, y.tb, TBSZ);
        DT_MEM("xtravbuf", x.xv, y.xv, 576);
        if (dt_bad && DT_SHOW()) {
            printf("  ^ route=%d bgmode=%u mosaic=%u winon=%u hofs=%u "
                   "scaddtype=%02x scrnon=%04x cache=%u off=%u\n",
                route, bgmode, curmosaicsz, winon, hofs, scaddtype, scrnon,
                cache, off);
        }
    }
    printf("  half=%ld halfw=%ld add=%ld addw=%ld sub=%ld subw=%ld skip=%ld\n",
        cov[0], cov[1], cov[2], cov[3], cov[4], cov[5], cov[6]);
    DT_DONE("makev16t 8x8 colour-maths drawers");
}
