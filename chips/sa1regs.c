/* C port of sa1regs.asm (SA-1 register interface). STAGE 1: data block.
   SA1Mode..IRAM must stay contiguous and in this exact order: zstate.c saves
   the run &SA1Mode .. PHnum2writesa1reg bytes. Shared with sa1proc.asm (asm). */
#include <stdint.h>

/* RTC (not part of the save block) */
uint8_t RTCData[16] = { [0] = 0x0F, [14] = 0x0F };
uint32_t RTCPtr, RTCPtr2, RTCRest;

__asm__(
    ".pushsection .data.sa1state,\"aw\",@progbits\n"
    ".global SA1Mode\nSA1Mode:\n.long 0\n"
    ".global SA1Control\nSA1Control:\n.long 0\n"
    ".global SA1BankPtr\nSA1BankPtr:\n.long 0\n"
    ".global SA1ResetV\nSA1ResetV:\n.long 0\n"
    ".global SA1NMIV\nSA1NMIV:\n.long 0\n"
    ".global SA1IRQV\nSA1IRQV:\n.long 0\n"
    ".global SA1RV\nSA1RV:\n.long 0\n"
    ".global CurBWPtr\nCurBWPtr:\n.long 0\n"
    ".global SA1TempVar\nSA1TempVar:\n.long 0\n"
    ".global SA1IRQEn\nSA1IRQEn:\n.long 0\n"
    ".global SA1Message\nSA1Message:\n.long 0\n"
    ".global SA1IRQExec\nSA1IRQExec:\n.long 0\n"
    ".global SA1IRQEnable\nSA1IRQEnable:\n.long 0\n"
    ".global SA1DoIRQ\nSA1DoIRQ:\n.long 0\n"
    ".global SA1ARC\nSA1ARC:\n.long 0\n"
    ".global SA1AR1\nSA1AR1:\n.long 0\n"
    ".global SA1AR2\nSA1AR2:\n.long 0\n"
    ".global SA1ARR1\nSA1ARR1:\n.long 0\n"
    ".global SA1ARR2\nSA1ARR2:\n.long 0\n"
    ".global SA1Stat\nSA1Stat:\n.long 0\n"
    ".global SNSNMIV\nSNSNMIV:\n.long 0\n"
    ".global SNSIRQV\nSNSIRQV:\n.long 0\n"
    ".global SA1DMACount\nSA1DMACount:\n.long 0\n"
    ".global SA1DMAInfo\nSA1DMAInfo:\n.long 0\n"
    ".global SA1DMAChar\nSA1DMAChar:\n.long 0\n"
    ".global SA1DMASource\nSA1DMASource:\n.long 0\n"
    ".global SA1DMADest\nSA1DMADest:\n.long 0\n"
    ".global SA1IRQTemp\nSA1IRQTemp:\n.long 0\n"
    ".global SA1BankSw\nSA1BankSw:\n.long 1\n"
    ".global SA1BankVal\nSA1BankVal:\n.byte 0,1,2,3\n"
    ".global BWShift\nBWShift:\n.long 0\n"
    ".global BWAndAddr\nBWAndAddr:\n.long 0\n"
    ".global BWAnd\nBWAnd:\n.long 0\n"
    ".global BWRAnd\nBWRAnd:\n.long 0\n"
    ".global SA1_in_cc1_dma\nSA1_in_cc1_dma:\n.long 0\n"
    ".global SA1_CC2_line\nSA1_CC2_line:\n.long 0\n"
    ".global SA1_BRF\nSA1_BRF:\n.zero 16\n"
    ".zero 432\n" /* SA1Reserved (unnamed padding) */
    ".global SA1xa\nSA1xa:\n.long 0\n"
    ".global SA1xx\nSA1xx:\n.long 0\n"
    ".global SA1xy\nSA1xy:\n.long 0\n"
    ".global SA1xd\nSA1xd:\n.long 0\n"
    ".global SA1xdb\nSA1xdb:\n.long 0\n"
    ".global SA1xpb\nSA1xpb:\n.long 0\n"
    ".global SA1xs\nSA1xs:\n.long 0\n"
    ".global SA1RegP\nSA1RegP:\n.long 0\n"
    ".global SA1RegE\nSA1RegE:\n.long 0\n"
    ".global SA1RegPCS\nSA1RegPCS:\n.long 0\n"
    ".global SA1BWPtr\nSA1BWPtr:\n.long 0\n"
    ".global SA1Ptr\nSA1Ptr:\n.long 0\n"
    ".global SA1Overflow\nSA1Overflow:\n.long 0\n"
    ".global VarLenAddr\nVarLenAddr:\n.long 0\n"
    ".global VarLenAddrB\nVarLenAddrB:\n.long 0\n"
    ".global VarLenBarrel\nVarLenBarrel:\n.long 0\n"
    ".global SA1TimerVal\nSA1TimerVal:\n.long 0\n"
    ".global SA1TimerSet\nSA1TimerSet:\n.long 0\n"
    ".global SA1TimerCount\nSA1TimerCount:\n.long 0\n"
    ".global SA1IRQData\nSA1IRQData:\n.long 0\n"
    ".global SNSRegP\nSNSRegP:\n.long 0\n"
    ".global SNSRegE\nSNSRegE:\n.long 0\n"
    ".global SNSRegPCS\nSNSRegPCS:\n.long 0\n"
    ".global SNSBWPtr\nSNSBWPtr:\n.long 0\n"
    ".global SNSPtr\nSNSPtr:\n.long 0\n"
    ".global IRAM\nIRAM:\n.zero 2049\n"
    ".global PHnum2writesa1reg\nPHnum2writesa1reg:\n.long . - SA1Mode\n" /* save-state block size */
    ".popsection\n");

