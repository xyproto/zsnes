/* The eight procmode716t* entry points of video/makev16t.asm against
 * video/c_mv16tm7.c. They gate a Mode 7 layer per scanline and pick one of six
 * renderers, so what is compared is which renderer each side selects and the
 * register state handed to it.
 *
 * The globals come from the emulator's own data objects, not declarations
 * here: several of these read wider than the declared type - `word[scrnon+1]`
 * and `word[winenabm]` both span two symbols - so only the real layout
 * agrees. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../video/c_m716gate.h"
#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;

extern u1 winon, curmosaicsz, mosaicon, mosaicsz, mode7set, scaddset;
extern u1 scaddtype, extbgdone, winbg1en, winenabm, winenabs;
extern u2 scrnon, m7starty, curypos, bg1scrolx_m7, bg1scroly_m7;
extern u4 M7TAX, M7TBX, M7TDX, M7TBP, M7TTail;
extern u1 scrndis, winonsp, sprprifix, bgfixer;
extern u1 alreadydrawn, curbgnum, curbgpr, bgmode, bgtilesz, drawn;
extern u1 bgcoloradder, winen[6];
extern u1* colormodeofs;
extern u4 bg1vbufloc[4], bg1xposloc[4], bg1yaddval[4];
extern u2* bg1tdatloc[4];
extern u2* bg1tdabloc[4];
extern u1* bg1cachloc[4];
extern u1* cursprloc;
extern u4 SPRAX, SPRBX, SPRCX, SPRBP, SPRDX, SPRTail;
extern u4 BGAX, BGBX, BGCX, BGDX, BGSI, BGDI, BGBP, BGTail;

/* Which renderer ran, and what it was handed. Both sides reach the same stub,
   so this is the whole observable: the routines' only job is to choose one. */
u4 rn_which, rn_eax, rn_ebx, rn_ecx, rn_edx, rn_esi, rn_edi, rn_ebp;

#define RENDERER(name, id)                   \
    __asm__(".pushsection .text\n"           \
            ".globl " #name "\n" #name ":\n" \
            "  movl %eax, rn_eax\n"          \
            "  movl %ebx, rn_ebx\n"          \
            "  movl %ecx, rn_ecx\n"          \
            "  movl %edx, rn_edx\n"          \
            "  movl %esi, rn_esi\n"          \
            "  movl %edi, rn_edi\n"          \
            "  movl %ebp, rn_ebp\n"          \
            "  movl $" #id ", rn_which\n"    \
            "  ret\n"                        \
            ".popsection\n")

RENDERER(drawmode716t, 1);
RENDERER(drawmode716b, 2);
RENDERER(drawmode716tb, 3);
RENDERER(drawmode716extbg, 4);
RENDERER(drawmode716textbg, 5);
RENDERER(drawmode716extbg2, 6);
RENDERER(drawmode716textbg2, 7);
/* The background and sprite renderers. The *_stub names are what mkoracle
   --stub-routine rewrote the file-internal ones into. */
RENDERER(draw8x816t_stub, 11);
RENDERER(draw16x1616t_stub, 12);
RENDERER(draw8x816bt_stub, 13);
RENDERER(draw16x1616bt_stub, 14);
RENDERER(draw8x816tms, 15);
RENDERER(draw16x1616tms, 16);
RENDERER(drawsprites16t_stub, 17);
RENDERER(drawsprites16bt_stub, 18);
/* SPRTail is deliberately not compared: it is a port-era variable and the
   pre-port assembly has no such thing - it makes the choice inline and jumps.
   Which also means the sprite tail dispatch in video/c_mv16tline.c is a
   transcription this test does not check; see the note below on why the sprite
   renderers are never reached here.

   drawsprites16t and drawsprites16bt live inside makev16t.asm, so the oracle
   has its own copies of them and they cannot be stubbed. The generator keeps
   the scanline's sprite count at zero on the paths that would reach them, so
   neither side runs real sprite code - which means those two tail choices are
   NOT covered here. Everything up to and including the drawsprites16b route
   is. */

/* makewindow is a real C function in the emulator, but it walks window tables
   this test does not model. Both sides call this instead; it sets winon the
   way the real one can, including the 0xFF that abandons the line. */
/* Wrapped so it preserves ecx: after a cdecl call ecx is undefined by
   contract, so the only ecx behaviour worth comparing is what the routines
   themselves do with it. Pinning it here makes that testable. */
