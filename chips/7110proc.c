/* C port of 7110proc.asm (SPC7110 register interface). STAGE 1: data block.
   SPCMultA..SPCCompressionRegs must stay contiguous and in this exact order:
   zstate.c saves the run &SPCMultA .. PHnum2writespc7110reg bytes. The
   decompression engine lives in 7110emu.c. */
#include <stdint.h>

#include "../asmdata.h"

extern void SPC7110initC(void); /* 7110emu.c */

/* Save-state block (101 bytes). SPCROMtoI initialises to &SPCROMPtr, so the
   whole run is laid out via inline asm to keep it contiguous and self-exact
   under -fdata-sections. */
__asm__(
    ASM_SEC_DATA(".data.spc7110state")
        ASM_GSYM(SPCMultA) ".long 0\n" ASM_GSYM(SPCMultB) ".long 0\n" ASM_GSYM(SPCDivEnd) ".long 0\n" ASM_GSYM(SPCMulRes) ".long 0\n" ASM_GSYM(SPCDivRes) ".long 0\n" ASM_GSYM(SPC7110BankA) ".long 0x020100\n" ASM_GSYM(SPC7110RTCStat) ".long 0\n" ASM_GSYM(SPC7110RTC) ".byte 0,0,0,0,0,0,1,0,1,0,0,0,0,0,0x0F,0\n" ASM_GSYM(SPC7110RTCB) ".byte 0,0,0,0,0,0,1,0,1,0,0,0,0,1,0x0F,6\n" ASM_GSYM(SPCROMPtr) ".long 0\n" ASM_GSYM(SPCROMtoI) ".long SPCROMPtr\n" ASM_GSYM(SPCROMAdj) ".long 0\n" ASM_GSYM(SPCROMInc) ".long 0\n" ASM_GSYM(SPCROMCom) ".long 0\n" ASM_GSYM(SPCCheckFix) ".long 0\n" ASM_GSYM(SPCSignedVal) ".long 0\n" ASM_GSYM(SPCCompressionRegs) ".zero 13\n" ASM_GSYM(PHnum2writespc7110reg) ".long . - SPCMultA\n" ASM_SEC_END);

extern uint32_t SPCMultA, SPCMultB, SPCDivEnd, SPCMulRes, SPCDivRes;
extern uint32_t SPC7110BankA, SPC7110RTCStat;
extern uint32_t SPCROMPtr, SPCROMtoI, SPCROMAdj, SPCROMInc, SPCROMCom, SPCCheckFix;

void SPC7110init(void)
{
    SPC7110initC();
    SPCMultA = 0;
    SPCMultB = 0;
    SPCDivEnd = 0;
    SPCMulRes = 0;
    SPCDivRes = 0;
    SPC7110BankA = 0x020100;
    SPC7110RTCStat = 0;
    SPCROMPtr = 0;
    SPCROMtoI = (uint32_t)(uintptr_t)&SPCROMPtr;
    SPCROMAdj = 0;
    SPCROMInc = 0;
    SPCROMCom = 0;
    SPCCheckFix = 0;
}

/* ===== Stage 2: compression status registers (0x4800-0x480C) ===== */
#include "regabi.h"

extern uint8_t SPCCompressionRegs[13];
extern void SPC7110_4800(void); /* 7110emu.c */
extern void SPC7110_4806w(void); /* 7110emu.c */

#define SPC_REG_R(name, idx) \
    REGABI_REG_READ8(name);  \
    uint8_t c_##name(void) { return SPCCompressionRegs[idx]; }
#define SPC_REG_W(name, idx) \
    REGABI_REG_WRITE8(name); \
    void c_##name(uint8_t al) { SPCCompressionRegs[idx] = al; }

