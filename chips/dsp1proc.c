/* C port of dsp1proc.asm: the DSP1 register/command interface.
   The math lives in dsp1emu.c; this marshals parameters/results. */
#include "regabi.h"
#include <stdint.h>

/* operands + commands, defined in dsp1emu.c */
extern short Op00Multiplicand;
extern short Op00Multiplier;
extern short Op00Result;
extern short Op20Multiplicand;
extern short Op20Multiplier;
extern short Op20Result;
extern signed short Op10Coefficient;
extern signed short Op10Exponent;
extern signed short Op10CoefficientR;
extern signed short Op10ExponentR;
extern short Op04Angle;
extern short Op04Radius;
extern short Op04Sin;
extern short Op04Cos;
extern short Op0CA;
extern short Op0CX1;
extern short Op0CY1;
extern short Op0CX2;
extern short Op0CY2;
extern short Op02FX;
extern short Op02FY;
extern short Op02FZ;
extern short Op02LFE;
extern short Op02LES;
extern short Op02AAS;
extern short Op02AZS;
extern short Op02VOF;
extern short Op02VVA;
extern short Op02CX;
extern short Op02CY;
extern short Op0AVS;
extern short Op0AA;
extern short Op0AB;
extern short Op0AC;
extern short Op0AD;
extern short Op06X;
extern short Op06Y;
extern short Op06Z;
extern short Op06H;
extern short Op06V;
extern short Op06M;
extern short Op01m;
extern short Op01Zr;
extern short Op01Xr;
extern short Op01Yr;
extern short Op11m;
extern short Op11Zr;
extern short Op11Xr;
extern short Op11Yr;
extern short Op21m;
extern short Op21Zr;
extern short Op21Xr;
extern short Op21Yr;
extern short Op0DX;
extern short Op0DY;
extern short Op0DZ;
extern short Op0DF;
extern short Op0DL;
extern short Op0DU;
extern short Op1DX;
extern short Op1DY;
extern short Op1DZ;
extern short Op1DF;
extern short Op1DL;
extern short Op1DU;
extern short Op2DX;
extern short Op2DY;
extern short Op2DZ;
extern short Op2DF;
extern short Op2DL;
extern short Op2DU;
extern short Op03F;
extern short Op03L;
extern short Op03U;
extern short Op03X;
extern short Op03Y;
extern short Op03Z;
extern short Op13F;
extern short Op13L;
extern short Op13U;
extern short Op13X;
extern short Op13Y;
extern short Op13Z;
extern short Op23F;
extern short Op23L;
extern short Op23U;
extern short Op23X;
extern short Op23Y;
extern short Op23Z;
extern short Op14Zr;
extern short Op14Xr;
extern short Op14Yr;
extern short Op14U;
extern short Op14F;
extern short Op14L;
extern short Op14Zrr;
extern short Op14Xrr;
extern short Op14Yrr;
extern short Op0EH;
extern short Op0EV;
extern short Op0EX;
extern short Op0EY;
extern short Op0BX;
extern short Op0BY;
extern short Op0BZ;
extern short Op0BS;
extern short Op1BX;
extern short Op1BY;
extern short Op1BZ;
extern short Op1BS;
extern short Op2BX;
extern short Op2BY;
extern short Op2BZ;
extern short Op2BS;
extern short Op08X, Op08Y, Op08Z, Op08Ll, Op08Lh;
extern short Op18X, Op18Y, Op18Z, Op18R, Op18D;
extern short Op38X, Op38Y, Op38Z, Op38R, Op38D;
extern short Op28X;
extern short Op28Y;
extern short Op28Z;
extern short Op28R;
extern short Op1CX, Op1CY, Op1CZ;
extern short Op1CXBR, Op1CYBR, Op1CZBR, Op1CXAR, Op1CYAR, Op1CZAR;
extern short Op1CX1;
extern short Op1CY1;
extern short Op1CZ1;
extern short Op1CX2;
extern short Op1CY2;
extern short Op1CZ2;
extern short Op2FUnknown;
extern short Op2FSize;
extern short Op0FPass;
void DSPOp00(void);
void DSPOp01(void);
void DSPOp02(void);
void DSPOp03(void);
void DSPOp04(void);
void DSPOp06(void);
void DSPOp08(void);
void DSPOp0A(void);
void DSPOp0B(void);
void DSPOp0C(void);
void DSPOp0D(void);
void DSPOp0E(void);
void DSPOp0F(void);
void DSPOp10(void);
void DSPOp11(void);
void DSPOp13(void);
void DSPOp14(void);
void DSPOp18(void);
void DSPOp1B(void);
void DSPOp1C(void);
void DSPOp1D(void);
void DSPOp21(void);
void DSPOp23(void);
void DSPOp28(void);
void DSPOp2B(void);
void DSPOp2D(void);
void DSPOp0A(void);

