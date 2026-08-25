/* Differential test: the tail of newengine16b in video/newgfx16.asm against
 * the C port at the end of video/c_ngline.c.
 *
 * The oracle is HEAD's copy of the file, not the pre-port one, because the
 * first two thirds of this routine - the per-line tables, the windows and the
 * sprite window - became C in an earlier pass and the assembly already reached
 * them by ccallv. Picking that revision leaves both sides sharing those three
 * and isolates what is under test here: the colour-add cache, the back area,
 * the hi-res line duplication and the sprite-priority flag.
 *
 * BackAreaFill and BuildWindow2 are recorded rather than run. Both sides reach
 * them cdecl, so one stand-in serves both, and what it records - the line and
 * the two fill colours - is the whole of what the routine hands over.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;

/* --- the state the routine reads and writes ------------------------------ */
extern u2 curypos;
extern u1 bgwinchange[], bgallchange[], bg1change[], bg2change[];
extern u1 bg3change[], bg4change[];
extern u4 palchanged, startlinet, endlinet, scfbl, bgcmsung;
extern u1 bg3highst, BG3PRI[], BGMA[], BGFB[], modeused[];
extern u1 FillSubScr[], clinemainsub, ngmsdraw, ngextbg;
extern u1 scaddtype, scaddset, scadtng[], scadsng[];
extern u1 bgmode, forceblnk, interlval, intrlng[];
extern u1 mosaicon, mosaicsz, mosenng[], mosszng[];
extern u1 BGMS1[], mode7st[], t16x161[], t16x162[], t16x163[], t16x164[];
extern u1 BG116x16t, BG216x16t, BG316x16t, BG416x16t;
extern u2 scrnon, cgram[];
extern u2 bg1scrolx[4], bg1scroly[4], bg1objptr[4], bg1ptr[4];
extern u4 bg1ptrx[4], bg1ptry[4];
extern u2 BG1SXl[], BG1SYl[], BGOPT1[], BGPT1[], BGPT1X[], BGPT1Y[];
extern u4 mode7A, mode7C, mode7X0;
extern u1 mode7set;
extern u4 mode7ab[], mode7cd[], mode7xy[];
extern u4 cpalptrng, cpalval[];
extern u1 winbg1en[], winenabm, winenabs, disableeffects;
extern u1 winbg1enval[], winbg1envalm[], winbg1envals[];
extern u1 winlogica, winl1, winlogicb, nglogicval;
extern u4 objwlrpos[], objclineptr[], ngwinen;
extern u4 ngwintable[32];
extern u2 objwen[], winlogicaval[];
extern u4 winboundary[];
extern u1 coladdr, coladdg, coladdb, vidbright;
extern u4 Prevcoladdr, ColResult;
extern u4 ngrposng, nggposng, ngbposng;
extern u4 BackAreaAdd, BackAreaUnFillCol, BackAreaFillCol, UnusedBit[2];
extern u4 sprleftpr[];
extern u1 SpecialLine[];
extern u1 prevbrightdc;

/* Not in the emulator objects this links: the test owns the buffers. */
u1 Mode7HiRes16b, scanlines, hiresstuff, res640;
u4 CSprWinPtr, ngwinptr;
u1* vbufdptr;
u1* vram;
extern u1* vidbuffer; /* ngtransp_extra.c */
extern u1 vidmemch2[], vidmemch2s[];

/* The second field of the last line sits 75036*6 bytes on, and the head half
   clears 1792 dwords from vidmemch2s - past its own 4096, as the assembly
   does. */
#define VB_SIZE (32u + 223u * 576u + 75036u * 6u + 512u)
static u1 vbstore[VB_SIZE], vbtemplate[VB_SIZE];
static u1 winstore[65536];
static u1 palstore[4096];

/* --- the recorders ------------------------------------------------------- */
enum { REC_MAX = 8 };
static struct {
    u4 n;
    u4 what[REC_MAX], y[REC_MAX], arg[REC_MAX];
    u4 fill[REC_MAX], unfill[REC_MAX], add[REC_MAX], logic[REC_MAX];
} rec;

/* Whether the window this line asks for exists at all: the routine branches on
   ngwinen afterwards, so a builder that always says "no" hides half of it. */
static u4 rec_ngwinen;