REGABI_REG_READ8(SPC4800);
uint8_t c_SPC4800(void)
{
    SPC7110_4800();
    return SPCCompressionRegs[0];
}
SPC_REG_R(SPC4801, 1)
SPC_REG_R(SPC4802, 2)
SPC_REG_R(SPC4803, 3)
SPC_REG_R(SPC4804, 4)
SPC_REG_R(SPC4805, 5)
SPC_REG_R(SPC4806, 6)
SPC_REG_R(SPC4807, 7)
SPC_REG_R(SPC4808, 8)
SPC_REG_R(SPC4809, 9)
SPC_REG_R(SPC480A, 10)
SPC_REG_R(SPC480B, 11)
REGABI_REG_READ8(SPC480C);
uint8_t c_SPC480C(void)
{
    uint8_t al = SPCCompressionRegs[12];
    SPCCompressionRegs[12] = 0;
    return al;
}

SPC_REG_W(SPC4801w, 1)
SPC_REG_W(SPC4802w, 2)
SPC_REG_W(SPC4803w, 3)
SPC_REG_W(SPC4804w, 4)
SPC_REG_W(SPC4805w, 5)
REGABI_REG_WRITE8(SPC4806w);
void c_SPC4806w(uint8_t al)
{
    SPCCompressionRegs[6] = al;
    SPC7110_4806w();
}
SPC_REG_W(SPC4807w, 7)
SPC_REG_W(SPC4808w, 8)
SPC_REG_W(SPC4809w, 9)
SPC_REG_W(SPC480Aw, 10)
SPC_REG_W(SPC480Bw, 11)

/* ===== Stage 3: math unit (0x4820-0x482F) =====
   The math registers are byte views into the wide values:
   0x4820-0x4823 -> SPCMultA (multiplicand / 32-bit dividend)
   0x4824-0x4825 -> SPCMultB (multiplier)
   0x4826-0x4827 -> SPCDivEnd (divisor)
   0x4828-0x482B -> SPCMulRes (product / quotient)
   0x482C-0x482D -> SPCDivRes (remainder)
   A multiply triggers on a write to 0x4825, a divide on a write to 0x4827. */
extern uint32_t SPCMulRes, SPCSignedVal;

/* Byte view of a 32-bit register (little-endian, as the asm assumed). */
#define SPC_MATH_R(name, base, idx) \
    REGABI_REG_READ8(name);         \
    uint8_t c_##name(void) { return ((uint8_t*)&base)[idx]; }
#define SPC_MATH_W(name, base, idx) \
    REGABI_REG_WRITE8(name);        \
    void c_##name(uint8_t al) { ((uint8_t*)&base)[idx] = al; }

static void spc7110_multiply(void)
{
    uint16_t a = (uint16_t)SPCMultA;
    uint16_t b = (uint16_t)SPCMultB;
    if (SPCSignedVal & 1)
        SPCMulRes = (uint32_t)((int32_t)(int16_t)a * (int32_t)(int16_t)b);
    else
        SPCMulRes = (uint32_t)a * (uint32_t)b;
}

static void spc7110_set_remainder(uint16_t rem)
{
    /* The asm stored only the low word, leaving SPCDivRes's upper bytes. */
    ((uint8_t*)&SPCDivRes)[0] = (uint8_t)rem;
    ((uint8_t*)&SPCDivRes)[1] = (uint8_t)(rem >> 8);
}

static void spc7110_divide(void)
{
    uint16_t divisor = (uint16_t)SPCDivEnd;
    if (divisor == 0) {
        /* Hardware yields quotient 0 and remainder = low word of the dividend
           (per bsnes and snes9x). The asm set 0xFFFFFFFF/0xFFFF, a bug. */
        SPCMulRes = 0;
        spc7110_set_remainder((uint16_t)SPCMultA);
        return;
    }
    if (SPCSignedVal & 1) {
        int32_t dividend = (int32_t)SPCMultA;
        int16_t d = (int16_t)divisor;
        SPCMulRes = (uint32_t)(dividend / d);
        spc7110_set_remainder((uint16_t)(dividend % d));
    } else {
        uint32_t dividend = SPCMultA;
        SPCMulRes = dividend / divisor;
        spc7110_set_remainder((uint16_t)(dividend % divisor));
    }
}

