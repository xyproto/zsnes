/* Differential test: StartDrawNewGfx16b and its screen clip in
 * video/newgfx16.asm against the C port in video/c_ngframe.c.
 *
 * The routine is a running order: for each layer and priority pass it decides
 * whether to draw and calls one of three workers, all of which are C already.
 * So what has to match is the *sequence* of those calls with their arguments,
 * plus the clip's writes into the video buffer and the register pair it hands
 * to the colour-maths pass - `c_transp_halfsub` takes the caller's eax and
 * `c_transp_halfadd` its edx, upper halves included.
 *
 * The three workers and BuildWindow are recorded rather than run. Both sides
 * reach them cdecl, so one set of stand-ins serves both, and video/c_ngprocbg.c
 * is not linked at all - a dispatcher and a recorded branch target must not
 * share a translation unit.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;

enum { R_EDI,
    R_ESI,
    R_EBP,
    R_ESP,
    R_EBX,
    R_EDX,
    R_ECX,
    R_EAX };

extern u1 WindowRedraw, ngmsdraw, ngextbg, scrndis;
extern u1 modeused[], scadsng[], winbg1enval[], nglogicval;
extern u1 prdata[], prdatb[], prdatc[];
extern u1 winbg1envals[], winbg1envalm[];
extern u2 winlogicaval[], resolutn;
extern u4 endlinet, scfbl, bgcmsung, mode0ads;
extern u4 CMainWinScr, CSubWinScr, ngwinen, ngwintable[32];
extern u4 bg1totng, bg2totng, bg3totng, bg4totng;
extern u4 bg1drwng, bg2drwng, bg3drwng, bg4drwng;
extern u4 UnusedBit[2], UnusedBitXor[2];
extern u4 sprtbng[256];
extern u4 sprtlng[64];
extern u1* vidbuffer;

/* Not in the emulator objects this links. */
u1* spritetablea;

/* Clipping reaches the sub screen 75036 words on, and the last line starts
   239*576 bytes in. */
#define VB_SIZE (75036u * 2u + 240u * 576u + 1024u)
static u1 vbstore[VB_SIZE], vbtemplate[VB_SIZE];

/* --- the recorders ------------------------------------------------------- */
enum { REC_MAX = 1024 };
struct rec_t {
    u4 n;
    u4 what[REC_MAX], a[REC_MAX], b[REC_MAX], c[REC_MAX], d[REC_MAX];
    u4 e[REC_MAX], f[REC_MAX], g[REC_MAX];
};
static struct rec_t rec;

static u4 rec_ngwinen;

static void put(u4 const what, u4 const a, u4 const b, u4 const c, u4 const d,
    u4 const e, u4 const f, u4 const g)
{
    if (rec.n >= REC_MAX)
        return;
    rec.what[rec.n] = what;
    rec.a[rec.n] = a;
    rec.b[rec.n] = b;
    rec.c[rec.n] = c;
    rec.d[rec.n] = d;
    rec.e[rec.n] = e;
    rec.f[rec.n] = f;
    rec.g[rec.n] = g;
    rec.n++;
}

static u4 proc_id(void const* fn);

void c_procbg16b(u4 layer, void (*lineproc)(u4*), void (*tileproc)(u4*),
    u1 const* prdat, int main_, u4 mask, int kind);