/* DSP1 register state (was .bss in dsp1proc.asm) */
uint8_t DSP1COp, DSP1RLeft, DSP1WLeft, DSP1CPtrW, DSP1CPtrR, DSPDet;
uint8_t DSPFuncUsed[256];
uint16_t DSP1VARS[16], DSP1RET[16];
uint32_t dsp1ptr;
uint8_t dsp1array[4096];

static void DSP1_00(void)
{
    DSPDet |= 0x01;
    Op00Multiplicand = DSP1VARS[0];
    Op00Multiplier = DSP1VARS[1];
    DSPOp00();
    DSP1RET[0] = (uint16_t)Op00Result;
    DSP1RLeft = 1;
}

static void DSP1_10(void)
{
    Op10Coefficient = DSP1VARS[0];
    Op10Exponent = DSP1VARS[1];
    DSPOp10();
    DSP1RET[0] = (uint16_t)Op10CoefficientR;
    DSP1RET[1] = (uint16_t)Op10ExponentR;
    DSP1RLeft = 2;
}

static void DSP1_04(void)
{
    DSPDet |= 0x02;
    Op04Angle = DSP1VARS[0];
    Op04Radius = DSP1VARS[1];
    DSPOp04();
    DSP1RET[0] = (uint16_t)Op04Sin;
    DSP1RET[1] = (uint16_t)Op04Cos;
    DSP1RLeft = 2;
}

static void DSP1_08(void)
{
    Op08X = DSP1VARS[0];
    Op08Y = DSP1VARS[1];
    Op08Z = DSP1VARS[2];
    DSPOp08();
    DSP1RET[0] = (uint16_t)Op08Ll;
    DSP1RET[1] = (uint16_t)Op08Lh;
    DSP1RLeft = 2;
}

static void DSP1_18(void)
{
    Op18X = DSP1VARS[0];
    Op18Y = DSP1VARS[1];
    Op18Z = DSP1VARS[2];
    Op18R = DSP1VARS[3];
    DSPOp18();
    DSP1RET[0] = (uint16_t)Op18D;
    DSP1RLeft = 1;
}

static void DSP1_28(void)
{
    DSPDet |= 0x04;
    Op28X = DSP1VARS[0];
    Op28Y = DSP1VARS[1];
    Op28Z = DSP1VARS[2];
    DSPOp28();
    DSP1RET[0] = (uint16_t)Op28R;
    DSP1RLeft = 1;
}

static void DSP1_0C(void)
{
    DSPDet |= 0x08;
    Op0CA = DSP1VARS[0];
    Op0CX1 = DSP1VARS[1];
    Op0CY1 = DSP1VARS[2];
    DSPOp0C();
    DSP1RET[0] = (uint16_t)Op0CX2;
    DSP1RET[1] = (uint16_t)Op0CY2;
    DSP1RLeft = 2;
}

static void DSP1_1C(void)
{
    Op1CZ = DSP1VARS[0];
    Op1CY = DSP1VARS[1];
    Op1CX = DSP1VARS[2];
    Op1CXBR = DSP1VARS[3];
    Op1CYBR = DSP1VARS[4];
    Op1CZBR = DSP1VARS[5];
    DSPOp1C();
    DSP1RET[0] = (uint16_t)Op1CXAR;
    DSP1RET[1] = (uint16_t)Op1CYAR;
    DSP1RET[2] = (uint16_t)Op1CZAR;
    DSP1RLeft = 3;
}

