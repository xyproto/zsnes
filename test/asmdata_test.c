/* Layout tests for the data-only asm ports (cpu/dspproc.c, cpu/c_spcdata.c,
 * video/makevid.c, video/newgfx.c, video/c_mode716data.c,
 * cpu/c_execdata.c).
 *
 * These files are pure data blocks emitted through asmdata.h.  Nothing in
 * them is a function, so the only thing that can regress is the layout: the
 * save-state code copies the DSP block by raw byte distance (PHdspsave and
 * friends), and the renderers reach some of the newgfx slots by offset from a
 * neighbour.  The numbers below are the ones the original NASM objects had. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "zstest.h"

typedef uint8_t u1;
typedef uint32_t u4;

/* chips/c_sa1data.c */
extern u1 SA1Status, CurrentExecSA1, CurrentCPU;
extern u4 SA1xpc;
extern void* prevedi;

/* video/c_makev16tdata.c */
extern u1 transpbuf[], DoTransp;
extern u4 prevrgbcol, prevrgbpal, coadder16;
extern uint16_t yadd, yflipadd;

/* cpu/c_regsdata.c */
extern u1 sndrot, sndrot2, INTEnab, NMIEnab, vidbright, forceblnk;
extern u1 objsize1, objsize2, scrndis, tempdat[473];
extern u4 PHnum2writeppureg, objptr, objptrn, oamaddr, bg1ptrx;
extern uint16_t invreg, VIRQLoc;
extern u1 MultiTap, hblank, cpu_mdr, ppu2_mdr;
extern u4 JoyARead, JoyBRead;

/* cpu/c_regswdata.c */
extern u1 reg2101w_objsize1[8], reg2101w_objsize2[8], reg2101w_objmovs1[8];
extern u1 reg2101w_objmovs2[8], bgscrolPrev, multchange, m7byte;
extern uint16_t reg2101w_objadds1[8], reg2101w_objadds2[8];
extern uint16_t bg1scrolx_m7, bg1scroly_m7;
extern u1 prevoamptr, oamlow, MultiTapStat;

/* video/c_newgfx16data.c */
extern u1 prevbrightdc, moscountdown;
extern u4 mosstart[4], BackAreaAdd, BackAreaUnFillCol, BackAreaFillCol;
extern u4 clinemainsub, cpalptrng, palchanged, ng16bbgval, ng16bprval;
extern u4 mosjmptab16b[15], mosjmptab16bt[15], mosjmptab16btms[15];
extern u4 mosjmptab16bntms[15], UnusedBit[2], HalfTrans[4], UnusedBitXor[2];
extern u4 ngrposng[2], NGNoTransp, HalfTransC[2];

/* cpu/dspproc.c */
extern u1 SBHDMA, BRRBuffer[32], VolumeTableb[256], AdsrSustLevLoc[8];
extern u1 echoon0, Voice7FirstBlock, Voice0Volume, Voice7Volume;
extern u4 dspPAdj, NumBRRconv, BufferSizeB, BufferSizeW, NoiseSpeeds[32];
extern u4 PSampleBuf[24 * 8], LPFsample1, LPFsample2, DLPFsamples[8 * 24];
extern u4 NoiseInc, NoisePointer, powhack;
extern uint16_t DSPInterP[1024], BRRreadahead[4];
extern u4 prev0, prev1, lastbl, loopbl;
extern u4 BRRPlace0, BRRTemp0, BRRPlace1, Voice0Freq, Voice1Freq, Voice7Freq;
extern u4 DSPBuffer[320 * 4], EchoBuffer[320 * 4], PModBuffer[320 * 4];
extern u4 MaxEcho, EchoRate[16], AttackRate[16], DecayRate[8];
extern u4 SustainRate[32], Increase[32], IncreaseBent[32], Decrease[32];
extern u4 DecreaseRateExp[32], useless[4], FiltLoop[16], FiltLoopR[16];
extern u4 PHdspsave, PHdspconvb, PHdspsave2, DSPInterpolate;

/* cpu/c_spcdata.c */
extern u1 SPCRAM[], SPCROM[64], spcextraram[64], FutureExpandS[192];
extern u1 reg1read, reg2read, reg3read, reg4read, timeron;
extern u1 timincr0, timincr1, timincr2, timinl0, timinl1, timinl2, timrcall;
extern u1 spcnumread;
extern u4 spcPCRamSt, spcA, spcX, spcY, spcP, spcNZ, spcS, spcRamDPSt, spcCycle;
extern u4 PHspcsave, timer2upd;

/* video/c_mode716data.c */
extern u4 mtemp, mmode7xpos, mtempa2, mmode7xrpos, mtempa, mmode7ypos;
extern u4 mtempb2, mmode7yrpos, mtempb, mmode7xadder, mtempc2, mmode7xadd2;
extern u4 mtempc, mmode7yadder, mtempd2, mmode7yadd2, mtempd, mmode7ptr;
extern u4 mmode7xinc, mmode7xincc, mmode7yinc, mmode7xsloc, mmode7ysloc;
extern u4 mmode7xsrl, mmode7ysrl;
extern uint16_t mcxloc, mcyloc;
extern u4 M7HROn, switchtorep3, m7xaddof, m7xaddof2, m7yaddof, m7yaddof2;
extern u4 pixelsleft, mm7xaddof, mm7xaddof2, mm7yaddof, mm7yaddof2;
extern u4 ngwleft, ngwleftb;
extern u4 mode7xpos[2], mode7ypos[2], mode7xrpos[2], mode7yrpos[2];
extern u4 mode7xadder[2], mode7yadder[2];

