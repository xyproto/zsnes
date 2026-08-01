/*
 * video/c_mode716ext2.c - drawmode7ngextbg216b, ported from video/mode716.asm.
 *
 * The EXTBG second pass. drawmode7ngextbg16b stashed one priority byte per
 * pixel a whole buffer on (+75036*8); this walks those 256 bytes and repaints
 * the ones with bit 7 set, so they land in front of what was drawn since.
 *
 * Reached by call from video/newgfx16.mac with the renderer's registers live:
 * ebx is the scanline and ebp the palette. esi is not an input - the assembly
 * loads it from curvidoffset on every path. See the thunk in mode716.asm.
 */
#include <stdint.h>

#include "../types.h"

#define EXT2_PIXELS 256u
#define EXT2_BUF 75036u /* one video buffer, in 16-bit pixels */

extern u4 M7SeamA, M7SeamB, M7SeamC, M7SeamD, M7SeamSI, M7SeamBP;

extern u1 scrndis; /* cpu/regs.inc */
extern u1 BGMS1[]; /* endmem.c: main/sub enable, two bytes per scanline */
extern u1 scadtng[256]; /* endmem.c: colour add/sub enable */
extern u1 FillSubScr[256]; /* endmem.c */
extern u4 UnusedBitXor[2]; /* video/newgfx16.asm */
extern u1* curvidoffset; /* video/makevid.c */

/* Five distinct pixel writers. The assembly has seven macros because two pairs
   (Normal/Normalnt and Normalst/Normalsnt) are byte-identical and exist only to
   give each dispatch arm its own name. */
enum ext2_mode {
    EXT2_MAIN, /* ExtBGNormal, ExtBGNormalnt   */
    EXT2_MAIN_T, /* ExtBGNormalt                 */
    EXT2_SUB, /* ExtBGNormalst, ExtBGNormalsnt */
    EXT2_BOTH_T, /* ExtBGNormalmst               */
    EXT2_BOTH /* ExtBGNormalmsnt              */
};

/* Returns what dx is left holding: the writers are 16-bit moves, so the top
   half of the caller's edx survives and has to be merged back. */
static u2 ext2_run(enum ext2_mode const mode, u2 const* const pal, u2 dx)
{
    u1* p = curvidoffset;

    for (u4 n = EXT2_PIXELS; n != 0; n--, p += 2) {
        u1 const prio = p[EXT2_BUF * 8];
        if (!(prio & 0x80u)) {
            continue;
        }
        u4 const i = prio & 0x7Fu;
        switch (mode) {
        case EXT2_MAIN:
            dx = pal[i];
            *(u2*)p = dx;
            break;
        case EXT2_MAIN_T:
            dx = pal[i + 256];
            *(u2*)p = dx;
            break;
        case EXT2_SUB:
            dx = pal[i];
            *(u2*)(p + EXT2_BUF * 2) = dx;
            break;
        case EXT2_BOTH_T:
            dx = pal[i + 256];
            *(u2*)p = dx;
            dx = (u2)(dx & UnusedBitXor[0]);
            *(u2*)(p + EXT2_BUF * 2) = dx;
            break;
        case EXT2_BOTH:
            dx = pal[i];
            *(u2*)p = dx;
            *(u2*)(p + EXT2_BUF * 2) = dx;
            break;
        }
    }
    return dx;
}

void c_drawmode7ngextbg216b(void)
{
    u4 const bx = M7SeamB;
    enum ext2_mode mode;

    if (scrndis & 1u) {
        return; /* the assembly returns here without touching a register */
    }
    /* The `cmp byte[mode7hr+ebx],1` at this point in the assembly feeds a jump
       that is commented out, and nothing below reads its flags. */

    if ((BGMS1[bx * 2] & 3u) && (FillSubScr[bx] & 1u)) {
        u4 const sub = BGMS1[bx * 2 + 1] & 1u;
        u4 const main = BGMS1[bx * 2] & 1u;
        if (scadtng[bx] & 1u) {
            mode = !sub ? EXT2_MAIN_T : main ? EXT2_BOTH_T : EXT2_SUB;
        } else {
            mode = !sub ? EXT2_MAIN : main ? EXT2_BOTH : EXT2_SUB;
        }
    } else {
        /* The assembly adds a buffer to esi before this arm, which ExtBG2 then
           overwrites from curvidoffset - dead either way. */
        mode = EXT2_MAIN;
    }

    M7SeamD = (M7SeamD & 0xFFFF0000u)
        | ext2_run(mode, (u2 const*)(uintptr_t)M7SeamBP, (u2)M7SeamD);
    M7SeamA = 0;
    M7SeamC = 0;
    M7SeamSI = (u4)(uintptr_t)(curvidoffset + EXT2_PIXELS * 2u);
}
