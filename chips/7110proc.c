/* C port of 7110proc.asm (SPC7110 register interface). STAGE 1: data block.
   SPCMultA..SPCCompressionRegs must stay contiguous and in this exact order:
   zstate.c saves the run &SPCMultA .. PHnum2writespc7110reg bytes. The
   decompression engine lives in 7110emu.c. */
#include <stdint.h>

#include "../asmdata.h"

extern void SPC7110initC(void); /* 7110emu.c */
void SPC7110RTCReset(void); /* Stage 4: seed the Epson RTC from the host clock */

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
    SPC7110RTCReset();
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
uint8_t c_SPC482E(void) { return (uint8_t)SPCSignedVal; } /* mode register read-back */
REGABI_REG_READ8(SPC482F);
uint8_t c_SPC482F(void) { return 0; } /* ALU never busy in the instant model */

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

/* ===== Stage 4: Epson RTC-4513 (0x4840-0x4842, 0x4850-0x485F) =====
   Independent implementation of the Epson RTC-4513 hardware: a real ticking
   clock with bit-accurate BCD and calendar rollover, a 4-bit register file, and
   the chip-select, mode, seek, read and write serial protocol.  The clock is
   seeded from the host time (GetTime/GetDate) and advances by the real seconds
   elapsed between accesses.  Behaviour cross-checked against the bsnes and
   snes9x emulators (used only as references).  SPC7110RTC[0..15] mirrors the 16
   registers for save-states and the direct 0x4850-0x485F reads. */
extern uint8_t SPC7110RTC[16];
extern uint32_t GetTime(void), GetDate(void); /* ztimec.c */

/* RTC register fields (bsnes EpsonRTC layout) */
static uint8_t rtc_secondlo, rtc_secondhi, rtc_batteryfailure;
static uint8_t rtc_minutelo, rtc_minutehi, rtc_resync;
static uint8_t rtc_hourlo, rtc_hourhi, rtc_meridian;
static uint8_t rtc_daylo, rtc_dayhi, rtc_dayram;
static uint8_t rtc_monthlo, rtc_monthhi, rtc_monthram;
static uint8_t rtc_yearlo, rtc_yearhi;
static uint8_t rtc_weekday;
static uint8_t rtc_hold, rtc_calendar, rtc_irqflag, rtc_roundseconds;
static uint8_t rtc_irqmask, rtc_irqduty, rtc_irqperiod;
static uint8_t rtc_pause, rtc_stop, rtc_atime, rtc_test;
static uint8_t rtc_holdtick;

/* serial protocol state */
enum { RTC_MODE,
    RTC_SEEK,
    RTC_READ,
    RTC_WRITE };
static uint8_t rtc_chipselect, rtc_state, rtc_offset, rtc_mdr, rtc_ready;

static int64_t rtc_last; /* host seconds at the previous sync */

/* --- bit-accurate tick functions (Epson RTC-4513 invalid-BCD behaviour) --- */
static void rtc_tickMinute(void);
static void rtc_tickHour(void);
static void rtc_tickDay(void);
static void rtc_tickMonth(void);
static void rtc_tickYear(void);

static void rtc_tickSecond(void)
{
    if (rtc_secondlo <= 8 || rtc_secondlo == 12) {
        rtc_secondlo++;
    } else {
        rtc_secondlo = 0;
        if (rtc_secondhi < 5) {
            rtc_secondhi++;
        } else {
            rtc_secondhi = 0;
            rtc_tickMinute();
        }
    }
}

static void rtc_tickMinute(void)
{
    if (rtc_minutelo <= 8 || rtc_minutelo == 12) {
        rtc_minutelo++;
    } else {
        rtc_minutelo = 0;
        if (rtc_minutehi < 5) {
            rtc_minutehi++;
        } else {
            rtc_minutehi = 0;
            rtc_tickHour();
        }
    }
}

static void rtc_tickHour(void)
{
    if (rtc_atime) {
        if (rtc_hourhi < 2) {
            if (rtc_hourlo <= 8 || rtc_hourlo == 12) {
                rtc_hourlo++;
            } else {
                rtc_hourlo = !(rtc_hourlo & 1);
                rtc_hourhi++;
            }
        } else {
            if (rtc_hourlo != 3 && !(rtc_hourlo & 4)) {
                if (rtc_hourlo <= 8 || rtc_hourlo >= 12) {
                    rtc_hourlo++;
                } else {
                    rtc_hourlo = !(rtc_hourlo & 1);
                    rtc_hourhi++;
                }
            } else {
                rtc_hourlo = !(rtc_hourlo & 1);
                rtc_hourhi = 0;
                rtc_tickDay();
            }
        }
    } else {
        if (rtc_hourhi == 0) {
            if (rtc_hourlo <= 8 || rtc_hourlo == 12) {
                rtc_hourlo++;
            } else {
                rtc_hourlo = !(rtc_hourlo & 1);
                rtc_hourhi ^= 1;
            }
        } else {
            if (rtc_hourlo & 1)
                rtc_meridian ^= 1;
            if (rtc_hourlo < 2 || rtc_hourlo == 4 || rtc_hourlo == 5 || rtc_hourlo == 8 || rtc_hourlo == 12) {
                rtc_hourlo++;
            } else {
                rtc_hourlo = !(rtc_hourlo & 1);
                rtc_hourhi ^= 1;
            }
            if (rtc_meridian == 0 && !(rtc_hourlo & 1))
                rtc_tickDay();
        }
    }
}

