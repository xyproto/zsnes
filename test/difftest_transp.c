/*
 * test/difftest_transp.c - the colour-maths loops of ProcessTransparencies
 * (video/newgfx16.asm) against video/c_ngtransp.c.
 *
 * These loops have no symbol of their own - they are labels inside a 342-line
 * routine - so there is nothing for mkoracle to extract. This is the no-oracle
 * fragment pattern (see difftest_sprwin.c): the assembly is transcribed
 * verbatim into inline asm here and the C is run against it on random screens.
 *
 * It exists because the whole-emulator A/B cannot settle this. Comparing an
 * unmodified tree against itself, zab.sh's per-frame pixel hashes report DIFFER
 * roughly one run in three, so a single DIFFER means nothing and a single SAME
 * means little. This does not have that problem.
 */
#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;

/* The sub screen, in u2 units from the main one. */
#define SUBOFF 75036u
#define LINE 256u

u2 fulladdtab[65536];
u4 UnusedBit[2], HalfTrans[2];

#include "../video/c_ngtransp.c"

static u2 bufC[SUBOFF + LINE * 2], bufA[SUBOFF + LINE * 2];

/* .fulltransp / .nextfa, transcribed from the assembly. */
static void asm_fulladd(u2* p, u4 ebx, u4 ebp, u4* pa, u4* pd)
{
    u4 ecx = LINE, eax = 0, edx = 0;
    __asm__ volatile("1:\n\t"
                     "movw (%%esi), %%ax\n\t"
                     "testw %%bx, %%ax\n\t"
                     "jz 2f\n\t"
                     "movw 150072(%%esi), %%dx\n\t"
                     "andl %[hm], %%eax\n\t"
                     "andl %[hm], %%edx\n\t"
                     "addl %%eax, %%edx\n\t"
                     "shrl $1, %%edx\n\t"
                     "movw fulladdtab(,%%edx,2), %%dx\n\t"
                     "movw %%dx, (%%esi)\n\t"
                     "2:\n\t"
                     "addl $2, %%esi\n\t"
                     "decl %%ecx\n\t"
                     "jnz 1b\n\t"
                     : "+a"(eax), "+c"(ecx), "+d"(edx), "+S"(p)
                     : "b"(ebx), [hm] "m"(ebp)
                     : "cc", "memory");
    *pa = eax;
    *pd = edx;
}

int main(void)
{
    u4 r[8];
    for (u4 i = 0; i < 65536u; i++)
        fulladdtab[i] = (u2)(i * 7919u + (i >> 3));

    DT_MAIN(4242, 3000)
    {
        u4 ea, ed;
        /* the real masks, plus occasional random ones so a port that hard-codes
           either the unused bit or the half-transparency mask is caught */
        u4 const ebp = dt_mod(4) ? 0xF7DEF7DEu : dt_u32();
        u4 const ebx = dt_mod(4) ? 0x0020u : (dt_u32() & 0xFFFFu);
        HalfTrans[0] = ebp;
        UnusedBit[0] = ebx;

        for (u4 i = 0; i < SUBOFF + LINE * 2; i++) {
            u2 const v = (u2)dt_u32();
            bufC[i] = bufA[i] = v;
        }

        memset(r, 0, sizeof r);
        r[R_ESI] = (u4)(uintptr_t)(bufC + 8);
        r[R_EBX] = 1;
        c_transp_fulladd(r);

        asm_fulladd(bufA + 8, 1u << 16 | ebx, ebp, &ea, &ed);
        /* the assembly leaves the line number in the top half of ebx; only the
           low half is ever tested, so feed it the same shape */

        DT_MEM("screen", bufA, bufC, sizeof bufA);
        DT_EQ("eax", ea, r[R_EAX]);
        DT_EQ("edx", ed, r[R_EDX]);
    }
    DT_DONE("ProcessTransparencies full add");
}