/* trailing state (after the save block) */
uint32_t SA1RAMArea, SA1Temp, Sdd1Mode, Sdd1Bank, Sdd1Addr, Sdd1NewAddr;

/* ===== Stage 2: status reads (0x2300-0x230B) + IRAM access ===== */
#include "regabi.h"

extern uint32_t SA1Message, SA1IRQExec, SA1IRQData, SA1ARR1, SA1ARR2, SA1Overflow;
extern uint32_t SA1TimerSet, SA1TimerCount;
extern uint8_t IRAM[2049]; /* defined in the asm state block above */
extern uint8_t CurrentExecSA1; /* sa1proc.asm */
extern uint16_t curypos; /* current scanline */

#define BYTE(v, n) (((uint8_t*)&(v))[n])

REGABI_REG_READ8(sa12300r);
uint8_t c_sa12300r(void)
{
    uint8_t al = BYTE(SA1Message, 1);
    if (BYTE(SA1IRQExec, 0) & 1)
        al |= 0x80;
    if (BYTE(SA1IRQExec, 0) & 2)
        al |= 0x20;
    al |= BYTE(SA1IRQData, 1) & 0x50;
    return al;
}

REGABI_REG_READ8(sa12301r);
uint8_t c_sa12301r(void)
{
    uint8_t al = BYTE(SA1Message, 0);
    if (BYTE(SA1IRQExec, 1) & 1)
        al |= 0x80;
    return al;
}

/* H/V free-running counters; the asm adds `dh` (a dispatch dot hint) the reg ABI
   can't carry, so the free-running path is approximate (dh treated as 0). */
REGABI_REG_READ8(sa12302r);
uint8_t c_sa12302r(void)
{
    if (BYTE(SA1TimerSet, 0) & 0x80)
        return BYTE(SA1TimerCount, 0);
    return (uint8_t)(CurrentExecSA1 << 2);
}
REGABI_REG_READ8(sa12303r);
uint8_t c_sa12303r(void)
{
    if (BYTE(SA1TimerSet, 0) & 0x80)
        return BYTE(SA1TimerCount, 1) & 1;
    return (uint8_t)(CurrentExecSA1 >> 3);
}
REGABI_REG_READ8(sa12304r);
uint8_t c_sa12304r(void)
{
    if (BYTE(SA1TimerSet, 0) & 0x80)
        return (uint8_t)(*(uint16_t*)((uint8_t*)&SA1TimerCount + 1) >> 1);
    return BYTE(curypos, 0);
}
REGABI_REG_READ8(sa12305r);
uint8_t c_sa12305r(void)
{
    if (BYTE(SA1TimerSet, 0) & 0x80)
        return (uint8_t)((*(uint16_t*)((uint8_t*)&SA1TimerCount + 2) >> 1) & 1);
    return BYTE(curypos, 1);
}

REGABI_REG_READ8(sa12306r);
uint8_t c_sa12306r(void) { return BYTE(SA1ARR1, 0); }
REGABI_REG_READ8(sa12307r);
uint8_t c_sa12307r(void) { return BYTE(SA1ARR1, 1); }
REGABI_REG_READ8(sa12308r);
uint8_t c_sa12308r(void) { return BYTE(SA1ARR1, 2); }
REGABI_REG_READ8(sa12309r);
uint8_t c_sa12309r(void) { return BYTE(SA1ARR1, 3); }
REGABI_REG_READ8(sa1230Ar);
uint8_t c_sa1230Ar(void) { return BYTE(SA1ARR2, 0); }
REGABI_REG_READ8(sa1230Br);
uint8_t c_sa1230Br(void) { return BYTE(SA1Overflow, 0); }