void c_procbg16b(u4 const layer, void (*const lineproc)(u4*),
    void (*const tileproc)(u4*), u1 const* const prdat, int const main_,
    u4 const mask, int const kind)
{
    static u4* const tot[4] = { &bg1totng, &bg2totng, &bg3totng, &bg4totng };
    static u4* const drw[4] = { &bg1drwng, &bg2drwng, &bg3drwng, &bg4drwng };

    put(1, layer, proc_id(lineproc), proc_id(tileproc),
        (u4)(uintptr_t)prdat, (u4)main_, mask, (u4)kind);
    /* mode0ads and the two window-table pointers are set right before every
       one of these, so they belong in the record rather than in a separate
       comparison of their final value. */
    rec.g[rec.n - 1] ^= mode0ads;
    rec.e[rec.n - 1] ^= CMainWinScr ^ CSubWinScr;
    /* The real one counts the lines it drew, and the priority-1 passes are
       gated on that count differing from the total - and read the total into
       eax on the way past, which is where the colour-maths pass gets its
       upper half from. So the counts have to be big enough to reach above the
       low byte, and equal about half the time. */
    {
        u4 const h = rec.n * 2654435761u ^ layer * 40503u ^ (u4)kind * 7919u;

        *tot[layer & 3u] += 0x400u + (h >> 18);
        *drw[layer & 3u] += (h & 8u) ? 0x400u + (h >> 18) : 0x400u;
    }
}

void c_procspr16b(int main_, u4 mask, int modes);
void c_procspr16b(int const main_, u4 const mask, int const modes)
{
    put(2, (u4)main_, mask, (u4)modes, 0, 0, 0, 0);
}

void c_procmode7ng16b(int main_, u4 mask, int kind);
void c_procmode7ng16b(int const main_, u4 const mask, int const kind)
{
    put(3, (u4)main_, mask, (u4)kind, 0, 0, 0, 0);
}

void BuildWindow(u4 y, u4 idx);
/* A run list shaped like the real one: alternating lengths that between them
   cover the 256 pixels. The last entry is long enough that the walk always
   terminates - the assembly's walk has no bound and a table that runs out
   would spin off the end of it. Zero-length runs and runs wider than the whole
   line both have their own branch, so both have to turn up. */
static void fill_wintable(u4 seed)
{
    u4 k, h = seed * 2654435761u + 1u;

    memset(ngwintable, 0, sizeof ngwintable);
    for (k = 0; k < 8u; k++) {
        h = h * 1103515245u + 12345u;
        /* Spread across the 256-pixel line rather than clustering short:
           the walk branches at 256 and the budget runs out at 256, and a
           table that never reaches either tests neither. */
        ngwintable[k] = 1u + (h >> 13) % 320u;
    }
    h = h * 1103515245u + 12345u;
    if ((h >> 16) % 4u == 0)
        ngwintable[0] = 0;
    ngwintable[8] = 400;
}

void BuildWindow(u4 const y, u4 const idx)
{
    put(4, y, idx, nglogicval, 0, 0, 0, 0);
    ngwinen = rec_ngwinen;
    fill_wintable(rec.n * 7u + y);
}

/* The colour-maths pass: what it is handed is the point of the test. */
void c_process_transparencies(u4* r);
void c_process_transparencies(u4* const r)
{
    put(5, r[R_EAX], r[R_EDX], r[R_EBX], r[R_ECX], r[R_ESI], r[R_EDI],
        r[R_EBP]);
}

/* The sixteen dispatchers are only ever passed along as pointers here. */
#define STUB(n)    \
    void n(u4* r); \
    void n(u4* r) { (void)r; }
STUB(c_drawbg1line16b)
STUB(c_drawbg2line16b)
STUB(c_drawbg3line16b)
STUB(c_drawbg4line16b)
STUB(c_drawbg1tile16b)
STUB(c_drawbg2tile16b)
STUB(c_drawbg3tile16b)
STUB(c_drawbg4tile16b)
STUB(c_drawbg1linepr116b)
STUB(c_drawbg2linepr116b)
STUB(c_drawbg3linepr116b)
STUB(c_drawbg4linepr116b)
STUB(c_drawbg1tilepr116b)
STUB(c_drawbg2tilepr116b)
STUB(c_drawbg3tilepr116b)
STUB(c_drawbg4tilepr116b)