static void rtc_tickDay(void)
{
    /* January - December = 0x01 - 0x09; 0x10 - 0x12 */
    static const uint8_t daysinmonth[32] = {
        30,
        31,
        28,
        31,
        30,
        31,
        30,
        31,
        31,
        30,
        31,
        30,
        31,
        30,
        31,
        30,
        31,
        30,
        31,
        30,
        31,
        30,
        31,
        30,
        31,
        30,
        31,
        30,
        31,
        30,
        31,
        30,
    };
    uint8_t days;
    if (rtc_calendar == 0)
        return;
    rtc_weekday = ((rtc_weekday + 1) + (rtc_weekday == 6)) & 7;
    days = daysinmonth[(rtc_monthhi << 4) | rtc_monthlo];
    if (days == 28) { /* add one day for leap years */
        if ((rtc_yearhi & 1) == 0 && ((rtc_yearlo - 0) & 3) == 0)
            days++;
        if ((rtc_yearhi & 1) == 1 && ((rtc_yearlo - 2) & 3) == 0)
            days++;
    }
    if (days == 28 && (rtc_dayhi == 3 || (rtc_dayhi == 2 && rtc_daylo >= 8))) {
        rtc_daylo = 1;
        rtc_dayhi = 0;
        rtc_tickMonth();
        return;
    }
    if (days == 29 && (rtc_dayhi == 3 || (rtc_dayhi == 2 && (rtc_daylo > 8 && rtc_daylo != 12)))) {
        rtc_daylo = 1;
        rtc_dayhi = 0;
        rtc_tickMonth();
        return;
    }
    if (days == 30 && (rtc_dayhi == 3 || (rtc_dayhi == 2 && (rtc_daylo == 10 || rtc_daylo == 14)))) {
        rtc_daylo = 1;
        rtc_dayhi = 0;
        rtc_tickMonth();
        return;
    }
    if (days == 31 && (rtc_dayhi == 3 && (rtc_daylo & 3))) {
        rtc_daylo = 1;
        rtc_dayhi = 0;
        rtc_tickMonth();
        return;
    }
    if (rtc_daylo <= 8 || rtc_daylo == 12) {
        rtc_daylo++;
    } else {
        rtc_daylo = !(rtc_daylo & 1);
        rtc_dayhi++;
    }
}

static void rtc_tickMonth(void)
{
    if (rtc_monthhi == 0 || !(rtc_monthlo & 2)) {
        if (rtc_monthlo <= 8 || rtc_monthlo == 12) {
            rtc_monthlo++;
        } else {
            rtc_monthlo = !(rtc_monthlo & 1);
            rtc_monthhi ^= 1;
        }
    } else {
        rtc_monthlo = !(rtc_monthlo & 1);
        rtc_monthhi = 0;
        rtc_tickYear();
    }
}

static void rtc_tickYear(void)
{
    if (rtc_yearlo <= 8 || rtc_yearlo == 12) {
        rtc_yearlo++;
    } else {
        rtc_yearlo = !(rtc_yearlo & 1);
        if (rtc_yearhi <= 8 || rtc_yearhi == 12) {
            rtc_yearhi++;
        } else {
            rtc_yearhi = !(rtc_yearhi & 1);
        }
    }
}

static void rtc_roundSeconds(void)
{
    if (rtc_roundseconds == 0)
        return;
    rtc_roundseconds = 0;
    if (rtc_secondhi >= 3)
        rtc_tickMinute();
    rtc_secondlo = 0;
    rtc_secondhi = 0;
}

/* --- register file access (serial protocol) --- */
static void rtc_reset(void)
{
    rtc_state = RTC_MODE;
    rtc_offset = 0;
    rtc_resync = 0;
    rtc_pause = 0;
    rtc_test = 0;
}

static uint8_t rtc_read_reg(uint8_t addr)
{
    switch (addr & 0x0F) {
    default:
    case 0:
        return rtc_secondlo;
    case 1:
        return rtc_secondhi | rtc_batteryfailure << 3;
    case 2:
        return rtc_minutelo;
    case 3:
        return rtc_minutehi | rtc_resync << 3;
    case 4:
        return rtc_hourlo;
    case 5:
        return rtc_hourhi | rtc_meridian << 2 | rtc_resync << 3;
    case 6:
        return rtc_daylo;
    case 7:
        return rtc_dayhi | rtc_dayram << 2 | rtc_resync << 3;
    case 8:
        return rtc_monthlo;
    case 9:
        return rtc_monthhi | rtc_monthram << 1 | rtc_resync << 3;
    case 10:
        return rtc_yearlo;
    case 11:
        return rtc_yearhi;
    case 12:
        return rtc_weekday | rtc_resync << 3;
    case 13: {
        /* The periodic IRQ flag is a sub-second pulse (1/64s, cleared by a
           1/128s duty) on real hardware.  Our lazy clock has only whole-second
           resolution, so the flag reads inactive, never generated here. */
        uint8_t readflag = rtc_irqflag & !rtc_irqmask;
        rtc_irqflag = 0;
        return rtc_hold | rtc_calendar << 1 | readflag << 2 | rtc_roundseconds << 3;
    }
    case 14:
        return rtc_irqmask | rtc_irqduty << 1 | rtc_irqperiod << 2;
    case 15:
        return rtc_pause | rtc_stop << 1 | rtc_atime << 2 | rtc_test << 3;
    }
}