/* cpu/c_execdata.c */
extern u4 tempedx, tempesi, tempedi, tempebp, RewindTimer;
extern u1 BackState;
extern u4 BackStateSize, DblRewTimer;
extern u1 romloadskip;
extern u4 SSKeyPressed, SPCKeyPressed, NoSoundReinit;
extern u1 NextNGDisplay;
extern u4 TempVidInfo;
extern u1 tempdh, invalid, invopcd, pressed[256 + 128 + 64], exiter;
extern u4 oldhand9o;
extern uint16_t oldhand9s;
extern u4 oldhand8o;
extern uint16_t oldhand8s;
extern u4 opcd, pdh, pcury, timercount, initaddrl, NetSent, nextframe;
extern u4 HIRQCycNext;
extern u1 HIRQNextExe;
extern u4 timeradj;
extern uint16_t t1cc;
extern u4 soundcycleft, curexecstate, nmiprevaddrl, nmiprevaddrh, nmirept;
extern u4 nmiprevline, nmistatus, joycontren;
extern u1 NextLineCache, ZMVZClose, ExecExitOkay;
extern u4 JoyABack, JoyBBack, JoyCBack, JoyDBack, JoyEBack, NetCommand;
extern u4 spc700read, lowestspc, highestspc, SA1UBound, SA1LBound, SA1SH;
extern u4 SA1SHb, NumberOfOpcodes2, ChangeOps, SFXProc;
extern u1 EMUPause, INCRFrame, NoHDMALine;

/* video/makevid.c */
extern u1 bgcoloradder, res512switch, pwinbgenab, windowdata[16], numwin;
extern u1 multiwin, multiclip, multitype, hirestiledat[256], temp, a16x16yinc;
extern uint16_t MosaicYAdder[16];
extern u4 bg1vbufloc, bg2vbufloc, bg4xposloc, tempbuffer[33], curmosaicsz;
extern u4 yadder, curvidoffset, bgsubby, winptrref;
extern u1* cwinptr;
extern u1 winbgdata[]; /* endmem.c */

/* video/newgfx.c */
extern u1 OrLogicTable[4], AndLogicTable[4], XorLogicTable[4], XNorLogicTable[4];
extern u1 NGNumSpr, Mode7HiRes;
extern u4 sprclprio;
extern u4 ngwintable[32], ngwintablec[32], pwinen, pngwinen;
extern u4 bgcmsung, modeused[2], reslbyl, mosjmptab[15], nglogicval;
extern u4* ngcwinptr;

#define GAP(a, b) ((int)((const u1*)&(b) - (const u1*)&(a)))

static void test_dsp_savestate_offsets(void)
{
    ZT_SECTION("dspproc: save-state block distances");
    /* zstate.c copies PHdspsave bytes from BRRBuffer and PHdspsave2 from
       echoon0; PHdspconvb is the conversion window used by old states. */
    ZT_CHECK_INT(PHdspsave, 0x42C);
    ZT_CHECK_INT(PHdspconvb, 0x3CC);
    ZT_CHECK_INT(PHdspsave2, 0x150);
    ZT_CHECK_INT(PHdspsave, GAP(BRRBuffer[0], echoon0));
    ZT_CHECK_INT(PHdspconvb, GAP(Voice0Freq, echoon0));
    ZT_CHECK_INT(PHdspsave2, GAP(echoon0, Voice7FirstBlock) + 1);
}

