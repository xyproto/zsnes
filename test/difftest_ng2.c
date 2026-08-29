/*
 * video/newg162.asm's tile and line drawers. Nineteen of the twenty are
 * unreachable by any ROM here, so this is the only thing that verifies them;
 * test/ng2_harness.h has the calling sequence and the state they need.
 *
 * Compares the pre-port assembly (asm_) against the worktree (cur_), which
 * routes ported leaves through C. A routine with no leaf ported yet compares
 * identical code against itself, so watch the `leaf hits` line: all zeros means
 * no C ran and the 20/20 is only the harness checking itself.
 *
 * Keep the control: point both oracles at the same assembly and confirm 20/20
 * before trusting a comparison.
 *
 * tltype* at zero lets all twenty run, but zero is the partial-tile path, so
 * that is one path each. Randomising it over {0,1,2} exercises all three and
 * shows eleven routines need state this harness does not set up yet. The lower
 * number is the honest one; do not pin tltype* back to zero to raise it.
 */
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../types.h"
#include "difftest.h"
#include "ng2_harness.h"

typedef uint8_t u1;
typedef uint32_t u4;

extern u1 BGMS1[], FillSubScr[], scadtng[], curmosaicsz, ngwinen, res640;
extern u4 CMainWinScr, CSubWinScr;
/* The windowed leaves walk this table; WinClipMacro seeds ngcwinptr from it. */
extern u4 ngwintable[];

extern void asm_drawtileng2b16b(void);
void c_ng_drawtileng2b16b(u4* r);
extern void asm_drawtileng4b16b(void);
void c_ng_drawtileng4b16b(u4* r);
extern void asm_drawtileng8b16b(void);
void c_ng_drawtileng8b16b(u4* r);
extern void asm_drawtileng16x162b16b(void);
void c_ng_drawtileng16x162b16b(u4* r);
extern void asm_drawtileng16x164b16b(void);
void c_ng_drawtileng16x164b16b(u4* r);
extern void asm_drawtileng16x168b16b(void);
void c_ng_drawtileng16x168b16b(u4* r);
extern void asm_drawlineng2b16b(void);
void c_ng_drawlineng2b16b(u4* r);
extern void asm_drawlineng4b16b(void);
void c_ng_drawlineng4b16b(u4* r);
extern void asm_drawlineng8b16b(void);
void c_ng_drawlineng8b16b(u4* r);
extern void asm_drawlineng16x162b16b(void);
void c_ng_drawlineng16x162b16b(u4* r);
extern void asm_drawlineng16x164b16b(void);
void c_ng_drawlineng16x164b16b(u4* r);
extern void asm_drawlineng16x168b16b(void);
void c_ng_drawlineng16x168b16b(u4* r);
extern void asm_drawlineng16x84b16b(void);
void c_ng_drawlineng16x84b16b(u4* r);
extern void asm_drawlineng16x82b16b(void);
void c_ng_drawlineng16x82b16b(u4* r);
extern void asm_drawlinengom2b16b(void);
void c_ng_drawlinengom2b16b(u4* r);
extern void asm_drawlinengom4b16b(void);
void c_ng_drawlinengom4b16b(u4* r);
extern void asm_drawlinengom8b16b(void);
void c_ng_drawlinengom8b16b(u4* r);
extern void asm_drawlinengom16x162b16b(void);
void c_ng_drawlinengom16x162b16b(u4* r);
extern void asm_drawlinengom16x164b16b(void);
void c_ng_drawlinengom16x164b16b(u4* r);
extern void asm_drawlinengom16x168b16b(void);
void c_ng_drawlinengom16x168b16b(u4* r);

/* The ms writers also store to edi+75036*2, so the buffer has to cover the sub
   screen as well - a short one puts those writes in whatever follows, which is
   different memory in each forked child and reads as non-determinism. */
