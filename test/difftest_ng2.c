/*
 * test/difftest_ng2.c - video/newg162.asm's tile and line drawers.
 *
 * Nineteen of the twenty are unreachable by any ROM here, so this is the only
 * thing that can verify them. See test/ng2_harness.h for the calling sequence
 * and the state they need.
 *
 * KNOWN DEFECT (2026-08-18): comparing identical code against itself reports
 * six of the twenty routines - exactly the tile drawers - as differing, and
 * does so reproducibly, with or without ASLR, with a private draw buffer, and
 * with the same symbol called both times. Until that is explained this harness
 * cannot certify anything; treat every result from it as unverified. The
 * control to re-run is: `git stash` video/newg162.asm so both oracles are the
 * same assembly, and check for 20/20.
 *
 * Compares the pre-port assembly (asm_) against the current worktree (cur_),
 * which routes whichever leaves have been ported through C. A routine with no
 * leaf ported yet compares identical code against itself, which is a harness
 * check rather than a real comparison - so the count of *ported* leaves is what
 * to watch, not the pass line.
 *
 * Note what "drives cleanly" is worth. With tltype* left at zero all twenty
 * run - but zero selects the partial-tile path, so that is one path each and
 * the number means very little. Randomising tltype* over {0,1,2} exercises all
 * three and immediately shows eleven routines need state this harness does not
 * set up yet (the 16x16 line drawers and every offset-mode variant). The lower
 * number is the honest one; do not "fix" it by pinning tltype* back to zero.
 */
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#include "difftest.h"
#include "ng2_harness.h"

typedef uint8_t u1;
typedef uint32_t u4;

extern u1 BGMS1[], FillSubScr[], scadtng[], curmosaicsz, ngwinen;
extern u4 CMainWinScr, CSubWinScr;
/* The windowed leaves walk this table; WinClipMacro seeds ngcwinptr from it. */
extern u4 ngwintable[];

extern void asm_drawtileng2b16b(void);
extern void cur_drawtileng2b16b(void);
extern void asm_drawtileng4b16b(void);
extern void cur_drawtileng4b16b(void);
extern void asm_drawtileng8b16b(void);
extern void cur_drawtileng8b16b(void);
extern void asm_drawtileng16x162b16b(void);
extern void cur_drawtileng16x162b16b(void);
extern void asm_drawtileng16x164b16b(void);
extern void cur_drawtileng16x164b16b(void);
extern void asm_drawtileng16x168b16b(void);
extern void cur_drawtileng16x168b16b(void);
extern void asm_drawlineng2b16b(void);
extern void cur_drawlineng2b16b(void);
extern void asm_drawlineng4b16b(void);
extern void cur_drawlineng4b16b(void);
extern void asm_drawlineng8b16b(void);
extern void cur_drawlineng8b16b(void);
extern void asm_drawlineng16x162b16b(void);
extern void cur_drawlineng16x162b16b(void);
extern void asm_drawlineng16x164b16b(void);
extern void cur_drawlineng16x164b16b(void);
extern void asm_drawlineng16x168b16b(void);
extern void cur_drawlineng16x168b16b(void);
extern void asm_drawlineng16x84b16b(void);
extern void cur_drawlineng16x84b16b(void);
extern void asm_drawlineng16x82b16b(void);
extern void cur_drawlineng16x82b16b(void);
extern void asm_drawlinengom2b16b(void);
extern void cur_drawlinengom2b16b(void);
extern void asm_drawlinengom4b16b(void);
extern void cur_drawlinengom4b16b(void);
extern void asm_drawlinengom8b16b(void);
extern void cur_drawlinengom8b16b(void);
extern void asm_drawlinengom16x162b16b(void);
extern void cur_drawlinengom16x162b16b(void);
extern void asm_drawlinengom16x164b16b(void);
extern void cur_drawlinengom16x164b16b(void);
extern void asm_drawlinengom16x168b16b(void);
extern void cur_drawlinengom16x168b16b(void);

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