static void test_dsp_layout(void)
{
    ZT_SECTION("dspproc: intra-block adjacency");
    /* The .bss chunks concatenate in emission order within the object, so the
       distances hold across the whole file, not just inside one chunk. */
    ZT_CHECK_INT(GAP(DSPInterP[0], PSampleBuf[0]), 0x800);
    ZT_CHECK_INT(GAP(PSampleBuf[0], LPFsample1), 0x300);
    ZT_CHECK_INT(GAP(LPFsample1, LPFsample2), 4);
    ZT_CHECK_INT(GAP(LPFsample2, BRRreadahead[0]), 4);
    ZT_CHECK_INT(GAP(BRRreadahead[0], DLPFsamples[0]), 8);
    ZT_CHECK_INT(GAP(DLPFsamples[0], DSPBuffer[0]), 0x300);
    ZT_CHECK_INT(GAP(DSPBuffer[0], EchoBuffer[0]), 320 * 4 * 4);
    ZT_CHECK_INT(GAP(EchoBuffer[0], PModBuffer[0]), 320 * 4 * 4);
    ZT_CHECK_INT(GAP(PModBuffer[0], BRRBuffer[0]), 320 * 4 * 4);
    ZT_CHECK_INT(GAP(BRRBuffer[0], BRRPlace0), 32);
    /* BRRPlaceN and BRRTempN interleave, so the per-voice stride is 8. */
    ZT_CHECK_INT(GAP(BRRPlace0, BRRTemp0), 4);
    ZT_CHECK_INT(GAP(BRRPlace0, BRRPlace1), 8);
    ZT_CHECK_INT(GAP(Voice0Freq, Voice1Freq), 4);
    ZT_CHECK_INT(GAP(Voice0Freq, Voice7Freq), 28);
    ZT_CHECK_INT(GAP(Voice0Volume, Voice7Volume), 7);
    /* "useless" only exists to keep the save-state alignment. */
    ZT_CHECK_INT(GAP(useless[0], useless[3]), 12);
    ZT_CHECK_INT(GAP(FiltLoop[0], FiltLoopR[0]), 64);
    /* The tail after the save-state block: these are not saved. */
    ZT_CHECK_INT(GAP(Voice7FirstBlock, NoiseInc), 1);
    ZT_CHECK_INT(GAP(NoiseInc, NoisePointer), 4);
    ZT_CHECK_INT(GAP(NoisePointer, powhack), 4);

    ZT_SECTION("dspproc: .data ordering and padding");
    ZT_CHECK_INT(GAP(SBHDMA, dspPAdj), 1); /* deliberately unaligned */
    ZT_CHECK_INT(GAP(dspPAdj, NumBRRconv), 4);
    ZT_CHECK_INT(GAP(NumBRRconv, BufferSizeB), 4);
    ZT_CHECK_INT(GAP(BufferSizeB, BufferSizeW), 4);
    ZT_CHECK_INT(GAP(BufferSizeW, NoiseSpeeds[0]), 4);
    /* ALIGN32 after NoiseSpeeds, then the prev0 group. */
    ZT_CHECK_INT(GAP(NoiseSpeeds[0], prev0), 143);
    ZT_CHECK_INT(GAP(prev0, prev1), 4);
    ZT_CHECK_INT(GAP(prev1, lastbl), 4);
    ZT_CHECK_INT(GAP(lastbl, loopbl), 4);
    /* loopbl, the private usenoisedata slot, then the volume table. */
    ZT_CHECK_INT(GAP(loopbl, VolumeTableb[0]), 8);
    /* ALIGN32 after VolumeTableb, then the rate tables back to back. */
    ZT_CHECK_INT(GAP(VolumeTableb[0], MaxEcho), 268);
    ZT_CHECK_INT(GAP(MaxEcho, EchoRate[0]), 4);
    ZT_CHECK_INT(GAP(EchoRate[0], AttackRate[0]), 64);
    ZT_CHECK_INT(GAP(AttackRate[0], DecayRate[0]), 64);
    ZT_CHECK_INT(GAP(DecayRate[0], SustainRate[0]), 32);
    ZT_CHECK_INT(GAP(SustainRate[0], Increase[0]), 128);
    ZT_CHECK_INT(GAP(Increase[0], IncreaseBent[0]), 128);
    ZT_CHECK_INT(GAP(IncreaseBent[0], Decrease[0]), 128);
    ZT_CHECK_INT(GAP(Decrease[0], DecreaseRateExp[0]), 128);
    ZT_CHECK_INT(GAP(DecreaseRateExp[0], AdsrSustLevLoc[0]), 128);
    ZT_CHECK_INT(GAP(AdsrSustLevLoc[0], PHdspsave), 8);
    /* ALIGN16 before the final slot. */
    ZT_CHECK_INT(GAP(PHdspsave, DSPInterpolate), 20);
}

static void test_dsp_values(void)
{
    ZT_SECTION("dspproc: table contents");
    ZT_CHECK_INT(SBHDMA, 0);
    ZT_CHECK_INT(BufferSizeB, 320);
    ZT_CHECK_INT(BufferSizeW, 640);
    ZT_CHECK_INT(MaxEcho, 172);
    ZT_CHECK_INT(NoiseSpeeds[0], 1);
    ZT_CHECK_INT(NoiseSpeeds[16], 500);
    ZT_CHECK_INT(NoiseSpeeds[31], 32000);
    ZT_CHECK_INT(EchoRate[1], 172);
    ZT_CHECK_INT(EchoRate[15], 2584);
    ZT_CHECK_INT(AttackRate[0], 45202);
    ZT_CHECK_INT(AttackRate[15], 4);
    ZT_CHECK_INT(DecayRate[7], 125);
    ZT_CHECK(SustainRate[0] == 0xFFFFFFFFu);
    ZT_CHECK_INT(SustainRate[31], 70);
    ZT_CHECK(Increase[0] == 0xFFFFFFFFu);
    ZT_CHECK_INT(Increase[31], 22);
    ZT_CHECK(IncreaseBent[0] == 0xFFFFFFFFu);
    ZT_CHECK_INT(IncreaseBent[31], 36);
    ZT_CHECK(Decrease[0] == 0xFFFFFFFFu);
    ZT_CHECK_INT(Decrease[31], 22);
    ZT_CHECK(DecreaseRateExp[0] == 0xFFFFFFFFu);
    ZT_CHECK_INT(DecreaseRateExp[31], 198);
    ZT_CHECK_INT(AdsrSustLevLoc[0], 58);
    ZT_CHECK_INT(AdsrSustLevLoc[7], 1);

    /* VolumeTableb ramps 00..7F then mirrors back down to 00. */
    int volok = 1;
    for (int i = 0; i < 128; i++) {
        if (VolumeTableb[i] != i || VolumeTableb[255 - i] != i) {
            volok = 0;
        }
    }
    ZT_CHECK(volok);
}

