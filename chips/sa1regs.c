/* C port of sa1regs.asm (SA-1 register interface). STAGE 1: data block.
   SA1Mode..IRAM must stay contiguous and in this exact order: zstate.c saves
   the run &SA1Mode .. PHnum2writesa1reg bytes. Shared with sa1proc.asm (asm). */
#include <stdint.h>

#include "../asmdata.h"

/* RTC (not part of the save block) */
uint8_t RTCData[16] = { [0] = 0x0F, [14] = 0x0F };
uint32_t RTCPtr, RTCPtr2, RTCRest;

__asm__(
    ASM_SEC_DATA(".data.sa1state")
        ASM_GSYM(SA1Mode) ".long 0\n" ASM_GSYM(SA1Control) ".long 0\n" ASM_GSYM(SA1BankPtr) ".long 0\n" ASM_GSYM(SA1ResetV) ".long 0\n" ASM_GSYM(SA1NMIV) ".long 0\n" ASM_GSYM(SA1IRQV) ".long 0\n" ASM_GSYM(SA1RV) ".long 0\n" ASM_GSYM(CurBWPtr) ".long 0\n" ASM_GSYM(SA1TempVar) ".long 0\n" ASM_GSYM(SA1IRQEn) ".long 0\n" ASM_GSYM(SA1Message) ".long 0\n" ASM_GSYM(SA1IRQExec) ".long 0\n" ASM_GSYM(SA1IRQEnable) ".long 0\n" ASM_GSYM(SA1DoIRQ) ".long 0\n" ASM_GSYM(SA1ARC) ".long 0\n" ASM_GSYM(SA1AR1) ".long 0\n" ASM_GSYM(SA1AR2) ".long 0\n" ASM_GSYM(SA1ARR1) ".long 0\n" ASM_GSYM(SA1ARR2) ".long 0\n" ASM_GSYM(SA1Stat) ".long 0\n" ASM_GSYM(SNSNMIV) ".long 0\n" ASM_GSYM(SNSIRQV) ".long 0\n" ASM_GSYM(SA1DMACount) ".long 0\n" ASM_GSYM(SA1DMAInfo) ".long 0\n" ASM_GSYM(SA1DMAChar) ".long 0\n" ASM_GSYM(SA1DMASource) ".long 0\n" ASM_GSYM(SA1DMADest) ".long 0\n" ASM_GSYM(SA1IRQTemp) ".long 0\n" ASM_GSYM(SA1BankSw) ".long 1\n" ASM_GSYM(SA1BankVal) ".byte 0,1,2,3\n" ASM_GSYM(BWShift) ".long 0\n" ASM_GSYM(BWAndAddr) ".long 0\n" ASM_GSYM(BWAnd) ".long 0\n" ASM_GSYM(BWRAnd) ".long 0\n" ASM_GSYM(SA1_in_cc1_dma) ".long 0\n" ASM_GSYM(SA1_CC2_line) ".long 0\n" ASM_GSYM(SA1_BRF) ".zero 16\n"
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 ".zero 432\n" /* SA1Reserved (unnamed padding) */
    ASM_GSYM(SA1xa) ".long 0\n" ASM_GSYM(SA1xx) ".long 0\n" ASM_GSYM(SA1xy) ".long 0\n" ASM_GSYM(SA1xd) ".long 0\n" ASM_GSYM(SA1xdb) ".long 0\n" ASM_GSYM(SA1xpb) ".long 0\n" ASM_GSYM(SA1xs) ".long 0\n" ASM_GSYM(SA1RegP) ".long 0\n" ASM_GSYM(SA1RegE) ".long 0\n" ASM_GSYM(SA1RegPCS) ".long 0\n" ASM_GSYM(SA1BWPtr) ".long 0\n" ASM_GSYM(SA1Ptr) ".long 0\n" ASM_GSYM(SA1Overflow) ".long 0\n" ASM_GSYM(VarLenAddr) ".long 0\n" ASM_GSYM(VarLenAddrB) ".long 0\n" ASM_GSYM(VarLenBarrel) ".long 0\n" ASM_GSYM(SA1TimerVal) ".long 0\n" ASM_GSYM(SA1TimerSet) ".long 0\n" ASM_GSYM(SA1TimerCount) ".long 0\n" ASM_GSYM(SA1IRQData) ".long 0\n" ASM_GSYM(SNSRegP) ".long 0\n" ASM_GSYM(SNSRegE) ".long 0\n" ASM_GSYM(SNSRegPCS) ".long 0\n" ASM_GSYM(SNSBWPtr) ".long 0\n" ASM_GSYM(SNSPtr) ".long 0\n" ASM_GSYM(IRAM) ".zero 2049\n" ASM_GSYM(PHnum2writesa1reg) ".long . - SA1Mode\n" /* save-state block size */
    ASM_SEC_END);

