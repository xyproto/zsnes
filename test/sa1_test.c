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

    ZT_RESULTS();
}