void asm_drawbg1line16b(void), asm_drawbg2line16b(void);
void asm_drawbg3line16b(void), asm_drawbg4line16b(void);
void asm_drawbg1tile16b(void), asm_drawbg2tile16b(void);
void asm_drawbg3tile16b(void), asm_drawbg4tile16b(void);
void asm_drawbg1linepr116b(void), asm_drawbg2linepr116b(void);
void asm_drawbg3linepr116b(void), asm_drawbg4linepr116b(void);
void asm_drawbg1tilepr116b(void), asm_drawbg2tilepr116b(void);
void asm_drawbg3tilepr116b(void), asm_drawbg4tilepr116b(void);

/* The oracle carries its own copies of the sixteen dispatchers under an asm_
   prefix, so the two sides pass different addresses for the same routine.
   Compare which one, not where it is. */
static u4 proc_id(void const* const fn)
{
    static void (*const c[16])(u4*) = { c_drawbg1line16b, c_drawbg2line16b,
        c_drawbg3line16b, c_drawbg4line16b, c_drawbg1tile16b,
        c_drawbg2tile16b, c_drawbg3tile16b, c_drawbg4tile16b,
        c_drawbg1linepr116b, c_drawbg2linepr116b, c_drawbg3linepr116b,
        c_drawbg4linepr116b, c_drawbg1tilepr116b, c_drawbg2tilepr116b,
        c_drawbg3tilepr116b, c_drawbg4tilepr116b };
    static void (*const a[16])(void) = { asm_drawbg1line16b,
        asm_drawbg2line16b, asm_drawbg3line16b, asm_drawbg4line16b,
        asm_drawbg1tile16b, asm_drawbg2tile16b, asm_drawbg3tile16b,
        asm_drawbg4tile16b, asm_drawbg1linepr116b, asm_drawbg2linepr116b,
        asm_drawbg3linepr116b, asm_drawbg4linepr116b, asm_drawbg1tilepr116b,
        asm_drawbg2tilepr116b, asm_drawbg3tilepr116b, asm_drawbg4tilepr116b };
    int k;

    for (k = 0; k < 16; k++)
        if (fn == (void const*)c[k] || fn == (void const*)a[k])
            return (u4)k + 1u;
    return 0xDEAD;
}

void asm_StartDrawNewGfx16b(void);
void c_startdrawnewgfx16b(u4* r);

/* The oracle takes and returns the whole register set, so it is driven the way
   the emulator drives this family - through the register bridge. */
unsigned int DLR[7];
void (*DLFN)(void);
void calldl16t(void);
__asm__(".text\n.globl calldl16t\ncalldl16t:\n"
        "  pushl %ebx\n  pushl %esi\n  pushl %edi\n  pushl %ebp\n"
        "  movl DLR+4, %ebx\n  movl DLR+8, %ecx\n  movl DLR+12, %edx\n"
        "  movl DLR+16, %esi\n movl DLR+20, %edi\n movl DLR+24, %ebp\n"
        "  movl DLR, %eax\n"
        "  call *DLFN\n"
        "  movl %eax, DLR\n   movl %ebx, DLR+4\n  movl %ecx, DLR+8\n"
        "  movl %edx, DLR+12\n movl %esi, DLR+16\n movl %edi, DLR+20\n"
        "  movl %ebp, DLR+24\n"
        "  popl %ebp\n  popl %edi\n  popl %esi\n  popl %ebx\n  ret\n");

static u4 IN[7];

static void run_asm(u4* const out)
{
    memcpy(DLR, IN, sizeof DLR);
    DLFN = asm_StartDrawNewGfx16b;
    calldl16t();
    out[R_EAX] = DLR[0];
    out[R_EBX] = DLR[1];
    out[R_ECX] = DLR[2];
    out[R_EDX] = DLR[3];
    out[R_ESI] = DLR[4];
    out[R_EDI] = DLR[5];
    out[R_EBP] = DLR[6];
}

static void run_c(u4* const out)
{
    out[R_EAX] = IN[0];
    out[R_EBX] = IN[1];
    out[R_ECX] = IN[2];
    out[R_EDX] = IN[3];
    out[R_ESI] = IN[4];
    out[R_EDI] = IN[5];
    out[R_EBP] = IN[6];
    out[R_ESP] = 0;
    c_startdrawnewgfx16b(out);
}

