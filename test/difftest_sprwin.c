/* Differential test: the sprite-window builder of newengine16b in
 * video/newgfx16.asm against the C port in video/c_ngline.c.
 *
 * This one is not built with mkoracle. The loop is a dozen instructions with
 * no symbol of its own - it sits in the middle of newengine16b - so instead
 * the original is transcribed verbatim into inline assembly below and the two
 * are run against the same run-length table.
 *
 * It is worth testing on its own because it is the trickiest arithmetic in the
 * file: the stores are dword-wide and overshoot the end of each run, and both
 * loop tests are *unsigned* borrows, so the run counter goes on being used
 * after it has gone negative. Only two of the local ROMs reach it at all.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "difftest.h"

typedef uint8_t u1;
typedef uint32_t u4;

#define BUFSZ 512u
#define TABSZ 32u

void ng_build_sprite_window(u1* dest);

/* The original, instruction for instruction. */
static void asm_build(u1* dest, u4 const* tab)
{
    __asm__ volatile("pushl %%ebx\n"
                     "  movl %0, %%ecx\n"
                     "  movl %1, %%ebx\n"
                     "  decl %%ecx\n"
                     "  movl $256, %%eax\n"
                     "1:\n"
                     "  movl (%%ebx), %%edx\n"
                     "  addl $4, %%ebx\n"
                     "  orl %%edx, %%edx\n"
                     "  jz 3f\n"
                     "  decl %%edx\n"
                     "2:\n"
                     "  movl $0, (%%ecx)\n"
                     "  addl $4, %%ecx\n"
                     "  subl $4, %%eax\n"
                     "  jc 5f\n"
                     "  subl $4, %%edx\n"
                     "  jnc 2b\n"
                     "  subl %%edx, %%eax\n"
                     "  addl %%edx, %%ecx\n"
                     "  decl %%eax\n"
                     "  incl %%ecx\n"
                     "3:\n"
                     "  movl (%%ebx), %%edx\n"
                     "  decl %%edx\n"
                     "  addl $4, %%ebx\n"
                     "4:\n"
                     "  movl $0x01010101, (%%ecx)\n"
                     "  addl $4, %%ecx\n"
                     "  subl $4, %%eax\n"
                     "  jc 5f\n"
                     "  subl $4, %%edx\n"
                     "  jnc 4b\n"
                     "  subl %%edx, %%eax\n"
                     "  addl %%edx, %%ecx\n"
                     "  decl %%eax\n"
                     "  incl %%ecx\n"
                     "  jmp 1b\n"
                     "5:\n"
                     "  popl %%ebx\n"
        :
        : "m"(dest), "m"(tab)
        : "eax", "ecx", "edx", "memory", "cc");
}

/* ngwintable is the real 32-entry global from video/newgfx.c; the emulator's
   own objects supply it and everything else c_ngline.c drags in. Only the
   builder is exercised here. */
extern u4 ngwintable[32];
u4 CSprWinPtr, ngwinptr;
uint16_t curypos;
u1* vbufdptr;
void BuildWindow2(u4 y, u4 idx);
void BuildWindow2(u4 y, u4 idx)
{
    (void)y;
    (void)idx;
}
void setpalette16bng(void);
void setpalette16bng(void) { }

static u1 abuf[BUFSZ], cbuf[BUFSZ];

int main(void)
{
    long runs = 0;

    DT_MAIN(20260814, 200000)
    {
        u4 total = 0;
        u4 n = 0;

        /* Alternating run lengths that cover the 256 pixels, the way
           BuildWindow2 leaves them - plus tables that stop short, so the
           builder runs off the end of the useful entries. */
        memset(ngwintable, 0, sizeof ngwintable);
        while (n < TABSZ - 2u && total < 300u) {
            u4 len;

            if (dt_mod(6) == 0) {
                len = 0; /* a zero-length run is skipped, not written */
            } else if (dt_mod(4) == 0) {
                len = dt_mod(300) + 1u;
            } else {
                len = dt_mod(20) + 1u;
            }
            ngwintable[n++] = len;
            total += len;
        }
        runs += n;

        memset(abuf, 0xCC, BUFSZ);
        memset(cbuf, 0xCC, BUFSZ);
        /* Both start one byte in, so the builder's deliberate dest-1 write
           lands inside the buffer. */
        asm_build(abuf + 8, ngwintable);
        ng_build_sprite_window(cbuf + 8);

        DT_MEM("window mask", abuf, cbuf, BUFSZ);
        if (dt_bad && DT_SHOW()) {
            printf("  ^ entries=%u total=%u\n", n, total);
        }
    }
    printf("  %ld runs generated\n", runs);
    DT_DONE("newgfx16 sprite window builder");
}