/* trailing state (after the save block) */
uint8_t* SA1RAMArea;
uint32_t SA1Temp, Sdd1Mode, Sdd1Bank, Sdd1Addr, Sdd1NewAddr;

/* DMA pointers; zstate.c saves them as one adjacent 8-byte block, so force
   their layout rather than letting -fdata-sections scatter them. */
__asm__(ASM_SEC_DATA(".data.sa1dmaptr")
        ASM_GSYM(sa1dmaptr) ".long 0\n" ASM_GSYM(sa1dmaptrs) ".long 0\n" ASM_SEC_END);

/* ===== Stage 2: status reads (0x2300-0x230B) + IRAM access ===== */
#include "regabi.h"

extern uint32_t SA1Message, SA1IRQExec, SA1IRQData, SA1ARR1, SA1ARR2;
extern uint16_t SA1Overflow;
extern uint32_t SA1TimerSet, SA1TimerCount, SA1TimerVal;
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
extern uint32_t SA1DoIRQ, SA1BankPtr, SA1ResetV, SA1xpb, SA1xs;
extern uint32_t SA1NMIV, SA1IRQV, SA1IRQEn, SNSNMIV, SNSIRQV;
extern uint8_t SA1Control, SA1IRQEnable;
extern uint8_t *SA1Ptr, *SA1RegPCS;
extern uint8_t* romdata;
extern uint16_t irqv, irqv2, nmiv, nmiv2; /* initdata.c */

REGABI_REG_WRITE8(sa12200w);
void c_sa12200w(uint8_t al) /* SA-1 CPU control */
{
    uint8_t oldctrl = SA1Control;
    BYTE(SA1Message, 0) = al & 0x0F;
    if (al & 0x80)
        BYTE(SA1DoIRQ, 0) |= 1;
    if (al & 0x10)
        BYTE(SA1DoIRQ, 0) |= 2;
    SA1Control = al;
    if ((oldctrl & 0x20) && !(al & 0x20)) { /* SA-1 leaving reset */
        SA1BankPtr = (uint32_t)(uintptr_t)romdata;
        SA1Ptr = romdata + (uint16_t)SA1ResetV - 0x8000;
        BYTE(SA1xpb, 0) = 0;
        *(uint16_t*)&SA1xs = 0x1FF;
        SA1RegPCS = romdata - 0x8000;
    }
}

REGABI_REG_WRITE8(sa12201w);
void c_sa12201w(uint8_t al) { SA1IRQEnable = al; }

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

/* ===== Stage 4: ROM/BWRAM bank mapping (0x2220-0x2225) + S-DD1 ===== */
extern uint8_t *SA1RAMArea, *SNSBWPtr, *CurBWPtr, *SA1BWPtr;
extern uint32_t BWShift, BWAndAddr, BWAnd, BWRAnd, Sdd1Mode;
extern uint8_t SA1BankVal[4];
extern uint32_t NumofBanks; /* initdata.c (byte 0 used) */
extern uint8_t *snesmmap[256], *snesmap2[256]; /* SNES memory map */
extern uint8_t SA1Status; /* 0 = 65816, 1 = SA-1 A, 2 = SA-1 B */
extern uint8_t BWUsed2, SDD1BankA[4], AddrNoIncr;
extern void* memtabler8[256];
void memaccessbankr8sdd1(void);

/* SA-1 ROM bank registers: remap snesmmap (LoROM area) + snesmap2/mmap (HiROM). */
static void bankswitch(int mlo, int mhi, uint32_t rom_off, uint8_t al)
{
    uint32_t e = (uint32_t)(((uint8_t)NumofBanks == 64 ? (al & 1u) : al) & 7u) << 20;
    uint8_t* base1 = romdata + ((al & 0x80) ? e : rom_off) - 0x8000;
    for (int i = 0; i < 0x20; i++)
        snesmmap[mlo + i] = base1 + i * 0x8000;
    uint8_t* base2 = romdata + e;
    for (int i = 0; i < 0x10; i++)
        snesmap2[mhi + i] = snesmmap[mhi + i] = base2 + i * 0x10000;
}