static void test_makevid(void)
{
    ZT_SECTION("makevid: table contents");
    ZT_CHECK(cwinptr == winbgdata);
    static const uint16_t expect[16] = { 0, 0, 0, 1, 0, 2, 1, 0, 0, 4, 2, 2, 3, 1, 0, 7 };
    int mok = 1;
    for (int i = 0; i < 16; i++) {
        if (MosaicYAdder[i] != expect[i]) {
            mok = 0;
        }
    }
    ZT_CHECK(mok);
    ZT_CHECK_INT(GAP(MosaicYAdder[0], cwinptr), 32);

    ZT_SECTION("makevid: scratch block layout");
    ZT_CHECK_INT(GAP(bgcoloradder, res512switch), 1);
    ZT_CHECK_INT(GAP(bgcoloradder, pwinbgenab), 2);
    ZT_CHECK_INT(GAP(bgcoloradder, windowdata[0]), 0x15);
    ZT_CHECK_INT(GAP(windowdata[0], numwin), 16);
    /* numwin, multiwin, multiclip, multitype are read as one dword. */
    ZT_CHECK_INT(GAP(numwin, multiwin), 1);
    ZT_CHECK_INT(GAP(numwin, multiclip), 2);
    ZT_CHECK_INT(GAP(numwin, multitype), 3);
    /* The per-BG pointer arrays are indexed as bg1xxx + bgnum*4. */
    ZT_CHECK_INT(GAP(bg1vbufloc, bg2vbufloc), 4);
    ZT_CHECK_INT(GAP(bg1vbufloc, bg4xposloc), 0x5C);
    ZT_CHECK_INT(GAP(bgcoloradder, tempbuffer[0]), 0x9C);
    ZT_CHECK_INT(GAP(tempbuffer[0], curmosaicsz), 0x88);
    ZT_CHECK_INT(GAP(bgcoloradder, winptrref), 0x12B);
    ZT_CHECK_INT(GAP(winptrref, hirestiledat[0]), 4);
    ZT_CHECK_INT(GAP(bgcoloradder, yadder), 0x22F);
    ZT_CHECK_INT(GAP(yadder, curvidoffset), 0x24);
    ZT_CHECK_INT(GAP(curvidoffset, bgsubby), 12);
    ZT_CHECK_INT(GAP(bgsubby, temp), 4);
    ZT_CHECK_INT(GAP(temp, a16x16yinc), 3);
}

static void test_newgfx(void)
{
    ZT_SECTION("newgfx: window tables");
    int winok = 1;
    for (int i = 0; i < 32; i++) {
        if (ngwintable[i] != 0xEE00 || ngwintablec[i] != 0xEE00) {
            winok = 0;
        }
    }
    ZT_CHECK(winok);
    ZT_CHECK(ngcwinptr == ngwintable);
    ZT_CHECK_INT(GAP(ngwintable[0], ngwintablec[0]), 128);
    ZT_CHECK_INT(GAP(ngwintablec[0], ngcwinptr), 128);
    ZT_CHECK_INT(pwinen, 0xFFFF);
    ZT_CHECK_INT(pngwinen, 0xFFFF);
    ZT_CHECK_INT(GAP(pwinen, pngwinen), 4);

    ZT_SECTION("newgfx: window logic tables");
    ZT_CHECK(OrLogicTable[0] == 0 && OrLogicTable[1] == 1 && OrLogicTable[2] == 1 && OrLogicTable[3] == 0);
    ZT_CHECK(AndLogicTable[0] == 0 && AndLogicTable[1] == 0 && AndLogicTable[2] == 1 && AndLogicTable[3] == 0);
    ZT_CHECK(XorLogicTable[0] == 0 && XorLogicTable[1] == 1 && XorLogicTable[2] == 0 && XorLogicTable[3] == 0);
    ZT_CHECK(XNorLogicTable[0] == 1 && XNorLogicTable[1] == 0 && XNorLogicTable[2] == 1 && XNorLogicTable[3] == 0);
    /* The four tables are indexed as one 16-byte block by window logic code. */
    ZT_CHECK_INT(GAP(OrLogicTable[0], AndLogicTable[0]), 4);
    ZT_CHECK_INT(GAP(AndLogicTable[0], XorLogicTable[0]), 4);
    ZT_CHECK_INT(GAP(XorLogicTable[0], XNorLogicTable[0]), 4);

    ZT_SECTION("newgfx: scratch block layout");
    ZT_CHECK_INT(GAP(bgcmsung, modeused[0]), 4);
    ZT_CHECK_INT(GAP(modeused[0], reslbyl), 8);
    ZT_CHECK_INT(GAP(nglogicval, mosjmptab[0]), 4);
    ZT_CHECK_INT(GAP(mosjmptab[0], Mode7HiRes), 60);
    /* NGNumSpr is a lone byte, so sprclprio after it is deliberately unaligned. */
    ZT_CHECK_INT(GAP(NGNumSpr, sprclprio), 1);
}

/* cpu/c_spcdata.c: the SPC700 state block. zstate.c saves PHspcsave bytes
 * starting at SPCRAM, so the whole run from SPCRAM to FutureExpandS is one
 * save-state record and every distance in it is load-bearing. */