/* drawsprites16b is C in the emulator too; same ecx treatment as makewindow. */
u4 ds_calls, ds_cl, ds_ebp;
/* The two background renderers that are already C. */
u4 bgb_calls, bgb_which;
void draw8x816b(u4 eax, u4 ecx, u2* edx, u1* ebx, u4 layer, u4 esi,
    u2 const* edi)
{
    (void)eax;
    (void)ecx;
    (void)edx;
    (void)ebx;
    (void)layer;
    (void)esi;
    (void)edi;
    bgb_calls++;
    bgb_which = 19;
}
void draw16x1616b(u4 eax, u4 ecx, u2* edx, u1* ebx, u4 esi, u2 const* edi)
{
    (void)eax;
    (void)ecx;
    (void)edx;
    (void)ebx;
    (void)esi;
    (void)edi;
    bgb_calls++;
    bgb_which = 20;
}
void c_drawsprites16b(u1 cl, u4 ebp)
{
    ds_calls++;
    ds_cl = cl;
    ds_ebp = ebp;
}
__asm__(".pushsection .text\n"
        ".globl drawsprites16b\n"
        "drawsprites16b:\n"
        "  pushl %ecx\n"
        "  pushl 12(%esp)\n"
        "  pushl 12(%esp)\n"
        "  call c_drawsprites16b\n"
        "  addl $8, %esp\n"
        "  popl %ecx\n"
        "  ret\n"
        ".popsection\n");

u4 mw_calls, mw_al, mw_layer, mw_next;
void c_makewindow(u1 al, u4 layer)
{
    mw_calls++;
    mw_al = al;
    mw_layer = layer;
    winon = (u1)mw_next;
}
__asm__(".pushsection .text\n"
        ".globl makewindow\n"
        "makewindow:\n"
        "  pushl %ecx\n"
        "  pushl 12(%esp)\n"
        "  pushl 12(%esp)\n"
        "  call c_makewindow\n"
        "  addl $8, %esp\n"
        "  popl %ecx\n"
        "  ret\n"
        ".popsection\n");

/* Call a routine with a known register set and hand back what it left. */
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

#define ROUTINES 16
static char const* const names[ROUTINES] = { "procmode716tsub",
    "procmode716tsubextbg", "procmode716tsubextbgb", "procmode716tsubextbg2",
    "procmode716tmain", "procmode716tmainextbg", "procmode716tmainextbgb",
    "procmode716tmainextbg2", "procspritessub16t", "procspritesmain16t",
    "procspritessub16tfix", "procspritesmain16tfix", "drawbackgrndsub16t",
    "drawbackgrndmain16t", "drawbackgrndsub16tfix", "drawbackgrndmain16tfix" };

/* Both sides are the real assembly: the oracle from the pre-port revision
   (asm_*) and the current file from the working tree (cur_*), each built by
   tools/mkoracle.py with the file-internal renderers rewritten into jumps to
   the stubs below. Without that rewrite a call between two routines in one
   file is a PC-relative displacement with no relocation, and nothing can
   intercept it. */
/* Only the oracle now: every gate on the port's side is C. */
#define DECL(n) void asm_##n(void)
DECL(procmode716tsub);
DECL(procmode716tsubextbg);
DECL(procmode716tsubextbgb);
DECL(procmode716tsubextbg2);
DECL(procmode716tmain);
DECL(procmode716tmainextbg);
DECL(procmode716tmainextbgb);
DECL(procmode716tmainextbg2);
DECL(procspritessub16t);
DECL(procspritesmain16t);
DECL(procspritessub16tfix);
DECL(procspritesmain16tfix);
DECL(drawbackgrndsub16t);
DECL(drawbackgrndmain16t);
DECL(drawbackgrndsub16tfix);
DECL(drawbackgrndmain16tfix);

/* The first eight are C now (video/c_m716gate.c): they take the registers in a
   struct and hand back the tail id instead of jumping, so the driver calls
   them directly and then reaches the renderer itself. The renderers are the
   same RENDERER stubs either way, entered with the same seven registers, so
   the comparison below is unchanged. The other eight are still assembly. */
#define GATES 16u
static u4 (*const port_gate[GATES])(m7regs*) = { procmode716tsub,
    procmode716tsubextbg, procmode716tsubextbgb, procmode716tsubextbg2,
    procmode716tmain, procmode716tmainextbg, procmode716tmainextbgb,
    procmode716tmainextbg2, procspritessub16t, procspritesmain16t,
    procspritessub16tfix, procspritesmain16tfix, drawbackgrndsub16t,
    drawbackgrndmain16t, drawbackgrndsub16tfix, drawbackgrndmain16tfix };
