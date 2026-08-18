/*
 * test/difftest_ng2.c - video/newg162.asm's tile and line drawers.
 *
 * Nineteen of the twenty are unreachable by any ROM here, so this is the only
 * thing that can verify them. See test/ng2_harness.h for the calling sequence
 * and the state they need.
 *
 * Until a C port exists this checks the harness itself: that each routine runs
 * to completion and is deterministic given the same inputs. That is not a
 * comparison, and it is reported as a harness check rather than a PASS, but it
 * is what proves the setup is complete enough to compare against.
 *
 * Note what "drives cleanly" is worth. With tltype* left at zero all twenty
 * run - but zero selects the partial-tile path, so that is one path each and
 * the number means very little. Randomising tltype* over {0,1,2} exercises all
 * three and immediately shows eleven routines need state this harness does not
 * set up yet (the 16x16 line drawers and every offset-mode variant). The lower
 * number is the honest one; do not "fix" it by pinning tltype* back to zero.
 */
#include <sys/wait.h>
#include <unistd.h>

#include "difftest.h"
#include "ng2_harness.h"

typedef uint8_t u1;
typedef uint32_t u4;

extern u1 BGMS1[], FillSubScr[], scadtng[], curmosaicsz, ngwinen;
extern u4 CMainWinScr, CSubWinScr;

extern void asm_drawtileng2b16b(void);
extern void asm_drawtileng4b16b(void);
extern void asm_drawtileng8b16b(void);
extern void asm_drawtileng16x162b16b(void);
extern void asm_drawtileng16x164b16b(void);
extern void asm_drawtileng16x168b16b(void);
extern void asm_drawlineng2b16b(void);
extern void asm_drawlineng4b16b(void);
extern void asm_drawlineng8b16b(void);
extern void asm_drawlineng16x162b16b(void);
extern void asm_drawlineng16x164b16b(void);
extern void asm_drawlineng16x168b16b(void);
extern void asm_drawlineng16x84b16b(void);
extern void asm_drawlineng16x82b16b(void);
extern void asm_drawlinengom2b16b(void);
extern void asm_drawlinengom4b16b(void);
extern void asm_drawlinengom8b16b(void);
extern void asm_drawlinengom16x162b16b(void);
extern void asm_drawlinengom16x164b16b(void);
extern void asm_drawlinengom16x168b16b(void);

#define OUTSZ 16384u
static u1 out_a[OUTSZ], out_b[OUTSZ];
static u1 winbuf[8192];

extern u1 ng2_vram[];
extern u1 ng2_palette[];
/* tltype* selects full tile / partial tile / skip. The tile cache fills it,
   but leaving it to do so means only one of the three paths ever runs, so it
   is randomised here - both sides see the same values. */
extern u1 tltype2b[], tltype4b[], tltype8b[];

static void setup(void)
{
    curmosaicsz = 1;
    ngwinen = 0;
    CMainWinScr = CSubWinScr = 0;
    memset(BGMS1, 0, 512);
    memset(FillSubScr, 0, 256);
    memset(scadtng, 0, 256);
    memset(winbuf, 0, sizeof winbuf);
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

static void run(void (*thunk)(void), u1* out)
{
    memset(out, 0xAA, OUTSZ);
    NG2_EAX = 0;
    NG2_EBX = 1;
    NG2_ECX = (u4)(uintptr_t)winbuf;
    NG2_EDX = 1;
    NG2_EBP = (u4)(uintptr_t)ng2_palette;
    NG2_EDI = (u4)(uintptr_t)out;
    NG2_ESI = (u4)(uintptr_t)out;
    thunk();
}

int main(void)
{
    static const struct {
        char const* name;
        void (*thunk)(void);
    } routines[] = {
        { "drawtileng2b16b", call_drawtileng2b16b },
        { "drawtileng4b16b", call_drawtileng4b16b },
        { "drawtileng8b16b", call_drawtileng8b16b },
        { "drawtileng16x162b16b", call_drawtileng16x162b16b },
        { "drawtileng16x164b16b", call_drawtileng16x164b16b },
        { "drawtileng16x168b16b", call_drawtileng16x168b16b },
        { "drawlineng2b16b", call_drawlineng2b16b },
        { "drawlineng4b16b", call_drawlineng4b16b },
        { "drawlineng8b16b", call_drawlineng8b16b },
        { "drawlineng16x162b16b", call_drawlineng16x162b16b },
        { "drawlineng16x164b16b", call_drawlineng16x164b16b },
        { "drawlineng16x168b16b", call_drawlineng16x168b16b },
        { "drawlineng16x84b16b", call_drawlineng16x84b16b },
        { "drawlineng16x82b16b", call_drawlineng16x82b16b },
        { "drawlinengom2b16b", call_drawlinengom2b16b },
        { "drawlinengom4b16b", call_drawlinengom4b16b },
        { "drawlinengom8b16b", call_drawlinengom8b16b },
        { "drawlinengom16x162b16b", call_drawlinengom16x162b16b },
        { "drawlinengom16x164b16b", call_drawlinengom16x164b16b },
        { "drawlinengom16x168b16b", call_drawlinengom16x168b16b },
    };
    int bad = 0;

    ng2_init();
    srand(99);
    /* Each routine runs in a child: one that still needs state the harness
       does not set up takes the whole process down otherwise, and the point
       here is to find out which those are. */
    for (size_t i = 0; i < sizeof routines / sizeof routines[0]; i++) {
        pid_t const pid = fork();
        int status = 0;
        if (pid == 0) {
            for (int it = 0; it < 64; it++) {
                dt_fill(ng2_vram, 4096);
                dt_fill(ng2_palette, 512);
                for (u4 k = 0; k < 4096; k++) {
                    u1 const t = (u1)dt_mod(3);
                    tltype2b[k] = tltype4b[k] = tltype8b[k] = t;
                }
                setup();
                run(routines[i].thunk, out_a);
                run(routines[i].thunk, out_b);
                if (memcmp(out_a, out_b, OUTSZ))
                    _exit(2);
            }
            _exit(0);
        }
        waitpid(pid, &status, 0);
        if (WIFSIGNALED(status)) {
            printf("  %-18s needs more state (signal %d)\n",
                routines[i].name, WTERMSIG(status));
            bad++;
        } else if (WEXITSTATUS(status) == 2) {
            printf("  %-18s NOT deterministic\n", routines[i].name);
            bad++;
        } else {
            printf("  %-18s drives, deterministic\n", routines[i].name);
        }
    }
    printf("newg162 harness: %zu/%zu routines drive cleanly over all three "
           "tltype paths (no C port to compare against yet)\n",
        sizeof routines / sizeof routines[0] - (size_t)bad,
        sizeof routines / sizeof routines[0]);
    return 0;
}