/* IRAM at 0x3000-0x37FF (handlers get the address in ECX -> BANK ABI) */
REGABI_BANK_READ8(IRamRead);
uint8_t c_IRamRead(uint32_t a) { return IRAM[(uint16_t)a - 0x3000]; }
REGABI_BANK_WRITE8(IRamWrite);
void c_IRamWrite(uint32_t a, uint8_t v) { IRAM[(uint16_t)a - 0x3000] = v; }
REGABI_BANK_WRITE8(IRamWrite2);
void c_IRamWrite2(uint32_t a, uint8_t v) { IRAM[(uint16_t)a - 0x3000] = v; }

/* ===== Stage 3: control / IRQ / vector writes (0x2200-0x220F) ===== */
extern uint32_t SA1DoIRQ, SA1Control, SA1BankPtr, SA1ResetV, SA1Ptr, SA1xpb, SA1xs;
extern uint32_t SA1RegPCS, SA1IRQEnable, SA1NMIV, SA1IRQV, SA1IRQEn, SNSNMIV, SNSIRQV;
extern uint8_t* romdata;
extern uint16_t irqv, irqv2, nmiv, nmiv2; /* initdata.c */

REGABI_REG_WRITE8(sa12200w);
void c_sa12200w(uint8_t al) /* SA-1 CPU control */
{
    uint8_t oldctrl = BYTE(SA1Control, 0);
    BYTE(SA1Message, 0) = al & 0x0F;
    if (al & 0x80)
        BYTE(SA1DoIRQ, 0) |= 1;
    if (al & 0x10)
        BYTE(SA1DoIRQ, 0) |= 2;
    BYTE(SA1Control, 0) = al;
    if ((oldctrl & 0x20) && !(al & 0x20)) { /* SA-1 leaving reset */
        SA1BankPtr = (uint32_t)(uintptr_t)romdata;
        SA1Ptr = (uint32_t)((uintptr_t)romdata + (uint16_t)SA1ResetV - 0x8000);
        BYTE(SA1xpb, 0) = 0;
        *(uint16_t*)&SA1xs = 0x1FF;
        SA1RegPCS = (uint32_t)((uintptr_t)romdata - 0x8000);
    }
}

REGABI_REG_WRITE8(sa12201w);
void c_sa12201w(uint8_t al) { BYTE(SA1IRQEnable, 0) = al; }

REGABI_REG_WRITE8(sa12202w); /* IRQ clear */
void c_sa12202w(uint8_t al)
{
    if (al & 0x80) {
        BYTE(SA1IRQExec, 0) &= 0xFE;
        BYTE(SA1DoIRQ, 0) &= 0xFB;
    }
    if (al & 0x20) {
        BYTE(SA1IRQExec, 0) &= 0xFD;
        BYTE(SA1DoIRQ, 0) &= 0xF7;
    }
}

#define SA1_QUICKW(name, target, byte) \
    REGABI_REG_WRITE8(name);           \
    void c_##name(uint8_t al) { BYTE(target, byte) = al; }

SA1_QUICKW(sa12203w, SA1ResetV, 0)
SA1_QUICKW(sa12204w, SA1ResetV, 1)
SA1_QUICKW(sa12205w, SA1NMIV, 0)
SA1_QUICKW(sa12206w, SA1NMIV, 1)
SA1_QUICKW(sa12207w, SA1IRQV, 0)
SA1_QUICKW(sa12208w, SA1IRQV, 1)

REGABI_REG_WRITE8(sa12209w); /* SNES IRQ from SA-1 */
void c_sa12209w(uint8_t al)
{
    BYTE(SA1IRQData, 1) = al;
    if (al & 0x80)
        BYTE(SA1DoIRQ, 0) |= 4;
    BYTE(SA1Message, 1) = al & 0x0F;
    irqv = (al & 0x40) ? (uint16_t)SNSIRQV : irqv2;
    nmiv = (al & 0x10) ? (uint16_t)SNSNMIV : nmiv2;
}

SA1_QUICKW(sa1220Aw, SA1IRQEn, 0)

REGABI_REG_WRITE8(sa1220Bw); /* SA-1 IRQ clear */
void c_sa1220Bw(uint8_t al)
{
    if (al & 0x80) {
        BYTE(SA1IRQExec, 1) = 0;
        BYTE(SA1DoIRQ, 0) &= 0xFE;
    }
    if (al & 0x10) {
        BYTE(SA1IRQExec, 2) = 0;
        BYTE(SA1DoIRQ, 0) &= 0xFD;
    }
}

SA1_QUICKW(sa1220Cw, SNSNMIV, 0)
SA1_QUICKW(sa1220Dw, SNSNMIV, 1)
SA1_QUICKW(sa1220Ew, SNSIRQV, 0)
SA1_QUICKW(sa1220Fw, SNSIRQV, 1)