REGABI_REG_WRITE8(sa12220w);
void c_sa12220w(uint8_t al)
{
    SA1BankVal[0] = al;
    bankswitch(0x00, 0xC0, 0x000000, al);
}
REGABI_REG_WRITE8(sa12221w);
void c_sa12221w(uint8_t al)
{
    SA1BankVal[1] = al;
    bankswitch(0x20, 0xD0, 0x100000, al);
}
REGABI_REG_WRITE8(sa12222w);
void c_sa12222w(uint8_t al)
{
    SA1BankVal[2] = al;
    bankswitch(0x80, 0xE0, 0x200000, al);
}
REGABI_REG_WRITE8(sa12223w);
void c_sa12223w(uint8_t al)
{
    SA1BankVal[3] = al;
    bankswitch(0xA0, 0xF0, 0x300000, al);
}

REGABI_REG_WRITE8(sa12224w); /* SNES BW-RAM bank */
void c_sa12224w(uint8_t al)
{
    uint8_t* e = SA1RAMArea + ((uint32_t)(al & 0x1F) << 13) - 0x6000;
    SNSBWPtr = e;
    if (SA1Status == 0)
        CurBWPtr = e;
}

REGABI_REG_WRITE8(sa12225w); /* SA-1 BW-RAM bank + bitmap mode */
void c_sa12225w(uint8_t al)
{
    BWUsed2 = al;
    uint8_t* e;
    if (!(al & 0x80)) {
        e = SA1RAMArea + ((uint32_t)(al & 0x1F) << 13) - 0x6000;
        BYTE(BWShift, 0) = 0;
        BYTE(BWAndAddr, 0) = 0;
        BYTE(BWAnd, 0) = 0xFF;
        BYTE(BWRAnd, 0) = 0;
    } else if (BYTE(SA1Overflow, 1) & 0x80) { /* 4-colour */
        BYTE(BWShift, 0) = 2;
        BYTE(BWAndAddr, 0) = 0x03;
        BYTE(BWAnd, 0) = 0x03;
        BYTE(BWRAnd, 0) = 0xFC;
        e = SA1RAMArea + ((uint32_t)(al & 0x7F) << 11);
    } else { /* 16-colour */
        BYTE(BWShift, 0) = 1;
        BYTE(BWAndAddr, 0) = 0x01;
        BYTE(BWAnd, 0) = 0x0F;
        BYTE(BWRAnd, 0) = 0xF0;
        e = SA1RAMArea + ((uint32_t)(al & 0x3F) << 12);
    }
    SA1BWPtr = e;
    if (SA1Status != 0)
        CurBWPtr = e;
}

/* S-DD1 (separate chip; its registers live here historically) */
REGABI_REG_READ8(sdd14804);
uint8_t c_sdd14804(void) { return SDD1BankA[0]; }
REGABI_REG_READ8(sdd14805);
uint8_t c_sdd14805(void) { return SDD1BankA[1]; }
REGABI_REG_READ8(sdd14806);
uint8_t c_sdd14806(void) { return SDD1BankA[2]; }
REGABI_REG_READ8(sdd14807);
uint8_t c_sdd14807(void) { return SDD1BankA[3]; }

static void bankswitch_sdd1(int idx, int mhi, uint8_t al)
{
    SDD1BankA[idx] = al;
    uint8_t* base = romdata + ((uint32_t)(al & 7) << 20);
    for (int i = 0; i < 0x10; i++)
        snesmap2[mhi + i] = snesmmap[mhi + i] = base + i * 0x10000;
}
REGABI_REG_WRITE8(sdd14804w);
void c_sdd14804w(uint8_t al) { bankswitch_sdd1(0, 0xC0, al); }
REGABI_REG_WRITE8(sdd14805w);
void c_sdd14805w(uint8_t al) { bankswitch_sdd1(1, 0xD0, al); }
REGABI_REG_WRITE8(sdd14806w);
void c_sdd14806w(uint8_t al) { bankswitch_sdd1(2, 0xE0, al); }
REGABI_REG_WRITE8(sdd14807w);
void c_sdd14807w(uint8_t al) { bankswitch_sdd1(3, 0xF0, al); }