static void rtc_write_reg(uint8_t addr, uint8_t data)
{
    data &= 0x0F;
    switch (addr & 0x0F) {
    case 0:
        rtc_secondlo = data;
        break;
    case 1:
        rtc_secondhi = data & 7;
        rtc_batteryfailure = data >> 3;
        break;
    case 2:
        rtc_minutelo = data;
        break;
    case 3:
        rtc_minutehi = data & 7;
        break;
    case 4:
        rtc_hourlo = data;
        break;
    case 5:
        rtc_hourhi = data & 3;
        rtc_meridian = (data >> 2) & 1;
        if (rtc_atime == 1)
            rtc_meridian = 0;
        if (rtc_atime == 0)
            rtc_hourhi &= 1;
        break;
    case 6:
        rtc_daylo = data;
        break;
    case 7:
        rtc_dayhi = data & 3;
        rtc_dayram = (data >> 2) & 1;
        break;
    case 8:
        rtc_monthlo = data;
        break;
    case 9:
        rtc_monthhi = data & 1;
        rtc_monthram = (data >> 1) & 3;
        break;
    case 10:
        rtc_yearlo = data;
        break;
    case 11:
        rtc_yearhi = data;
        break;
    case 12:
        rtc_weekday = data & 7;
        break;
    case 13: {
        uint8_t held = rtc_hold;
        rtc_hold = data & 1;
        rtc_calendar = (data >> 1) & 1;
        rtc_roundseconds = (data >> 3) & 1;
        if (held == 1 && rtc_hold == 0 && rtc_holdtick == 1) {
            /* a second passed during hold; apply it on resume */
            rtc_holdtick = 0;
            rtc_tickSecond();
        }
    } break;
    case 14:
        rtc_irqmask = data & 1;
        rtc_irqduty = (data >> 1) & 1;
        rtc_irqperiod = (data >> 2) & 3;
        break;
    case 15:
        rtc_pause = data & 1;
        rtc_stop = (data >> 1) & 1;
        rtc_atime = (data >> 2) & 1;
        rtc_test = (data >> 3) & 1;
        if (rtc_atime == 1)
            rtc_meridian = 0;
        if (rtc_atime == 0)
            rtc_hourhi &= 1;
        if (rtc_pause) {
            rtc_secondlo = 0;
            rtc_secondhi = 0;
        }
        break;
    }
}

/* --- host clock seeding and elapsed-time catch-up --- */
static int64_t rtc_bcd(uint8_t b) { return (b >> 4) * 10 + (b & 0x0F); }

static int64_t rtc_host_seconds(void)
{
    uint32_t t = GetTime(), d = GetDate();
    int64_t s = rtc_bcd(t & 0xFF), mi = rtc_bcd((t >> 8) & 0xFF), h = rtc_bcd((t >> 16) & 0xFF);
    int64_t day = rtc_bcd(d & 0xFF);
    int64_t mon = (d >> 8) & 0xFF; /* binary 1-12 */
    int64_t year = 2000 + rtc_bcd((d >> 16) & 0xFF);
    int64_t y = year - (mon <= 2); /* days_from_civil (H. Hinnant) */
    int64_t era = (y >= 0 ? y : y - 399) / 400;
    int64_t yoe = y - era * 400;
    int64_t mp = (mon > 2) ? (mon - 3) : (mon + 9);
    int64_t doy = (153 * mp + 2) / 5 + day - 1;
    int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    int64_t days = era * 146097 + doe - 719468;
    return days * 86400 + h * 3600 + mi * 60 + s;
}

static void rtc_synchronize(void)
{
    uint32_t t = GetTime(), d = GetDate();
    uint32_t sec = (uint32_t)rtc_bcd(t & 0xFF);
    uint32_t mn = (uint32_t)rtc_bcd((t >> 8) & 0xFF);
    uint32_t hr = (uint32_t)rtc_bcd((t >> 16) & 0xFF);
    uint32_t day = (uint32_t)rtc_bcd(d & 0xFF);
    uint32_t mon = (d >> 8) & 0xFF; /* binary 1-12 */
    uint32_t yr = (uint32_t)rtc_bcd((d >> 16) & 0xFF) % 100;
    if (sec > 59)
        sec = 59;
    rtc_secondlo = sec % 10;
    rtc_secondhi = sec / 10;
    rtc_minutelo = mn % 10;
    rtc_minutehi = mn / 10;
    if (rtc_atime) {
        rtc_hourlo = hr % 10;
        rtc_hourhi = hr / 10;
    } else {
        rtc_meridian = hr >= 12;
        hr %= 12;
        if (hr == 0)
            hr = 12;
        rtc_hourlo = hr % 10;
        rtc_hourhi = hr / 10;
    }
    rtc_daylo = day % 10;
    rtc_dayhi = day / 10;
    rtc_monthlo = mon % 10;
    rtc_monthhi = mon / 10;
    rtc_yearlo = yr % 10;
    rtc_yearhi = yr / 10;
    rtc_weekday = (d >> 28) & 7;
    rtc_resync = 1; /* alert the program that the time changed */
}

static void rtc_sync(void)
{
    int64_t now = rtc_host_seconds();
    int64_t diff = now - rtc_last;
    rtc_last = now;
    if (rtc_roundseconds)
        rtc_roundSeconds();
    if (diff <= 0)
        return;
    if (rtc_stop || rtc_pause)
        return; /* a halted clock loses the elapsed time */
    if (rtc_hold) {
        rtc_holdtick = 1;
        return;
    }
    if (diff > (int64_t)86400 * 4000)
        diff = (int64_t)86400 * 4000; /* sanity clamp */
    rtc_resync = 1;
    while (diff >= 86400) {
        rtc_tickDay();
        diff -= 86400;
    }
    while (diff >= 3600) {
        rtc_tickHour();
        diff -= 3600;
    }
    while (diff >= 60) {
        rtc_tickMinute();
        diff -= 60;
    }
    while (diff-- > 0)
        rtc_tickSecond();
}

