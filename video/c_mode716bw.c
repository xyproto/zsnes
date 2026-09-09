/*
 * ProcessBuildWindow, from video/mode716.mac: builds the per-scanline window
 * run list the ProcessMode7ngwin*16b walk consumes. All four call sites pass a
 * layer offset of 0, so that argument is folded away. Entered with ebx = the
 * scanline; eax and edx come back as the coordinates Mode7Startup16b reads.
 *
 * One deliberate difference from the assembly: this call site used to reach
 * BuildWindow by the old register ABI and passed its saved ebx and eax rather
 * than the scanline twice, so the second argument indexed winbg1enval[] at the
 * wrong scanline. This passes what the assembly meant; difftest_m7bw.c pins
 * both behaviours.
 */
#include <stdint.h>

#include "../types.h"

zreg M7BWBX;

extern u4 nglogicval; /* video/c_makev16b.c */
extern u4 ngwintable[];
extern u4* ngcwinptr;
extern u1 winlogicaval[]; /* endmem.c */

void BuildWindow(u4 eax, u4 ebx); /* video/c_makev16b.c */

void c_ProcessMode7BuildWindow(void)
{
    u4 const bx = M7BWBX;

    /* A byte write into a dword slot: BuildWindow reads the whole thing. */
    nglogicval = (nglogicval & ~0xFFu) | (winlogicaval[bx * 2] & 0x03u);
    BuildWindow(bx, bx);
    if (ngwintable[0] != 0) {
        ngwintable[0]--;
    } else {
        ngwintable[1]--;
    }
    ngcwinptr = ngwintable;
}