REGABI_REG_WRITE8(sdd14801w); /* enable S-DD1 decompression mapping */
void c_sdd14801w(uint8_t al)
{
    if (al == 0)
        return;
    AddrNoIncr = 0;
    for (int i = 0xC0; i < 0x100; i++)
        memtabler8[i] = (void*)memaccessbankr8sdd1;
    Sdd1Mode = 1;
}

/* ===== Stage 5: arithmetic (0x2250-0x2254) + DMA (0x2230-0x2239) ===== */
extern uint32_t SA1ARC;
extern uint16_t SA1AR1, SA1AR2, SA1DMACount;
extern uint32_t SA1DMAChar, SA1DMASource, SA1DMADest;
extern uint8_t SA1DMAInfo;
extern uint32_t SA1_CC2_line, SA1_in_cc1_dma;
void UpdateArithStuff(void); /* c_sa1regs.c */
void sa1dmairam(void);
void sa1dmabwram(void);
void sa1chconv(void); /* Stage 6 */

REGABI_REG_WRITE8(sa12250w); /* arithmetic control */
void c_sa12250w(uint8_t al)
{
    BYTE(SA1ARC, 0) = al;
    BYTE(SA1ARC, 1) = 1;
    if (al & 2) { /* cumulative-sum reset: clear the 48-bit result */
        BYTE(SA1ARR1, 0) = BYTE(SA1ARR1, 1) = BYTE(SA1ARR1, 2) = BYTE(SA1ARR1, 3) = 0;
        BYTE(SA1ARR2, 0) = BYTE(SA1ARR2, 1) = 0;
    }
}
REGABI_REG_WRITE8(sa12251w);
void c_sa12251w(uint8_t al)
{
    BYTE(SA1AR1, 0) = al;
    BYTE(SA1ARC, 1) = 1;
}
REGABI_REG_WRITE8(sa12252w);
void c_sa12252w(uint8_t al)
{
    BYTE(SA1AR1, 1) = al;
    BYTE(SA1ARC, 1) = 1;
}
REGABI_REG_WRITE8(sa12253w);
void c_sa12253w(uint8_t al)
{
    BYTE(SA1AR2, 0) = al;
    BYTE(SA1ARC, 1) = 1;
}
REGABI_REG_WRITE8(sa12254w); /* writing AR2 hi triggers the operation */
void c_sa12254w(uint8_t al)
{
    BYTE(SA1AR2, 1) = al;
    BYTE(SA1ARC, 1) = 1;
    UpdateArithStuff();
}

REGABI_REG_WRITE8(sa12230w);
void c_sa12230w(uint8_t al)
{
    if (!(al & 0x80))
        SA1_CC2_line = 0;
    BYTE(SA1DMAInfo, 0) = al;
}
REGABI_REG_WRITE8(sa12231w);
void c_sa12231w(uint8_t al)
{
    BYTE(SA1DMAChar, 0) = al;
    if (al & 0x80)
        SA1_in_cc1_dma = 0;
}
SA1_QUICKW(sa12232w, SA1DMASource, 0)
SA1_QUICKW(sa12233w, SA1DMASource, 1)
SA1_QUICKW(sa12234w, SA1DMASource, 2)
SA1_QUICKW(sa12235w, SA1DMADest, 0)
REGABI_REG_WRITE8(sa12236w); /* writing dest mid byte kicks IRAM DMA / char-conv */
void c_sa12236w(uint8_t al)
{
    BYTE(SA1DMADest, 1) = al;
    if (BYTE(SA1DMAInfo, 0) & 0x10) {
        sa1chconv();
        return;
    }
    if (!(BYTE(SA1DMAInfo, 0) & 4))
        sa1dmairam();
}
REGABI_REG_WRITE8(sa12237w); /* writing dest hi byte kicks BW-RAM DMA */
void c_sa12237w(uint8_t al)
{
    BYTE(SA1DMADest, 2) = al;
    if (!(BYTE(SA1DMAInfo, 0) & 0x10) && (BYTE(SA1DMAInfo, 0) & 4))
        sa1dmabwram();
}
SA1_QUICKW(sa12238w, SA1DMACount, 0)
SA1_QUICKW(sa12239w, SA1DMACount, 1)