/* RENDERER emits the symbol in assembly, so C needs telling it exists. */
void drawmode716t(void), drawmode716b(void), drawmode716tb(void);
void drawmode716extbg(void), drawmode716textbg(void);
void drawmode716extbg2(void), drawmode716textbg2(void);
void drawsprites16t_stub(void), drawsprites16bt_stub(void);
void draw8x816t_stub(void), draw16x1616t_stub(void), draw8x816bt_stub(void);
void draw16x1616bt_stub(void), draw8x816tms(void), draw16x1616tms(void);
static void (*const bg_renderer[])(void) = { 0, draw8x816t_stub,
    draw16x1616t_stub, draw8x816bt_stub, draw16x1616bt_stub, draw8x816tms,
    draw16x1616tms };

static void (*const m7_renderer[])(void) = { 0, drawmode716t, drawmode716b,
    drawmode716tb, drawmode716extbg, drawmode716textbg, drawmode716extbg2,
    drawmode716textbg2 };

/* Every gate is C now; c_side is gone with them. */
static void (*const a_side[ROUTINES])(void) = { asm_procmode716tsub,
    asm_procmode716tsubextbg, asm_procmode716tsubextbgb,
    asm_procmode716tsubextbg2, asm_procmode716tmain,
    asm_procmode716tmainextbg, asm_procmode716tmainextbgb,
    asm_procmode716tmainextbg2, asm_procspritessub16t,
    asm_procspritesmain16t, asm_procspritessub16tfix,
    asm_procspritesmain16tfix,
    asm_drawbackgrndsub16t, asm_drawbackgrndmain16t,
    asm_drawbackgrndsub16tfix, asm_drawbackgrndmain16tfix };

typedef struct {
    u4 which, eax, ebx, ecx, edx, esi, edi, ebp;
    u4 calls, al, layer;
    u4 oax, obx, ocx, odx, osi, odi, obp;
    u1 winon, curmos, done;
    u2 starty;
    u4 dscalls, dscl, dsebp, sprloc;
    u4 bgbcalls, bgbwhich, tail;
    u1 adrawn, coladd;
} snapshot;

static u1* sprloc0;
static u1 adrawn0, drawn0;

static void reset(u4 const* const in, u1 const done0)
{
    rn_which = rn_eax = rn_ebx = rn_ecx = rn_edx = rn_esi = rn_edi = rn_ebp = 0;
    mw_calls = mw_al = mw_layer = 0;
    ds_calls = ds_cl = ds_ebp = 0;
    bgb_calls = bgb_which = 0;
    alreadydrawn = adrawn0;
    drawn = drawn0;
    extbgdone = done0;
    /* The routines advance this, so it has to be put back before the second
       run or that side reads a different scanline's count byte. */
    cursprloc = sprloc0;
    winon = 0xEE;
    curmosaicsz = 0xEE;
    m7starty = 0xEEEE;

    rg_eax = in[0];
    rg_ebx = in[1];
    rg_ecx = in[2];
    rg_edx = in[3];
    rg_esi = in[4];
    rg_edi = in[5];
    rg_ebp = in[6];
}

static void snap(snapshot* const out);

static void run(void (*fn)(void), u4 const* const in, u1 const done0,
    snapshot* const out)
{
    reset(in, done0);
    if (fn == 0) { /* a short initialiser list left a hole here once */
        fprintf(stderr, "difftest: routine table has a hole\n");
        exit(1);
    }
    rg_fn = (u4)(uintptr_t)fn;
    dt_call();
    snap(out);
}

/* The ported gates: same reset, but they take the registers as a struct and
   return the tail id, so the renderer is reached from here. */
static void run_port(u4 const idx, u4 const* const in, u1 const done0,
    snapshot* const out)
{
    m7regs r;
    u4 tail;

    reset(in, done0);
    r.ax = in[0];
    r.bx = in[1];
    r.cx = in[2];
    r.dx = in[3];
    r.si = in[4];
    r.di = in[5];
    r.bp = in[6];

    tail = port_gate[idx](&r);

    rg_eax = r.ax;
    rg_ebx = r.bx;
    rg_ecx = r.cx;
    rg_edx = r.dx;
    rg_esi = r.si;
    rg_edi = r.di;
    rg_ebp = r.bp;
    if (idx >= 12u) { /* background: call the renderer, then the second half */
        if (tail != 0) {
            if (tail <= 6u) {
                rg_fn = (u4)(uintptr_t)bg_renderer[tail];
                dt_call();
            }
            r.ax = rg_eax;
            r.bx = rg_ebx;
            r.cx = rg_ecx;
            r.dx = rg_edx;
            r.si = rg_esi;
            r.di = rg_edi;
            r.bp = rg_ebp;
            drawbackgrnd_mark(&r);
            rg_eax = r.ax;
            rg_ebx = r.bx;
            rg_ecx = r.cx;
            rg_edx = r.dx;
            rg_esi = r.si;
            rg_edi = r.di;
            rg_ebp = r.bp;
        }
    } else if (tail != 0) {
        rg_fn = (u4)(uintptr_t)(idx < 8u
                ? m7_renderer[tail]
                : (tail == 1u ? drawsprites16t_stub : drawsprites16bt_stub));
        dt_call();
    }
    snap(out);
}

