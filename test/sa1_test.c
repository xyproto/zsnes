/*
 * SA-1 register interface unit tests (chips/sa1regs.c).
 *
 * Stage 1 covers the save-state-critical data block: the run from SA1Mode
 * through IRAM must stay contiguous and in the exact order the asm used, since
 * zstate.c saves/loads &SA1Mode .. PHnum2writesa1reg as one block.  Later
 * stages add handler tests (RTC, control/IRQ, mapping, DMA, math, ...).
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "zstest.h"

/* --- SA-1 state block (chips/sa1regs.c) ----------------------------------- */
extern uint32_t SA1Mode;
extern uint8_t SA1BankVal[4];
extern uint32_t SA1BankSw;
extern uint8_t IRAM[2049];
extern uint32_t PHnum2writesa1reg;
extern uint8_t RTCData[16];

/* Stage 2 state + handlers */
extern uint32_t SA1Message, SA1IRQExec, SA1IRQData, SA1ARR1, SA1ARR2, SA1Overflow;
extern uint32_t SA1TimerSet, SA1TimerCount;
extern uint8_t IRAM[2049];
/* externs that sa1regs.c references; the test owns them here */
uint8_t CurrentExecSA1;
uint16_t curypos;

uint8_t c_sa12300r(void), c_sa12301r(void), c_sa12302r(void), c_sa12303r(void);
uint8_t c_sa12304r(void), c_sa12305r(void), c_sa12306r(void), c_sa12309r(void);
uint8_t c_sa1230Ar(void), c_sa1230Br(void);
uint8_t c_IRamRead(uint32_t);
void c_IRamWrite(uint32_t, uint8_t);

/* Stage 3 state + handlers */
extern uint32_t SA1Control, SA1DoIRQ, SA1BankPtr, SA1Ptr, SA1ResetV, SA1xpb, SA1xs;
extern uint32_t SA1RegPCS, SA1IRQEnable, SA1NMIV, SA1IRQV, SNSNMIV, SNSIRQV;
/* externs sa1regs.c needs; test owns them */
uint8_t rombuf[0x10000];
uint8_t* romdata = rombuf;
uint16_t irqv, irqv2, nmiv, nmiv2;

void c_sa12200w(uint8_t), c_sa12201w(uint8_t), c_sa12202w(uint8_t), c_sa12203w(uint8_t);
void c_sa12205w(uint8_t), c_sa12209w(uint8_t), c_sa1220Bw(uint8_t), c_sa1220Cw(uint8_t);

/* Stage 4 state + handlers */
extern uint32_t SA1RAMArea, SNSBWPtr, CurBWPtr, SA1BWPtr, BWShift, BWAnd, Sdd1Mode;
/* externs sa1regs.c needs; test owns them */
uint32_t NumofBanks;
uint8_t *snesmmap[256], *snesmap2[256];
uint8_t SA1Status, BWUsed2, SDD1BankA[4], AddrNoIncr;
void* memtabler8[256];
void memaccessbankr8sdd1(void) { }

void c_sa12220w(uint8_t), c_sa12224w(uint8_t), c_sa12225w(uint8_t);
void c_sdd14804w(uint8_t), c_sdd14801w(uint8_t);
uint8_t c_sdd14804(void);

/* Stage 5 state + handlers */
extern uint32_t SA1ARC, SA1AR1, SA1AR2;
extern uint32_t SA1DMAInfo, SA1DMAChar, SA1DMASource, SA1DMADest, SA1DMACount;
extern uint32_t SA1_CC2_line, SA1_in_cc1_dma;
/* called by the handlers; the test owns them and counts invocations */
int arith_calls, dmairam_calls, dmabwram_calls, cc2_calls;
void UpdateArithStuff(void) { arith_calls++; }
void sa1dmairam(void) { dmairam_calls++; }
void sa1dmabwram(void) { dmabwram_calls++; }
void SA1_DMA_CC2(void) { cc2_calls++; }
/* RTC handlers pull wall-clock time; the test owns deterministic stubs */
uint8_t debuggeron;
uint32_t GetTime(void) { return 0x231259; } /* 12:59:23 BCD */
uint32_t GetDate(void) { return 0x30190625; }

void c_sa12250w(uint8_t), c_sa12251w(uint8_t), c_sa12252w(uint8_t);
void c_sa12253w(uint8_t), c_sa12254w(uint8_t);
void c_sa12230w(uint8_t), c_sa12231w(uint8_t), c_sa12236w(uint8_t), c_sa12237w(uint8_t);