/* --- mirror the live fields into the save-state register array --- */
static void rtc_pack(void)
{
    uint8_t* R = SPC7110RTC;
    R[0] = rtc_secondlo;
    R[1] = rtc_secondhi | rtc_batteryfailure << 3;
    R[2] = rtc_minutelo;
    R[3] = rtc_minutehi | rtc_resync << 3;
    R[4] = rtc_hourlo;
    R[5] = rtc_hourhi | rtc_meridian << 2 | rtc_resync << 3;
    R[6] = rtc_daylo;
    R[7] = rtc_dayhi | rtc_dayram << 2 | rtc_resync << 3;
    R[8] = rtc_monthlo;
    R[9] = rtc_monthhi | rtc_monthram << 1 | rtc_resync << 3;
    R[10] = rtc_yearlo;
    R[11] = rtc_yearhi;
    R[12] = rtc_weekday | rtc_resync << 3;
    R[13] = rtc_hold | rtc_calendar << 1 | rtc_irqflag << 2 | rtc_roundseconds << 3;
    R[14] = rtc_irqmask | rtc_irqduty << 1 | rtc_irqperiod << 2;
    R[15] = rtc_pause | rtc_stop << 1 | rtc_atime << 2 | rtc_test << 3;
}

static void rtc_unpack(void)
{
    const uint8_t* R = SPC7110RTC;
    rtc_secondlo = R[0] & 0x0F;
    rtc_secondhi = R[1] & 7;
    rtc_batteryfailure = R[1] >> 3;
    rtc_minutelo = R[2] & 0x0F;
    rtc_minutehi = R[3] & 7;
    rtc_resync = (R[3] >> 3) & 1;
    rtc_hourlo = R[4] & 0x0F;
    rtc_hourhi = R[5] & 3;
    rtc_meridian = (R[5] >> 2) & 1;
    rtc_daylo = R[6] & 0x0F;
    rtc_dayhi = R[7] & 3;
    rtc_dayram = (R[7] >> 2) & 1;
    rtc_monthlo = R[8] & 0x0F;
    rtc_monthhi = R[9] & 1;
    rtc_monthram = (R[9] >> 1) & 3;
    rtc_yearlo = R[10] & 0x0F;
    rtc_yearhi = R[11] & 0x0F;
    rtc_weekday = R[12] & 7;
    rtc_hold = R[13] & 1;
    rtc_calendar = (R[13] >> 1) & 1;
    rtc_irqflag = (R[13] >> 2) & 1;
    rtc_roundseconds = (R[13] >> 3) & 1;
    rtc_irqmask = R[14] & 1;
    rtc_irqduty = (R[14] >> 1) & 1;
    rtc_irqperiod = (R[14] >> 2) & 3;
    rtc_pause = R[15] & 1;
    rtc_stop = (R[15] >> 1) & 1;
    rtc_atime = (R[15] >> 2) & 1;
    rtc_test = (R[15] >> 3) & 1;
}

void SPC7110RTCReset(void)
{
    /* power-on: all fields zero, battery flagged failed */
    rtc_secondlo = rtc_secondhi = rtc_batteryfailure = 0;
    rtc_minutelo = rtc_minutehi = rtc_resync = 0;
    rtc_hourlo = rtc_hourhi = rtc_meridian = 0;
    rtc_daylo = rtc_dayhi = rtc_dayram = 0;
    rtc_monthlo = rtc_monthhi = rtc_monthram = 0;
    rtc_yearlo = rtc_yearhi = 0;
    rtc_weekday = 0;
    rtc_hold = rtc_calendar = rtc_irqflag = rtc_roundseconds = 0;
    rtc_irqmask = rtc_irqduty = rtc_irqperiod = 0;
    rtc_pause = rtc_stop = rtc_atime = rtc_test = 0;
    rtc_batteryfailure = 1;
    /* idle the serial protocol */
    rtc_chipselect = 0;
    rtc_state = RTC_MODE;
    rtc_offset = 0;
    rtc_ready = 0;
    rtc_holdtick = 0;
    rtc_mdr = 0;
    /* ZSNES: seed from the host clock so the RTC shows real time, with the
       battery healthy and the calendar running. */
    rtc_calendar = 1;
    rtc_synchronize();
    rtc_batteryfailure = 0;
    rtc_last = rtc_host_seconds();
    rtc_pack();
}

void SPC7110RTCLoad(void)
{
    rtc_unpack();
    rtc_last = rtc_host_seconds();
} /* after a save-state restore */

REGABI_REG_WRITE8(SPC4840w);
void c_SPC4840w(uint8_t al) /* addr 0: chip select */
{
    rtc_chipselect = al & 3;
    if (rtc_chipselect != 1)
        rtc_reset();
    rtc_ready = 1;
    rtc_pack();
}
REGABI_REG_READ8(SPC4840);
uint8_t c_SPC4840(void) { return rtc_chipselect; }

REGABI_REG_WRITE8(SPC4841w);
void c_SPC4841w(uint8_t al) /* addr 1: mode / seek / write */
{
    uint8_t data = al & 0x0F;
    if (rtc_chipselect != 1)
        return;
    if (rtc_ready == 0)
        return;
    if (rtc_state == RTC_MODE) {
        if (data != 0x03 && data != 0x0C)
            return; /* only linear/indexed modes */
        rtc_state = RTC_SEEK;
        rtc_mdr = data;
    } else if (rtc_state == RTC_SEEK) {
        if (rtc_mdr == 0x03)
            rtc_state = RTC_WRITE;
        if (rtc_mdr == 0x0C)
            rtc_state = RTC_READ;
        rtc_offset = data;
        rtc_mdr = data;
    } else if (rtc_state == RTC_WRITE) {
        rtc_write_reg(rtc_offset, data);
        rtc_offset = (rtc_offset + 1) & 0x0F;
        rtc_mdr = data;
    }
    rtc_pack();
}
REGABI_REG_READ8(SPC4841);
uint8_t c_SPC4841(void) /* addr 1: read the register file */
{
    uint8_t val;
    rtc_sync();
    if (rtc_chipselect != 1) {
        rtc_pack();
        return 0;
    }
    if (rtc_ready == 0) {
        rtc_pack();
        return 0;
    }
    if (rtc_state == RTC_WRITE) {
        rtc_pack();
        return rtc_mdr;
    }
    if (rtc_state != RTC_READ) {
        rtc_pack();
        return 0;
    }
    val = rtc_read_reg(rtc_offset);
    rtc_offset = (rtc_offset + 1) & 0x0F;
    rtc_pack();
    return val;
}