extern u1 ng2_vram[];
extern u4 ng2_leafhits[4];
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
    dt_fill(winbuf, sizeof winbuf);
    curmosaicsz = (u1)((sel & 1u) ? 1 : 2);
    BGMS1[2] = (u1)((BGMS1[2] & ~1u) | ((sel >> 1) & 1u));
    BGMS1[3] = (u1)((BGMS1[3] & ~1u) | ((sel >> 2) & 1u));
    scadtng[1] = (u1)((scadtng[1] & ~1u) | ((sel >> 3) & 1u));
    FillSubScr[1] = (u1)((FillSubScr[1] & ~1u) | ((sel >> 4) & 1u));
    ngwinen = (u1)((sel >> 5) & 1u);
    for (u4 k = 0; k < 64; k++)
        ngwintable[k] = dt_mod(2) ? 0 : (dt_mod(200) + 1);
    CMainWinScr = CSubWinScr = 0;
}

static void setup(void)
{
    curmosaicsz = (u1)(dt_mod(2) ? 1 : 2);
    /* Windows on for half the runs: the gating tree has windowed leaves with
       their own writers, and leaving ngwinen at zero never reaches them. */
    ngwinen = (u1)dt_mod(2);
    for (u4 k = 0; k < 64; k++)
        ngwintable[k] = dt_mod(2) ? 0 : (dt_mod(200) + 1);
    CMainWinScr = CSubWinScr = 0;
    dt_fill(BGMS1, 512);
    dt_fill(FillSubScr, 256);
    dt_fill(scadtng, 256);
    dt_fill(winbuf, sizeof winbuf);
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

static void curcall_drawtileng2b16b(void) { NG2_CALL(cur_drawtileng2b16b); }
static void curcall_drawtileng4b16b(void) { NG2_CALL(cur_drawtileng4b16b); }
static void curcall_drawtileng8b16b(void) { NG2_CALL(cur_drawtileng8b16b); }
static void curcall_drawtileng16x162b16b(void) { NG2_CALL(cur_drawtileng16x162b16b); }
static void curcall_drawtileng16x164b16b(void) { NG2_CALL(cur_drawtileng16x164b16b); }
static void curcall_drawtileng16x168b16b(void) { NG2_CALL(cur_drawtileng16x168b16b); }
static void curcall_drawlineng2b16b(void) { NG2_CALL(cur_drawlineng2b16b); }
static void curcall_drawlineng4b16b(void) { NG2_CALL(cur_drawlineng4b16b); }
static void curcall_drawlineng8b16b(void) { NG2_CALL(cur_drawlineng8b16b); }
static void curcall_drawlineng16x162b16b(void) { NG2_CALL(cur_drawlineng16x162b16b); }
static void curcall_drawlineng16x164b16b(void) { NG2_CALL(cur_drawlineng16x164b16b); }
static void curcall_drawlineng16x168b16b(void) { NG2_CALL(cur_drawlineng16x168b16b); }
static void curcall_drawlineng16x84b16b(void) { NG2_CALL(cur_drawlineng16x84b16b); }
static void curcall_drawlineng16x82b16b(void) { NG2_CALL(cur_drawlineng16x82b16b); }
static void curcall_drawlinengom2b16b(void) { NG2_CALL(cur_drawlinengom2b16b); }
static void curcall_drawlinengom4b16b(void) { NG2_CALL(cur_drawlinengom4b16b); }
static void curcall_drawlinengom8b16b(void) { NG2_CALL(cur_drawlinengom8b16b); }
static void curcall_drawlinengom16x162b16b(void) { NG2_CALL(cur_drawlinengom16x162b16b); }
static void curcall_drawlinengom16x164b16b(void) { NG2_CALL(cur_drawlinengom16x164b16b); }
static void curcall_drawlinengom16x168b16b(void) { NG2_CALL(cur_drawlinengom16x168b16b); }

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
            for (int it = 0; it < 256; it++) {
                dt_fill(ng2_vram, 4096);
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
                        _exit(0);
                    }
                    waitpid(c, NULL, 0);
                    if (k == 0)
                        memcpy(out_b, out_a, OUTSZ);
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
        } else {
            printf("  %-18s asm == worktree\n", routines[i].name);
        }
    }
    printf("  first diff at byte %u (SUB is %u), iteration %u\n",
        hits_shared[4], (unsigned)SUB_OFF, hits_shared[5]);
    printf("  leaf hits: nt=%u t=%u mst=%u msnt=%u\n", hits_shared[0],
        hits_shared[1], hits_shared[2], hits_shared[3]);
    printf("newg162: %zu/%zu routines match the assembly\n",
        sizeof routines / sizeof routines[0] - (size_t)bad,
        sizeof routines / sizeof routines[0]);
    return 0;
}