/* Everything the routine writes outside the video buffer. */
typedef struct {
    char const* name;
    void* p;
    u4 n;
} region;

static region const state[] = {
    { "WindowRedraw", &WindowRedraw, 1 },
    { "endlinet", &endlinet, 4 },
    { "CMainWinScr", &CMainWinScr, 4 },
    { "CSubWinScr", &CSubWinScr, 4 },
    { "mode0ads", &mode0ads, 4 },
    { "ngwinen", &ngwinen, 4 },
    { "nglogicval", &nglogicval, 1 },
    { "ngwintable", ngwintable, 128 },
    { "sprtbng", sprtbng, 1024 },
    { "sprtlng", sprtlng, 256 },
    { "bg1totng", &bg1totng, 4 },
    { "bg2totng", &bg2totng, 4 },
    { "bg3totng", &bg3totng, 4 },
    { "bg4totng", &bg4totng, 4 },
    { "bg1drwng", &bg1drwng, 4 },
    { "bg2drwng", &bg2drwng, 4 },
    { "bg3drwng", &bg3drwng, 4 },
    { "bg4drwng", &bg4drwng, 4 },
};

enum { NSNAP = (int)(sizeof state / sizeof state[0]),
    SNAP = 1500 };

static void snap(u1* const dest)
{
    u4 o = 0;
    int k;

    for (k = 0; k < NSNAP; k++) {
        memcpy(dest + o, state[k].p, state[k].n);
        o += state[k].n;
    }
}

/* Several of these are read-modify-write - endlinet is decremented, the
   counters are cleared - so the second side has to start where the first did. */
static void unsnap(u1 const* const src)
{
    u4 o = 0;
    int k;

    for (k = 0; k < NSNAP; k++) {
        memcpy(state[k].p, src + o, state[k].n);
        o += state[k].n;
    }
}

static u4 vb_sum(void)
{
    u4 h = 2166136261u, q;

    for (q = 0; q < VB_SIZE; q++)
        h = (h ^ vbstore[q]) * 16777619u;
    return h;
}