static void test_spcdata(void)
{
    static u1 const iplrom[] = { 0xCD, 0xEF, 0xBD, 0xE8, 0x00, 0xC6, 0x1D, 0xD0 };

    ZT_SECTION("spc700: save-state block distance");
    ZT_CHECK_INT(PHspcsave, 0x10140);
    ZT_CHECK_INT(GAP(SPCRAM[0], PHspcsave), 0x10140);
    /* The saved record ends where PHspcsave starts, i.e. right after the
       expansion padding. */
    ZT_CHECK_INT(GAP(FutureExpandS[0], PHspcsave), 192);

    ZT_SECTION("spc700: 64KB address space and the boot ROM window");
    /* SPCRAM holds the SPC700's 64KB plus the boot ROM that overlays $FFC0. */
    ZT_CHECK_INT(GAP(SPCRAM[0], spcPCRamSt), 65552);
    ZT_CHECK(SPCRAM[0] == 0xFF && SPCRAM[65471] == 0xFF);
    ZT_CHECK(memcmp(SPCRAM + 65472, iplrom, sizeof iplrom) == 0);
    ZT_CHECK(memcmp(SPCROM, iplrom, sizeof iplrom) == 0);
    ZT_CHECK(memcmp(SPCRAM + 65472, SPCROM, 64) == 0);
    /* The 16 bytes past the boot ROM are a scratch pattern, not part of it. */
    ZT_CHECK(SPCRAM[65472 + 64] == 0xAA && SPCRAM[65472 + 79] == 0x99);

    ZT_SECTION("spc700: register and timer block layout");
    /* spcPCRam and spcRamDP hold host pointers, so the live variables sit
       outside this block and a dword shadow keeps the save-state layout. */
    ZT_CHECK_INT(GAP(spcPCRamSt, spcA), 4);
    ZT_CHECK_INT(GAP(spcA, spcX), 4);
    ZT_CHECK_INT(GAP(spcX, spcY), 4);
    ZT_CHECK_INT(GAP(spcY, spcP), 4);
    ZT_CHECK_INT(GAP(spcP, spcNZ), 4);
    ZT_CHECK_INT(GAP(spcNZ, spcS), 4);
    ZT_CHECK_INT(GAP(spcS, spcRamDPSt), 4);
    ZT_CHECK_INT(GAP(spcRamDPSt, spcCycle), 4);
    ZT_CHECK_INT(spcS, 0x1FF); /* the only non-zero initialiser */
    /* $F4-$F7 and the seven timer bytes are read as one run of bytes. */
    ZT_CHECK_INT(GAP(spcCycle, reg1read), 4);
    ZT_CHECK_INT(GAP(reg1read, reg2read), 1);
    ZT_CHECK_INT(GAP(reg2read, reg3read), 1);
    ZT_CHECK_INT(GAP(reg3read, reg4read), 1);
    ZT_CHECK_INT(GAP(reg4read, timeron), 1);
    ZT_CHECK_INT(GAP(timeron, timincr0), 1);
    ZT_CHECK_INT(GAP(timincr0, timincr1), 1);
    ZT_CHECK_INT(GAP(timincr1, timincr2), 1);
    ZT_CHECK_INT(GAP(timincr2, timinl0), 1);
    ZT_CHECK_INT(GAP(timinl0, timinl1), 1);
    ZT_CHECK_INT(GAP(timinl1, timinl2), 1);
    ZT_CHECK_INT(GAP(timinl2, timrcall), 1);
    ZT_CHECK_INT(GAP(timrcall, spcextraram[0]), 1);
    ZT_CHECK_INT(GAP(spcextraram[0], FutureExpandS[0]), 64);

    ZT_SECTION("spc700: tail after the saved block");
    ZT_CHECK_INT(GAP(PHspcsave, SPCROM[0]), 4);
    ZT_CHECK_INT(GAP(SPCROM[0], timer2upd), 64);
    ZT_CHECK_INT(GAP(timer2upd, spcnumread), 4);
}

/* video/c_mode716data.c: the Mode 7 renderer scratch block. The position and
 * adder variables are read 8 bytes at a time, so each is followed by a spacer
 * the assembly marked "keep this blank!". A spacer that goes missing shifts
 * everything after it and silently changes what the renderer reads. */
