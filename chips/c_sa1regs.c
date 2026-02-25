#include <string.h>

#include "../endmem.h"
#include "../init.h"
#include "../ui.h"
#include "c_sa1regs.h"
#include "sa1proc.h"
#include "sa1regs.h"

void RTCinit(void)
{
    RTCPtr = 0;
}

void RTCReset(void)
{
    REGPTR(0x2800) = RTC2800;
}

void RTCReset2(void)
{
    REGPTW(0x2801) = RTC2801w;
}

void SA1Reset(void)
{
    memset(SA1_BRF, 0, sizeof(SA1_BRF));
    SA1_in_cc1_dma = 0;
    SA1_CC2_line = 0;
    SA1IRQData[1] = 0;
    SA1Mode = SA1Mode & 0xFFFFFF00;
    SA1Status = 0;
    SA1Control = 0x20;
    SA1DoIRQ = 0;
    irqv = irqv2;
    nmiv = nmiv2;
    SA1RegPCS = romdata - 0x8000;
    CurBWPtr = SA1RAMArea - 0x6000;
    SA1BWPtr = SA1RAMArea - 0x6000;
    SNSBWPtr = SA1RAMArea - 0x6000;
    SA1xa = 0;
    SA1xx = 0;
    SA1xy = 0;
    SA1xd = 0;
    SA1xdb = 0;
    SA1xpb = 0;
    SA1xs = 0x01FF;
    SA1RegP = 0;
    SA1RegE = 0;
    SA1IRQExec = 0;
    SA1IRQEnable = 0;
    SA1Message = 0;
    SA1Overflow = 0;
}

void UpdateArithStuff(void)
{
    if (SA1ARC[1] != 1)
        return;

    SA1ARC[1] = 0;
    if (SA1ARC[0] & 0x02) { // Cumultative sum
        s4 const product = (s2)SA1AR1 * (s2)SA1AR2;
        s8 const sum = ((s8)((u8)SA1ARR2 << 32 | (u8)SA1ARR1) << 24 >> 24) + product;
        s8 const sign = sum >> 39;
        SA1ARR1 = (u4)sum;
        SA1ARR2 = sum >> 32 & 0x000000FF;
        // Set overflow bit if sum exceeds 40bits.
        SA1Overflow = sign != 0 && sign != -1 ? 0x80 : 0x00;
    } else if (SA1ARC[0] & 0x01) { // Divison
        s2 const dividend = SA1AR1;
        u2 const divisor = SA1AR2;
        if (divisor != 0) {
            s2 const quotient = dividend / divisor;
            u2 const reminder = dividend % divisor;
            SA1ARR1 = (u4)reminder << 16 | (u2)quotient;
        } else { // Invalid
            SA1ARR1 = 0;
        }
    } else { // Multiplication
        SA1ARR1 = (s4)(s2)SA1AR1 * (s2)SA1AR2;
    }
}

static void executesa1dma(void)
{
    sa1dmaptrs = SA1DMAInfo & 0x01 ? &SA1RAMArea[SA1DMASource & 0x0003FFFF] : // BWRAM
        SA1DMAInfo & 0x02 ? &IRAM[SA1DMASource & 0x000007FF]
                          : // IRAM
        ((u1* const*)snesmmap)[SA1DMASource >> 16 & 0xFF] + (SA1DMASource & 0x0000FFFF);
    memcpy(sa1dmaptr, sa1dmaptrs, SA1DMACount);
}

void sa1dmairam(void)
{
    sa1dmaptr = &IRAM[SA1DMADest & 0x000007FF];
    executesa1dma();
}

void sa1dmabwram(void)
{
    sa1dmaptr = &SA1RAMArea[SA1DMADest & 0x03FFFF];
    executesa1dma();
}

void initSA1regs(void)
{
    REGPTR(0x2300) = sa12300r;
    REGPTR(0x2301) = sa12301r;
    REGPTR(0x2302) = sa12302r;
    REGPTR(0x2303) = sa12303r;
    REGPTR(0x2304) = sa12304r;
    REGPTR(0x2305) = sa12305r;
    REGPTR(0x2306) = sa12306r;
    REGPTR(0x2307) = sa12307r;
    REGPTR(0x2308) = sa12308r;
    REGPTR(0x2309) = sa12309r;
    REGPTR(0x230A) = sa1230Ar;
    REGPTR(0x230B) = sa1230Br;
    REGPTR(0x230C) = sa1230Cr;
    REGPTR(0x230D) = sa1230Dr;
    REGPTR(0x230E) = sa1230Er;

    // Set IRam, memory address 3000-37FF
    eop** i = &REGPTR(0x3000);
    do
        *i = IRamRead;
    while (++i != &REGPTR(0x3800));
}