#define SUB_OFF (75036u * 2u)
#define OUTSZ (SUB_OFF + 32768u)
/* Shared so a forked child's result reaches the parent. */
static u1* out_a;
static u1* out_b;
static u4* hits_shared;
static u1 winbuf[8192];

/* The window tables are mostly-zero in practice, and the gate branches on a
   byte being zero: filled with uniform random bytes, "no window here" comes up
   one time in 256, and the main-only and sub-only leaves are never reached at
   all. Half and half gets all four branches. */
static void fill_win(void)
{
    for (size_t k = 0; k < sizeof winbuf; k++)
        winbuf[k] = (u1)(dt_mod(2) ? 0 : (dt_mod(255) + 1));
}

extern u1 ng2_vram[];
extern u1 vrama[]; /* the tile map, ng2_harness.c */
extern u1 ng2_ofsbuf[]; /* the offset-per-tile table */
extern u4 ng2_leafhits[4];
extern u4 ng2_winhits[8];
extern u4 ng2_bighits[4];
extern u4 ng2_bigwinhits[8];
extern u4 ng2_linehits[4];
extern u4 ng2_linewinhits[8];
extern u4 ng2_line16hits[4];
extern u4 ng2_line16winhits[8];
extern u4 ng2_line168hits[6];
extern u4 ng2_lineomhits[4];
extern u4 ng2_lineomwinhits[8];
extern u4 ng2_lineom16hits[4];
extern u4 ng2_lineom16winhits[8];
extern u1 ng2_src2[], ng2_src4[], ng2_src8[];
extern u1 ng2_palette[];
/* tltype* selects full tile / partial tile / skip. The tile cache fills it,
   but leaving it to do so means only one of the three paths ever runs, so it
   is randomised here - both sides see the same values. */
extern u1 tltype2b[], tltype4b[], tltype8b[];
/* The gating tree keys off these, so leaving them zero pins every routine to
   one leaf. */
/*
 * With ebx = 1 and dl = 1 the gating tree turns on exactly five bits, so
 * enumerate them instead of sampling: random inputs hit the transparent leaf
 * about three times in a hundred, which is how a mutation there can look
 * "clean". `sel` walks all 32 combinations.
 */
static void setup_gated(u4 sel)
{
    dt_fill(BGMS1, 512);
    dt_fill(FillSubScr, 256);
    dt_fill(scadtng, 256);
    fill_win();
    curmosaicsz = (u1)((sel & 1u) ? 1 : 2);
    BGMS1[2] = (u1)((BGMS1[2] & ~1u) | ((sel >> 1) & 1u));
    BGMS1[3] = (u1)((BGMS1[3] & ~1u) | ((sel >> 2) & 1u));
    scadtng[1] = (u1)((scadtng[1] & ~1u) | ((sel >> 3) & 1u));
    FillSubScr[1] = (u1)((FillSubScr[1] & ~1u) | ((sel >> 4) & 1u));
    ngwinen = (u1)((sel >> 5) & 1u);
    /* Gates the 16x8 drawer's two-field path; left at zero it never runs. */
    res640 = (u1)((sel >> 6) & 1u);
    for (u4 k = 0; k < 64; k++)
        ngwintable[k] = dt_mod(2) ? 0 : (dt_mod(200) + 1);
    /* Two distinct tables, as the emulator has. Pointing both at the same
       byte makes c_determinewindow's second probe re-read its first, so it can
       only ever answer "both windows" and the main-only and sub-only leaves
       are unreachable - they showed zero hits until this split. */
    CMainWinScr = 0;
    CSubWinScr = 4096;
}