static void test_mode716data(void)
{
    ZT_SECTION("mode716: paired variables keep their spacers");
    ZT_CHECK_INT(GAP(mtemp, mmode7xpos), 4);
    ZT_CHECK_INT(GAP(mmode7xpos, mtempa2), 4);
    ZT_CHECK_INT(GAP(mmode7xpos, mmode7xrpos), 8);
    ZT_CHECK_INT(GAP(mmode7xrpos, mtempa), 4);
    ZT_CHECK_INT(GAP(mmode7xrpos, mmode7ypos), 8);
    ZT_CHECK_INT(GAP(mmode7ypos, mtempb2), 4);
    ZT_CHECK_INT(GAP(mmode7ypos, mmode7yrpos), 8);
    ZT_CHECK_INT(GAP(mmode7yrpos, mtempb), 4);
    ZT_CHECK_INT(GAP(mmode7yrpos, mmode7xadder), 8);
    ZT_CHECK_INT(GAP(mmode7xadder, mtempc2), 4);
    ZT_CHECK_INT(GAP(mmode7xadder, mmode7xadd2), 8);
    ZT_CHECK_INT(GAP(mmode7xadd2, mtempc), 4);
    ZT_CHECK_INT(GAP(mmode7xadd2, mmode7yadder), 8);
    ZT_CHECK_INT(GAP(mmode7yadder, mtempd2), 4);
    ZT_CHECK_INT(GAP(mmode7yadder, mmode7yadd2), 8);
    ZT_CHECK_INT(GAP(mmode7yadd2, mtempd), 4);
    ZT_CHECK_INT(GAP(mmode7yadd2, mmode7ptr), 8);

    ZT_SECTION("mode716: unpaired run after the pointer");
    ZT_CHECK_INT(GAP(mmode7ptr, mmode7xinc), 4);
    ZT_CHECK_INT(GAP(mmode7xinc, mmode7xincc), 4);
    ZT_CHECK_INT(GAP(mmode7xincc, mmode7yinc), 4);
    ZT_CHECK_INT(GAP(mmode7yinc, mmode7xsloc), 4);
    ZT_CHECK_INT(GAP(mmode7xsloc, mmode7ysloc), 4);
    ZT_CHECK_INT(GAP(mmode7ysloc, mmode7xsrl), 4);
    ZT_CHECK_INT(GAP(mmode7xsrl, mmode7ysrl), 4);
    /* The two cursor locations are words, not dwords. */
    ZT_CHECK_INT(GAP(mmode7ysrl, mcxloc), 4);
    ZT_CHECK_INT(GAP(mcxloc, mcyloc), 2);
    ZT_CHECK_INT(GAP(mcyloc, M7HROn), 2);
    ZT_CHECK_INT(GAP(M7HROn, switchtorep3), 4);
    ZT_CHECK_INT(GAP(switchtorep3, m7xaddof), 4);
    ZT_CHECK_INT(GAP(m7xaddof, m7xaddof2), 4);
    ZT_CHECK_INT(GAP(m7xaddof2, m7yaddof), 4);
    ZT_CHECK_INT(GAP(m7yaddof, m7yaddof2), 4);
    ZT_CHECK_INT(GAP(m7yaddof2, pixelsleft), 4);
    ZT_CHECK_INT(GAP(pixelsleft, mm7xaddof), 4);
    ZT_CHECK_INT(GAP(mm7xaddof, mm7xaddof2), 4);
    ZT_CHECK_INT(GAP(mm7xaddof2, mm7yaddof), 4);
    ZT_CHECK_INT(GAP(mm7yaddof, mm7yaddof2), 4);

    ZT_SECTION("mode716: exported tail");
    ZT_CHECK_INT(GAP(mm7yaddof2, ngwleft), 4);
    ZT_CHECK_INT(GAP(ngwleft, ngwleftb), 4);
    /* Same pairing as above, with the spacer folded into the reservation. */
    ZT_CHECK_INT(GAP(ngwleftb, mode7xpos[0]), 4);
    ZT_CHECK_INT(GAP(mode7xpos[0], mode7ypos[0]), 8);
    ZT_CHECK_INT(GAP(mode7ypos[0], mode7xrpos[0]), 8);
    ZT_CHECK_INT(GAP(mode7xrpos[0], mode7yrpos[0]), 8);
    ZT_CHECK_INT(GAP(mode7yrpos[0], mode7xadder[0]), 8);
    ZT_CHECK_INT(GAP(mode7xadder[0], mode7yadder[0]), 8);
    /* Whole block, as NASM laid it out. */
    ZT_CHECK_INT(GAP(mtemp, mode7yadder[0]) + 8, 204);
}

/* cpu/c_execdata.c: the emulation-loop state block. Mostly ordinary scalars,
 * but two ALIGN32 gaps sit inside it and NASM pads those with nop bytes, not
 * zeroes - so the padding is part of the image, and the run of five JoyxBack
 * dwords is read as a block. */
static void test_execdata(void)
{
    ZT_SECTION("execute: scalar run before the first gap");
    ZT_CHECK_INT(GAP(tempedx, tempesi), 4);
    ZT_CHECK_INT(GAP(tempesi, tempedi), 4);
    ZT_CHECK_INT(GAP(tempedi, tempebp), 4);
    ZT_CHECK_INT(GAP(tempebp, RewindTimer), 4);
    /* BackState is a byte, so everything after it is deliberately unaligned. */
    ZT_CHECK_INT(GAP(RewindTimer, BackState), 4);
    ZT_CHECK_INT(GAP(BackState, BackStateSize), 1);
    ZT_CHECK_INT(GAP(BackStateSize, DblRewTimer), 4);
    ZT_CHECK_INT(GAP(DblRewTimer, romloadskip), 4);
    ZT_CHECK_INT(GAP(invopcd, pressed[0]), 1);
    ZT_CHECK_INT(GAP(pressed[0], exiter), 256 + 128 + 64);
    ZT_CHECK_INT(GAP(oldhand9o, oldhand9s), 4);
    ZT_CHECK_INT(GAP(oldhand9s, oldhand8o), 2);
    ZT_CHECK_INT(GAP(oldhand8o, oldhand8s), 4);
    ZT_CHECK_INT(GAP(oldhand8s, opcd), 2);
    ZT_CHECK_INT(GAP(HIRQCycNext, HIRQNextExe), 4);
    ZT_CHECK_INT(GAP(HIRQNextExe, timeradj), 1);

    ZT_SECTION("execute: the two ALIGN32 gaps");
    /* t1cc is a word at 0x224; soundcycleft is realigned to 0x240. */
    ZT_CHECK_INT(GAP(timeradj, t1cc), 4);
    ZT_CHECK_INT(GAP(t1cc, soundcycleft), 28);
    ZT_CHECK(((const u1*)&soundcycleft - (const u1*)&tempedx) % 32 == 0);
    /* Same again from ZMVZClose to ExecExitOkay. */
    ZT_CHECK_INT(GAP(NextLineCache, ZMVZClose), 1);
    ZT_CHECK_INT(GAP(ZMVZClose, ExecExitOkay), 31);
    ZT_CHECK(((const u1*)&ExecExitOkay - (const u1*)&tempedx) % 32 == 0);
    /* NASM fills an ALIGN with nops; zero-filling here would change the image. */
    ZT_CHECK_INT(((const u1*)&soundcycleft)[-1], 0x90);
    ZT_CHECK_INT(((const u1*)&ExecExitOkay)[-1], 0x90);

    ZT_SECTION("execute: block after the second gap");
    ZT_CHECK_INT(GAP(ExecExitOkay, JoyABack), 1);
    ZT_CHECK_INT(GAP(JoyABack, JoyBBack), 4);
    ZT_CHECK_INT(GAP(JoyBBack, JoyCBack), 4);
    ZT_CHECK_INT(GAP(JoyCBack, JoyDBack), 4);
    ZT_CHECK_INT(GAP(JoyDBack, JoyEBack), 4);
    ZT_CHECK_INT(GAP(JoyEBack, NetCommand), 4);
    ZT_CHECK_INT(GAP(NetCommand, spc700read), 4);
    ZT_CHECK_INT(GAP(SA1SH, SA1SHb), 4);
    ZT_CHECK_INT(GAP(SFXProc, EMUPause), 4);
    ZT_CHECK_INT(GAP(EMUPause, INCRFrame), 1);
    ZT_CHECK_INT(GAP(INCRFrame, NoHDMALine), 1);

    ZT_SECTION("execute: non-zero initialisers");
    ZT_CHECK_INT(BackState, 1);
    ZT_CHECK_INT(BackStateSize, 6);
    ZT_CHECK_INT(timeradj, 65536);
    ZT_CHECK_INT(nmiprevline, 224);
    ZT_CHECK_INT(NumberOfOpcodes2, 370);
    ZT_CHECK_INT(ExecExitOkay, 1);
}