/* ===== Stage 6: char-conversion DMA, variable-length decode, bitmap RF ===== */
extern uint8_t SA1_BRF[16];
extern uint32_t SA1DoIRQ, VarLenAddr, VarLenAddrB, VarLenBarrel;
void SA1_DMA_CC2(void); /* sa1emu.c */

/* The bit-plane conversion path is disabled upstream (its guard is commented
   out in sa1regs.asm), so this only raises the IRQ and flags CC#1 in progress. */
void sa1chconv(void)
{
    BYTE(SA1DoIRQ, 0) |= 8;
    SA1_in_cc1_dma = 1;
}

REGABI_REG_WRITE8(sa1223Fw);
void c_sa1223Fw(uint8_t al) { BYTE(SA1Overflow, 1) = al; }

REGABI_REG_WRITE8(sa12258w);
void c_sa12258w(uint8_t al)
{
    uint8_t bl = al & 0x0F;
    if (bl == 0)
        bl = 16;
    BYTE(VarLenBarrel, 2) = al;
    BYTE(VarLenBarrel, 3) = bl;
    if (al & 0x80) {
        BYTE(VarLenBarrel, 0) = bl;
        BYTE(VarLenBarrel, 1) = bl;
    }
}
REGABI_REG_WRITE8(sa12259w);
void c_sa12259w(uint8_t al)
{
    BYTE(VarLenAddr, 0) = al;
    BYTE(VarLenAddrB, 0) = al;
    BYTE(VarLenBarrel, 0) = 0;
    BYTE(VarLenBarrel, 1) = 0;
}
REGABI_REG_WRITE8(sa1225Aw);
void c_sa1225Aw(uint8_t al)
{
    BYTE(VarLenAddr, 1) = al;
    BYTE(VarLenAddrB, 1) = al;
    BYTE(VarLenBarrel, 0) = 0;
    BYTE(VarLenBarrel, 1) = 0;
}
REGABI_REG_WRITE8(sa1225Bw);
void c_sa1225Bw(uint8_t al)
{
    BYTE(VarLenAddr, 2) = al;
    BYTE(VarLenAddrB, 2) = al;
    BYTE(VarLenBarrel, 0) = 0;
    BYTE(VarLenBarrel, 1) = 0;
}

REGABI_REG_READ8(sa1230Cr);
uint8_t c_sa1230Cr(void)
{
    uint8_t bank = BYTE(VarLenAddr, 2);
    uint16_t addr16 = (uint16_t)VarLenAddr;
    uint8_t* base = (addr16 & 0x8000) ? snesmmap[bank] : snesmap2[bank];
    uint32_t val = *(uint32_t*)(base + addr16) >> BYTE(VarLenBarrel, 1);
    if (BYTE(VarLenBarrel, 2) & 0x80) {
        BYTE(VarLenBarrel, 0) += BYTE(VarLenBarrel, 3);
        if (BYTE(VarLenBarrel, 0) > 16) {
            BYTE(VarLenBarrel, 0) -= 16;
            VarLenAddr += 2;
            BYTE(VarLenAddr, 3) = 0;
        }
    }
    return (uint8_t)val;
}
REGABI_REG_READ8(sa1230Dr);
uint8_t c_sa1230Dr(void)
{
    uint8_t bank = BYTE(VarLenAddrB, 2);
    uint16_t addr16 = (uint16_t)VarLenAddrB;
    uint8_t* base = (addr16 & 0x8000) ? snesmmap[bank] : snesmap2[bank];
    uint32_t val = *(uint32_t*)(base + addr16) >> BYTE(VarLenBarrel, 1);
    if (BYTE(VarLenBarrel, 2) & 0x80) {
        BYTE(VarLenBarrel, 1) += BYTE(VarLenBarrel, 3);
        if (BYTE(VarLenBarrel, 1) > 16) {
            BYTE(VarLenBarrel, 1) -= 16;
            VarLenAddrB += 2;
            BYTE(VarLenAddrB, 3) = 0;
        }
    }
    return (uint8_t)(val >> 8);
}
REGABI_REG_READ8(sa1230Er);
uint8_t c_sa1230Er(void) { return 0x10; }

/* Bitmap register file: each address writes its own SA1_BRF byte (the asm
   shares one body and indexes by the register address in ecx). */
#define SA1_BRF_W(name, idx) \
    REGABI_REG_WRITE8(name); \
    void c_##name(uint8_t al) { SA1_BRF[idx] = al; }