SPC_MATH_R(SPC4820, SPCMultA, 0)
SPC_MATH_R(SPC4821, SPCMultA, 1)
SPC_MATH_R(SPC4822, SPCMultA, 2)
SPC_MATH_R(SPC4823, SPCMultA, 3)
SPC_MATH_R(SPC4824, SPCMultB, 0)
SPC_MATH_R(SPC4825, SPCMultB, 1)
SPC_MATH_R(SPC4826, SPCDivEnd, 0)
SPC_MATH_R(SPC4827, SPCDivEnd, 1)
SPC_MATH_R(SPC4828, SPCMulRes, 0)
SPC_MATH_R(SPC4829, SPCMulRes, 1)
SPC_MATH_R(SPC482A, SPCMulRes, 2)
SPC_MATH_R(SPC482B, SPCMulRes, 3)
SPC_MATH_R(SPC482C, SPCDivRes, 0)
SPC_MATH_R(SPC482D, SPCDivRes, 1)

REGABI_REG_READ8(SPC482E);
uint8_t c_SPC482E(void) { return 0; }
REGABI_REG_READ8(SPC482F);
uint8_t c_SPC482F(void) { return 0; }

SPC_MATH_W(SPC4820w, SPCMultA, 0)
SPC_MATH_W(SPC4821w, SPCMultA, 1)
SPC_MATH_W(SPC4822w, SPCMultA, 2)
SPC_MATH_W(SPC4823w, SPCMultA, 3)
SPC_MATH_W(SPC4824w, SPCMultB, 0)
REGABI_REG_WRITE8(SPC4825w);
void c_SPC4825w(uint8_t al)
{
    ((uint8_t*)&SPCMultB)[1] = al;
    spc7110_multiply();
}
SPC_MATH_W(SPC4826w, SPCDivEnd, 0)
REGABI_REG_WRITE8(SPC4827w);
void c_SPC4827w(uint8_t al)
{
    ((uint8_t*)&SPCDivEnd)[1] = al;
    spc7110_divide();
}
REGABI_REG_WRITE8(SPC482Ew);
void c_SPC482Ew(uint8_t al)
{
    SPCSignedVal = al;
    SPCMultA = 0;
    SPCMultB = 0;
    SPCDivEnd = 0;
    SPCMulRes = 0;
    SPCDivRes = 0;
}

/* ===== Stage 4: RTC (0x4840-0x4842, 0x4850-0x485F) =====
   SPC7110RTCStat is a byte view: [0] enable, [1] index/mode, [2] command byte.
   Mode 0xFE expects the command byte, 0xFF expects the register index, and
   0x00-0x0F is the live register index (auto-incrementing after each access).
   SPC7110RTC[0..15] holds the BCD time/date; an index-0 read latches the host
   clock via GetTime/GetDate, matching the SA-1 RTC. */
extern uint8_t SPC7110RTC[16];
extern uint32_t GetTime(void), GetDate(void); /* ztimec.c */
#ifndef NO_DEBUGGER
extern uint8_t debuggeron;
#endif

#define RTC_STAT(i) (((uint8_t *)&SPC7110RTCStat)[i])

static void spc7110_latch_clock(void)
{
    uint32_t e = GetTime(); /* sec | min<<8 | hour<<16, each BCD */
    SPC7110RTC[0] = e & 0x0F;
    e >>= 4;
    SPC7110RTC[1] = e & 0x0F;
    e >>= 4;
    SPC7110RTC[2] = e & 0x0F;
    e >>= 4;
    SPC7110RTC[3] = e & 0x0F;
    e >>= 4;
    SPC7110RTC[4] = e & 0x0F; /* always the 24-hour path; asm's 12-hour path is dead */
    e >>= 4;
    SPC7110RTC[5] = e & 0x0F;

    e = GetDate(); /* mday | (mon+1)<<8 | year<<16 | wday<<28 */
    SPC7110RTC[6] = e & 0x0F;
    e >>= 4;
    SPC7110RTC[7] = e & 0x0F;
    e >>= 4;
    {
        uint8_t bl = e & 0x0F, bh = 0; /* month is binary 1-12 here */
        if (bl > 9) {
            bl -= 10;
            bh = 1;
        }
        SPC7110RTC[8] = bl;
        SPC7110RTC[9] = bh;
    }
    e >>= 8;
    SPC7110RTC[10] = e & 0x0F;
    e >>= 4;
    {
        uint8_t bl = e & 0x1F; /* year 10's digit, reduced mod 10 */
        while (bl > 9)
            bl -= 10;
        SPC7110RTC[11] = bl;
    }
    e >>= 8;
    SPC7110RTC[12] = e & 0x0F; /* day of week */
}