/* chips/c_sa1proc.c: the block that was left in chips/sa1proc.asm. zstate.c
 * saves three bytes from &SA1Status, so those three must stay adjacent and in
 * order; prevedi holds a host pointer, so it is pointer-sized and aligned. */
static void test_sa1proc(void)
{
    ZT_SECTION("sa1proc: the three save-state bytes");
    ZT_CHECK_INT(GAP(SA1Status, CurrentExecSA1), 1);
    ZT_CHECK_INT(GAP(CurrentExecSA1, CurrentCPU), 1);

    ZT_SECTION("sa1proc: unaligned tail");
    /* prevedi holds a host pointer now, so it is pointer-sized and aligned
       rather than following the assembly's commented-out ALIGN32. Only the
       three bytes above it are saved, so nothing depends on where it lands. */
    ZT_CHECK_INT(GAP(CurrentCPU, prevedi), (int)sizeof(void*) - 2);
    ZT_CHECK_INT(GAP(prevedi, SA1xpc), (int)sizeof(void*));
}

/* video/c_makev16tdata.c: the .bss blocks from video/makev16t.asm. The
 * transparency buffer is indexed with signed displacements off its middle, so
 * what follows it is part of the shape; coadder16 is deliberately unaligned. */
static void test_makev16t(void)
{
    ZT_SECTION("makev16t: transparency buffer and its tail");
    ZT_CHECK_INT(GAP(transpbuf[0], prevrgbcol), 576 + 16 + 288 * 2);
    ZT_CHECK_INT(GAP(prevrgbcol, prevrgbpal), 4);
    ZT_CHECK_INT(GAP(prevrgbpal, DoTransp), 4);

    ZT_SECTION("makev16t: unaligned run after DoTransp");
    ZT_CHECK_INT(GAP(DoTransp, coadder16), 1);
    ZT_CHECK_INT(GAP(coadder16, yadd), 4);
    ZT_CHECK_INT(GAP(yadd, yflipadd), 2);
}

/* video/c_newgfx16data.c: the .data blocks from video/newgfx16.asm. The two
 * gaps are nop fill, not zero - NASM's ALIGN pads with 0x90 in a data section
 * too - and BackAreaAdd is unaligned because the assembly put it there. */
static void test_newgfx16data(void)
{
    ZT_SECTION("newgfx16: unaligned head");
    ZT_CHECK_INT(GAP(prevbrightdc, mosstart[0]), 1);
    ZT_CHECK_INT(GAP(mosstart[0], moscountdown), 16);
    ZT_CHECK_INT(GAP(moscountdown, BackAreaAdd), 1);
    ZT_CHECK_INT(GAP(BackAreaAdd, BackAreaUnFillCol), 4);
    ZT_CHECK_INT(GAP(BackAreaUnFillCol, BackAreaFillCol), 4);
    ZT_CHECK_INT(GAP(BackAreaFillCol, clinemainsub), 4);

    ZT_SECTION("newgfx16: the two 32-byte gaps");
    ZT_CHECK_INT(GAP(clinemainsub, cpalptrng), 0x22);
    ZT_CHECK_INT(GAP(mosjmptab16bntms[0], UnusedBit[0]), 0x50);
    /* The padding is nops; a zero here means someone used .balign without a
       fill byte. */
    ZT_CHECK_INT(((const u1*)&cpalptrng)[-1], 0x90);
    ZT_CHECK_INT(((const u1*)&UnusedBit)[-1], 0x90);

    ZT_SECTION("newgfx16: jump tables and constants");
    ZT_CHECK_INT(GAP(cpalptrng, palchanged), 0x30);
    ZT_CHECK_INT(GAP(palchanged, ng16bbgval), 4);
    ZT_CHECK_INT(GAP(ng16bbgval, ng16bprval), 4);
    ZT_CHECK_INT(GAP(ng16bprval, mosjmptab16b[0]), 4);
    ZT_CHECK_INT(GAP(mosjmptab16b[0], mosjmptab16bt[0]), 60);
    ZT_CHECK_INT(GAP(mosjmptab16bt[0], mosjmptab16btms[0]), 60);
    ZT_CHECK_INT(GAP(mosjmptab16btms[0], mosjmptab16bntms[0]), 60);
    ZT_CHECK_INT(GAP(UnusedBit[0], HalfTrans[0]), 8);
    ZT_CHECK_INT(GAP(HalfTrans[0], UnusedBitXor[0]), 16);
    ZT_CHECK_INT(GAP(UnusedBitXor[0], ngrposng[0]), 8);
    ZT_CHECK_INT(GAP(HalfTransC[0], NGNoTransp), 8);
    ZT_CHECK_INT(UnusedBit[0], 0x00200020);
    ZT_CHECK_INT(HalfTrans[0], (int)0xF7DEF7DE);
    ZT_CHECK_INT(UnusedBitXor[0], (int)0xFFDFFFDF);
    ZT_CHECK_INT(ngrposng[0], 11);
    ZT_CHECK_INT(prevbrightdc, 16);
}

