/* Differential test: every draw*ms routine in video/mv16tms.asm against the C
 * port in video/c_mv16tms.c + video/c_mv16tsms.c + video/c_mv16msgate.c.
 *
 * The oracle here is the *whole* pre-port file (tools/mkoracle.py), not a cut
 * of one routine, so this drives the two top-level entries and lets their own
 * dispatch reach all eleven: bgmode / curmosaicsz / winon / scaddtype /
 * scrnon[1] pick the writer and the masking, and the prologue derives the tile
 * cache, the clip boundary and the biased output pointers from the registers.
 * That keeps the downstream state self-consistent, which hand-built inputs for
 * the individual loops were not.
 *
 * Both sides share these globals, so everything the routines mutate is saved
 * and restored between the two runs. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;

/* --- the globals the oracle leaves undefined ------------------------------ */
u1 bgcoloradder, bgmode, curbgpr;
u1 curmosaicsz, coadder16, drawn, tileleft16b, winon, scaddtype, curypos;

/* The 16x16 prologue stores a whole dword to `temp`, which in the emulator is
   a byte with bshifter, a16x16xinc and a16x16yinc packed right after it
   (video/makevid.c) - so that one instruction writes all four, and the
   prologue then reads a16x16yinc back as bits 24-31 of the caller's eax.
   Declaring them as ordinary C globals lets the compiler reorder them, and
   the write lands on whatever happens to follow. Lay them out by hand. */
__asm__(".pushsection .bss\n"
        ".globl temp\n"
        "temp: .skip 1\n"
        ".globl bshifter\n"
        "bshifter: .skip 1\n"
        ".globl a16x16xinc\n"
        "a16x16xinc: .skip 1\n"
        ".globl a16x16yinc\n"
        "a16x16yinc: .skip 1\n"
        ".popsection\n");
extern u1 temp, bshifter, a16x16xinc, a16x16yinc;
u1 scrnon[4];
u4 bgofwptr, bgsubby, tempcach, temptile, yadder, yrevadder;
u2 yadd, yflipadd;
u4 pal16b[256], pal16bcl[256], pal16bxcl[256];
/* One entry of slack: the writers' dword load reads past the last index. */
u2 fulladdtab[65537];

/* The three caches are one allocation, 2-bit then 4-bit then 8-bit; the
   prologue's compare chain only makes sense against that layout. */
#define C2SZ 262144u
#define C4SZ 131072u
#define C8SZ 65536u
static u1 cache[C2SZ + C4SZ + C8SZ];
u1 *vcache2b = cache, *vcache4b = cache + C2SZ, *vcache8b = cache + C2SZ + C4SZ;

/* Output buffers. A row is 33 tiles x 16 bytes and the pointers are biased
   backwards by twice the horizontal offset, so both ends need slack. */
#define SLACK 256u
#define ROW 528u
#define BUFSZ (SLACK + ROW + SLACK)
u1 transpbuf[BUFSZ], xtravbuf[BUFSZ];
static u1 vidbuf[BUFSZ], winbuf[BUFSZ];
u1 *curvidoffset, *cwinptr, *winptrref;
static u1 tilemap[512];

/* --- the two external tail-jump targets ---------------------------------- */
/* A top-level __asm__ block lands in whatever section gcc last emitted, which
   after these globals is .bss - and code in a NOBITS section makes the linker
   report "file truncated". Push .text explicitly around every one of them. */
#define ASM_TEXT ".pushsection .text\n"
#define ASM_END ".popsection\n"

/* Both sides jump here identically; the point is to capture the registers at
   the jump, which is where the mosaic tail hands over dh = curmosaicsz. */
