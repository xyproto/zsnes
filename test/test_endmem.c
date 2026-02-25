/*
 * test_endmem.c — verify endmem.asm buffer sizes and initial values
 *
 * These tests protect the future C replacement of endmem.asm by asserting
 * that buffer sizes and initial values match what the emulator expects.
 */

#include <stddef.h>
#include <string.h>

#include "zstest.h"

/* ── symbols from endmem.asm (declared via NEWSYM) ── */

extern unsigned char  wramdataa[65536];
extern unsigned char  ram7fa[65536];
extern unsigned int   opcjmptab[256];
extern unsigned int   tableA[256];
extern unsigned int   tableB[256];
extern unsigned int   tableC[256];
extern unsigned int   tableD[256];
extern unsigned int   tableE[256];
extern unsigned int   tableF[256];
extern unsigned int   tableG[256];
extern unsigned int   tableH[256];
extern unsigned int   SA1tableA[256];
extern unsigned int   SA1tableB[256];
extern unsigned int   SA1tableC[256];
extern unsigned int   SA1tableD[256];
extern unsigned int   SA1tableE[256];
extern unsigned int   SA1tableF[256];
extern unsigned int   SA1tableG[256];
extern unsigned int   SA1tableH[256];
extern unsigned char  vidmemch2[4096];
extern unsigned char  vidmemch4[4096];
extern unsigned char  vidmemch8[4096];
extern unsigned char  vidmemch2s[4096];
extern unsigned char  vidmemch4s[2048];
extern unsigned char  vidmemch8s[1024];
extern unsigned int   pal16b[256];
extern unsigned int   pal16bcl[256];
extern unsigned int   pal16bxcl[256];
extern unsigned char  sprlefttot[256];
extern unsigned char  sprcnt[256];
extern unsigned char  sprstart[256];
extern unsigned char  sprtilecnt[256];
extern unsigned char  sprend[256];
extern unsigned short sprendx[256];
extern unsigned char  sprpriodata[288];
extern unsigned char  winbgdata[288];
extern unsigned char  winspdata[288];
extern unsigned int   FxTable[256];
extern unsigned int   PLOTJmpa[64];
extern unsigned int   PLOTJmpb[64];
extern unsigned int   objwlrpos[256];
extern unsigned short objwen[256];
extern unsigned short prevpal[256];
extern unsigned short PrevPicture[64 * 56];

void test_endmem(void)
{
    ZT_SECTION("endmem: bss buffers are zero-initialized");

    /* WRAM and extra RAM (.bss — zero) */
    {
        int nonzero = 0;
        for (int i = 0; i < 65536; i++)
            nonzero += wramdataa[i] != 0;
        ZT_CHECK_INT(nonzero, 0);
    }
    {
        int nonzero = 0;
        for (int i = 0; i < 65536; i++)
            nonzero += ram7fa[i] != 0;
        ZT_CHECK_INT(nonzero, 0);
    }

    /* Opcode jump table (.bss — zero) */
    {
        int nonzero = 0;
        for (int i = 0; i < 256; i++)
            nonzero += opcjmptab[i] != 0;
        ZT_CHECK_INT(nonzero, 0);
    }

    /* CPU tables (.bss — zero) */
    {
        int nonzero = 0;
        for (int i = 0; i < 256; i++)
            nonzero += tableA[i] != 0;
        ZT_CHECK_INT(nonzero, 0);
    }

    /* Video memory change flags (.bss — zero) */
    {
        int nonzero = 0;
        for (int i = 0; i < 4096; i++)
            nonzero += vidmemch2[i] != 0;
        ZT_CHECK_INT(nonzero, 0);
    }

    ZT_SECTION("endmem: .data buffers have correct initial values");

    /* vidmemch2s: initialized to 0xFF */
    {
        int bad = 0;
        for (int i = 0; i < 4096; i++)
            bad += vidmemch2s[i] != 0xFF;
        ZT_CHECK_INT(bad, 0);
    }

    /* vidmemch4s: initialized to 0xFF */
    {
        int bad = 0;
        for (int i = 0; i < 2048; i++)
            bad += vidmemch4s[i] != 0xFF;
        ZT_CHECK_INT(bad, 0);
    }

    /* vidmemch8s: initialized to 0xFF */
    {
        int bad = 0;
        for (int i = 0; i < 1024; i++)
            bad += vidmemch8s[i] != 0xFF;
        ZT_CHECK_INT(bad, 0);
    }

    /* objwlrpos: initialized to 0xFFFFFFFF */
    {
        int bad = 0;
        for (int i = 0; i < 256; i++)
            bad += objwlrpos[i] != 0xFFFFFFFF;
        ZT_CHECK_INT(bad, 0);
    }

    /* objwen: initialized to 0xFFFF */
    {
        int bad = 0;
        for (int i = 0; i < 256; i++)
            bad += objwen[i] != 0xFFFF;
        ZT_CHECK_INT(bad, 0);
    }

    ZT_SECTION("endmem: sprite/palette/window arrays are zero");

    {
        int nonzero = 0;
        for (int i = 0; i < 256; i++)
            nonzero += sprlefttot[i] != 0;
        ZT_CHECK_INT(nonzero, 0);
    }
    {
        int nonzero = 0;
        for (int i = 0; i < 256; i++)
            nonzero += prevpal[i] != 0;
        ZT_CHECK_INT(nonzero, 0);
    }
    {
        int nonzero = 0;
        for (int i = 0; i < 288; i++)
            nonzero += winbgdata[i] != 0;
        ZT_CHECK_INT(nonzero, 0);
    }
    {
        int nonzero = 0;
        for (int i = 0; i < 288; i++)
            nonzero += winspdata[i] != 0;
        ZT_CHECK_INT(nonzero, 0);
    }
}