static void setup(void)
{
    curmosaicsz = (u1)(dt_mod(2) ? 1 : 2);
    /* Windows on for half the runs: the gating tree has windowed leaves with
       their own writers, and leaving ngwinen at zero never reaches them. */
    ngwinen = (u1)dt_mod(2);
    res640 = (u1)dt_mod(2);
    for (u4 k = 0; k < 64; k++)
        ngwintable[k] = dt_mod(2) ? 0 : (dt_mod(200) + 1);
    /* Two distinct tables, as the emulator has. Pointing both at the same
       byte makes c_determinewindow's second probe re-read its first, so it can
       only ever answer "both windows" and the main-only and sub-only leaves
       are unreachable - they showed zero hits until this split. */
    CMainWinScr = 0;
    CSubWinScr = 4096;
    dt_fill(BGMS1, 512);
    dt_fill(FillSubScr, 256);
    dt_fill(scadtng, 256);
    fill_win();
}

static void call_drawtileng2b16b(void) { NG2_CALL(asm_drawtileng2b16b); }
static void call_drawtileng4b16b(void) { NG2_CALL(asm_drawtileng4b16b); }
static void call_drawtileng8b16b(void) { NG2_CALL(asm_drawtileng8b16b); }
static void call_drawtileng16x162b16b(void) { NG2_CALL(asm_drawtileng16x162b16b); }
static void call_drawtileng16x164b16b(void) { NG2_CALL(asm_drawtileng16x164b16b); }
static void call_drawtileng16x168b16b(void) { NG2_CALL(asm_drawtileng16x168b16b); }
static void call_drawlineng2b16b(void) { NG2_CALL(asm_drawlineng2b16b); }
static void call_drawlineng4b16b(void) { NG2_CALL(asm_drawlineng4b16b); }
static void call_drawlineng8b16b(void) { NG2_CALL(asm_drawlineng8b16b); }
static void call_drawlineng16x162b16b(void) { NG2_CALL(asm_drawlineng16x162b16b); }
static void call_drawlineng16x164b16b(void) { NG2_CALL(asm_drawlineng16x164b16b); }
static void call_drawlineng16x168b16b(void) { NG2_CALL(asm_drawlineng16x168b16b); }
static void call_drawlineng16x84b16b(void) { NG2_CALL(asm_drawlineng16x84b16b); }
static void call_drawlineng16x82b16b(void) { NG2_CALL(asm_drawlineng16x82b16b); }
static void call_drawlinengom2b16b(void) { NG2_CALL(asm_drawlinengom2b16b); }
static void call_drawlinengom4b16b(void) { NG2_CALL(asm_drawlinengom4b16b); }
static void call_drawlinengom8b16b(void) { NG2_CALL(asm_drawlinengom8b16b); }
static void call_drawlinengom16x162b16b(void) { NG2_CALL(asm_drawlinengom16x162b16b); }
static void call_drawlinengom16x164b16b(void) { NG2_CALL(asm_drawlinengom16x164b16b); }
static void call_drawlinengom16x168b16b(void) { NG2_CALL(asm_drawlinengom16x168b16b); }

/* The current side is C: the seam it used to go through in
   video/newg162.asm built this block with pushad, and took the mosaic tail
   afterwards - which video/c_ngbg.c does for itself now. */
enum { R_EDI,
    R_ESI,
    R_EBP,
    R_ESP,
    R_EBX,
    R_EDX,
    R_ECX,
    R_EAX };

extern u4 ng2_mosaic;
void domosaicng16b(void);

static void cur_call(void (*const fn)(u4*))
{
    zreg r[8];

    r[R_EAX] = NG2_EAX;
    r[R_EBX] = NG2_EBX;
    r[R_ECX] = NG2_ECX;
    r[R_EDX] = NG2_EDX;
    r[R_ESI] = NG2_ESI;
    r[R_EDI] = NG2_EDI;
    r[R_EBP] = NG2_EBP;
    r[R_ESP] = 0;
    fn(r);
    if (ng2_mosaic != 0) {
        domosaicng16b();
    }
}