/* Stage 6 state + handlers */
extern uint8_t SA1_BRF[16];
extern uint32_t VarLenAddr, VarLenAddrB, VarLenBarrel;
void c_sa12258w(uint8_t), c_sa12259w(uint8_t), c_sa1225Aw(uint8_t), c_sa1225Bw(uint8_t);
void c_sa12240w(uint8_t), c_sa12247w(uint8_t), c_sa1224Fw(uint8_t);
uint8_t c_sa1230Cr(void), c_sa1230Dr(void), c_sa1230Er(void);
void sa1chconv(void);

/* Stage 7 state + handlers */
extern uint32_t RTCPtr, RTCPtr2;
void c_sa12212w(uint8_t), c_sa12213w(uint8_t), c_sa12214w(uint8_t), c_sa12215w(uint8_t);
void c_RTC2801w(uint8_t);
uint8_t c_RTC2800(void);

#define OFF(sym) ((char*)&(sym) - (char*)&SA1Mode)
#define BYTE(v, n) (((uint8_t*)&(v))[n])

int main(void)
{
    ZT_SECTION("save-state block layout (must match sa1regs.asm)");
    /* Offsets within the &SA1Mode block, verified against the asm object. */
    ZT_CHECK_INT(OFF(SA1BankVal), 116);
    ZT_CHECK_INT(OFF(IRAM), 692);
    /* zstate.c saves this many bytes starting at &SA1Mode. */
    ZT_CHECK_INT(PHnum2writesa1reg, 2741);
    /* IRAM is the last member; block size = IRAM offset + sizeof(IRAM). */
    ZT_CHECK_INT(OFF(IRAM) + (int)sizeof(IRAM), (int)PHnum2writesa1reg);

    ZT_SECTION("data block initial values");
    ZT_CHECK_INT(SA1BankSw, 1);
    ZT_CHECK_INT(SA1BankVal[0], 0);
    ZT_CHECK_INT(SA1BankVal[1], 1);
    ZT_CHECK_INT(SA1BankVal[2], 2);
    ZT_CHECK_INT(SA1BankVal[3], 3);
    ZT_CHECK_INT(RTCData[0], 0x0F);
    ZT_CHECK_INT(RTCData[14], 0x0F);
    ZT_CHECK_INT(RTCData[1], 0);

    ZT_SECTION("status reads (0x2300-0x230B)");
    /* 2300r: message hi byte + IRQ-exec/data flag bits */
    SA1Message = 0x1234;
    SA1IRQExec = 0;
    SA1IRQData = 0;
    ZT_CHECK_INT(c_sa12300r(), 0x12); /* hi byte of message */
    ZT_CHECK_INT(c_sa12301r(), 0x34); /* lo byte of message */
    SA1IRQExec = 0x0001; /* byte0 bit0 -> 0x80 in 2300r */
    ZT_CHECK_INT(c_sa12300r(), 0x12 | 0x80);
    SA1IRQExec = 0x0100; /* byte1 bit0 -> 0x80 in 2301r */
    ZT_CHECK_INT(c_sa12301r(), 0x34 | 0x80);
    SA1IRQExec = 0;
    SA1IRQData = 0x5000; /* byte1 & 0x50 -> ORed into 2300r */
    ZT_CHECK_INT(c_sa12300r(), 0x12 | 0x50);

    /* 2302/2303: free-running H counter (timer off) from CurrentExecSA1 */
    SA1TimerSet = 0;
    CurrentExecSA1 = 1;
    ZT_CHECK_INT(c_sa12302r(), 1 << 2);
    /* timer on -> reads SA1TimerCount */
    SA1TimerSet = 0x80;
    SA1TimerCount = 0xAB;
    ZT_CHECK_INT(c_sa12302r(), 0xAB);

    /* 2304/2305: V counter (timer off) from curypos */
    SA1TimerSet = 0;
    curypos = 0x0140;
    ZT_CHECK_INT(c_sa12304r(), 0x40);
    ZT_CHECK_INT(c_sa12305r(), 0x01);

    /* arithmetic result + overflow reads */
    SA1ARR1 = 0xDEADBEEF;
    ZT_CHECK_INT(c_sa12306r(), 0xEF);
    ZT_CHECK_INT(c_sa12309r(), 0xDE);
    SA1ARR2 = 0x77;
    ZT_CHECK_INT(c_sa1230Ar(), 0x77);
    SA1Overflow = 1;
    ZT_CHECK_INT(c_sa1230Br(), 1);

    ZT_SECTION("IRAM access (0x3000-0x37FF)");
    c_IRamWrite(0x3000, 0xAA);
    c_IRamWrite(0x37FF, 0x55);
    ZT_CHECK_INT(IRAM[0], 0xAA);
    ZT_CHECK_INT(IRAM[0x7FF], 0x55);
    ZT_CHECK_INT(c_IRamRead(0x3000), 0xAA);
    ZT_CHECK_INT(c_IRamRead(0x37FF), 0x55);

    ZT_SECTION("control / IRQ / vector writes (0x2200-0x220F)");
    /* 2200: leaving reset (old ctrl bit5 set, new clear) sets up SA-1 PC */
    SA1Control = 0x20;
    SA1DoIRQ = 0;
    SA1ResetV = 0x8123;
    c_sa12200w(0x00);
    ZT_CHECK_INT(BYTE(SA1Control, 0), 0x00);
    ZT_CHECK_INT(SA1BankPtr, (uint32_t)(uintptr_t)romdata);
    ZT_CHECK_INT(SA1Ptr - SA1BankPtr, 0x123); /* ResetV - 0x8000 */
    ZT_CHECK_INT(SA1BankPtr - SA1RegPCS, 0x8000);
    ZT_CHECK_INT((uint16_t)SA1xs, 0x1FF);
    /* 2200: IRQ/NMI request bits set SA1DoIRQ */
    SA1DoIRQ = 0;
    SA1Control = 0;
    c_sa12200w(0x90); /* 0x80|0x10 */
    ZT_CHECK_INT(BYTE(SA1DoIRQ, 0) & 3, 3);

    /* 2202: clearing IRQ flags */
    SA1IRQExec = 0xFFFFFFFF;
    SA1DoIRQ = 0xFF;
    c_sa12202w(0x80);
    ZT_CHECK_INT(BYTE(SA1IRQExec, 0), 0xFE);
    ZT_CHECK_INT(BYTE(SA1DoIRQ, 0), 0xFB);

    /* 2209: SNES IRQ/NMI vector switching */
    irqv2 = 0x1111;
    nmiv2 = 0x2222;
    SNSIRQV = 0x3333;
    SNSNMIV = 0x4444;
    c_sa12209w(0x40); /* bit6 -> use SNSIRQV; bit4 clear -> nmiv2 */
    ZT_CHECK_INT(irqv, 0x3333);
    ZT_CHECK_INT(nmiv, 0x2222);
    c_sa12209w(0x10); /* bit4 -> use SNSNMIV; bit6 clear -> irqv2 */
    ZT_CHECK_INT(irqv, 0x1111);
    ZT_CHECK_INT(nmiv, 0x4444);

    /* SA1QuickW byte writes */
    SA1ResetV = 0;
    c_sa12203w(0x9C);
    ZT_CHECK_INT(BYTE(SA1ResetV, 0), 0x9C);
    SNSNMIV = 0;
    c_sa1220Cw(0x7E);
    ZT_CHECK_INT(BYTE(SNSNMIV, 0), 0x7E);

    ZT_SECTION("ROM bank mapping (0x2220-0x2223)");
    NumofBanks = 0; /* != 64 -> use al & 7 */
    c_sa12220w(0x00);
    ZT_CHECK(snesmmap[0] == romdata - 0x8000);
    ZT_CHECK(snesmmap[1] == romdata); /* +0x8000 per LoROM bank */
    ZT_CHECK(snesmap2[0xC0] == romdata); /* HiROM region */
    ZT_CHECK(snesmap2[0xC1] == romdata + 0x10000);
    ZT_CHECK(snesmmap[0xC0] == romdata);
    ZT_CHECK_INT(SA1BankVal[0], 0);
    c_sa12220w(0x82); /* upper set, bank 2 -> 0x200000 */
    ZT_CHECK(snesmmap[0] == romdata + 0x200000 - 0x8000);
    ZT_CHECK(snesmap2[0xC0] == romdata + 0x200000);
    ZT_CHECK_INT(SA1BankVal[0], 0x82);

    ZT_SECTION("BW-RAM bank registers (0x2224-0x2225)");
    SA1RAMArea = 0x01000000;
    SA1Status = 0;
    c_sa12224w(0x03);
    ZT_CHECK_INT(SNSBWPtr, (3u << 13) + SA1RAMArea - 0x6000);
    ZT_CHECK_INT(CurBWPtr, SNSBWPtr); /* SA1Status==0 mirrors SNS */
    SA1Overflow = 0; /* 16-colour path (bit15 clear) */
    c_sa12225w(0x80);
    ZT_CHECK_INT(BYTE(BWShift, 0), 1);
    ZT_CHECK_INT(BYTE(BWAnd, 0), 0x0F);
    c_sa12225w(0x05); /* no bit7 -> linear */
    ZT_CHECK_INT(SA1BWPtr, (5u << 13) + SA1RAMArea - 0x6000);
    ZT_CHECK_INT(BYTE(BWAnd, 0), 0xFF);

    ZT_SECTION("S-DD1 bank registers (0x4804-0x4807, 0x4801)");
    c_sdd14804w(0x05);
    ZT_CHECK_INT(SDD1BankA[0], 0x05);
    ZT_CHECK_INT(c_sdd14804(), 0x05);
    ZT_CHECK(snesmap2[0xC0] == romdata + (5u << 20));
    Sdd1Mode = 0;
    c_sdd14801w(0x01);
    ZT_CHECK_INT(Sdd1Mode, 1);
    ZT_CHECK(memtabler8[0xC0] == (void*)memaccessbankr8sdd1);
    ZT_CHECK(memtabler8[0xFF] == (void*)memaccessbankr8sdd1);
    Sdd1Mode = 7;
    c_sdd14801w(0x00); /* al==0 -> no-op */
    ZT_CHECK_INT(Sdd1Mode, 7);

    ZT_SECTION("arithmetic registers (0x2250-0x2254)");
    SA1ARC = 0;
    SA1ARR1 = 0xFFFFFFFF;
    SA1ARR2 = 0xFFFF;
    c_sa12250w(0x02); /* bit1 -> cumulative reset clears 48-bit result */
    ZT_CHECK_INT(BYTE(SA1ARC, 0), 0x02);
    ZT_CHECK_INT(BYTE(SA1ARC, 1), 1);
    ZT_CHECK_INT(SA1ARR1, 0);
    ZT_CHECK_INT((uint16_t)SA1ARR2, 0);
    arith_calls = 0;
    c_sa12251w(0x12);
    c_sa12252w(0x34);
    c_sa12253w(0x56);
    ZT_CHECK_INT((uint16_t)SA1AR1, 0x3412);
    ZT_CHECK_INT(BYTE(SA1AR2, 0), 0x56);
    ZT_CHECK_INT(arith_calls, 0); /* not triggered until AR2 hi byte */
    c_sa12254w(0x78);
    ZT_CHECK_INT((uint16_t)SA1AR2, 0x7856);
    ZT_CHECK_INT(arith_calls, 1);

    ZT_SECTION("DMA registers (0x2230-0x2239)");
    SA1_CC2_line = 1;
    c_sa12230w(0x00); /* no bit7 -> clears CC2 line */
    ZT_CHECK_INT(SA1_CC2_line, 0);
    ZT_CHECK_INT(BYTE(SA1DMAInfo, 0), 0x00);
    SA1_in_cc1_dma = 1;
    c_sa12231w(0x80); /* bit7 -> clears in-cc1-dma */
    ZT_CHECK_INT(SA1_in_cc1_dma, 0);
    ZT_CHECK_INT(BYTE(SA1DMAChar, 0), 0x80);
    /* 2236: IRAM DMA when DMAInfo has neither char-conv (0x10) nor BW dest (4) */
    SA1DMAInfo = 0;
    dmairam_calls = 0;
    c_sa12236w(0x11);
    ZT_CHECK_INT(BYTE(SA1DMADest, 1), 0x11);
    ZT_CHECK_INT(dmairam_calls, 1);
    /* 2237: BW-RAM DMA when DMAInfo bit2 set and char-conv bit clear */
    SA1DMAInfo = 0x04;
    dmabwram_calls = 0;
    c_sa12237w(0x22);
    ZT_CHECK_INT(BYTE(SA1DMADest, 2), 0x22);
    ZT_CHECK_INT(dmabwram_calls, 1);
    /* 2236 with char-conv bit -> CC#1 path (sets IRQ + in-cc1-dma, no IRAM DMA) */
    SA1DMAInfo = 0x10;
    SA1DoIRQ = 0;
    SA1_in_cc1_dma = 0;
    dmairam_calls = 0;
    c_sa12236w(0x33);
    ZT_CHECK_INT(BYTE(SA1DoIRQ, 0) & 8, 8);
    ZT_CHECK_INT(SA1_in_cc1_dma, 1);
    ZT_CHECK_INT(dmairam_calls, 0);

    ZT_SECTION("char-conversion DMA (sa1chconv)");
    SA1DoIRQ = 0;
    SA1_in_cc1_dma = 0;
    sa1chconv();
    ZT_CHECK_INT(BYTE(SA1DoIRQ, 0) & 8, 8);
    ZT_CHECK_INT(SA1_in_cc1_dma, 1);

    ZT_SECTION("variable-length bit reads (0x2258-0x225B, 0x230C-0x230E)");
    /* 2258: low nibble is shift step (0 -> 16); bit7 also seeds the barrel */
    c_sa12258w(0x84);
    ZT_CHECK_INT(BYTE(VarLenBarrel, 2), 0x84);
    ZT_CHECK_INT(BYTE(VarLenBarrel, 3), 4);
    ZT_CHECK_INT(BYTE(VarLenBarrel, 0), 4);
    ZT_CHECK_INT(BYTE(VarLenBarrel, 1), 4);
    c_sa12258w(0x00); /* step nibble 0 -> 16 */
    ZT_CHECK_INT(BYTE(VarLenBarrel, 3), 16);
    /* set up a 24-bit address into a stub ROM page and read shifted bytes */
    {
        static uint8_t page[0x10000];
        page[0x100] = 0x21;
        page[0x101] = 0x43;
        page[0x102] = 0x65;
        snesmap2[0x12] = page; /* bank 0x12, addr 0x0100 (bit15 clear -> snesmap2) */
        VarLenAddr = 0x120100;
        VarLenAddrB = 0x120100;
        VarLenBarrel = 0; /* shift 0, manual (no auto-inc) */
        ZT_CHECK_INT(c_sa1230Cr(), 0x21); /* low byte */
        ZT_CHECK_INT(c_sa1230Dr(), 0x43); /* second byte */
    }
    ZT_CHECK_INT(c_sa1230Er(), 0x10);

    ZT_SECTION("bitmap register file (0x2240-0x224F)");
    c_sa12240w(0xA0);
    c_sa12247w(0xA7);
    c_sa1224Fw(0xAF);
    ZT_CHECK_INT(SA1_BRF[0], 0xA0);
    ZT_CHECK_INT(SA1_BRF[7], 0xA7);
    ZT_CHECK_INT(SA1_BRF[15], 0xAF);
    /* 0x224F triggers CC#2 only when DMAInfo has bit5/7 set and char-conv clear */
    SA1DMAInfo = 0x80;
    cc2_calls = 0;
    c_sa1224Fw(0x01);
    ZT_CHECK_INT(cc2_calls, 1);
    SA1DMAInfo = 0x80 | 0x10; /* char-conv bit suppresses CC#2 */
    cc2_calls = 0;
    c_sa1224Fw(0x02);
    ZT_CHECK_INT(cc2_calls, 0);

    ZT_SECTION("SA-1 timer count (0x2212-0x2215)");
    c_sa12212w(0x11);
    c_sa12213w(0x22);
    c_sa12214w(0x33);
    c_sa12215w(0x44);
    ZT_CHECK_INT(SA1TimerCount, 0x44332211);

    ZT_SECTION("RTC (0x2800 read, 0x2801 write)");
    RTCPtr = 0;
    RTCPtr2 = 0;
    /* first read latches GetTime/GetDate, returns RTCData[0] then advances */
    ZT_CHECK_INT(c_RTC2800(), 0x0F); /* RTCData[0] */
    ZT_CHECK_INT(RTCData[1], 9); /* seconds ones from 0x231259 */
    ZT_CHECK_INT(RTCData[2], 5); /* seconds tens */
    ZT_CHECK_INT(RTCData[5], 3); /* hours ones */
    ZT_CHECK_INT(c_RTC2800(), 9); /* RTCData[1] */
    /* read wraps to 0 at index 0x0F */
    RTCPtr = 0x0E;
    c_RTC2800();
    ZT_CHECK_INT(RTCPtr, 0);
    /* 0x2801 == 0x0E/0x0D resets RTCPtr2; otherwise stores from index 1 */
    RTCPtr2 = 5;
    c_RTC2801w(0x0E);
    ZT_CHECK_INT(RTCPtr2, 0);
    c_RTC2801w(0x00); /* index 0: no store, advances */
    ZT_CHECK_INT(RTCPtr2, 1);
    c_RTC2801w(0x07); /* index 1: store */
    ZT_CHECK_INT(RTCData[1], 0x07);
    ZT_CHECK_INT(RTCPtr2, 2);
    RTCPtr2 = 14; /* >13: no store, no advance */
    c_RTC2801w(0x99);
    ZT_CHECK_INT(RTCPtr2, 14);

    ZT_RESULTS();
}