static void DSP1_02(void)
{
    DSPDet |= 0x10;
    Op02FX = DSP1VARS[0];
    Op02FY = DSP1VARS[1];
    Op02FZ = DSP1VARS[2];
    Op02LFE = DSP1VARS[3];
    Op02LES = DSP1VARS[4];
    Op02AAS = DSP1VARS[5];
    Op02AZS = DSP1VARS[6];
    DSPOp02();
    DSP1RET[0] = (uint16_t)Op02VOF;
    DSP1RET[1] = (uint16_t)Op02VVA;
    DSP1RET[2] = (uint16_t)Op02CX;
    DSP1RET[3] = (uint16_t)Op02CY;
    DSP1RLeft = 4;
}

static void DSP1_0A(void)
{
    DSP1COp = 0x0A;
    DSPDet |= 0x20;
    Op0AVS = DSP1VARS[0];
    DSPOp0A();
    DSP1RET[0] = (uint16_t)Op0AA;
    DSP1RET[1] = (uint16_t)Op0AB;
    DSP1RET[2] = (uint16_t)Op0AC;
    DSP1RET[3] = (uint16_t)Op0AD;
    DSP1RLeft = 4;
}

static void DSP1_06(void)
{
    DSPDet |= 0x40;
    Op06X = DSP1VARS[0];
    Op06Y = DSP1VARS[1];
    Op06Z = DSP1VARS[2];
    DSPOp06();
    DSP1RET[0] = (uint16_t)Op06H;
    DSP1RET[1] = (uint16_t)Op06V;
    DSP1RET[2] = (uint16_t)Op06M;
    DSP1RLeft = 3;
}

static void DSP1_0E(void)
{
    Op0EH = DSP1VARS[0];
    Op0EV = DSP1VARS[1];
    DSPOp0E();
    DSP1RET[0] = (uint16_t)Op0EX;
    DSP1RET[1] = (uint16_t)Op0EY;
    DSP1RLeft = 2;
}

static void DSP1_01(void)
{
    Op01m = DSP1VARS[0];
    Op01Zr = DSP1VARS[1];
    Op01Yr = DSP1VARS[2];
    Op01Xr = DSP1VARS[3];
    DSPOp01();
}

static void DSP1_11(void)
{
    Op11m = DSP1VARS[0];
    Op11Zr = DSP1VARS[1];
    Op11Yr = DSP1VARS[2];
    Op11Xr = DSP1VARS[3];
    DSPOp11();
}

static void DSP1_21(void)
{
    Op21m = DSP1VARS[0];
    Op21Zr = DSP1VARS[1];
    Op21Yr = DSP1VARS[2];
    Op21Xr = DSP1VARS[3];
    DSPOp21();
}

static void DSP1_0D(void)
{
    Op0DX = DSP1VARS[0];
    Op0DY = DSP1VARS[1];
    Op0DZ = DSP1VARS[2];
    DSPOp0D();
    DSP1RET[0] = (uint16_t)Op0DF;
    DSP1RET[1] = (uint16_t)Op0DL;
    DSP1RET[2] = (uint16_t)Op0DU;
    DSP1RLeft = 3;
}

static void DSP1_0F(void)
{
    DSPOp0F();
    DSP1RET[0] = (uint16_t)Op0FPass;
    DSP1RLeft = 1;
}

static void DSP1_1D(void)
{
    Op1DX = DSP1VARS[0];
    Op1DY = DSP1VARS[1];
    Op1DZ = DSP1VARS[2];
    DSPOp1D();
    DSP1RET[0] = (uint16_t)Op1DF;
    DSP1RET[1] = (uint16_t)Op1DL;
    DSP1RET[2] = (uint16_t)Op1DU;
    DSP1RLeft = 3;
}

static void DSP1_2D(void)
{
    Op2DX = DSP1VARS[0];
    Op2DY = DSP1VARS[1];
    Op2DZ = DSP1VARS[2];
    DSPOp2D();
    DSP1RET[0] = (uint16_t)Op2DF;
    DSP1RET[1] = (uint16_t)Op2DL;
    DSP1RET[2] = (uint16_t)Op2DU;
    DSP1RLeft = 3;
}

