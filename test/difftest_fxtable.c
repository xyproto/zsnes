/* Differential test: original InitFxTables (assembly) vs the C port. */
#include "../chips/fxtable.h"
#include "../endmem.h"
#include "../types.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

u1 *romdata, *sfxramdata;
void InitFxTablesAsm(void);
extern u4 asm_sfx128lineloc, asm_sfx160lineloc, asm_sfx192lineloc, asm_sfxobjlineloc;
void InitFxTables(void);

struct {
    char const* name;
    u4* p;
    u4 n;
} tabs[] = {
    { "FxTable", FxTable, 256 },
    { "FxTableA1", FxTableA1, 256 },
    { "FxTableA2", FxTableA2, 256 },
    { "FxTableA3", FxTableA3, 256 },
    { "FxTableb", FxTableb, 256 },
    { "FxTablebA1", FxTablebA1, 256 },
    { "FxTablebA2", FxTablebA2, 256 },
    { "FxTablebA3", FxTablebA3, 256 },
    { "FxTablec", FxTablec, 256 },
    { "FxTablecA1", FxTablecA1, 256 },
    { "FxTablecA2", FxTablecA2, 256 },
    { "FxTablecA3", FxTablecA3, 256 },
    { "FxTabled", FxTabled, 256 },
    { "FxTabledA1", FxTabledA1, 256 },
    { "FxTabledA2", FxTabledA2, 256 },
    { "FxTabledA3", FxTabledA3, 256 },
    { "SfxMemTable", SfxMemTable, 256 },
    { "PLOTJmpa", PLOTJmpa, 64 },
    { "PLOTJmpb", PLOTJmpb, 64 },
    { "fxxand", fxxand, 256 },
    { "fxbit01", fxbit01, 256 },
    { "fxbit23", fxbit23, 256 },
    { "fxbit45", fxbit45, 256 },
    { "fxbit67", fxbit67, 256 },
};
#define NTAB (sizeof tabs / sizeof *tabs)
#define LINEBYTES (4u * 256u * 256u * 4u) /* four 256x256 dword tables */

static u4 saved[NTAB][256];
static u1* lines;

static void snapshot(u1* dst)
{
    for (u4 i = 0; i != NTAB; ++i)
        memcpy(saved[i], tabs[i].p, tabs[i].n * 4);
    memcpy(dst, sfxramdata + 1024 * 1024, LINEBYTES);
}

int main(void)
{
    /* Never dereferenced - only address arithmetic lands in SfxMemTable. */
    romdata = (u1*)0x10000000;
    sfxramdata = malloc(1024 * 1024 + LINEBYTES);
    lines = malloc(LINEBYTES);
    u4 locs[4];

    InitFxTablesAsm();
    snapshot(lines);
    locs[0] = asm_sfx128lineloc;
    locs[1] = asm_sfx160lineloc;
    locs[2] = asm_sfx192lineloc;
    locs[3] = asm_sfxobjlineloc;

    /* Poison everything so a table the C port forgets cannot pass by accident. */
    for (u4 i = 0; i != NTAB; ++i)
        memset(tabs[i].p, 0xA5, tabs[i].n * 4);
    memset(sfxramdata + 1024 * 1024, 0xA5, LINEBYTES);
    sfx128lineloc = sfx160lineloc = sfx192lineloc = sfxobjlineloc = 0xA5A5A5A5;

    InitFxTables();

    u4 bad = 0;
    for (u4 i = 0; i != NTAB; ++i) {
        for (u4 j = 0; j != tabs[i].n; ++j) {
            if (tabs[i].p[j] != saved[i][j]) {
                if (bad++ < 10)
                    printf("  %s[%u]: asm %08X != C %08X\n", tabs[i].name, j, saved[i][j], tabs[i].p[j]);
            }
        }
    }
    if (memcmp(lines, sfxramdata + 1024 * 1024, LINEBYTES) != 0) {
        u4 *a = (u4*)lines, *b = (u4*)(sfxramdata + 1024 * 1024);
        for (u4 i = 0, shown = 0; i != LINEBYTES / 4; ++i)
            if (a[i] != b[i] && shown++ < 10)
                printf("  line table +%u: asm %08X != C %08X\n", i, a[i], b[i]), bad++;
    }
    u4 const* now[4] = { &sfx128lineloc, &sfx160lineloc, &sfx192lineloc, &sfxobjlineloc };
    char const* ln[4] = { "sfx128lineloc", "sfx160lineloc", "sfx192lineloc", "sfxobjlineloc" };
    for (u4 i = 0; i != 4; ++i)
        if (*now[i] != locs[i])
            printf("  %s: asm %08X != C %08X\n", ln[i], locs[i], *now[i]), bad++;

    if (bad) {
        printf("SuperFX InitFxTables: FAIL (%u mismatches)\n", bad);
        return 1;
    }
    printf("SuperFX InitFxTables: PASS (%u tables + %u line entries bit-identical to asm)\n",
        (u4)NTAB, LINEBYTES / 4);
    return 0;
}