void BuildWindow2(u4 y, u4 idx);
void BuildWindow2(u4 const y, u4 const idx)
{
    if (rec.n < REC_MAX) {
        rec.what[rec.n] = 1;
        rec.y[rec.n] = y;
        rec.arg[rec.n] = idx;
        rec.logic[rec.n] = nglogicval;
        rec.n++;
    }
    ngwinen = rec_ngwinen;
    ngwintable[0] = 40;
    ngwintable[1] = 60;
}

void BackAreaFill(u4 y);
void BackAreaFill(u4 const y)
{
    if (rec.n < REC_MAX) {
        rec.what[rec.n] = 2;
        rec.y[rec.n] = y;
        rec.fill[rec.n] = BackAreaFillCol;
        rec.unfill[rec.n] = BackAreaUnFillCol;
        rec.add[rec.n] = BackAreaAdd;
        rec.n++;
    }
}

void setpalette16bng(void);
void setpalette16bng(void)
{
    if (rec.n < REC_MAX) {
        rec.what[rec.n] = 3;
        rec.n++;
    }
}

void asm_newengine16b(void);
void newengine16b(void); /* video/c_ngline.c */

/* The assembly ends `xor ebx,ebx / ret` and leaves ebx clobbered - its one
   caller in the emulator declares every register clobbered too, which is why
   the port could drop the register contract entirely. Called as a plain C
   function it would eat whatever gcc had parked there. */