REGABI_REG_WRITE8(SPC4842w);
void c_SPC4842w(uint8_t al) { (void)al; } /* status port: no write */
REGABI_REG_READ8(SPC4842);
uint8_t c_SPC4842(void) { return rtc_ready << 7; }

/* 0x4850-485F are ZSNES shadow mirrors of the register file, read directly
   like the old asm (single table load).  The live clock advances and repacks
   the shadow through the serial protocol (0x4841), so no per-read sync here. */
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

/* ===== Stage 5: data ROM port (0x4810-0x481A) and bank mapping (0x4831-0x4834) =====
   The data port walks ROM at romdata+0x100000+SPCROMPtr, optionally offset by
   SPCROMAdj, auto-incrementing the pointer SPCROMtoI selects (either SPCROMPtr
   or SPCROMAdj) by 1, by SPCROMInc, or by SPCROMAdj depending on the command
   byte SPCROMCom.  SPCROMtoI holds the address of the live pointer.  See the
   asm command-mode bit table for the encoding. */
extern uint8_t* romdata; /* gblvars.h */
extern uint8_t curromsize; /* initc.c */
extern uint8_t *snesmmap[256], *snesmap2[256]; /* SNES memory map */

#define ROM_INDIRECT (*(uint32_t*)(uintptr_t)SPCROMtoI)

REGABI_REG_READ8(SPC4810);
uint8_t c_SPC4810(void)
{
    uint8_t* com = (uint8_t*)&SPCROMCom;
    uint8_t* p;
    uint8_t al;
    if (SPCCheckFix == 0)
        return 0;
    p = romdata + 0x100000 + SPCROMPtr;
    if (com[0] & 2) {
        p = (uint8_t*)((uintptr_t)p + SPCROMAdj);
        *(uint16_t*)&SPCROMAdj += 1;
        return *p;
    }
    al = *p;
    if (com[1] == 0)
        ROM_INDIRECT += 1; /* add 1 after the read */
    else if (com[1] == 1)
        ROM_INDIRECT += SPCROMInc; /* add 4816 after the read */
    return al;
}

REGABI_REG_READ8(SPC481A);
uint8_t c_SPC481A(void)
{
    uint8_t* com = (uint8_t*)&SPCROMCom;
    uint8_t al;
    if (SPCCheckFix == 0)
        return 0;
    al = romdata[0x100000 + SPCROMPtr + *(uint16_t*)&SPCROMAdj];
    if (com[1] == 4)
        ROM_INDIRECT += SPCROMAdj; /* 16-bit 4814 add after 481A */
    return al;
}

#define SPC_BYTE_R(name, base, idx) \
    REGABI_REG_READ8(name);         \
    uint8_t c_##name(void) { return ((uint8_t*)&base)[idx]; }
#define SPC_BYTE_W(name, base, idx) \
    REGABI_REG_WRITE8(name);        \
    void c_##name(uint8_t al) { ((uint8_t*)&base)[idx] = al; }
SPC_BYTE_R(SPC4811, SPCROMPtr, 0)
SPC_BYTE_R(SPC4812, SPCROMPtr, 1)
SPC_BYTE_R(SPC4813, SPCROMPtr, 2)
SPC_BYTE_R(SPC4814, SPCROMAdj, 0)
SPC_BYTE_R(SPC4815, SPCROMAdj, 1)
SPC_BYTE_R(SPC4816, SPCROMInc, 0)
SPC_BYTE_R(SPC4817, SPCROMInc, 1)
SPC_BYTE_R(SPC4818, SPCROMCom, 0)

REGABI_REG_WRITE8(SPC4811w);
void c_SPC4811w(uint8_t al)
{
    ((uint8_t*)&SPCROMPtr)[0] = al;
    ((uint8_t*)&SPCCheckFix)[0] = 1;
}
SPC_BYTE_W(SPC4812w, SPCROMPtr, 1)
SPC_BYTE_W(SPC4813w, SPCROMPtr, 2)

REGABI_REG_WRITE8(SPC4814w);
void c_SPC4814w(uint8_t al)
{
    uint8_t* com = (uint8_t*)&SPCROMCom;
    uint8_t* adj = (uint8_t*)&SPCROMAdj;
    adj[0] = al;
    if (com[1] == 2) { /* 8-bit 4814 add */
        int32_t inc = (com[0] & 0x08) ? (int8_t)adj[0] : (int32_t)(uint8_t)adj[0];
        ROM_INDIRECT += (uint32_t)inc;
    }
}

REGABI_REG_WRITE8(SPC4815w);
void c_SPC4815w(uint8_t al)
{
    uint8_t* com = (uint8_t*)&SPCROMCom;
    uint8_t* adj = (uint8_t*)&SPCROMAdj;
    adj[1] = al;
    *(uint16_t*)&adj[2] = ((com[0] & 0x08) && (adj[1] & 0x80)) ? 0xFFFF : 0;
    if (com[1] == 3) /* 16-bit 4814 add */
        ROM_INDIRECT += SPCROMAdj;
}

