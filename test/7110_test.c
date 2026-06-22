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

    ZT_RESULTS();
}