void initSA1regsw(void)
{
    REGPTW(0x2200) = sa12200w;
    REGPTW(0x2201) = sa12201w;
    REGPTW(0x2202) = sa12202w;
    REGPTW(0x2203) = sa12203w;
    REGPTW(0x2204) = sa12204w;
    REGPTW(0x2205) = sa12205w;
    REGPTW(0x2206) = sa12206w;
    REGPTW(0x2207) = sa12207w;
    REGPTW(0x2208) = sa12208w;
    REGPTW(0x2209) = sa12209w;
    REGPTW(0x220A) = sa1220Aw;
    REGPTW(0x220B) = sa1220Bw;
    REGPTW(0x220C) = sa1220Cw;
    REGPTW(0x220D) = sa1220Dw;
    REGPTW(0x220E) = sa1220Ew;
    REGPTW(0x220F) = sa1220Fw;
    REGPTW(0x2210) = sa12210w;
    REGPTW(0x2211) = sa12211w;
    REGPTW(0x2212) = sa12212w;
    REGPTW(0x2213) = sa12213w;
    REGPTW(0x2214) = sa12214w;
    REGPTW(0x2215) = sa12215w;

    REGPTW(0x2220) = sa12220w;
    REGPTW(0x2221) = sa12221w;
    REGPTW(0x2222) = sa12222w;
    REGPTW(0x2223) = sa12223w;

    REGPTW(0x2224) = sa12224w;
    REGPTW(0x2225) = sa12225w;
    // Missing 2226-222A

    // Bitmap register file
    REGPTW(0x2240) = sa12240w;
    REGPTW(0x2241) = sa12241w;
    REGPTW(0x2242) = sa12242w;
    REGPTW(0x2243) = sa12243w;
    REGPTW(0x2244) = sa12244w;
    REGPTW(0x2245) = sa12245w;
    REGPTW(0x2246) = sa12246w;
    REGPTW(0x2247) = sa12247w;
    REGPTW(0x2248) = sa12248w;
    REGPTW(0x2249) = sa12249w;
    REGPTW(0x224A) = sa1224Aw;
    REGPTW(0x224B) = sa1224Bw;
    REGPTW(0x224C) = sa1224Cw;
    REGPTW(0x224D) = sa1224Dw;
    REGPTW(0x224E) = sa1224Ew;
    REGPTW(0x224F) = sa1224Fw;

    REGPTW(0x2230) = sa12230w;
    REGPTW(0x2231) = sa12231w;
    REGPTW(0x2232) = sa12232w;
    REGPTW(0x2233) = sa12233w;
    REGPTW(0x2234) = sa12234w;
    REGPTW(0x2235) = sa12235w;
    REGPTW(0x2236) = sa12236w;
    REGPTW(0x2237) = sa12237w;
    REGPTW(0x2238) = sa12238w;
    REGPTW(0x2239) = sa12239w;
    REGPTW(0x223F) = sa1223Fw;

    REGPTW(0x2250) = sa12250w;
    REGPTW(0x2251) = sa12251w;
    REGPTW(0x2252) = sa12252w;
    REGPTW(0x2253) = sa12253w;
    REGPTW(0x2254) = sa12254w;

    REGPTW(0x2259) = sa12259w;
    REGPTW(0x225A) = sa1225Aw;
    REGPTW(0x225B) = sa1225Bw;

    eop** i = &REGPTW(0x3000);
    *i++ = IRamWrite2;
    do
        *i = IRamWrite;
    while (++i != &REGPTW(0x3800));
}

void SDD1Reset(void)
{
    REGPTW(0x4801) = sdd14801w;
    REGPTW(0x4802) = dbstop;
    REGPTW(0x4803) = dbstop;
    REGPTW(0x4804) = sdd14804w;
    REGPTW(0x4805) = sdd14805w;
    REGPTW(0x4806) = sdd14806w;
    REGPTW(0x4807) = sdd14807w;
    REGPTW(0x4808) = dbstop;
    REGPTW(0x4809) = dbstop;
    REGPTW(0x480A) = dbstop;
    REGPTW(0x480B) = dbstop;
    REGPTW(0x480C) = dbstop;
    REGPTW(0x480D) = dbstop;
    REGPTW(0x480E) = dbstop;
    REGPTW(0x480F) = dbstop;
}

void initSDD1regs(void)
{
    REGPTR(0x4804) = sdd14804;
    REGPTR(0x4805) = sdd14805;
    REGPTR(0x4806) = sdd14806;
    REGPTR(0x4807) = sdd14807;
}