SPC_BYTE_W(SPC4816w, SPCROMInc, 0)

REGABI_REG_WRITE8(SPC4817w);
void c_SPC4817w(uint8_t al)
{
    uint8_t* com = (uint8_t*)&SPCROMCom;
    uint8_t* inc = (uint8_t*)&SPCROMInc;
    inc[1] = al;
    *(uint16_t*)&inc[2] = ((com[0] & 0x04) && (inc[1] & 0x40)) ? 0xFFFF : 0;
}

REGABI_REG_WRITE8(SPC4818w);
void c_SPC4818w(uint8_t al)
{
    uint8_t* com = (uint8_t*)&SPCROMCom;
    uint8_t* adj = (uint8_t*)&SPCROMAdj;
    uint8_t* inc = (uint8_t*)&SPCROMInc;
    com[0] = al;
    *(uint16_t*)&adj[2] = ((com[0] & 0x08) && (adj[1] & 0x80)) ? 0xFFFF : 0;
    *(uint16_t*)&inc[2] = ((com[0] & 0x04) && (inc[1] & 0x40)) ? 0xFFFF : 0;
    SPCROMtoI = (uint32_t)(uintptr_t)((com[0] & 0x10) ? &SPCROMAdj : &SPCROMPtr);
    if (al & 0x02) {
        if (al & 0x40)
            com[1] = (al & 0x20) ? 4 : 3; /* 16-bit 4814 (after 481A / direct) */
        else
            com[1] = (al & 0x20) ? 2 : 0xFF; /* 8-bit 4814 / no offset add */
    } else {
        com[1] = (al & 0x01) ? 1 : 0; /* add 4816 / add 1 after 4810 read */
    }
}

static void spc7110_bankswitch(int abyte, int bank, uint8_t al)
{
    uint32_t v = (uint8_t)(al + 1);
    uint8_t* base;
    int i;
    ((uint8_t*)&SPC7110BankA)[abyte] = al;
    if (curromsize == 13) {
        while (v >= 5)
            v -= 4;
    } else {
        while (v >= 3)
            v -= 2;
    }
    v &= 0x07;
    base = romdata + ((uintptr_t)v << 20);
    for (i = 0; i < 16; i++) {
        snesmap2[bank + i] = base;
        snesmmap[bank + i] = base;
        base += 0x10000;
    }
}

REGABI_REG_WRITE8(SPC4831w);
void c_SPC4831w(uint8_t al) { spc7110_bankswitch(0, 0xD0, al); }
REGABI_REG_WRITE8(SPC4832w);
void c_SPC4832w(uint8_t al) { spc7110_bankswitch(1, 0xE0, al); }
REGABI_REG_WRITE8(SPC4833w);
void c_SPC4833w(uint8_t al) { spc7110_bankswitch(2, 0xF0, al); }

SPC_BYTE_R(SPC4831, SPC7110BankA, 0)
SPC_BYTE_R(SPC4832, SPC7110BankA, 1)
SPC_BYTE_R(SPC4833, SPC7110BankA, 2)
REGABI_REG_READ8(SPC4834);
uint8_t c_SPC4834(void) { return 0; }
/* ===== Stage 6: register dispatch + memory/SRAM glue =====
   Replaces the asm initSPC7110regs / SPC7110Reset handler registration and the
   memaccess / SRAM bank handlers so 7110proc.asm can be dropped.  The SRAM
   handlers still bridge into the asm memory core (memaccessbank*, regaccessbank*,
   sramaccessbank*b in memory.asm) via register-ABI trampolines; they collapse to
   plain C once that core is ported. */

/* register dispatch tables in the cpu memory core (see ui.h) */
typedef void eop();
extern eop* regptra[0x3000];
extern eop* regptwa[0x3000];
#define SPC_REGR(x) (regptra[(x) - 0x2000])
#define SPC_REGW(x) (regptwa[(x) - 0x2000])

/* register-ABI trampolines emitted by the REGABI_REG_* macros above */
extern void SPC4800(), SPC4801(), SPC4802(), SPC4803(), SPC4804(), SPC4805();
extern void SPC4806(), SPC4807(), SPC4808(), SPC4809(), SPC480A(), SPC480B();
extern void SPC480C(), SPC4810(), SPC4811(), SPC4812(), SPC4813(), SPC4814();
extern void SPC4815(), SPC4816(), SPC4817(), SPC4818(), SPC481A(), SPC4820();
extern void SPC4821(), SPC4822(), SPC4823(), SPC4824(), SPC4825(), SPC4826();
extern void SPC4827(), SPC4828(), SPC4829(), SPC482A(), SPC482B(), SPC482C();
extern void SPC482D(), SPC482E(), SPC482F(), SPC4831(), SPC4832(), SPC4833();
extern void SPC4834(), SPC4840(), SPC4841(), SPC4842(), SPC4850(), SPC4851();
extern void SPC4852(), SPC4853(), SPC4854(), SPC4855(), SPC4856(), SPC4857();
extern void SPC4858(), SPC4859(), SPC485A(), SPC485B(), SPC485C(), SPC485D();
extern void SPC485E(), SPC485F();
extern void SPC4801w(), SPC4802w(), SPC4803w(), SPC4804w(), SPC4805w(), SPC4806w();
extern void SPC4807w(), SPC4808w(), SPC4809w(), SPC480Aw(), SPC480Bw(), SPC4811w();
extern void SPC4812w(), SPC4813w(), SPC4814w(), SPC4815w(), SPC4816w(), SPC4817w();
extern void SPC4818w(), SPC4820w(), SPC4821w(), SPC4822w(), SPC4823w(), SPC4824w();
extern void SPC4825w(), SPC4826w(), SPC4827w(), SPC482Ew(), SPC4831w(), SPC4832w();
extern void SPC4833w(), SPC4840w(), SPC4841w(), SPC4842w();

