/*
 * SPC7110 register interface unit tests (chips/7110proc.c).
 *
 * Stage 1 covers the save-state-critical data block: the run from SPCMultA
 * through SPCCompressionRegs must stay contiguous and in the exact order the
 * asm used, since zstate.c saves &SPCMultA .. PHnum2writespc7110reg as one
 * block. Later stages add register-handler tests.
 */

#include <stdint.h>

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

    ZT_RESULTS();
}
