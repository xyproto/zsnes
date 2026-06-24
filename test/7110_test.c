/*
 * SPC7110 register interface unit tests (chips/7110proc.c).
 *
 * Stage 1 covers the save-state-critical data block: the run from SPCMultA
 * through SPCCompressionRegs must stay contiguous and in the exact order the
 * asm used, since zstate.c saves &SPCMultA .. PHnum2writespc7110reg as one
 * block. Later stages add register-handler tests.
 */

#include <stdint.h>
#include <stdlib.h>

#include "zstest.h"

/* --- SPC7110 state block (chips/7110proc.c) ------------------------------- */
extern uint32_t SPCMultA, SPCMultB, SPCDivEnd, SPCMulRes, SPCDivRes;
extern uint32_t SPC7110BankA, SPC7110RTCStat, SPCROMPtr, SPCROMtoI;
extern uint8_t SPC7110RTC[16], SPC7110RTCB[16], SPCCompressionRegs[13];
extern uint32_t PHnum2writespc7110reg;

/* externs 7110proc.c references; the test owns them */
int spc7110initc_calls, spc4800_calls, spc4806w_calls;
void SPC7110initC(void) { spc7110initc_calls++; }
void SPC7110_4800(void) { spc4800_calls++; }
void SPC7110_4806w(void) { spc4806w_calls++; }

void SPC7110init(void);

/* Stage 2 handlers */
uint8_t c_SPC4800(void), c_SPC4801(void), c_SPC480B(void), c_SPC480C(void);
void c_SPC4801w(uint8_t), c_SPC4806w(uint8_t), c_SPC480Bw(uint8_t);

/* Stage 3 math unit (0x4820-0x482F) */
extern uint32_t SPCMulRes, SPCSignedVal;
uint8_t c_SPC4820(void), c_SPC4821(void), c_SPC4822(void), c_SPC4823(void);
uint8_t c_SPC4824(void), c_SPC4825(void), c_SPC4826(void), c_SPC4827(void);
uint8_t c_SPC4828(void), c_SPC4829(void), c_SPC482A(void), c_SPC482B(void);
uint8_t c_SPC482C(void), c_SPC482D(void), c_SPC482E(void), c_SPC482F(void);
void c_SPC4820w(uint8_t), c_SPC4821w(uint8_t), c_SPC4822w(uint8_t), c_SPC4823w(uint8_t);
void c_SPC4824w(uint8_t), c_SPC4825w(uint8_t), c_SPC4826w(uint8_t), c_SPC4827w(uint8_t);
void c_SPC482Ew(uint8_t);

/* Stage 4 RTC (0x4840-0x4842, 0x4850-0x485F) */
uint8_t c_SPC4840(void), c_SPC4841(void), c_SPC4842(void);
void c_SPC4840w(uint8_t), c_SPC4841w(uint8_t), c_SPC4842w(uint8_t);
uint8_t c_SPC4850(void), c_SPC4851(void), c_SPC4852(void), c_SPC4853(void);
uint8_t c_SPC4854(void), c_SPC4855(void), c_SPC4856(void), c_SPC485F(void);

/* The RTC seeds and advances from these; the test drives them deterministically.
   GetTime: sec | min<<8 | hour<<16 (BCD).  GetDate: mday | (mon+1)<<8 |
   year<<16 | wday<<28 (mday/year BCD, month binary 1-12). */
uint32_t test_time = 0x231259; /* 23:12:59 */
uint32_t test_date = 0x30190625; /* wday 3, year 19, mon 6, mday 25 */
uint32_t GetTime(void) { return test_time; }
uint32_t GetDate(void) { return test_date; }
void SPC7110RTCReset(void);

/* Stage 5 data-port / bank-map externs the test owns */
extern uint32_t SPCROMPtr, SPCROMtoI, SPCROMAdj, SPCROMInc, SPCROMCom, SPCCheckFix;
uint8_t* romdata;
uint8_t curromsize;
uint8_t *snesmmap[256], *snesmap2[256];