static void snap(snapshot* const out)
{
    out->which = rn_which ? rn_which : bgb_which;
    out->eax = rn_eax;
    out->ebx = rn_ebx;
    out->ecx = rn_ecx;
    out->edx = rn_edx;
    out->esi = rn_esi;
    out->edi = rn_edi;
    out->ebp = rn_ebp;
    out->calls = mw_calls;
    out->al = mw_al;
    out->layer = mw_layer;
    out->oax = rg_eax;
    out->obx = rg_ebx;
    out->ocx = rg_ecx;
    out->odx = rg_edx;
    out->osi = rg_esi;
    out->odi = rg_edi;
    out->obp = rg_ebp;
    out->winon = winon;
    out->curmos = curmosaicsz;
    out->done = extbgdone;
    out->starty = m7starty;
    out->dscalls = ds_calls;
    out->dscl = ds_cl;
    out->dsebp = ds_ebp;
    out->sprloc = (u4)(uintptr_t)cursprloc;
    out->tail = BGTail;
    out->bgbcalls = bgb_calls;
    out->bgbwhich = bgb_which;
    out->adrawn = alreadydrawn;
    out->coladd = bgcoloradder;
}

int main(void)
{
    static u1 sprbuf[512];
    static u1 colmode[8];
    /* Wide enough for every renderer id, including the background and sprite
       ones - a short row here overran into the next and crashed. */
    long cov[ROUTINES][24];
    long drew[ROUTINES];
    memset(cov, 0, sizeof cov);
    memset(drew, 0, sizeof drew);

    DT_MAIN(20260803, 200000)
    {
        snapshot x, y;
        u4 in[7];
        u4 const r = dt_mod(ROUTINES);
        u1 const done0 = (u1)dt_mod(2);

        scrnon = (u2)(dt_mod(2) ? dt_mod(0x404u) : dt_u32());
        winenabm = (u1)dt_mod(2);
        winenabs = (u1)dt_mod(2);
        winbg1en = (u1)dt_u32();
        /* 0xFF abandons the line; anything else carries on. */
        mw_next = dt_mod(3) ? 0 : 0xFFu;
        mode7set = (u1)dt_u32();
        scaddset = (u1)dt_u32();
        scaddtype = (u1)dt_u32();
        mosaicon = (u1)dt_mod(2);
        /* The divide is `div bx` with bx = mosaicsz + 1; 0xFF would wrap it to
           zero and fault the oracle, which the PPU's 4-bit field cannot do. */
        mosaicsz = (u1)dt_mod(16);
        curypos = (u2)(dt_mod(2) ? dt_mod(240) : dt_u32());
        bg1scrolx_m7 = (u2)dt_u32();
        bg1scroly_m7 = (u2)dt_u32();
        scrndis = (u1)dt_u32();
        winonsp = (u1)(dt_mod(3) ? dt_mod(2) : 0xFFu);
        sprprifix = (u1)dt_mod(2);
        bgfixer = (u1)dt_mod(2);
        /* The sprite line buffer. drawsprites16t and drawsprites16bt are
           inside makev16t.asm and so exist in both objects; keeping the count
           at zero on the paths that would reach them means neither side runs
           real sprite code, which this test does not set up. */
        sprloc0 = sprbuf;
        memset(sprbuf, 0, sizeof sprbuf);
        if (dt_mod(2)) {
            sprbuf[(u1)curypos] = (u1)(1u + dt_mod(255));
            /* drawsprites16t and drawsprites16bt are stubbed by
               mkoracle --stub-routine now, so both tails are reachable. */
        }

        /* Background inputs. ebp is the layer, and curbgnum is its bit. */
        {
            u4 const layer = dt_mod(4);
            curbgnum = (u1)(1u << layer);
            curbgpr = (u1)(dt_mod(2) ? 0x00u : 0x20u);
            bgmode = (u1)dt_mod(8);
            bgtilesz = (u1)dt_u32();
            adrawn0 = (u1)(dt_mod(2) ? 0 : dt_u32());
            drawn0 = (u1)(dt_mod(2) ? 33 : dt_mod(40));
            colormodeofs = colmode;
            for (u4 i = 0; i < 4; i++) {
                colmode[i] = (u1)(dt_mod(3) ? (1u + dt_mod(255)) : 0);
                winen[i] = (u1)dt_u32();
                bg1vbufloc[i] = dt_u32();
                bg1xposloc[i] = dt_u32();
                bg1yaddval[i] = dt_u32();
                bg1tdatloc[i] = (u2*)(uintptr_t)dt_u32();
                bg1tdabloc[i] = (u2*)(uintptr_t)dt_u32();
                bg1cachloc[i] = (u1*)(uintptr_t)dt_u32();
            }
            for (u4 i = 0; i < 7; i++) {
                in[i] = dt_u32();
            }
            in[6] = layer; /* ebp */
        }

        run(a_side[r], in, done0, &x);
        run_port(r, in, done0, &y);
        cov[r][x.which < 24 ? x.which : 0]++;
        if (x.dscalls) {
            drew[r]++;
        }

        DT_EQ("renderer", x.which, y.which);
        DT_EQ("eax at renderer", x.eax, y.eax);
        DT_EQ("ebx at renderer", x.ebx, y.ebx);
        DT_EQ("ecx at renderer", x.ecx, y.ecx);
        DT_EQ("edx at renderer", x.edx, y.edx);
        DT_EQ("esi at renderer", x.esi, y.esi);
        DT_EQ("edi at renderer", x.edi, y.edi);
        DT_EQ("ebp at renderer", x.ebp, y.ebp);
        DT_EQ("makewindow calls", x.calls, y.calls);
        DT_EQ("makewindow al", x.al, y.al);
        DT_EQ("makewindow layer", x.layer, y.layer);
        DT_EQ("eax out", x.oax, y.oax);
        DT_EQ("ebx out", x.obx, y.obx);
        DT_EQ("ecx out", x.ocx, y.ocx);
        DT_EQ("edx out", x.odx, y.odx);
        DT_EQ("esi out", x.osi, y.osi);
        DT_EQ("edi out", x.odi, y.odi);
        DT_EQ("ebp out", x.obp, y.obp);
        DT_EQ("winon", x.winon, y.winon);
        DT_EQ("curmosaicsz", x.curmos, y.curmos);
        DT_EQ("extbgdone", x.done, y.done);
        DT_EQ("m7starty", x.starty, y.starty);
        DT_EQ("drawsprites16b calls", x.dscalls, y.dscalls);
        DT_EQ("drawsprites16b cl", x.dscl, y.dscl);
        DT_EQ("drawsprites16b ebp", x.dsebp, y.dsebp);
        DT_EQ("cursprloc", x.sprloc, y.sprloc);
        DT_EQ("bg renderer calls", x.bgbcalls, y.bgbcalls);
        DT_EQ("bg renderer which", x.bgbwhich, y.bgbwhich);
        DT_EQ("alreadydrawn", x.adrawn, y.adrawn);
        DT_EQ("bgcoloradder", x.coladd, y.coladd);
        if (dt_bad && DT_SHOW()) {
            printf("  ^ %s scrnon=%04x winm=%u wins=%u mos=%u/%u y=%u "
                   "m7set=%02x scadd=%02x/%02x done0=%u\n",
                names[r], scrnon, winenabm, winenabs, mosaicon, mosaicsz,
                curypos, mode7set, scaddset, scaddtype, done0);
            printf("    BGTail a=%u c=%u bgn=%02x tilesz=%02x mode=%u "
                   "adrawn0=%02x colmode=%02x\n",
                x.tail, y.tail, curbgnum,
                bgtilesz, bgmode, adrawn0, colmode[in[6] & 3u]);
            printf("    in eax=%08x mw_next=%02x calls a=%u c=%u winon a=%02x "
                   "c=%02x which a=%u c=%u\n",
                in[0], mw_next, x.calls,
                y.calls, x.winon, y.winon, x.which, y.which);
        }
    }
    for (u4 i = 0; i < ROUTINES; i++) {
        printf("  %-24s off=%ld", names[i], cov[i][0]);
        for (u4 j = 1; j < 24; j++) {
            if (cov[i][j]) {
                printf(" r%u=%ld", j, cov[i][j]);
            }
        }
        printf(" drew=%ld\n", drew[i]);
    }
    DT_DONE("makev16t scanline gates (8 mode7, 4 sprite, 4 background)");
}