u4 dm_hit, dm_eax, dm_ebx, dm_ecx, dm_edx, dm_esi, dm_edi, dm_ebp;
void domosaic16b(void);
__asm__(ASM_TEXT ".globl domosaic16b\n"
                 "domosaic16b:\n"
                 "  movl %eax, dm_eax\n"
                 "  movl %ebx, dm_ebx\n"
                 "  movl %ecx, dm_ecx\n"
                 "  movl %edx, dm_edx\n"
                 "  movl %esi, dm_esi\n"
                 "  movl %edi, dm_edi\n"
                 "  movl %ebp, dm_ebp\n"
                 "  incl dm_hit\n"
                 "  ret\n" ASM_END);
/* The oracle jumps to draw16x816t, which is external to mv16tms.asm - so it
   cannot be rewritten by --stub-routine. nasm's -D renames the reference
   instead, leaving the name free for the port's own C stand-in below. */
u4 d16_hit;
__asm__(ASM_TEXT ".globl draw16x816t_stub\n"
                 "draw16x816t_stub:\n"
                 "  incl d16_hit\n"
                 "  ret\n" ASM_END);
void draw16x816t_stub(void);

/* --- calling a routine with a full register set --------------------------- */
u4 rg_eax, rg_ebx, rg_ecx, rg_edx, rg_esi, rg_edi, rg_ebp;
u4 rg_fn;
__asm__(ASM_TEXT ".globl dt_call\n"
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
                 "  ret\n" ASM_END);
void dt_call(void);

#include "../video/c_mv16draw.h"

void asm_draw8x816tms(void), asm_draw16x1616tms(void);

static void call_regs(m7regs* r, void (*fn)(void));

/* Reached by a jump, so the registers at the call are what the stub sees and
   what the dispatcher returns with - hence the round trip. */
u4 draw16x816t(m7regs* const r)
{
    call_regs(r, draw16x816t_stub);
    return 0;
}

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

/* Everything either side may write. */
typedef struct {
    u4 ax, bx, cx, dx, si, di, bp;
    u4 ofw, subby, cach, tile, yadder, yrevadder;
    u2 yadd, yflip;
    u1 tmp, left, drew, coadd, xinc;
    u4 wptr;
    u4 mos, mdx, max, mbx, mcx, msi, mdi, mbp, big;
    u1 vid[BUFSZ], tr[BUFSZ], xtra[BUFSZ];
} snapshot;

/* State the routines mutate, so it can be put back before the second run. */
typedef struct {
    u1 xinc, tmp, left, drew, coadd;
    u4 ofw, subby, cach, tile, yadder, yrevadder;
    u2 yadd, yflip;
    u1 vid[BUFSZ], tr[BUFSZ], xtra[BUFSZ];
} state;

static void save(state* const s)
{
    s->xinc = a16x16xinc;
    s->tmp = temp;
    s->left = tileleft16b;
    s->drew = drawn;
    s->coadd = coadder16;
    s->ofw = bgofwptr;
    s->subby = bgsubby;
    s->cach = tempcach;
    s->tile = temptile;
    s->yadder = yadder;
    s->yrevadder = yrevadder;
    s->yadd = yadd;
    s->yflip = yflipadd;
    memcpy(s->vid, vidbuf, BUFSZ);
    memcpy(s->tr, transpbuf, BUFSZ);
    memcpy(s->xtra, xtravbuf, BUFSZ);
}

static void restore(state const* const s)
{
    a16x16xinc = s->xinc;
    temp = s->tmp;
    tileleft16b = s->left;
    drawn = s->drew;
    coadder16 = s->coadd;
    bgofwptr = s->ofw;
    bgsubby = s->subby;
    tempcach = s->cach;
    temptile = s->tile;
    yadder = s->yadder;
    yrevadder = s->yrevadder;
    yadd = s->yadd;
    yflipadd = s->yflip;
    memcpy(vidbuf, s->vid, BUFSZ);
    memcpy(transpbuf, s->tr, BUFSZ);
    memcpy(xtravbuf, s->xtra, BUFSZ);
}

static void reset(void)
{
    dm_hit = dm_eax = dm_ebx = dm_ecx = dm_edx = dm_esi = dm_edi = dm_ebp = 0;
    d16_hit = 0;
    winptrref = 0;
}

