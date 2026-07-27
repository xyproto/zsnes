/* Layout tests for the data-only asm ports (cpu/dspproc.c, video/makevid.c,
 * video/newgfx.c).
 *
 * These files are pure data blocks emitted through asmdata.h.  Nothing in
 * them is a function, so the only thing that can regress is the layout: the
 * save-state code copies the DSP block by raw byte distance (PHdspsave and
 * friends), and the renderers reach some of the newgfx slots by offset from a
 * neighbour.  The numbers below are the ones the original NASM objects had. */
#include <stdint.h>
#include <stdio.h>

#include "zstest.h"

typedef uint8_t u1;
typedef uint32_t u4;

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

int main(void)
{
    test_dsp_savestate_offsets();
    test_dsp_layout();
    test_dsp_values();
    test_makevid();
    test_newgfx();
    printf("data-only asm port layout tests\n");
    ZT_RESULTS();
}