SA1_BRF_W(sa12240w, 0)
SA1_BRF_W(sa12241w, 1)
SA1_BRF_W(sa12242w, 2)
SA1_BRF_W(sa12243w, 3)
SA1_BRF_W(sa12244w, 4)
SA1_BRF_W(sa12245w, 5)
SA1_BRF_W(sa12246w, 6)
SA1_BRF_W(sa12248w, 8)
SA1_BRF_W(sa12249w, 9)
SA1_BRF_W(sa1224Aw, 10)
SA1_BRF_W(sa1224Bw, 11)
SA1_BRF_W(sa1224Cw, 12)
SA1_BRF_W(sa1224Dw, 13)
SA1_BRF_W(sa1224Ew, 14)

/* 0x2247 / 0x224F additionally trigger CC#2 DMA */
static void brf_cc2(void)
{
    if ((BYTE(SA1DMAInfo, 0) & 0xA0) && !(BYTE(SA1DMAInfo, 0) & 0x10))
        SA1_DMA_CC2();
}
REGABI_REG_WRITE8(sa12247w);
void c_sa12247w(uint8_t al)
{
    SA1_BRF[7] = al;
    brf_cc2();
}
REGABI_REG_WRITE8(sa1224Fw);
void c_sa1224Fw(uint8_t al)
{
    SA1_BRF[15] = al;
    brf_cc2();
}

/* ===== Stage 7: RTC (0x2800/0x2801), SA-1 timer count, debug no-op ===== */
extern uint32_t GetTime(void), GetDate(void); /* ztimec.c */
#ifndef NO_DEBUGGER
extern uint8_t debuggeron;
#endif

REGABI_REG_READ8(RTC2800);
uint8_t c_RTC2800(void)
{
#ifndef NO_DEBUGGER
    if (RTCPtr == 0 && debuggeron != 1) { /* first read of the sequence: latch now */
#else
    if (RTCPtr == 0) {
#endif
        uint32_t e = GetTime();
        RTCData[1] = e & 0x0F;
        e >>= 4;
        RTCData[2] = e & 0x0F;
        e >>= 4;
        RTCData[3] = e & 0x0F;
        e >>= 4;
        RTCData[4] = e & 0x0F;
        e >>= 4;
        RTCData[5] = e & 0x0F;
        e >>= 4;
        RTCData[6] = e & 0x0F;
        e = GetDate();
        RTCData[7] = e & 0x0F;
        e >>= 4;
        RTCData[8] = e & 0x0F;
        e >>= 4;
        RTCData[9] = e & 0x0F;
        e >>= 8;
        RTCData[10] = e & 0x0F;
        e >>= 4;
        {
            uint8_t bl = e & 0x1F, bh = 0;
            while (bl > 9) {
                bh++;
                bl -= 10;
            }
            RTCData[11] = bl;
            RTCData[12] = bh + 9;
        }
        e >>= 8;
        RTCData[13] = e & 0x0F;
    }
    {
        uint8_t al = RTCData[RTCPtr];
        if (++RTCPtr == 0x0F)
            RTCPtr = 0;
        return al;
    }
}
REGABI_REG_WRITE8(RTC2801w);
void c_RTC2801w(uint8_t al)
{
    RTCRest = 0;
    RTCPtr = 0;
    if (al == 0x0E || al == 0x0D) {
        RTCPtr2 = 0;
        return;
    }
    if (RTCPtr2 <= 13) { /* >13 leaves the pointer untouched */
        if (RTCPtr2 != 0)
            RTCData[RTCPtr2] = al;
        RTCPtr2++;
    }
}

REGABI_REG_WRITE8(sa12210w); /* timer settings */
void c_sa12210w(uint8_t al) { BYTE(SA1TimerSet, 0) = al; }
REGABI_REG_WRITE8(sa12211w); /* timer clear */
void c_sa12211w(uint8_t al)
{
    (void)al;
    SA1TimerVal = 0;
}

SA1_QUICKW(sa12212w, SA1TimerCount, 0)
SA1_QUICKW(sa12213w, SA1TimerCount, 1)
SA1_QUICKW(sa12214w, SA1TimerCount, 2)
SA1_QUICKW(sa12215w, SA1TimerCount, 3)

REGABI_REG_WRITE8(dbstop);
void c_dbstop(uint8_t al) { (void)al; }