/* Stage 5 handlers */
uint8_t c_SPC4810(void), c_SPC4811(void), c_SPC4812(void), c_SPC4813(void);
uint8_t c_SPC4814(void), c_SPC4815(void), c_SPC4816(void), c_SPC4817(void);
uint8_t c_SPC4818(void), c_SPC481A(void);
void c_SPC4811w(uint8_t), c_SPC4812w(uint8_t), c_SPC4813w(uint8_t);
void c_SPC4814w(uint8_t), c_SPC4815w(uint8_t), c_SPC4816w(uint8_t);
void c_SPC4817w(uint8_t), c_SPC4818w(uint8_t);
uint8_t c_SPC4831(void), c_SPC4832(void), c_SPC4833(void), c_SPC4834(void);
void c_SPC4831w(uint8_t), c_SPC4832w(uint8_t), c_SPC4833w(uint8_t);

#define OFF(sym) ((char*)&(sym) - (char*)&SPCMultA)

int main(void)
{
    ZT_SECTION("save-state block layout (must match 7110proc.asm)");
    /* Offsets within the &SPCMultA block, verified against the asm object. */
    ZT_CHECK_INT(OFF(SPCMultB), 4);
    ZT_CHECK_INT(OFF(SPCDivRes), 16);
    ZT_CHECK_INT(OFF(SPC7110BankA), 20);
    ZT_CHECK_INT(OFF(SPC7110RTC), 28);
    ZT_CHECK_INT(OFF(SPC7110RTCB), 44);
    ZT_CHECK_INT(OFF(SPCROMPtr), 60);
    ZT_CHECK_INT(OFF(SPCROMtoI), 64);
    ZT_CHECK_INT(OFF(SPCCompressionRegs), 88);
    /* zstate.c saves this many bytes starting at &SPCMultA. */
    ZT_CHECK_INT(PHnum2writespc7110reg, 101);

    ZT_SECTION("data block initial values");
    ZT_CHECK_INT(SPC7110BankA, 0x020100);
    ZT_CHECK_INT(SPC7110RTC[6], 1);
    ZT_CHECK_INT(SPC7110RTC[8], 1);
    ZT_CHECK_INT(SPC7110RTC[14], 0x0F);
    ZT_CHECK_INT(SPC7110RTCB[13], 1);
    ZT_CHECK_INT(SPC7110RTCB[15], 6);
    /* SPCROMtoI is a self-referential pointer to SPCROMPtr */
    ZT_CHECK(SPCROMtoI == (uint32_t)(uintptr_t)&SPCROMPtr);

    ZT_SECTION("SPC7110init");
    SPCMultA = 0xDEAD;
    SPC7110BankA = 0;
    SPCROMtoI = 0;
    spc7110initc_calls = 0;
    SPC7110init();
    ZT_CHECK_INT(spc7110initc_calls, 1); /* delegates to the C decompressor */
    ZT_CHECK_INT(SPCMultA, 0);
    ZT_CHECK_INT(SPC7110BankA, 0x020100);
    ZT_CHECK(SPCROMtoI == (uint32_t)(uintptr_t)&SPCROMPtr);

    ZT_SECTION("compression registers (0x4800-0x480C)");
    SPCCompressionRegs[0] = 0xA0;
    SPCCompressionRegs[1] = 0xA1;
    SPCCompressionRegs[11] = 0xBB;
    spc4800_calls = 0;
    ZT_CHECK_INT(c_SPC4800(), 0xA0); /* read 0 + side effect */
    ZT_CHECK_INT(spc4800_calls, 1);
    ZT_CHECK_INT(c_SPC4801(), 0xA1);
    ZT_CHECK_INT(c_SPC480B(), 0xBB);
    /* 0x480C clears itself on read */
    SPCCompressionRegs[12] = 0x7F;
    ZT_CHECK_INT(c_SPC480C(), 0x7F);
    ZT_CHECK_INT(SPCCompressionRegs[12], 0);
    /* writes land in the matching index; 0x4806 also pokes the decompressor */
    c_SPC4801w(0x11);
    ZT_CHECK_INT(SPCCompressionRegs[1], 0x11);
    c_SPC480Bw(0x22);
    ZT_CHECK_INT(SPCCompressionRegs[11], 0x22);
    spc4806w_calls = 0;
    c_SPC4806w(0x33);
    ZT_CHECK_INT(SPCCompressionRegs[6], 0x33);
    ZT_CHECK_INT(spc4806w_calls, 1);

    ZT_SECTION("math unit: unsigned multiply (0x4820/0x4824)");
    SPCSignedVal = 0;
    c_SPC482Ew(0); /* reset to clear state, signed flag off */
    c_SPC4820w(0x34); /* multiplicand low  */
    c_SPC4821w(0x12); /* multiplicand high -> 0x1234 */
    c_SPC4824w(0x78); /* multiplier low    */
    c_SPC4825w(0x56); /* multiplier high -> 0x5678, triggers multiply */
    /* 0x1234 * 0x5678 = 0x06260060 */
    ZT_CHECK_INT(SPCMulRes, 0x06260060);
    ZT_CHECK_INT(c_SPC4828(), 0x60);
    ZT_CHECK_INT(c_SPC4829(), 0x00);
    ZT_CHECK_INT(c_SPC482A(), 0x26);
    ZT_CHECK_INT(c_SPC482B(), 0x06);

    ZT_SECTION("math unit: signed multiply");
    c_SPC482Ew(1); /* signed mode */
    c_SPC4820w(0xFF);
    c_SPC4821w(0xFF); /* -1 */
    c_SPC4824w(0x02);
    c_SPC4825w(0x00); /* 2 -> result -2 */
    ZT_CHECK_INT(SPCMulRes, 0xFFFFFFFE);

    ZT_SECTION("math unit: unsigned divide (0x4826)");
    c_SPC482Ew(0);
    c_SPC4820w(0xFF);
    c_SPC4821w(0xFF);
    c_SPC4822w(0x00);
    c_SPC4823w(0x00); /* dividend 0x0000FFFF = 65535 */
    c_SPC4826w(0x0A);
    c_SPC4827w(0x00); /* divisor 10, triggers divide */
    ZT_CHECK_INT(SPCMulRes, 6553); /* 65535 / 10 */
    ZT_CHECK_INT(c_SPC482C(), 5); /* 65535 % 10 = 5 */
    ZT_CHECK_INT(c_SPC482D(), 0);

    ZT_SECTION("math unit: signed divide");
    c_SPC482Ew(1);
    c_SPC4820w(0xF6);
    c_SPC4821w(0xFF);
    c_SPC4822w(0xFF);
    c_SPC4823w(0xFF); /* dividend -10 */
    c_SPC4826w(0x03);
    c_SPC4827w(0x00); /* divisor 3 -> quotient -3, remainder -1 */
    ZT_CHECK_INT(SPCMulRes, 0xFFFFFFFD); /* -3 */
    ZT_CHECK_INT(c_SPC482C(), 0xFF); /* remainder -1 low byte */
    ZT_CHECK_INT(c_SPC482D(), 0xFF);

    ZT_SECTION("math unit: divide by zero (bsnes/snes9x behaviour)");
    c_SPC482Ew(0);
    c_SPC4820w(0x21);
    c_SPC4821w(0x43); /* dividend low word 0x4321 */
    c_SPC4822w(0x00);
    c_SPC4823w(0x00);
    c_SPC4826w(0x00);
    c_SPC4827w(0x00); /* divisor 0 */
    ZT_CHECK_INT(SPCMulRes, 0); /* quotient 0, not 0xFFFFFFFF */
    ZT_CHECK_INT(c_SPC482C(), 0x21); /* remainder = low word of dividend */
    ZT_CHECK_INT(c_SPC482D(), 0x43);

    ZT_SECTION("math unit: reset (0x482E) and read-as-zero status");
    c_SPC4820w(0xAA);
    c_SPC482Ew(1);
    ZT_CHECK_INT(c_SPC4820(), 0); /* SPCMultA cleared */
    ZT_CHECK_INT(SPCMulRes, 0);
    ZT_CHECK_INT(SPCSignedVal, 1); /* mode byte retained */
    ZT_CHECK_INT(c_SPC482E(), 0); /* status regs read as zero */
    ZT_CHECK_INT(c_SPC482F(), 0);

    ZT_SECTION("RTC: seeded from the host clock (23:12:59, 12-hour mode)");
    test_time = 0x231259; /* 23:12:59 BCD */
    test_date = 0x30190625; /* wday 3, year 19, mon 6, mday 25 */
    SPC7110init(); /* SPC7110RTCReset seeds the RTC */
    ZT_CHECK_INT(c_SPC4850(), 9); /* seconds 1's */
    ZT_CHECK_INT(c_SPC4852(), 2); /* minutes 1's */
    ZT_CHECK_INT(c_SPC4853() & 7, 1); /* minutes 10's (resync in bit3) */
    ZT_CHECK_INT(c_SPC4854(), 1); /* hour 1's (23 -> 11 PM) */
    ZT_CHECK_INT(c_SPC4855() & 3, 1); /* hour 10's */
    ZT_CHECK_INT((c_SPC4855() >> 2) & 1, 1); /* meridian = PM */

    ZT_SECTION("RTC: serial read protocol with chip select and seek");
    c_SPC4840w(0x01); /* chip select on */
    ZT_CHECK_INT(c_SPC4840(), 0x01);
    ZT_CHECK_INT(c_SPC4842(), 0x80); /* ready */
    c_SPC4841w(0x0C); /* mode: read/indexed */
    c_SPC4841w(0x00); /* seek to register 0 -> READ state */
    ZT_CHECK_INT(c_SPC4841(), 9); /* reg 0: seconds 1's, offset++ */
    ZT_CHECK_INT(c_SPC4841(), 5); /* reg 1: seconds 10's */
    ZT_CHECK_INT(c_SPC4841(), 2); /* reg 2: minutes 1's */

    ZT_SECTION("RTC: serial write protocol writes a register");
    c_SPC4840w(0x00); /* deselect to start a clean transaction */
    c_SPC4840w(0x01);
    c_SPC4841w(0x03); /* mode: write/linear */
    c_SPC4841w(0x00); /* seek to register 0 -> WRITE state */
    c_SPC4841w(0x07); /* reg 0 = 7 */
    c_SPC4840w(0x00); /* deselect resets the protocol */
    c_SPC4840w(0x01);
    c_SPC4841w(0x0C);
    c_SPC4841w(0x00);
    ZT_CHECK_INT(c_SPC4841(), 7); /* read back the written seconds 1's */

    ZT_SECTION("RTC: elapsed real seconds advance the clock");
    test_time = 0x231259; /* re-seed at 23:12:59 */
    SPC7110init();
    test_time = 0x231300; /* one second later: 23:13:00 */
    ZT_CHECK_INT(c_SPC4850(), 0); /* seconds rolled 59 -> 00 */
    ZT_CHECK_INT(c_SPC4852(), 3); /* minute carried 12 -> 13 */

    ZT_SECTION("RTC: direct reads 0x4850-0x485F mirror the registers");
    ZT_CHECK_INT(c_SPC4856(), SPC7110RTC[6]);
    ZT_CHECK_INT(c_SPC485F(), SPC7110RTC[15]);

    ZT_SECTION("RTC: chip select != 1 resets the serial protocol");
    c_SPC4840w(0x00); /* deselect to start a clean transaction */
    c_SPC4840w(0x01);
    c_SPC4841w(0x0C);
    c_SPC4841w(0x00); /* now in READ state */
    c_SPC4840w(0x02); /* invalid chip select -> reset */
    ZT_CHECK_INT(c_SPC4841(), 0); /* reads as zero when not selected */

    ZT_SECTION("data port: reads as zero before first 0x4811 write");
    romdata = calloc(0x200000, 1);
    for (int i = 0; i < 0x10000; i++)
        romdata[0x100000 + i] = (uint8_t)i; /* offset == byte value */
    SPC7110init(); /* clears SPCCheckFix */
    ZT_CHECK_INT(c_SPC4810(), 0);
    ZT_CHECK_INT(c_SPC481A(), 0);

    ZT_SECTION("data port: sequential read, +1 after read (cmd 0x00)");
    c_SPC4811w(0x10); /* ptr low = 0x10, also sets SPCCheckFix */
    c_SPC4812w(0x00);
    c_SPC4813w(0x00);
    c_SPC4818w(0x00); /* add 1 to SPCROMPtr after each 4810 read */
    ZT_CHECK_INT(c_SPC4810(), 0x10);
    ZT_CHECK_INT(c_SPC4810(), 0x11);
    ZT_CHECK_INT(c_SPC4810(), 0x12);
    ZT_CHECK_INT(c_SPC4811(), 0x13); /* pointer advanced */

    ZT_SECTION("data port: read with SPCROMInc step (cmd 0x01)");
    c_SPC4811w(0x00);
    c_SPC4812w(0x00);
    c_SPC4813w(0x00);
    c_SPC4816w(0x04); /* SPCROMInc = 4 */
    c_SPC4817w(0x00);
    c_SPC4818w(0x01); /* add SPCROMInc after each 4810 read */
    ZT_CHECK_INT(c_SPC4810(), 0x00);
    ZT_CHECK_INT(c_SPC4810(), 0x04);
    ZT_CHECK_INT(c_SPC4810(), 0x08);

    ZT_SECTION("data port: 4810 with SPCROMAdj offset (cmd bit1)");
    c_SPC4811w(0x00);
    c_SPC4812w(0x00);
    c_SPC4813w(0x00);
    c_SPC4814w(0x05); /* SPCROMAdj = 5 */
    c_SPC4815w(0x00);
    c_SPC4818w(0x02); /* read at ptr+adj, post-increment adj */
    ZT_CHECK_INT(c_SPC4810(), 0x05);
    ZT_CHECK_INT(c_SPC4814(), 0x06); /* adj auto-incremented */
    ZT_CHECK_INT(c_SPC4810(), 0x06);

    ZT_SECTION("bank map: 0x4831-0x4833 fill snesmmap/snesmap2");
    curromsize = 0; /* 24-bit reduction path */
    c_SPC4831w(0x00); /* bank base 1<<20, banks 0xD0-0xDF */
    ZT_CHECK(snesmmap[0xD0] == romdata + 0x100000);
    ZT_CHECK(snesmap2[0xD0] == romdata + 0x100000);
    ZT_CHECK(snesmmap[0xDF] == romdata + 0x100000 + 15 * 0x10000);
    ZT_CHECK_INT(c_SPC4831(), 0x00);
    c_SPC4832w(0x01); /* al+1=2 -> v=2; banks 0xE0-0xEF, base 2<<20 */
    ZT_CHECK(snesmmap[0xE0] == romdata + 0x200000);
    ZT_CHECK_INT(c_SPC4832(), 0x01);
    c_SPC4833w(0x02); /* al+1=3 -> reduce to 1; banks 0xF0-0xFF, base 1<<20 */
    ZT_CHECK(snesmmap[0xF0] == romdata + 0x100000);
    ZT_CHECK_INT(c_SPC4833(), 0x02);
    ZT_CHECK_INT(c_SPC4834(), 0x00);

    free(romdata);
    romdata = NULL;

    ZT_RESULTS();
}