REGABI_REG_WRITE8(SPC4840w);
void c_SPC4840w(uint8_t al)
{
    if (al & 1) {
        RTC_STAT(0) = al;
        RTC_STAT(1) = 0xFE;
    }
}
REGABI_REG_READ8(SPC4840);
uint8_t c_SPC4840(void) { return RTC_STAT(0); }

REGABI_REG_WRITE8(SPC4841w);
void c_SPC4841w(uint8_t al)
{
    uint8_t mode = RTC_STAT(1);
    if (mode == 0xFE) { /* first write after enable: command byte */
        RTC_STAT(1) = 0xFF;
        RTC_STAT(2) = al;
        return;
    }
    if (mode == 0xFF) { /* second write: register index */
        RTC_STAT(1) = al & 0x0F;
        return;
    }
    SPC7110RTC[mode] = al;
    if (mode == 0x0F && (al & 1)) { /* control reg reset: day and month back to 1 */
        SPC7110RTC[0] = SPC7110RTC[1] = SPC7110RTC[2] = SPC7110RTC[3] = 0;
        SPC7110RTC[4] = SPC7110RTC[5] = 0;
        SPC7110RTC[6] = 1;
        SPC7110RTC[7] = 0;
        SPC7110RTC[8] = 1;
        SPC7110RTC[9] = SPC7110RTC[10] = SPC7110RTC[11] = 0;
        SPC7110RTC[12] = 0;
    }
    RTC_STAT(1) = (mode + 1) & 0x0F;
}
REGABI_REG_READ8(SPC4841);
uint8_t c_SPC4841(void)
{
    uint8_t mode = RTC_STAT(1);
    uint8_t al;
    if (mode == 0xFE || mode == 0xFF) { /* return the stored command byte */
        RTC_STAT(1) = mode + 1;
        return RTC_STAT(2);
    }
    if (mode == 0 && !(SPC7110RTC[0x0F] & 0x03) && !(SPC7110RTC[0x0D] & 0x01)
#ifndef NO_DEBUGGER
        && debuggeron != 1
#endif
    ) {
        spc7110_latch_clock();
    }
    al = SPC7110RTC[mode];
    RTC_STAT(1) = (mode + 1) & 0x0F;
    return al;
}

REGABI_REG_WRITE8(SPC4842w);
void c_SPC4842w(uint8_t al) { (void)al; }
REGABI_REG_READ8(SPC4842);
uint8_t c_SPC4842(void) { return 0x80; } /* always ready */

#define SPC_RTC_R(name, idx) \
    REGABI_REG_READ8(name);  \
    uint8_t c_##name(void) { return SPC7110RTC[idx]; }
SPC_RTC_R(SPC4850, 0x00)
SPC_RTC_R(SPC4851, 0x01)
SPC_RTC_R(SPC4852, 0x02)
SPC_RTC_R(SPC4853, 0x03)
SPC_RTC_R(SPC4854, 0x04)
SPC_RTC_R(SPC4855, 0x05)
SPC_RTC_R(SPC4856, 0x06)
SPC_RTC_R(SPC4857, 0x07)
SPC_RTC_R(SPC4858, 0x08)
SPC_RTC_R(SPC4859, 0x09)
SPC_RTC_R(SPC485A, 0x0A)
SPC_RTC_R(SPC485B, 0x0B)
SPC_RTC_R(SPC485C, 0x0C)
SPC_RTC_R(SPC485D, 0x0D)
SPC_RTC_R(SPC485E, 0x0E)
SPC_RTC_R(SPC485F, 0x0F)