static void finish(m7regs const* const r, snapshot* const out)
{
    out->ax = r->ax;
    out->bx = r->bx;
    out->cx = r->cx;
    out->dx = r->dx;
    out->si = r->si;
    out->di = r->di;
    out->bp = r->bp;
    out->ofw = bgofwptr;
    out->subby = bgsubby;
    out->cach = tempcach;
    out->tile = temptile;
    out->yadder = yadder;
    out->yrevadder = yrevadder;
    out->yadd = yadd;
    out->yflip = yflipadd;
    out->tmp = temp;
    out->left = tileleft16b;
    out->drew = drawn;
    out->coadd = coadder16;
    out->xinc = a16x16xinc;
    out->wptr = (u4)(uintptr_t)winptrref;
    out->mos = dm_hit;
    out->mdx = dm_edx;
    out->max = dm_eax;
    out->mbx = dm_ebx;
    out->mcx = dm_ecx;
    out->msi = dm_esi;
    out->mdi = dm_edi;
    out->mbp = dm_ebp;
    out->big = d16_hit;
    memcpy(out->vid, vidbuf, BUFSZ);
    memcpy(out->tr, transpbuf, BUFSZ);
    memcpy(out->xtra, xtravbuf, BUFSZ);
}

static void regs_in(m7regs* const r, u4 const* const in)
{
    r->ax = in[0];
    r->bx = in[1];
    r->cx = in[2];
    r->dx = in[3];
    r->si = in[4];
    r->di = in[5];
    r->bp = in[6];
}

static void run(void (*const fn)(void), u4 const* const in,
    snapshot* const out)
{
    m7regs r;

    reset();
    regs_in(&r, in);
    call_regs(&r, fn);
    finish(&r, out);
}

static void run_port(int const big, u4 const* const in, snapshot* const out)
{
    m7regs r;

    reset();
    regs_in(&r, in);
    if ((big ? draw16x1616tms(&r) : draw8x816tms(&r)) != 0) {
        /* The mosaic tail was a jump: run it with the registers the drawer
           ended on, which is what the stub records. */
        call_regs(&r, domosaic16b);
    }
    finish(&r, out);
}

/* Which routine the dispatch picks. This is a MODEL of the assembly's dispatch,
   not a measurement, and it lied once already: the flags it reads were being
   clobbered before the dispatch saw them, so it reported routines that never
   ran. Treat the numbers as a smoke test, and when it matters, measure - copy
   video/c_mv16tsms.c, add a counter to each c_draw* entry, and link that copy
   instead. */
static char const* const names[12] = { "draw16x816t (stub)", "draw8x816twinonms",
    "draw8x816tsms", "draw8x8fulladdms", "draw8x816tms body",
    "draw16x1616twinonms", "draw16x1616tsms", "draw16x16fulladdms",
    "draw16x1616tms body", "draw16x1616tswinonms", "draw16x16fulladdwinonms",
    "draw8x816tswinonms" };
static long cov[12];

static int picked(int const big)
{
    int const base = big ? 5 : 1;
    if (bgmode == 5) {
        return 0;
    }
    if (curmosaicsz == 1 && winon) {
        /* The windowed entry dispatches once more on scaddtype. */
        if (!big) {
            return (scaddtype & 0x80u) ? 11 : base;
        }
        return (scaddtype & 0x80u) ? 9 : (!(scaddtype & 0x40u) || !scrnon[1]) ? 10
                                                                              : base;
    }
    if (scaddtype & 0x80u) {
        return base + 1;
    }
    if (!(scaddtype & 0x40u) || !scrnon[1]) {
        return base + 2;
    }
    return base + 3;
}

