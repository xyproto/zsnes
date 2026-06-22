/* C port of 7110proc.asm (SPC7110 register interface). STAGE 1: data block.
   SPCMultA..SPCCompressionRegs must stay contiguous and in this exact order:
   zstate.c saves the run &SPCMultA .. PHnum2writespc7110reg bytes. The
   decompression engine lives in 7110emu.c. */
#include <stdint.h>

extern void SPC7110initC(void); /* 7110emu.c */

/* Save-state block (101 bytes). SPCROMtoI initialises to &SPCROMPtr, so the
   whole run is laid out via inline asm to keep it contiguous and self-exact
   under -fdata-sections. */
__asm__(
    ".pushsection .data.spc7110state,\"aw\",@progbits\n"
    ".global SPCMultA\nSPCMultA:\n.long 0\n"
    ".global SPCMultB\nSPCMultB:\n.long 0\n"
    ".global SPCDivEnd\nSPCDivEnd:\n.long 0\n"
    ".global SPCMulRes\nSPCMulRes:\n.long 0\n"
    ".global SPCDivRes\nSPCDivRes:\n.long 0\n"
    ".global SPC7110BankA\nSPC7110BankA:\n.long 0x020100\n"
    ".global SPC7110RTCStat\nSPC7110RTCStat:\n.long 0\n"
    ".global SPC7110RTC\nSPC7110RTC:\n.byte 0,0,0,0,0,0,1,0,1,0,0,0,0,0,0x0F,0\n"
    ".global SPC7110RTCB\nSPC7110RTCB:\n.byte 0,0,0,0,0,0,1,0,1,0,0,0,0,1,0x0F,6\n"
    ".global SPCROMPtr\nSPCROMPtr:\n.long 0\n"
    ".global SPCROMtoI\nSPCROMtoI:\n.long SPCROMPtr\n"
    ".global SPCROMAdj\nSPCROMAdj:\n.long 0\n"
    ".global SPCROMInc\nSPCROMInc:\n.long 0\n"
    ".global SPCROMCom\nSPCROMCom:\n.long 0\n"
    ".global SPCCheckFix\nSPCCheckFix:\n.long 0\n"
    ".global SPCSignedVal\nSPCSignedVal:\n.long 0\n"
    ".global SPCCompressionRegs\nSPCCompressionRegs:\n.zero 13\n"
    ".global PHnum2writespc7110reg\nPHnum2writespc7110reg:\n.long . - SPCMultA\n"
    ".popsection\n");

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