static void curcall_drawtileng2b16b(void) { cur_call(c_ng_drawtileng2b16b); }
static void curcall_drawtileng4b16b(void) { cur_call(c_ng_drawtileng4b16b); }
static void curcall_drawtileng8b16b(void) { cur_call(c_ng_drawtileng8b16b); }
static void curcall_drawtileng16x162b16b(void) { cur_call(c_ng_drawtileng16x162b16b); }
static void curcall_drawtileng16x164b16b(void) { cur_call(c_ng_drawtileng16x164b16b); }
static void curcall_drawtileng16x168b16b(void) { cur_call(c_ng_drawtileng16x168b16b); }
static void curcall_drawlineng2b16b(void) { cur_call(c_ng_drawlineng2b16b); }
static void curcall_drawlineng4b16b(void) { cur_call(c_ng_drawlineng4b16b); }
static void curcall_drawlineng8b16b(void) { cur_call(c_ng_drawlineng8b16b); }
static void curcall_drawlineng16x162b16b(void) { cur_call(c_ng_drawlineng16x162b16b); }
static void curcall_drawlineng16x164b16b(void) { cur_call(c_ng_drawlineng16x164b16b); }
static void curcall_drawlineng16x168b16b(void) { cur_call(c_ng_drawlineng16x168b16b); }
static void curcall_drawlineng16x84b16b(void) { cur_call(c_ng_drawlineng16x84b16b); }
static void curcall_drawlineng16x82b16b(void) { cur_call(c_ng_drawlineng16x82b16b); }
static void curcall_drawlinengom2b16b(void) { cur_call(c_ng_drawlinengom2b16b); }
static void curcall_drawlinengom4b16b(void) { cur_call(c_ng_drawlinengom4b16b); }
static void curcall_drawlinengom8b16b(void) { cur_call(c_ng_drawlinengom8b16b); }
static void curcall_drawlinengom16x162b16b(void) { cur_call(c_ng_drawlinengom16x162b16b); }
static void curcall_drawlinengom16x164b16b(void) { cur_call(c_ng_drawlinengom16x164b16b); }
static void curcall_drawlinengom16x168b16b(void) { cur_call(c_ng_drawlinengom16x168b16b); }

/* The routine draws into a private buffer, and the child copies the result to
   shared memory afterwards. Letting it write MAP_SHARED memory directly made
   two runs of the *same* symbol disagree. */
static u1 privbuf[OUTSZ + 65536u];

static void run(void (*thunk)(void), u1* out)
{
    u1* const draw = privbuf;
    memset(privbuf, 0xAA, sizeof privbuf);
    (void)out;
    ng2_reset();
    NG2_EAX = 0;
    NG2_EBX = 1;
    NG2_ECX = (u4)(uintptr_t)winbuf;
    NG2_EDX = 1;
    NG2_EBP = (u4)(uintptr_t)ng2_palette;
    NG2_EDI = (u4)(uintptr_t)draw;
    NG2_ESI = (u4)(uintptr_t)draw;
    thunk();
    memcpy(out, draw, OUTSZ);
}