static void call_asm(void)
{
    __asm__ volatile("push %%ebp\n\t call %P0\n\t pop %%ebp" ::"X"(
        asm_newengine16b)
        : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
}

/* --- the state image ----------------------------------------------------- */
typedef struct {
    char const* name;
    void* p;
    u4 n;
} region;

static region const img[] = {
    { "curypos", &curypos, 2 },
    { "bgwinchange", bgwinchange, 256 },
    { "bgallchange", bgallchange, 256 },
    { "bg1change", bg1change, 256 },
    { "bg2change", bg2change, 256 },
    { "bg3change", bg3change, 256 },
    { "bg4change", bg4change, 256 },
    { "palchanged", &palchanged, 4 },
    { "startlinet", &startlinet, 4 },
    { "endlinet", &endlinet, 4 },
    { "scfbl", &scfbl, 4 },
    { "bgcmsung", &bgcmsung, 4 },
    { "bg3highst", &bg3highst, 1 },
    { "BG3PRI", BG3PRI, 256 },
    { "BGMA", BGMA, 256 },
    { "BGFB", BGFB, 256 },
    { "modeused", modeused, 8 },
    { "FillSubScr", FillSubScr, 256 },
    { "clinemainsub", &clinemainsub, 4 },
    { "ngmsdraw", &ngmsdraw, 1 },
    { "ngextbg", &ngextbg, 1 },
    { "scaddtype", &scaddtype, 1 },
    { "scaddset", &scaddset, 1 },
    { "scadtng", scadtng, 256 },
    { "scadsng", scadsng, 256 },
    { "bgmode", &bgmode, 1 },
    { "forceblnk", &forceblnk, 1 },
    { "interlval", &interlval, 1 },
    { "intrlng", intrlng, 256 },
    { "mosaicon", &mosaicon, 1 },
    { "mosaicsz", &mosaicsz, 1 },
    { "mosenng", mosenng, 256 },
    { "mosszng", mosszng, 256 },
    { "BGMS1", BGMS1, 2048 },
    { "mode7st", mode7st, 256 },
    { "t16x161", t16x161, 256 },
    { "t16x162", t16x162, 256 },
    { "t16x163", t16x163, 256 },
    { "t16x164", t16x164, 256 },
    { "BG1SXl", BG1SXl, 2048 }, /* BG1..BG4 are contiguous */
    { "BG1SYl", BG1SYl, 2048 },
    { "BGOPT1", BGOPT1, 2048 },
    { "BGPT1", BGPT1, 2048 },
    { "BGPT1X", BGPT1X, 2048 },
    { "BGPT1Y", BGPT1Y, 2048 },
    { "mode7ab", mode7ab, 1024 },
    { "mode7cd", mode7cd, 1024 },
    { "mode7xy", mode7xy, 1024 },
    { "mode7set", &mode7set, 1 },
    { "cpalptrng", &cpalptrng, 4 },
    { "cpalval", cpalval, 1024 },
    { "winbg1enval", winbg1enval, 6 * 256 },
    { "winbg1envals", winbg1envals, 6 * 256 },
    { "winbg1envalm", winbg1envalm, 6 * 256 },
    { "winlogicaval", winlogicaval, 512 },
    { "winboundary", winboundary, 1024 },
    { "nglogicval", &nglogicval, 1 },
    { "ngwinen", &ngwinen, 4 },
    { "ngwintable", ngwintable, 128 },
    { "objwlrpos", objwlrpos, 1024 },
    { "objwen", objwen, 512 },
    { "objclineptr", objclineptr, 1024 },
    { "CSprWinPtr", &CSprWinPtr, 4 },
    { "ngrposng", &ngrposng, 4 },
    { "nggposng", &nggposng, 4 },
    { "ngbposng", &ngbposng, 4 },
    { "Prevcoladdr", &Prevcoladdr, 4 },
    { "ColResult", &ColResult, 4 },
    { "BackAreaAdd", &BackAreaAdd, 4 },
    { "BackAreaFillCol", &BackAreaFillCol, 4 },
    { "BackAreaUnFillCol", &BackAreaUnFillCol, 4 },
    { "SpecialLine", SpecialLine, 256 },
    { "hiresstuff", &hiresstuff, 1 },
    { "sprleftpr", sprleftpr, 1024 },
    { "prevbrightdc", &prevbrightdc, 1 },
    { "vidmemch2s", vidmemch2s, 8192 },
    { "spritewindow", winstore, 32768 },
};

enum { NIMG = (int)(sizeof img / sizeof img[0]) };

static u1 seed[262144], sa[262144], sb[262144];

/* The video buffer is too big to snapshot per iteration, so it is refilled
   from a position-dependent template - every offset distinguishable - and
   compared by checksum. */
static u4 vb_sum(void)
{
    u4 h = 2166136261u, q;

    for (q = 0; q < VB_SIZE; q++)
        h = (h ^ vbstore[q]) * 16777619u;
    return h;
}

static u4 img_bytes(void)
{
    u4 n = 0;
    int q;

    for (q = 0; q < NIMG; q++)
        n += img[q].n;
    return n;
}

static void img_out(u1* const dest)
{
    u4 o = 0;
    int q;

    for (q = 0; q < NIMG; q++) {
        memcpy(dest + o, img[q].p, img[q].n);
        o += img[q].n;
    }
}

static void img_in(u1 const* const src)
{
    u4 o = 0;
    int q;

    for (q = 0; q < NIMG; q++) {
        memcpy(img[q].p, src + o, img[q].n);
        o += img[q].n;
    }
}

int main(void)
{
    u4 hires = 0, backsub = 0, colrebuilt = 0, colcached = 0, wins = 0;

    u4 q0;

    for (q0 = 0; q0 < VB_SIZE; q0++)
        vbtemplate[q0] = (u1)(q0 * 31u + 7u);
    vidbuffer = vbstore;
    vbufdptr = palstore;
    vram = vidmemch2;
    ngwinptr = (u4)(uintptr_t)winstore - 0x10000u;
    if (img_bytes() > sizeof seed) {
        printf("image too large\n");
        return 1;
    }

    DT_MAIN(20260825, 4000)
    {
        u4 q, va, vb, oldkey;
        u1 arec[sizeof rec], brec[sizeof rec];

        for (q = 0; q < NIMG; q++)
            dt_fill((u1*)img[q].p, img[q].n);
        dt_fill(palstore, sizeof palstore);
        dt_fill((u1*)cgram, 512);
        dt_fill((u1*)bg1scrolx, 8);
        dt_fill((u1*)bg1scroly, 8);
        dt_fill((u1*)bg1objptr, 8);
        dt_fill((u1*)bg1ptr, 8);
        dt_fill((u1*)bg1ptrx, 16);
        dt_fill((u1*)bg1ptry, 16);
        dt_fill(winbg1en, 6);
        dt_fill(&winl1, 8);
        mode7A = dt_u32();
        mode7C = dt_u32();
        mode7X0 = dt_u32();
        scrnon = (u2)dt_u32();
        winenabm = (u1)dt_mod(256);
        winenabs = (u1)dt_mod(256);
        winlogica = (u1)dt_mod(256);
        /* The emulator only ever puts 0..15 here, but the assembly shifts by
           cl, so the port has to agree about counts the hardware masks. */
        ngrposng = dt_mod(32);
        nggposng = dt_mod(32);
        ngbposng = dt_mod(32);
        winlogicb = (u1)dt_mod(256);
        BG116x16t = (u1)dt_mod(2);
        BG216x16t = (u1)dt_mod(2);
        BG316x16t = (u1)dt_mod(2);
        BG416x16t = (u1)dt_mod(2);

        /* Structured where a random byte would starve a branch. */
        /* The line. Line 1 has a special case in the head half, so it has to
           come up more often than one time in 224. */
        curypos = (u2)(dt_mod(4) == 0 ? dt_mod(3) : dt_mod(224));
        bgmode = (u1)dt_mod(8);
        vidbright = (u1)dt_mod(16);
        coladdr = (u1)dt_mod(32);
        coladdg = (u1)dt_mod(32);
        coladdb = (u1)dt_mod(32);
        prevbrightdc = (u1)dt_mod(16);
        scanlines = (u1)dt_mod(2);
        res640 = (u1)dt_mod(2);
        Mode7HiRes16b = (u1)dt_mod(2);
        interlval = (u1)dt_mod(256);
        forceblnk = (u1)dt_mod(2);
        disableeffects = (u1)dt_mod(2);
        clinemainsub = (u1)dt_mod(2);
        mosaicon = (u1)dt_mod(2);
        ngmsdraw = (u1)dt_mod(2);
        mode7set = (u1)dt_mod(2);
        scaddset = (u1)dt_mod(256);
        scaddtype = (u1)dt_mod(256);
        cpalptrng = dt_mod(2048) & ~1u;
        /* The sprite-window cursor and the per-line pointers into it are
           addresses, so they cannot be random: the builder follows them. */
        CSprWinPtr = 0x10000u + dt_mod(4096);
        for (q = 0; q < 256u; q++)
            objclineptr[q] = dt_mod(4) == 0 ? 0xFFFFFFFFu
                                            : 0x10000u + dt_mod(16384);
        rec_ngwinen = dt_mod(2);
        /* One line in three finds the fixed colour already cached, which is
           the branch a random Prevcoladdr never lands on. */
        if (dt_mod(3) == 0)
            Prevcoladdr = (u4)vidbright | (u4)coladdr << 8
                | (u4)coladdg << 16 | (u4)coladdb << 24;
        for (q = 0; q < 256u; q++) {
            FillSubScr[q] = (u1)dt_mod(4);
            sprleftpr[q] = dt_mod(3) == 0
                ? (u4)1u << (8u * dt_mod(4))
                : dt_u32();
        }
        /* The layer-window bytes are tested against 0 and against bit masks,
           so mostly-zero is the shape the emulator has. */
        for (q = 0; q < 6u * 256u; q++)
            winbg1enval[q] = (u1)(dt_mod(2) ? 0 : dt_mod(256));

        oldkey = Prevcoladdr;
        img_out(seed);
        memset(&rec, 0, sizeof rec);
        memcpy(vbstore, vbtemplate, VB_SIZE);
        call_asm();
        img_out(sa);
        memcpy(arec, &rec, sizeof rec);
        va = vb_sum();

        img_in(seed);
        memset(&rec, 0, sizeof rec);
        memcpy(vbstore, vbtemplate, VB_SIZE);
        newengine16b();
        img_out(sb);
        memcpy(brec, &rec, sizeof rec);
        vb = vb_sum();

        {
            u4 o = 0;
            int k;

            for (k = 0; k < NIMG; k++) {
                DT_MEM(img[k].name, sa + o, sb + o, img[k].n);
                o += img[k].n;
            }
        }
        DT_MEM("calls", (u1*)arec, (u1*)brec, sizeof rec);
        DT_EQ("vidbuffer", va, vb);

        /* Counted from what actually happened, not from a flag the routine
           only ever sets: hiresstuff is sticky and Prevcoladdr is a cache key
           that is almost never zero. */
        if (SpecialLine[curypos & 0xFFu] & 3u)
            hires++;
        {
            u4 k, fills = 0;

            for (k = 0; k < rec.n; k++) {
                if (rec.what[k] == 2)
                    fills++;
                if (rec.what[k] == 1 && rec.arg[k] >= 5u * 256u)
                    wins++;
            }
            if (fills > 1u)
                backsub++;
        }
        if (oldkey != Prevcoladdr)
            colrebuilt++;
        else
            colcached++;
    }
    printf("  hi-res lines %u, sub-screen back fills %u, colour rebuilt %u, "
           "cached %u, back windows %u\n",
        hires, backsub, colrebuilt, colcached, wins);
    DT_DONE("newengine16b tail");
}