int main(void)
{
    u4 clipped = 0, entire = 0, windowed = 0, drawn = 0, blanked = 0;
    u4 q0;

    for (q0 = 0; q0 < VB_SIZE; q0++)
        vbtemplate[q0] = (u1)(q0 * 31u + 7u);
    vidbuffer = vbstore;
    spritetablea = vbstore;

    DT_MAIN(20260825, 4000)
    {
        u4 a[8], b[8], va, vb, q;
        static struct rec_t arec, brec;
        u1 sa[SNAP], sb[SNAP], seed[SNAP];

        /* One frame in eight is blanked before anything is drawn. */
        scfbl = dt_mod(8) == 0 ? dt_u32() : 0;
        scrndis = (u1)dt_mod(32);
        ngmsdraw = (u1)dt_mod(2);
        ngextbg = (u1)dt_mod(2);
        bgcmsung = dt_u32() & 0x1F1Fu;
        resolutn = (u2)(dt_mod(3) == 0 ? dt_mod(240) : 224);
        rec_ngwinen = dt_mod(2);
        for (q = 0; q < 8u; q++)
            modeused[q] = (u1)dt_mod(2);
        dt_fill(scadsng, 256);
        dt_fill((u1*)sprtlng, 256);
        dt_fill((u1*)winlogicaval, 512);
        for (q = 0; q < 6u * 256u; q++)
            winbg1enval[q] = (u1)(dt_mod(2) ? 0 : dt_mod(256));
        /* Big enough that the count reaches eax's upper half, and equal to
           the drawn count often enough to take both sides of the gate. */
        bg1totng = dt_u32() & 0xFFFFFu;
        bg2totng = dt_u32() & 0xFFFFFu;
        bg3totng = dt_u32() & 0xFFFFFu;
        bg4totng = dt_u32() & 0xFFFFFu;
        bg1drwng = dt_mod(2) ? bg1totng : dt_u32() & 0xFFFFFu;
        bg2drwng = dt_mod(2) ? bg2totng : dt_u32() & 0xFFFFFu;
        bg3drwng = dt_mod(2) ? bg3totng : dt_u32() & 0xFFFFFu;
        bg4drwng = dt_mod(2) ? bg4totng : dt_u32() & 0xFFFFFu;
        UnusedBit[0] = dt_u32();
        UnusedBitXor[0] = dt_u32();
        endlinet = dt_u32();
        WindowRedraw = 0;
        for (q = 0; q < 7u; q++)
            IN[q] = dt_u32();
        /* esi and edi are not read, but a wild value in them would hide a
           port that used one by accident. */

        snap(seed);
        memset(&rec, 0, sizeof rec);
        memcpy(vbstore, vbtemplate, VB_SIZE);
        run_asm(a);
        va = vb_sum();
        memcpy(&arec, &rec, sizeof rec);
        snap(sa);

        unsnap(seed);
        memset(&rec, 0, sizeof rec);
        memcpy(vbstore, vbtemplate, VB_SIZE);
        run_c(b);
        vb = vb_sum();
        memcpy(&brec, &rec, sizeof rec);
        snap(sb);

        {
            static char const* const rn[8] = { "edi", "esi", "ebp", "esp",
                "ebx", "edx", "ecx", "eax" };
            for (q = 0; q < 8u; q++)
                if (q != R_ESP)
                    DT_EQ(rn[q], a[q], b[q]);
        }
        DT_MEM("calls", (u1*)&arec, (u1*)&brec, sizeof rec);
        if (dt_bad && DT_SHOW()) {
            struct rec_t const* const A = &arec;
            struct rec_t const* const B = &brec;
            u4 k;

            printf("    n asm=%u c=%u\n", A->n, B->n);
            for (k = 0; k < A->n && k < B->n; k++)
                if (A->what[k] != B->what[k] || A->a[k] != B->a[k]
                    || A->b[k] != B->b[k] || A->c[k] != B->c[k]
                    || A->d[k] != B->d[k] || A->e[k] != B->e[k]
                    || A->f[k] != B->f[k] || A->g[k] != B->g[k]) {
                    printf("    #%u asm %u %u %u %u %u %u %u %u\n", k,
                        A->what[k], A->a[k], A->b[k], A->c[k], A->d[k],
                        A->e[k], A->f[k], A->g[k]);
                    printf("    #%u c   %u %u %u %u %u %u %u %u\n", k,
                        B->what[k], B->a[k], B->b[k], B->c[k], B->d[k],
                        B->e[k], B->f[k], B->g[k]);
                    break;
                }
        }
        DT_EQ("vidbuffer", va, vb);
        {
            u4 o = 0;
            int k;

            for (k = 0; k < NSNAP; k++) {
                DT_MEM(state[k].name, sa + o, sb + o, state[k].n);
                o += state[k].n;
            }
        }

        if (scfbl != 0) {
            blanked++;
        } else {
            drawn++;
            for (q = 0; q < rec.n; q++)
                if (rec.what[q] == 4)
                    windowed++;
            for (q = 1; q < 240u && q <= resolutn; q++) {
                u1 const s = scadsng[q];

                if ((s & 0xC0u) == 0xC0u || ((u1)(s << 2) & 0xC0u) == 0xC0u)
                    entire++;
                else if ((s & 0xC0u) || ((u1)(s << 2) & 0xC0u))
                    clipped++;
            }
        }
    }
    printf("  frames drawn %u, blanked %u; lines fully clipped %u, "
           "windowed %u, windows built %u\n",
        drawn, blanked, entire, clipped, windowed);
    DT_DONE("StartDrawNewGfx16b");
}