int main(void)
{
    static const struct {
        char const* name;
        void (*thunk)(void);
        void (*curthunk)(void);
    } routines[] = {
        { "drawtileng2b16b", call_drawtileng2b16b, curcall_drawtileng2b16b },
        { "drawtileng4b16b", call_drawtileng4b16b, curcall_drawtileng4b16b },
        { "drawtileng8b16b", call_drawtileng8b16b, curcall_drawtileng8b16b },
        { "drawtileng16x162b16b", call_drawtileng16x162b16b, curcall_drawtileng16x162b16b },
        { "drawtileng16x164b16b", call_drawtileng16x164b16b, curcall_drawtileng16x164b16b },
        { "drawtileng16x168b16b", call_drawtileng16x168b16b, curcall_drawtileng16x168b16b },
        { "drawlineng2b16b", call_drawlineng2b16b, curcall_drawlineng2b16b },
        { "drawlineng4b16b", call_drawlineng4b16b, curcall_drawlineng4b16b },
        { "drawlineng8b16b", call_drawlineng8b16b, curcall_drawlineng8b16b },
        { "drawlineng16x162b16b", call_drawlineng16x162b16b, curcall_drawlineng16x162b16b },
        { "drawlineng16x164b16b", call_drawlineng16x164b16b, curcall_drawlineng16x164b16b },
        { "drawlineng16x168b16b", call_drawlineng16x168b16b, curcall_drawlineng16x168b16b },
        { "drawlineng16x84b16b", call_drawlineng16x84b16b, curcall_drawlineng16x84b16b },
        { "drawlineng16x82b16b", call_drawlineng16x82b16b, curcall_drawlineng16x82b16b },
        { "drawlinengom2b16b", call_drawlinengom2b16b, curcall_drawlinengom2b16b },
        { "drawlinengom4b16b", call_drawlinengom4b16b, curcall_drawlinengom4b16b },
        { "drawlinengom8b16b", call_drawlinengom8b16b, curcall_drawlinengom8b16b },
        { "drawlinengom16x162b16b", call_drawlinengom16x162b16b, curcall_drawlinengom16x162b16b },
        { "drawlinengom16x164b16b", call_drawlinengom16x164b16b, curcall_drawlinengom16x164b16b },
        { "drawlinengom16x168b16b", call_drawlinengom16x168b16b, curcall_drawlinengom16x168b16b },
    };
    int bad = 0;

    /* One mapping with PROT_NONE guards, so a write past either buffer faults
       here instead of quietly landing in the other one. Two plain mmaps can be
       placed adjacently, and then an overrun from the assembly run corrupts the
       C run's buffer and hides the very difference being looked for - which is
       exactly what happened, and it showed up as the result depending on
       whether ASLR was on. */
    {
        size_t const page = 4096u;
        size_t const span = (OUTSZ + page - 1u) / page * page;
        u1* const base = mmap(0, span * 2u + page * 3u, PROT_NONE,
            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        out_a = base + page;
        out_b = base + page * 2u + span;
        mprotect(out_a, span, PROT_READ | PROT_WRITE);
        mprotect(out_b, span, PROT_READ | PROT_WRITE);
    }
    hits_shared = mmap(0, 4096, PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    ng2_init();
    srand(99);
    /* Each routine runs in a child: one that still needs state the harness
       does not set up takes the whole process down otherwise, and the point
       here is to find out which those are. */
    for (size_t i = 0; i < sizeof routines / sizeof routines[0]; i++) {
        pid_t const pid = fork();
        int status = 0;
        if (pid == 0) {
            /* 1024, not 256: setup_gated cycles 32 gating combinations, and
               the windowed leaves below them need enough repeats of each to
               land on all three DetermineWindow answers. At 256 two of the
               eight came up empty from run to run. */
            for (int it = 0; it < 1024; it++) {
                dt_fill(ng2_vram, 4096);
                /* The tile map. Left zero this is one tile value repeated,
                   which is no flip bits and a single palette and tile index. */
                dt_fill(vrama, 65536);
                /* The offset table the om drawers walk. */
                dt_fill(ng2_ofsbuf, 1 << 16);
                /* the raw tile bitmaps the cache decodes from */
                dt_fill(ng2_src2, 65536);
                dt_fill(ng2_src4, 65536);
                dt_fill(ng2_src8, 65536);
                dt_fill(ng2_palette, 512);
                for (u4 k = 0; k < 4096; k++) {
                    u1 const t = (u1)dt_mod(3);
                    tltype2b[k] = tltype4b[k] = tltype8b[k] = t;
                }
                setup_gated((u4)it);
                /* One run per child, both forked from the same parent state:
                   these routines mutate the tile cache, the vidmemch maps and
                   several counters, so running twice in one process compares
                   the second run against the first one's leftovers. Forking
                   makes the starting state identical by construction. */
                /* Both runs write the *same* buffer: giving them different
                   ones makes the output buffer's address the only asymmetry
                   left, and these routines turn out to be sensitive to it.
                   The parent copies the result out between runs. */
                for (int k = 0; k < 2; k++) {
                    pid_t const c = fork();
                    if (c == 0) {
                        run(k ? routines[i].curthunk : routines[i].thunk, out_a);
                        for (int q = 0; q < 4; q++)
                            hits_shared[q] += ng2_leafhits[q];
                        for (int q = 0; q < 8; q++)
                            hits_shared[9 + q] += ng2_winhits[q];
                        for (int q = 0; q < 4; q++)
                            hits_shared[17 + q] += ng2_bighits[q];
                        for (int q = 0; q < 8; q++)
                            hits_shared[21 + q] += ng2_bigwinhits[q];
                        for (int q = 0; q < 4; q++)
                            hits_shared[30 + q] += ng2_linehits[q];
                        for (int q = 0; q < 8; q++)
                            hits_shared[34 + q] += ng2_linewinhits[q];
                        for (int q = 0; q < 4; q++)
                            hits_shared[42 + q] += ng2_line16hits[q];
                        for (int q = 0; q < 8; q++)
                            hits_shared[46 + q] += ng2_line16winhits[q];
                        for (int q = 0; q < 6; q++)
                            hits_shared[54 + q] += ng2_line168hits[q];
                        for (int q = 0; q < 4; q++)
                            hits_shared[60 + q] += ng2_lineomhits[q];
                        for (int q = 0; q < 8; q++)
                            hits_shared[64 + q] += ng2_lineomwinhits[q];
                        for (int q = 0; q < 4; q++)
                            hits_shared[72 + q] += ng2_lineom16hits[q];
                        for (int q = 0; q < 8; q++)
                            hits_shared[76 + q] += ng2_lineom16winhits[q];
                        if (k == 0) {
                            /* How much of the line the routine actually
                               painted. Zero means the harness never got it
                               drawing, and comparing it against itself proves
                               nothing - see the leaf-hit note above. */
                            u4 painted = 0, q;
                            for (q = 0; q < OUTSZ; q++)
                                if (out_a[q] != 0xAAu)
                                    painted++;
                            if (painted > hits_shared[29])
                                hits_shared[29] = painted;
                        }
                        hits_shared[6 + k] = ng2_mosaic_hits;
                        hits_shared[8] += ng2_mosaic_hits;
                        _exit(0);
                    }
                    waitpid(c, NULL, 0);
                    if (k == 0)
                        memcpy(out_b, out_a, OUTSZ);
                }
                /* The mosaic tail writes nothing here, so an unequal count
                   is a divergence the buffers cannot show. */
                if (hits_shared[6] != hits_shared[7]) {
                    hits_shared[4] = 0;
                    hits_shared[5] = (u4)it;
                    _exit(3);
                }
                if (memcmp(out_a, out_b, OUTSZ)) {
                    u4 o = 0;
                    while (o < OUTSZ && out_a[o] == out_b[o])
                        o++;
                    hits_shared[4] = o;
                    hits_shared[5] = (u4)it;
                    _exit(2);
                }
            }
            _exit(0);
        }
        waitpid(pid, &status, 0);
        if (WIFSIGNALED(status)) {
            printf("  %-18s needs more state (signal %d)\n",
                routines[i].name, WTERMSIG(status));
            bad++;
        } else if (WEXITSTATUS(status) == 2) {
            printf("  %-18s DIFFERS from the assembly\n", routines[i].name);
            bad++;
        } else if (WEXITSTATUS(status) == 3) {
            printf("  %-18s DIFFERS: mosaic tail taken %u vs %u times\n",
                routines[i].name, hits_shared[6], hits_shared[7]);
            bad++;
        } else {
            printf("  %-18s asm == worktree (max %u bytes painted)\n",
                routines[i].name, hits_shared[29]);
        }
        hits_shared[29] = 0;
    }
    printf("  first diff at byte %u (SUB is %u), iteration %u\n",
        hits_shared[4], (unsigned)SUB_OFF, hits_shared[5]);
    printf("  leaf hits: nt=%u t=%u mst=%u msnt=%u; mosaic tail %u\n",
        hits_shared[0], hits_shared[1], hits_shared[2], hits_shared[3],
        hits_shared[8]);
    printf("  windowed leaf hits: win=%u wint=%u mstmsw=%u msntmsw=%u "
           "mstmw=%u mstsw=%u msntmw=%u msntsw=%u\n",
        hits_shared[9], hits_shared[10], hits_shared[11], hits_shared[12],
        hits_shared[13], hits_shared[14], hits_shared[15], hits_shared[16]);
    printf("  16x16 leaf hits: nt=%u t=%u mst=%u msnt=%u\n", hits_shared[17],
        hits_shared[18], hits_shared[19], hits_shared[20]);
    printf("  16x16 windowed hits: win=%u wint=%u mstmsw=%u msntmsw=%u "
           "mstmw=%u mstsw=%u msntmw=%u msntsw=%u\n",
        hits_shared[21], hits_shared[22], hits_shared[23], hits_shared[24],
        hits_shared[25], hits_shared[26], hits_shared[27], hits_shared[28]);
    printf("  line leaf hits: nt=%u t=%u mst=%u msnt=%u\n", hits_shared[30],
        hits_shared[31], hits_shared[32], hits_shared[33]);
    printf("  line windowed hits: win=%u wint=%u mstmsw=%u msntmsw=%u "
           "mstmw=%u mstsw=%u msntmw=%u msntsw=%u\n",
        hits_shared[34], hits_shared[35], hits_shared[36], hits_shared[37],
        hits_shared[38], hits_shared[39], hits_shared[40], hits_shared[41]);
    printf("  16x16 line hits: nt=%u t=%u mst=%u msnt=%u\n", hits_shared[42],
        hits_shared[43], hits_shared[44], hits_shared[45]);
    printf("  16x16 line windowed: win=%u wint=%u mstmsw=%u msntmsw=%u "
           "mstmw=%u mstsw=%u msntmw=%u msntsw=%u\n",
        hits_shared[46], hits_shared[47], hits_shared[48], hits_shared[49],
        hits_shared[50], hits_shared[51], hits_shared[52], hits_shared[53]);
    printf("  16x8 line hits: nt=%u t=%u mst=%u msnt=%u; hi-res %u, plain %u\n",
        hits_shared[54], hits_shared[55], hits_shared[56], hits_shared[57],
        hits_shared[58], hits_shared[59]);
    printf("  offset-mode line hits: nt=%u t=%u mst=%u msnt=%u\n",
        hits_shared[60], hits_shared[61], hits_shared[62], hits_shared[63]);
    printf("  offset-mode windowed: win=%u wint=%u mstmsw=%u msntmsw=%u "
           "mstmw=%u mstsw=%u msntmw=%u msntsw=%u\n",
        hits_shared[64], hits_shared[65], hits_shared[66], hits_shared[67],
        hits_shared[68], hits_shared[69], hits_shared[70], hits_shared[71]);
    printf("  16x16 offset-mode hits: nt=%u t=%u mst=%u msnt=%u\n",
        hits_shared[72], hits_shared[73], hits_shared[74], hits_shared[75]);
    printf("  16x16 om windowed: win=%u wint=%u mstmsw=%u msntmsw=%u "
           "mstmw=%u mstsw=%u msntmw=%u msntsw=%u\n",
        hits_shared[76], hits_shared[77], hits_shared[78], hits_shared[79],
        hits_shared[80], hits_shared[81], hits_shared[82], hits_shared[83]);
    printf("newg162: %zu/%zu routines match the assembly\n",
        sizeof routines / sizeof routines[0] - (size_t)bad,
        sizeof routines / sizeof routines[0]);
    return 0;
}
