/*
 * video/c_mode716bw.c - ProcessBuildWindow, ported from video/mode716.mac.
 *
 * Builds the per-scanline window run list the ProcessMode7ngwin*16b walk then
 * consumes. All four call sites instantiate the macro with 0, so the layer
 * offset it took as an argument is folded away here.
 *
 * Reached by call with ebx = the scanline. The macro in mode716.mac keeps the
 * ngwinen clear and the enable test, because the original touched no register
 * at all when it did not window - and eax and edx are the coordinates
 * Mode7Startup16b reads immediately afterwards.
 *
 * NOTE - one deliberate difference from the assembly. BuildWindow became a
 * cdecl C function (video/c_makev16b.c) in an earlier port, but this call site
 * was left reaching it by the old register ABI, so it was passing its saved
 * ebx and eax as the two arguments instead of the scanline twice. The second
 * argument indexes winbg1enval[], i.e. the wrong scanline's window enable.
 * This passes what the assembly meant to; see difftest_m7bw.c, which pins both
 * behaviours so the difference stays visible.
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
