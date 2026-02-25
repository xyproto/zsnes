/*
 * test_init.c — verify init.asm variable initial values
 *
 * These tests protect the future C replacement of init.asm by asserting
 * that every global variable has the expected initial value.
 */

#include <stddef.h>
#include <string.h>

#include "zstest.h"

/* ── symbols from init.asm (declared via NEWSYM) ── */

extern unsigned char  regsbackup[3019];
extern unsigned char  forceromtype;
extern unsigned char  autoloadstate;
extern unsigned char  autoloadmovie;
extern unsigned char  ZMVRawDump;
extern unsigned char  romtype;
extern unsigned short resetv;
extern unsigned short abortv;
extern unsigned short nmiv2;
extern unsigned short nmiv;
extern unsigned short irqv;
extern unsigned short irqv2;
extern unsigned short brkv;
extern unsigned short copv;
extern unsigned short abortv8;
extern unsigned short nmiv8;
extern unsigned short irqv8;
extern unsigned short brkv8;
extern unsigned short copv8;
extern unsigned char  cycpb268;
extern unsigned char  cycpb358;
extern unsigned char  cycpbl2;
extern unsigned char  cycpblt2;
extern unsigned char  writeon;
extern unsigned short totlines;
extern unsigned char  curcyc;
extern unsigned char  cacheud;
extern unsigned char  ccud;
extern unsigned char  spcon;
extern unsigned short xat;
extern unsigned char  xdbt;
extern unsigned char  xpbt;
extern unsigned short xst;
extern unsigned short xdt;
extern unsigned short xxt;
extern unsigned short xyt;
extern unsigned short xpc;
extern unsigned char  debugger;
extern unsigned char  curnmi;
extern unsigned int   cycpbl;
extern unsigned int   cycpblt;
extern unsigned int   xa;
extern unsigned int   xdb;
extern unsigned int   xpb;
extern unsigned int   xs;
extern unsigned int   xd;
extern unsigned int   xx;
extern unsigned int   xy;
extern unsigned int   flagnz;
extern unsigned int   flago;
extern unsigned int   flagc;
extern unsigned int   bankkp;
extern unsigned int   Sflagnz;
extern unsigned int   Sflago;
extern unsigned int   Sflagc;
extern unsigned char  disablespcclr;
extern unsigned char  ENVDisable;
extern unsigned char  IPSPatched;
extern unsigned char  SramExists;
extern unsigned int   NumofBanks;
extern unsigned int   NumofBytes;
extern unsigned char  DSP1Type;
extern unsigned char  yesoutofmemory;

void test_init(void)
{
    ZT_SECTION("init: .data variables have correct initial values");

    /* regsbackup: initialized to all zeros */
    {
        int nonzero = 0;
        for (int i = 0; i < 3019; i++)
            nonzero += regsbackup[i] != 0;
        ZT_CHECK_INT(nonzero, 0);
    }

    ZT_CHECK_INT(forceromtype, 0);
    ZT_CHECK_INT(autoloadstate, 0);
    ZT_CHECK_INT(autoloadmovie, 0);
    ZT_CHECK_INT(ZMVRawDump, 0);
    ZT_CHECK_INT(romtype, 0);

    /* Vectors: all zero */
    ZT_CHECK_INT(resetv, 0);
    ZT_CHECK_INT(abortv, 0);
    ZT_CHECK_INT(nmiv2, 0);
    ZT_CHECK_INT(nmiv, 0);
    ZT_CHECK_INT(irqv, 0);
    ZT_CHECK_INT(irqv2, 0);
    ZT_CHECK_INT(brkv, 0);
    ZT_CHECK_INT(copv, 0);
    ZT_CHECK_INT(abortv8, 0);
    ZT_CHECK_INT(nmiv8, 0);
    ZT_CHECK_INT(irqv8, 0);
    ZT_CHECK_INT(brkv8, 0);
    ZT_CHECK_INT(copv8, 0);

    /* Cycle percentages */
    ZT_CHECK_INT(cycpb268, 109);
    ZT_CHECK_INT(cycpb358, 149);
    ZT_CHECK_INT(cycpbl2, 109);
    ZT_CHECK_INT(cycpblt2, 149);

    ZT_CHECK_INT(writeon, 0);
    ZT_CHECK_INT(totlines, 263);

    ZT_SECTION("init: saved-in-states variables");

    ZT_CHECK_INT(curcyc, 0);
    ZT_CHECK_INT(cacheud, 1);
    ZT_CHECK_INT(ccud, 0);
    ZT_CHECK_INT(spcon, 0);

    ZT_SECTION("init: 65816 registers");

    ZT_CHECK_INT(xat, 0);
    ZT_CHECK_INT(xdbt, 0);
    ZT_CHECK_INT(xpbt, 0);
    ZT_CHECK_INT(xst, 0);
    ZT_CHECK_INT(xdt, 0);
    ZT_CHECK_INT(xxt, 0);
    ZT_CHECK_INT(xyt, 0);
    ZT_CHECK_INT(xpc, 0);
    ZT_CHECK_INT(debugger, 0);
    ZT_CHECK_INT(curnmi, 0);

    ZT_SECTION("init: 32-bit cycle counters and registers");

    ZT_CHECK_INT(cycpbl, 110);
    ZT_CHECK_INT(cycpblt, 110);
    ZT_CHECK_INT(xa, 0);
    ZT_CHECK_INT(xdb, 0);
    ZT_CHECK_INT(xpb, 0);
    ZT_CHECK_INT(xs, 0);
    ZT_CHECK_INT(xd, 0);
    ZT_CHECK_INT(xx, 0);
    ZT_CHECK_INT(xy, 0);
    ZT_CHECK_INT(flagnz, 0);
    ZT_CHECK_INT(flago, 0);
    ZT_CHECK_INT(flagc, 0);
    ZT_CHECK_INT(bankkp, 0);
    ZT_CHECK_INT(Sflagnz, 0);
    ZT_CHECK_INT(Sflago, 0);
    ZT_CHECK_INT(Sflagc, 0);

    ZT_SECTION("init: misc flags");

    ZT_CHECK_INT(disablespcclr, 0);
    ZT_CHECK_INT(ENVDisable, 0);

    ZT_SECTION("init: .bss variables are zero");

    ZT_CHECK_INT(IPSPatched, 0);
    ZT_CHECK_INT(SramExists, 0);
    ZT_CHECK_INT(NumofBanks, 0);
    ZT_CHECK_INT(NumofBytes, 0);
    ZT_CHECK_INT(DSP1Type, 0);
    ZT_CHECK_INT(yesoutofmemory, 0);
}