int main(void)
{
    for (u4 i = 0; i < 65536; i++) {
        fulladdtab[i] = (u2)(i * 5u + (i >> 3));
    }
    for (u4 i = 0; i < 256; i++) {
        /* Real entries are 16-bit colours; anything wider would push the
           fulladdtab index past the table, which the emulator never does. */
        pal16b[i] = (i * 0x0101u + 0x1234u) & 0xFFFFu;
        pal16bcl[i] = (i * 0x0303u + 0x4321u) & 0xFFFFu;
        pal16bxcl[i] = (i * 0x0505u + 0x2143u) & 0xFFFFu;
    }

    DT_MAIN(20260803, 20000)
    {
        state s;
        snapshot x, y;
        u4 in[7];
        int const big = (int)dt_mod(2);
        /* The pointers are biased backwards by twice this, and it is a
           position within a tile in the emulator. */
        u4 const hofs = dt_mod(16);
        u4 tileoff;

        bgmode = (u1)(dt_mod(8) ? dt_mod(5) : 5);
        curmosaicsz = (u1)(dt_mod(2) ? 1 : 1 + dt_mod(16));
        winon = (u1)dt_mod(2);
        scaddtype = (u1)(dt_mod(4) ? (dt_mod(2) ? 0x80u : 0x40u) : dt_u32());
        scrnon[1] = (u1)dt_mod(2);
        curypos = (u1)dt_mod(224);
        a16x16xinc = (u1)dt_mod(2);
        a16x16yinc = (u1)dt_mod(2);
        bshifter = (u1)(dt_mod(2) ? dt_mod(8) : dt_u32());
        /* Bits 6 and 7 matter: the entry's flip bits are read both raw (the
           16x16 half-tile step) and xored with this (everything else), so a
           curbgpr that never sets them hides the difference. */
        curbgpr = (u1)(dt_mod(2) ? (dt_mod(2) ? 0x00u : 0x20u) : dt_u32());
        bgcoloradder = (u1)dt_u32();

        /* Non-zero bytes drop a pixel, so keep zeros common. */
        for (u4 i = 0; i < BUFSZ; i++) {
            winbuf[i] = (u1)(dt_mod(3) ? 0 : dt_u32());
        }
        /* Zero dwords are the group-skip fast path; make them common. */
        for (u4 i = 0; i < sizeof cache; i++) {
            cache[i] = (u1)(dt_mod(3) ? 0 : dt_u32());
        }
        /* Now and then make the whole row skip on priority, so the tails that
           only run when drawn == 0 are reached at all. */
        {
            int const allskip = dt_mod(12) == 0;
            u2 const pri = (u2)(((curbgpr & 0x20u) ^ 0x20u) << 8);
            for (u4 i = 0; i < sizeof tilemap; i += 2) {
                u2 e = (u2)dt_u32();
                /* Bound the tile number so tempcach + tile*64 stays in the
                   cache; bits 10-15 stay random - bit 10 is the low palette
                   bit and the tile mask has to drop it. */
                e = (u2)((e & 0xFC00u) | dt_mod(1024));
                if (allskip) {
                    e = (u2)((e & ~0x2000u) | pri);
                }
                *(u2*)(tilemap + i) = e;
            }
        }
        dt_fill(vidbuf, BUFSZ);
        dt_fill(transpbuf, BUFSZ);
        dt_fill(xtravbuf, BUFSZ);

        curvidoffset = vidbuf + SLACK;
        cwinptr = winbuf + SLACK;
        temp = (u1)(dt_mod(2) ? (0x1Cu + dt_mod(8)) : dt_u32());
        /* Straddle each cache boundary: which one the prologue picks is the
           whole point of its compare chain. */
        /* Straddle each cache boundary - which cache the prologue picks is the
           point of its compare chain - and keep the offset a multiple of a
           tile, so tempcach + tile*64 can land exactly on bgofwptr and the
           clip's >= is distinguishable from >. */
        tileoff = dt_mod(3) == 0 ? C2SZ - dt_mod(0x2000u)
            : dt_mod(2) == 0     ? C2SZ + C4SZ - dt_mod(0x2000u)
                                 : dt_mod(C2SZ + C4SZ);
        tileoff &= ~63u;

        /* eax is stored to `temp` as a whole dword, so its four bytes are the
           column seed, bshifter, a16x16xinc and a16x16yinc - the separate
           assignments above are overwritten on the 16x16 path. Seed the column
           near 0x20 often, or the wrap that resets it is never reached. */
        in[0] = (u4)(dt_mod(2) ? (0x18u + dt_mod(12)) : dt_mod(256))
            | ((u4)bshifter << 8) | ((u4)a16x16xinc << 16)
            | ((u4)a16x16yinc << 24);
        in[1] = (u4)(uintptr_t)(cache + tileoff);
        in[2] = dt_mod(56); /* the y adder */
        in[3] = (u4)(uintptr_t)tilemap;
        in[4] = hofs;
        in[5] = (u4)(uintptr_t)(tilemap + 2u * dt_mod(16));
        in[6] = 0xB0000000u;

        cov[picked(big)]++;
        save(&s);
        run(big ? asm_draw16x1616tms : asm_draw8x816tms, in, &x);
        restore(&s);
        run_port(big, in, &y);

        DT_EQ("eax", x.ax, y.ax);
        DT_EQ("ebx", x.bx, y.bx);
        DT_EQ("ecx", x.cx, y.cx);
        DT_EQ("edx", x.dx, y.dx);
        DT_EQ("esi", x.si, y.si);
        DT_EQ("edi", x.di, y.di);
        DT_EQ("ebp", x.bp, y.bp);
        DT_EQ("bgofwptr", x.ofw, y.ofw);
        DT_EQ("bgsubby", x.subby, y.subby);
        DT_EQ("tempcach", x.cach, y.cach);
        DT_EQ("temptile", x.tile, y.tile);
        DT_EQ("yadder", x.yadder, y.yadder);
        DT_EQ("yrevadder", x.yrevadder, y.yrevadder);
        DT_EQ("yadd", x.yadd, y.yadd);
        DT_EQ("yflipadd", x.yflip, y.yflip);
        DT_EQ("temp", x.tmp, y.tmp);
        DT_EQ("tileleft16b", x.left, y.left);
        DT_EQ("drawn", x.drew, y.drew);
        DT_EQ("coadder16", x.coadd, y.coadd);
        DT_EQ("a16x16xinc", x.xinc, y.xinc);
        DT_EQ("winptrref", x.wptr, y.wptr);
        DT_EQ("domosaic16b taken", x.mos, y.mos);
        DT_EQ("domosaic edx", x.mdx, y.mdx);
        DT_EQ("domosaic eax", x.max, y.max);
        DT_EQ("domosaic ebx", x.mbx, y.mbx);
        DT_EQ("domosaic ecx", x.mcx, y.mcx);
        DT_EQ("domosaic esi", x.msi, y.msi);
        DT_EQ("domosaic edi", x.mdi, y.mdi);
        DT_EQ("domosaic ebp", x.mbp, y.mbp);
        DT_EQ("draw16x816t taken", x.big, y.big);
        DT_MEM("video buffer", x.vid, y.vid, BUFSZ);
        DT_MEM("transparency buffer", x.tr, y.tr, BUFSZ);
        DT_MEM("mosaic scratch", x.xtra, y.xtra, BUFSZ);
        if (dt_bad && DT_SHOW()) {
            printf("  ^ %s bgmode=%u mosaic=%u winon=%u scaddtype=%02x "
                   "scrnon1=%u hofs=%u temp=%02x xinc=%u yinc=%u\n",
                big ? "16x16" : "8x8", bgmode, curmosaicsz, winon, scaddtype,
                scrnon[1], hofs, s.tmp, s.xinc, a16x16yinc);
        }
    }
    for (int i = 0; i < 12; i++) {
        printf("  %-26s %ld\n", names[i], cov[i]);
    }
    DT_DONE("mv16tms (all eleven draw*ms routines)");
}