void initSPC7110regs(void) /* register the read handlers */
{
    SPC_REGR(0x4800) = SPC4800;
    SPC_REGR(0x4801) = SPC4801;
    SPC_REGR(0x4802) = SPC4802;
    SPC_REGR(0x4803) = SPC4803;
    SPC_REGR(0x4804) = SPC4804;
    SPC_REGR(0x4805) = SPC4805;
    SPC_REGR(0x4806) = SPC4806;
    SPC_REGR(0x4807) = SPC4807;
    SPC_REGR(0x4808) = SPC4808;
    SPC_REGR(0x4809) = SPC4809;
    SPC_REGR(0x480A) = SPC480A;
    SPC_REGR(0x480B) = SPC480B;
    SPC_REGR(0x480C) = SPC480C;
    SPC_REGR(0x4810) = SPC4810;
    SPC_REGR(0x4811) = SPC4811;
    SPC_REGR(0x4812) = SPC4812;
    SPC_REGR(0x4813) = SPC4813;
    SPC_REGR(0x4814) = SPC4814;
    SPC_REGR(0x4815) = SPC4815;
    SPC_REGR(0x4816) = SPC4816;
    SPC_REGR(0x4817) = SPC4817;
    SPC_REGR(0x4818) = SPC4818;
    SPC_REGR(0x481A) = SPC481A;
    SPC_REGR(0x4820) = SPC4820;
    SPC_REGR(0x4821) = SPC4821;
    SPC_REGR(0x4822) = SPC4822;
    SPC_REGR(0x4823) = SPC4823;
    SPC_REGR(0x4824) = SPC4824;
    SPC_REGR(0x4825) = SPC4825;
    SPC_REGR(0x4826) = SPC4826;
    SPC_REGR(0x4827) = SPC4827;
    SPC_REGR(0x4828) = SPC4828;
    SPC_REGR(0x4829) = SPC4829;
    SPC_REGR(0x482A) = SPC482A;
    SPC_REGR(0x482B) = SPC482B;
    SPC_REGR(0x482C) = SPC482C;
    SPC_REGR(0x482D) = SPC482D;
    SPC_REGR(0x482E) = SPC482E;
    SPC_REGR(0x482F) = SPC482F;
    SPC_REGR(0x4831) = SPC4831;
    SPC_REGR(0x4832) = SPC4832;
    SPC_REGR(0x4833) = SPC4833;
    SPC_REGR(0x4834) = SPC4834;
    SPC_REGR(0x4840) = SPC4840;
    SPC_REGR(0x4841) = SPC4841;
    SPC_REGR(0x4842) = SPC4842;
    SPC_REGR(0x4850) = SPC4850;
    SPC_REGR(0x4851) = SPC4851;
    SPC_REGR(0x4852) = SPC4852;
    SPC_REGR(0x4853) = SPC4853;
    SPC_REGR(0x4854) = SPC4854;
    SPC_REGR(0x4855) = SPC4855;
    SPC_REGR(0x4856) = SPC4856;
    SPC_REGR(0x4857) = SPC4857;
    SPC_REGR(0x4858) = SPC4858;
    SPC_REGR(0x4859) = SPC4859;
    SPC_REGR(0x485A) = SPC485A;
    SPC_REGR(0x485B) = SPC485B;
    SPC_REGR(0x485C) = SPC485C;
    SPC_REGR(0x485D) = SPC485D;
    SPC_REGR(0x485E) = SPC485E;
    SPC_REGR(0x485F) = SPC485F;
}

void SPC7110Reset(void) /* register the write handlers */
{
    SPC_REGW(0x4801) = SPC4801w;
    SPC_REGW(0x4802) = SPC4802w;
    SPC_REGW(0x4803) = SPC4803w;
    SPC_REGW(0x4804) = SPC4804w;
    SPC_REGW(0x4805) = SPC4805w;
    SPC_REGW(0x4806) = SPC4806w;
    SPC_REGW(0x4807) = SPC4807w;
    SPC_REGW(0x4808) = SPC4808w;
    SPC_REGW(0x4809) = SPC4809w;
    SPC_REGW(0x480A) = SPC480Aw;
    SPC_REGW(0x480B) = SPC480Bw;
    SPC_REGW(0x4811) = SPC4811w;
    SPC_REGW(0x4812) = SPC4812w;
    SPC_REGW(0x4813) = SPC4813w;
    SPC_REGW(0x4814) = SPC4814w;
    SPC_REGW(0x4815) = SPC4815w;
    SPC_REGW(0x4816) = SPC4816w;
    SPC_REGW(0x4817) = SPC4817w;
    SPC_REGW(0x4818) = SPC4818w;
    SPC_REGW(0x4820) = SPC4820w;
    SPC_REGW(0x4821) = SPC4821w;
    SPC_REGW(0x4822) = SPC4822w;
    SPC_REGW(0x4823) = SPC4823w;
    SPC_REGW(0x4824) = SPC4824w;
    SPC_REGW(0x4825) = SPC4825w;
    SPC_REGW(0x4826) = SPC4826w;
    SPC_REGW(0x4827) = SPC4827w;
    SPC_REGW(0x482E) = SPC482Ew;
    SPC_REGW(0x4831) = SPC4831w;
    SPC_REGW(0x4832) = SPC4832w;
    SPC_REGW(0x4833) = SPC4833w;
    SPC_REGW(0x4840) = SPC4840w;
    SPC_REGW(0x4841) = SPC4841w;
    SPC_REGW(0x4842) = SPC4842w;
}