static void DSP1_03(void)
{
    Op03F = DSP1VARS[0];
    Op03L = DSP1VARS[1];
    Op03U = DSP1VARS[2];
    DSPOp03();
    DSP1RET[0] = (uint16_t)Op03X;
    DSP1RET[1] = (uint16_t)Op03Y;
    DSP1RET[2] = (uint16_t)Op03Z;
    DSP1RLeft = 3;
}

static void DSP1_13(void)
{
    Op13F = DSP1VARS[0];
    Op13L = DSP1VARS[1];
    Op13U = DSP1VARS[2];
    DSPOp13();
    DSP1RET[0] = (uint16_t)Op13X;
    DSP1RET[1] = (uint16_t)Op13Y;
    DSP1RET[2] = (uint16_t)Op13Z;
    DSP1RLeft = 3;
}

static void DSP1_23(void)
{
    Op23F = DSP1VARS[0];
    Op23L = DSP1VARS[1];
    Op23U = DSP1VARS[2];
    DSPOp23();
    DSP1RET[0] = (uint16_t)Op23X;
    DSP1RET[1] = (uint16_t)Op23Y;
    DSP1RET[2] = (uint16_t)Op23Z;
    DSP1RLeft = 3;
}

static void DSP1_0B(void)
{
    Op0BX = DSP1VARS[0];
    Op0BY = DSP1VARS[1];
    Op0BZ = DSP1VARS[2];
    DSPOp0B();
    DSP1RET[0] = (uint16_t)Op0BS;
    DSP1RLeft = 1;
}

static void DSP1_1B(void)
{
    Op1BX = DSP1VARS[0];
    Op1BY = DSP1VARS[1];
    Op1BZ = DSP1VARS[2];
    DSPOp1B();
    DSP1RET[0] = (uint16_t)Op1BS;
    DSP1RLeft = 1;
}

static void DSP1_2B(void)
{
    Op2BX = DSP1VARS[0];
    Op2BY = DSP1VARS[1];
    Op2BZ = DSP1VARS[2];
    DSPOp2B();
    DSP1RET[0] = (uint16_t)Op2BS;
    DSP1RLeft = 1;
}

static void DSP1_14(void)
{
    Op14Zr = DSP1VARS[0];
    Op14Xr = DSP1VARS[1];
    Op14Yr = DSP1VARS[2];
    Op14U = DSP1VARS[3];
    Op14F = DSP1VARS[4];
    Op14L = DSP1VARS[5];
    DSPOp14();
    DSP1RET[0] = (uint16_t)Op14Zrr;
    DSP1RET[1] = (uint16_t)Op14Xrr;
    DSP1RET[2] = (uint16_t)Op14Yrr;
    DSP1RLeft = 3;
}

static void dsp1_process(void)
{
    DSP1CPtrR = 0;
    DSP1RLeft = 0;
    switch (DSP1COp) {
    case 0x00:
        DSP1_00();
        break;
    case 0x10:
        DSP1_10();
        break;
    case 0x04:
        DSP1_04();
        break;
    case 0x08:
        DSP1_08();
        break;
    case 0x18:
        DSP1_18();
        break;
    case 0x28:
        DSP1_28();
        break;
    case 0x0C:
        DSP1_0C();
        break;
    case 0x1C:
        DSP1_1C();
        break;
    case 0x02:
        DSP1_02();
        break;
    case 0x0A:
    case 0x1A:
        DSP1_0A();
        break;
    case 0x06:
        DSP1_06();
        break;
    case 0x0E:
        DSP1_0E();
        break;
    case 0x01:
        DSP1_01();
        break;
    case 0x11:
        DSP1_11();
        break;
    case 0x21:
        DSP1_21();
        break;
    case 0x0D:
        DSP1_0D();
        break;
    case 0x0F:
        DSP1_0F();
        break;
    case 0x1D:
        DSP1_1D();
        break;
    case 0x2D:
        DSP1_2D();
        break;
    case 0x03:
        DSP1_03();
        break;
    case 0x13:
        DSP1_13();
        break;
    case 0x23:
        DSP1_23();
        break;
    case 0x0B:
        DSP1_0B();
        break;
    case 0x1B:
        DSP1_1B();
        break;
    case 0x2B:
        DSP1_2B();
        break;
    case 0x14:
        DSP1_14();
        break;
    }
}