/* cpu/c_regsdata.c: the CPU/PPU register file from cpu/regs.inc. zstate.c
 * saves PHnum2writeppureg bytes starting at sndrot, and that length is
 * assembled from this block's own layout, so it and every distance inside it
 * are the save-state format. */
static void test_regsdata(void)
{
    ZT_SECTION("regs: the save-state record length");
    ZT_CHECK_INT(PHnum2writeppureg,
        (int)((const u1*)&PHnum2writeppureg - (const u1*)&sndrot));
    ZT_CHECK_INT(GAP(sndrot, tempdat[0]) + 473,
        (int)PHnum2writeppureg);

    ZT_SECTION("regs: head of the record");
    ZT_CHECK_INT(GAP(invreg, sndrot), 2);
    ZT_CHECK_INT(GAP(sndrot, sndrot2), 1);
    ZT_CHECK_INT(GAP(sndrot2, INTEnab), 1);
    ZT_CHECK_INT(GAP(INTEnab, NMIEnab), 1);
    ZT_CHECK_INT(GAP(NMIEnab, VIRQLoc), 1);
    ZT_CHECK_INT(GAP(VIRQLoc, vidbright), 2);
    ZT_CHECK_INT(GAP(objptr, objptrn), 4);
    ZT_CHECK_INT(GAP(objsize1, objsize2), 1);

    ZT_SECTION("regs: the ALIGN32 gap after scrndis");
    ZT_CHECK_INT(GAP(PHnum2writeppureg, scrndis), 4);
    ZT_CHECK_INT(GAP(oamaddr, bg1ptrx), 4);
    /* nop fill, not zero. */
    ZT_CHECK_INT(((const u1*)&oamaddr)[-1], 0x90);

    ZT_SECTION("regs: initialisers and the bss run");
    ZT_CHECK_INT(NMIEnab, 1);
    ZT_CHECK_INT(forceblnk, 0x80);
    ZT_CHECK_INT(objsize1, 1);
    ZT_CHECK_INT(objsize2, 4);
    ZT_CHECK_INT(GAP(JoyARead, JoyBRead), 4);
    ZT_CHECK_INT(GAP(MultiTap, hblank), 1);
    ZT_CHECK_INT(GAP(hblank, cpu_mdr), 1);
    ZT_CHECK_INT(GAP(cpu_mdr, ppu2_mdr), 1);
}

/* cpu/c_regswdata.c: what was left in cpu/regsw.inc. The six sprite tables are
 * indexed off each other's base by the $2101 handler, so their order and sizes
 * are load-bearing. */
static void test_regswdata(void)
{
    ZT_SECTION("regsw: sprite tables");
    ZT_CHECK_INT(GAP(reg2101w_objsize1[0], reg2101w_objsize2[0]), 8);
    ZT_CHECK_INT(GAP(reg2101w_objsize2[0], reg2101w_objmovs1[0]), 8);
    ZT_CHECK_INT(GAP(reg2101w_objmovs1[0], reg2101w_objmovs2[0]), 8);
    ZT_CHECK_INT(GAP(reg2101w_objmovs2[0], reg2101w_objadds1[0]), 8);
    ZT_CHECK_INT(GAP(reg2101w_objadds1[0], reg2101w_objadds2[0]), 16);
    ZT_CHECK_INT(GAP(reg2101w_objadds2[0], bgscrolPrev), 16);
    ZT_CHECK_INT(reg2101w_objsize1[5], 16);
    ZT_CHECK_INT(reg2101w_objsize2[2], 64);
    ZT_CHECK_INT(reg2101w_objadds1[5], 12);
    ZT_CHECK_INT(reg2101w_objadds2[2], 8);

    ZT_SECTION("regsw: the rest");
    ZT_CHECK_INT(GAP(bgscrolPrev, bg1scrolx_m7), 1);
    ZT_CHECK_INT(GAP(bg1scrolx_m7, bg1scroly_m7), 2);
    ZT_CHECK_INT(GAP(bg1scroly_m7, multchange), 2);
    ZT_CHECK_INT(GAP(multchange, m7byte), 1);
    ZT_CHECK_INT(multchange, 1);
    ZT_CHECK_INT(GAP(prevoamptr, oamlow), 1);
    ZT_CHECK_INT(GAP(oamlow, MultiTapStat), 1);
}

int main(void)
{
    test_dsp_savestate_offsets();
    test_dsp_layout();
    test_dsp_values();
    test_spcdata();
    test_mode716data();
    test_execdata();
    test_makevid();
    test_newgfx();
    test_sa1proc();
    test_makev16t();
    test_newgfx16data();
    test_regsdata();
    test_regswdata();
    printf("data-only asm port layout tests\n");
    ZT_RESULTS();
}