/* data ROM mapped to $50:0000-$50:FFFF: reads pull from data port 0x4800 */
REGABI_BANK_READ8(memaccessspc7110r8);
uint8_t c_memaccessspc7110r8(uint32_t addr)
{
    (void)addr;
    return c_SPC4800();
}

REGABI_BANK_READ16(memaccessspc7110r16);
uint16_t c_memaccessspc7110r16(uint32_t addr)
{
    uint8_t lo, hi;
    (void)addr;
    SPC7110_4800();
    lo = SPCCompressionRegs[0];
    SPC7110_4800();
    hi = SPCCompressionRegs[0];
    return lo | (uint16_t)hi << 8;
}

REGABI_BANK_WRITE8(memaccessspc7110w8);
void c_memaccessspc7110w8(uint32_t addr, uint8_t val)
{
    (void)addr;
    (void)val;
}

REGABI_BANK_WRITE16(memaccessspc7110w16);
void c_memaccessspc7110w16(uint32_t addr, uint16_t val)
{
    (void)addr;
    (void)val;
}

/* SPC7110 SRAM window $x0:6000-$x0:7FFF: bit15 -> ROM/WRAM, <0x6000 -> regs,
   else 8KB-per-bank SRAM.  Register ABI: address in ECX, bank in EBX, value
   in AL/AX.  Faithful to the asm until the memory core is C. */
#if defined(__GNUC__) && defined(__i386__)

__asm__(
    ".globl " REGABI_SYM(SPC7110ReadSRAM8b) "\n" REGABI_SYM(SPC7110ReadSRAM8b) ":\n"
                                                                               "testw $0x8000, %cx\n"
                                                                               "jnz " REGABI_SYM(memaccessbankr8) "\n"
                                                                                                                  "cmpl $0x6000, %ecx\n"
                                                                                                                  "jb " REGABI_SYM(regaccessbankr8) "\n"
                                                                                                                                                    "pushl %ecx\n"
                                                                                                                                                    "subl $0x6000, %ecx\n"
                                                                                                                                                    "shll $13, %ebx\n"
                                                                                                                                                    "addl %ebx, %ecx\n"
                                                                                                                                                    "andl $0xFFFF, %ecx\n"
                                                                                                                                                    "call " REGABI_SYM(sramaccessbankr8b) "\n"
                                                                                                                                                                                          "popl %ecx\n"
                                                                                                                                                                                          "ret\n");

__asm__(
    ".globl " REGABI_SYM(SPC7110ReadSRAM16b) "\n" REGABI_SYM(SPC7110ReadSRAM16b) ":\n"
                                                                                 "testw $0x8000, %cx\n"
                                                                                 "jnz " REGABI_SYM(memaccessbankr16) "\n"
                                                                                                                     "cmpl $0x6000, %ecx\n"
                                                                                                                     "jb " REGABI_SYM(regaccessbankr16) "\n"
                                                                                                                                                        "pushl %ecx\n"
                                                                                                                                                        "subl $0x6000, %ecx\n"
                                                                                                                                                        "shll $13, %ebx\n"
                                                                                                                                                        "addl %ebx, %ecx\n"
                                                                                                                                                        "andl $0xFFFF, %ecx\n"
                                                                                                                                                        "call " REGABI_SYM(sramaccessbankr16b) "\n"
                                                                                                                                                                                               "popl %ecx\n"
                                                                                                                                                                                               "ret\n");

__asm__(
    ".globl " REGABI_SYM(SPC7110WriteSRAM8b) "\n" REGABI_SYM(SPC7110WriteSRAM8b) ":\n"
                                                                                 "testw $0x8000, %cx\n"
                                                                                 "jnz " REGABI_SYM(memaccessbankw8) "\n"
                                                                                                                    "cmpl $0x6000, %ecx\n"
                                                                                                                    "jb " REGABI_SYM(regaccessbankw8) "\n"
                                                                                                                                                      "pushl %ecx\n"
                                                                                                                                                      "subl $0x6000, %ecx\n"
                                                                                                                                                      "shll $13, %ebx\n"
                                                                                                                                                      "addl %ebx, %ecx\n"
                                                                                                                                                      "andl $0xFFFF, %ecx\n"
                                                                                                                                                      "call " REGABI_SYM(sramaccessbankw8b) "\n"
                                                                                                                                                                                            "popl %ecx\n"
                                                                                                                                                                                            "ret\n");

__asm__(
    ".globl " REGABI_SYM(SPC7110WriteSRAM16b) "\n" REGABI_SYM(SPC7110WriteSRAM16b) ":\n"
                                                                                   "testw $0x8000, %cx\n"
                                                                                   "jnz " REGABI_SYM(memaccessbankw16) "\n"
                                                                                                                       "cmpl $0x6000, %ecx\n"
                                                                                                                       "jb " REGABI_SYM(regaccessbankw16) "\n"
                                                                                                                                                          "pushl %ecx\n"
                                                                                                                                                          "subl $0x6000, %ecx\n"
                                                                                                                                                          "shll $13, %ebx\n"
                                                                                                                                                          "addl %ebx, %ecx\n"
                                                                                                                                                          "andl $0xFFFF, %ecx\n"
                                                                                                                                                          "call " REGABI_SYM(sramaccessbankw16b) "\n"
                                                                                                                                                                                                 "popl %ecx\n"
                                                                                                                                                                                                 "ret\n");

#endif