static const uint8_t dsp1_pcount[256] = { 2, 4, 7, 3, 2, 0, 3, 0, 3, 0, 1, 3, 3, 3, 2, 1, 2, 4, 0, 3, 6, 0, 0, 0, 4, 0, 1, 3, 6, 3, 0, 0, 0, 4, 0, 3, 0, 0, 0, 0, 3, 0, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static uint16_t dsp1_read_data(void)
{
    uint16_t r = DSP1RET[DSP1CPtrR++];
    if (--DSP1RLeft == 0 && DSP1COp == 0x0A) {
        DSPOp0A();
        DSP1RET[0] = (uint16_t)Op0AA;
        DSP1RET[1] = (uint16_t)Op0AB;
        DSP1RET[2] = (uint16_t)Op0AC;
        DSP1RET[3] = (uint16_t)Op0AD;
        DSP1RLeft = 4;
        DSP1CPtrR = 0;
    }
    return r;
}

REGABI_BANK_READ8(DSP1Read8b);
REGABI_BANK_READ16(DSP1Read16b);
REGABI_BANK_WRITE8(DSP1Write8b);
REGABI_BANK_WRITE16(DSP1Write16b);

uint8_t c_DSP1Read8b(uint32_t addr)
{
    return addr >= 0x7000 ? 0x80 : 0;
}

uint16_t c_DSP1Read16b(uint32_t addr)
{
    if (addr >= 0x7000)
        return DSP1WLeft ? 0xC000 : 0x8000;
    if (DSP1RLeft == 0)
        return 0;
    return dsp1_read_data();
}

void c_DSP1Write8b(uint32_t addr, uint8_t val)
{
    (void)addr;
    DSPFuncUsed[val] = 1;
    DSP1COp = val;
    DSP1CPtrW = 0;
    if (dsp1_pcount[val])
        DSP1WLeft = dsp1_pcount[val];
}

void c_DSP1Write16b(uint32_t addr, uint16_t val)
{
    (void)addr;
    if (DSP1WLeft == 0)
        return;
    DSP1VARS[DSP1CPtrW++] = val;
    if (--DSP1WLeft == 0)
        dsp1_process();
}

/* bank-0x3F (lo)/0xE0 (hi) mapping: status window at >=0xC000 */
uint16_t c_DSP1Read16b3Farea(uint32_t off)
{
    off |= 0x8000;
    if (off >= 0xC000)
        return DSP1WLeft ? 0xC000 : 0x8000;
    if (DSP1RLeft == 0)
        return 0;
    return dsp1_read_data();
}

#if defined(__GNUC__) && defined(__i386__)
__asm__(
    ".globl DSP1Write8b3F\nDSP1Write8b3F:\n"
    "  testl $0x8000, %ecx\n  jnz 1f\n"
    "  cmpb $0xE0, %bl\n  je 1f\n"
    "  jmp regaccessbankw8\n"
    "1:jmp DSP1Write8b\n"

    ".globl DSP1Write16b3F\nDSP1Write16b3F:\n"
    "  testl $0x8000, %ecx\n  jnz 2f\n"
    "  cmpb $0xE0, %bl\n  je 2f\n"
    "  jmp regaccessbankw16\n"
    "2:jmp DSP1Write16b\n"

    ".globl DSP1Read8b3F\nDSP1Read8b3F:\n"
    "  testl $0x8000, %ecx\n  jnz 3f\n"
    "  cmpb $0xE0, %bl\n  je 3f\n"
    "  jmp regaccessbankr8\n"
    "3:movb $0x80, %al\n  ret\n"

    ".globl DSP1Read16b3F\nDSP1Read16b3F:\n"
    "  testl $0x8000, %ecx\n  jnz 4f\n"
    "  cmpb $0xE0, %bl\n  je 4f\n"
    "  jmp regaccessbankr16\n"
    "4:pushl %ecx\n  pushl %edx\n  pushl %eax\n  pushl %ecx\n"
    "  call c_DSP1Read16b3Farea\n  addl $4, %esp\n"
    "  movw %ax, (%esp)\n  popl %eax\n  popl %edx\n  popl %ecx\n  ret\n");
#endif
