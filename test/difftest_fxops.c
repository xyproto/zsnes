/*
 * Differential test: every SuperFX opcode handler ported into chips/fx_ops.h -
 * all 92 from chips/fxemu2b.asm (branches, TO rN / FROM rN in the b and c
 * table groups, ALT1/ALT2/ALT3) and the 523-strong group from
 * chips/fxemu2.asm (ADD/ADC/SUB/SBC/CMP/AND/BIC/OR/XOR/INC/DEC/MULT/UMULT,
 * register
 * and immediate).
 *
 * Run `make fxops` in this directory. Not part of `all`: the oracle is the
 * original assembly, pulled out of git by mkfxops.sh.
 *
 * Both sides run against the same random flags, registers, program counter and
 * instruction stream, and both dispatch the next opcode into the same recording
 * stub. The comparison covers everything a handler can touch: the resulting
 * program counter, the ALT-mode/opcode word, the source/destination pointers,
 * the whole SuperFX register file, the flag words, and what the nested dispatch
 * saw.
 */
#include "difftest.h"

typedef uint8_t u1;
typedef int8_t s1;
typedef uint32_t u4;
typedef int32_t s4;
typedef uint16_t u2;
typedef int16_t s2;

/* Shared SuperFX state the handlers read (normally chips/fxemu2.asm). */
u4 SfxCarry, SfxSignZero, SfxOverflow;
u4 SfxB, SfxCPB, SfxCROM, SfxRomBuffer, withr15sk;
u4 SfxRAMMem, SfxLastRamAdr;
u4 SfxCBR, SfxPBR, SfxCacheActive;
u4 SfxMemTable[256];

/* chips/fxemu2.asm's FlushCache is a stub; the CACHE opcode only has to reach
 * it, so a stub here matches. */
void FlushCache(void) { }
u4 SfxR0[16];
u4 FxTable[1024];

/* The assembly names R14 and R15 individually; they are just the last two
 * slots of the contiguous register file. */
__asm__(".globl SfxR4\n.set SfxR4, SfxR0+16\n"
        ".globl SfxR6\n.set SfxR6, SfxR0+24\n"
        ".globl SfxR11\n.set SfxR11, SfxR0+44\n"
        ".globl SfxR12\n.set SfxR12, SfxR0+48\n"
        ".globl SfxR13\n.set SfxR13, SfxR0+52\n"
        ".globl SfxR14\n.set SfxR14, SfxR0+56\n"
        ".globl SfxR15\n.set SfxR15, SfxR0+60\n");

/* ecx indexes (ALT mode << 8) | opcode, so the b and c tables need all four
 * ALT sub-tables' worth of entries laid out adjacently, as in endmem.c. */
u4 FxTableb[1024];
u4 FxTablec[1024];

/* The seam block (normally chips/c_fxemu2b.c). */
u1* FxSeamPC;
u4* FxSeamSrc;
u4* FxSeamDst;
u4 FxSeamCX;

/* What the nested dispatch saw. StubTable identifies which of the three tables
 * it came through, so choosing the wrong one is a visible mismatch. */
u4 StubPC, StubCX, StubSrc, StubDst, StubHits, StubTable, StubB;

void FxDispatch(u4 const* table); /* _fxops.o */
void asm_fxcall(void* fn); /* _fxops.o */
void fxstub(void), fxstubb(void), fxstubc(void); /* _fxops.o */

#include "../chips/fx_ops.h"

extern void asm_FxOpb05(void), asm_FxOpb06(void), asm_FxOpb07(void), asm_FxOpb08(void);
extern void asm_FxOpb09(void), asm_FxOpb0A(void), asm_FxOpb0B(void), asm_FxOpb0C(void);
extern void asm_FxOpb0D(void), asm_FxOpb0E(void), asm_FxOpb0F(void), asm_FxOpb10(void);
extern void asm_FxOpb11(void), asm_FxOpb12(void), asm_FxOpb13(void), asm_FxOpb14(void);
extern void asm_FxOpb15(void), asm_FxOpb16(void), asm_FxOpb17(void), asm_FxOpb18(void);
extern void asm_FxOpb19(void), asm_FxOpb1A(void), asm_FxOpb1B(void), asm_FxOpb1C(void);
extern void asm_FxOpb1D(void), asm_FxOpb1E(void), asm_FxOpb1F(void), asm_FxOpb3D(void);
extern void asm_FxOpb3E(void), asm_FxOpb3F(void), asm_FxOpbB0(void), asm_FxOpbB1(void);
extern void asm_FxOpbB2(void), asm_FxOpbB3(void), asm_FxOpbB4(void), asm_FxOpbB5(void);
extern void asm_FxOpbB6(void), asm_FxOpbB7(void), asm_FxOpbB8(void), asm_FxOpbB9(void);
extern void asm_FxOpbBA(void), asm_FxOpbBB(void), asm_FxOpbBC(void), asm_FxOpbBD(void);
extern void asm_FxOpbBE(void), asm_FxOpbBF(void), asm_FxOpc05(void), asm_FxOpc06(void);
extern void asm_FxOpc07(void), asm_FxOpc08(void), asm_FxOpc09(void), asm_FxOpc0A(void);
extern void asm_FxOpc0B(void), asm_FxOpc0C(void), asm_FxOpc0D(void), asm_FxOpc0E(void);
extern void asm_FxOpc0F(void), asm_FxOpc10(void), asm_FxOpc11(void), asm_FxOpc12(void);
extern void asm_FxOpc13(void), asm_FxOpc14(void), asm_FxOpc15(void), asm_FxOpc16(void);
extern void asm_FxOpc17(void), asm_FxOpc18(void), asm_FxOpc19(void), asm_FxOpc1A(void);
extern void asm_FxOpc1B(void), asm_FxOpc1C(void), asm_FxOpc1D(void), asm_FxOpc1E(void);
extern void asm_FxOpc1F(void), asm_FxOpc3D(void), asm_FxOpc3E(void), asm_FxOpc3F(void);
extern void asm_FxOpcB0(void), asm_FxOpcB1(void), asm_FxOpcB2(void), asm_FxOpcB3(void);
extern void asm_FxOpcB4(void), asm_FxOpcB5(void), asm_FxOpcB6(void), asm_FxOpcB7(void);
extern void asm_FxOpcB8(void), asm_FxOpcB9(void), asm_FxOpcBA(void), asm_FxOpcBB(void);
extern void asm_FxOpcBC(void), asm_FxOpcBD(void), asm_FxOpcBE(void), asm_FxOpcBF(void);
extern void asm_FxOp50(void), asm_FxOp51(void), asm_FxOp52(void), asm_FxOp53(void);
extern void asm_FxOp54(void), asm_FxOp55(void), asm_FxOp56(void), asm_FxOp57(void);
extern void asm_FxOp58(void), asm_FxOp59(void), asm_FxOp5A(void), asm_FxOp5B(void);
extern void asm_FxOp5C(void), asm_FxOp5D(void), asm_FxOp5E(void), asm_FxOp50A1(void);
extern void asm_FxOp51A1(void), asm_FxOp52A1(void), asm_FxOp53A1(void), asm_FxOp54A1(void);
extern void asm_FxOp55A1(void), asm_FxOp56A1(void), asm_FxOp57A1(void), asm_FxOp58A1(void);
extern void asm_FxOp59A1(void), asm_FxOp5AA1(void), asm_FxOp5BA1(void), asm_FxOp5CA1(void);
extern void asm_FxOp5DA1(void), asm_FxOp5EA1(void), asm_FxOp50A2(void), asm_FxOp51A2(void);
extern void asm_FxOp52A2(void), asm_FxOp53A2(void), asm_FxOp54A2(void), asm_FxOp55A2(void);
extern void asm_FxOp56A2(void), asm_FxOp57A2(void), asm_FxOp58A2(void), asm_FxOp59A2(void);
extern void asm_FxOp5AA2(void), asm_FxOp5BA2(void), asm_FxOp5CA2(void), asm_FxOp5DA2(void);
extern void asm_FxOp5EA2(void), asm_FxOp5FA2(void), asm_FxOp50A3(void), asm_FxOp51A3(void);
extern void asm_FxOp52A3(void), asm_FxOp53A3(void), asm_FxOp54A3(void), asm_FxOp55A3(void);
extern void asm_FxOp56A3(void), asm_FxOp57A3(void), asm_FxOp58A3(void), asm_FxOp59A3(void);
extern void asm_FxOp5AA3(void), asm_FxOp5BA3(void), asm_FxOp5CA3(void), asm_FxOp5DA3(void);
extern void asm_FxOp5EA3(void), asm_FxOp5FA3(void), asm_FxOp60(void), asm_FxOp61(void);
extern void asm_FxOp62(void), asm_FxOp63(void), asm_FxOp64(void), asm_FxOp65(void);
extern void asm_FxOp66(void), asm_FxOp67(void), asm_FxOp68(void), asm_FxOp69(void);
extern void asm_FxOp6A(void), asm_FxOp6B(void), asm_FxOp6C(void), asm_FxOp6D(void);
extern void asm_FxOp6E(void), asm_FxOp60A1(void), asm_FxOp61A1(void), asm_FxOp62A1(void);
extern void asm_FxOp63A1(void), asm_FxOp64A1(void), asm_FxOp65A1(void), asm_FxOp66A1(void);
extern void asm_FxOp67A1(void), asm_FxOp68A1(void), asm_FxOp69A1(void), asm_FxOp6AA1(void);
extern void asm_FxOp6BA1(void), asm_FxOp6CA1(void), asm_FxOp6DA1(void), asm_FxOp6EA1(void);
extern void asm_FxOp60A2(void), asm_FxOp61A2(void), asm_FxOp62A2(void), asm_FxOp63A2(void);
extern void asm_FxOp64A2(void), asm_FxOp65A2(void), asm_FxOp66A2(void), asm_FxOp67A2(void);
extern void asm_FxOp68A2(void), asm_FxOp69A2(void), asm_FxOp6AA2(void), asm_FxOp6BA2(void);
extern void asm_FxOp6CA2(void), asm_FxOp6DA2(void), asm_FxOp6EA2(void), asm_FxOp6FA2(void);
extern void asm_FxOp60A3(void), asm_FxOp61A3(void), asm_FxOp62A3(void), asm_FxOp63A3(void);
extern void asm_FxOp64A3(void), asm_FxOp65A3(void), asm_FxOp66A3(void), asm_FxOp67A3(void);
extern void asm_FxOp68A3(void), asm_FxOp69A3(void), asm_FxOp6AA3(void), asm_FxOp6BA3(void);
extern void asm_FxOp6CA3(void), asm_FxOp6DA3(void), asm_FxOp6EA3(void), asm_FxOp71(void);
extern void asm_FxOp72(void), asm_FxOp73(void), asm_FxOp74(void), asm_FxOp75(void);
extern void asm_FxOp76(void), asm_FxOp77(void), asm_FxOp78(void), asm_FxOp79(void);
extern void asm_FxOp7A(void), asm_FxOp7B(void), asm_FxOp7C(void), asm_FxOp7D(void);
extern void asm_FxOp7E(void), asm_FxOp71A1(void), asm_FxOp72A1(void), asm_FxOp73A1(void);
extern void asm_FxOp74A1(void), asm_FxOp75A1(void), asm_FxOp76A1(void), asm_FxOp77A1(void);
extern void asm_FxOp78A1(void), asm_FxOp79A1(void), asm_FxOp7AA1(void), asm_FxOp7BA1(void);
extern void asm_FxOp7CA1(void), asm_FxOp7DA1(void), asm_FxOp7EA1(void), asm_FxOp71A2(void);
extern void asm_FxOp72A2(void), asm_FxOp73A2(void), asm_FxOp74A2(void), asm_FxOp75A2(void);
extern void asm_FxOp76A2(void), asm_FxOp77A2(void), asm_FxOp78A2(void), asm_FxOp79A2(void);
extern void asm_FxOp7AA2(void), asm_FxOp7BA2(void), asm_FxOp7CA2(void), asm_FxOp7DA2(void);
extern void asm_FxOp7EA2(void), asm_FxOp7FA2(void), asm_FxOp71A3(void), asm_FxOp72A3(void);
extern void asm_FxOp73A3(void), asm_FxOp74A3(void), asm_FxOp75A3(void), asm_FxOp76A3(void);
extern void asm_FxOp77A3(void), asm_FxOp78A3(void), asm_FxOp79A3(void), asm_FxOp7AA3(void);
extern void asm_FxOp7BA3(void), asm_FxOp7CA3(void), asm_FxOp7DA3(void), asm_FxOp7EA3(void);
extern void asm_FxOp7FA3(void), asm_FxOpC1(void), asm_FxOpC2(void), asm_FxOpC3(void);
extern void asm_FxOpC4(void), asm_FxOpC5(void), asm_FxOpC6(void), asm_FxOpC7(void);
extern void asm_FxOpC8(void), asm_FxOpC9(void), asm_FxOpCA(void), asm_FxOpCB(void);
extern void asm_FxOpCC(void), asm_FxOpCD(void), asm_FxOpCE(void), asm_FxOpC1A1(void);
extern void asm_FxOpC2A1(void), asm_FxOpC3A1(void), asm_FxOpC4A1(void), asm_FxOpC5A1(void);
extern void asm_FxOpC6A1(void), asm_FxOpC7A1(void), asm_FxOpC8A1(void), asm_FxOpC9A1(void);
extern void asm_FxOpCAA1(void), asm_FxOpCBA1(void), asm_FxOpCCA1(void), asm_FxOpCDA1(void);
extern void asm_FxOpCEA1(void), asm_FxOpC1A2(void), asm_FxOpC2A2(void), asm_FxOpC3A2(void);
extern void asm_FxOpC4A2(void), asm_FxOpC5A2(void), asm_FxOpC6A2(void), asm_FxOpC7A2(void);
extern void asm_FxOpC8A2(void), asm_FxOpC9A2(void), asm_FxOpCAA2(void), asm_FxOpCBA2(void);
extern void asm_FxOpCCA2(void), asm_FxOpCDA2(void), asm_FxOpCEA2(void), asm_FxOpCFA2(void);
extern void asm_FxOpC1A3(void), asm_FxOpC2A3(void), asm_FxOpC3A3(void), asm_FxOpC4A3(void);
extern void asm_FxOpC5A3(void), asm_FxOpC6A3(void), asm_FxOpC7A3(void), asm_FxOpC8A3(void);
extern void asm_FxOpC9A3(void), asm_FxOpCAA3(void), asm_FxOpCBA3(void), asm_FxOpCCA3(void);
extern void asm_FxOpCDA3(void), asm_FxOpCEA3(void), asm_FxOpCFA3(void), asm_FxOpD0(void);
extern void asm_FxOpD1(void), asm_FxOpD2(void), asm_FxOpD3(void), asm_FxOpD4(void);
extern void asm_FxOpD5(void), asm_FxOpD6(void), asm_FxOpD7(void), asm_FxOpD8(void);
extern void asm_FxOpD9(void), asm_FxOpDA(void), asm_FxOpDB(void), asm_FxOpDC(void);
extern void asm_FxOpDD(void), asm_FxOpE0(void), asm_FxOpE1(void), asm_FxOpE2(void);
extern void asm_FxOpE3(void), asm_FxOpE4(void), asm_FxOpE5(void), asm_FxOpE6(void);
extern void asm_FxOpE7(void), asm_FxOpE8(void), asm_FxOpE9(void), asm_FxOpEA(void);
extern void asm_FxOpEB(void), asm_FxOpEC(void), asm_FxOpED(void), asm_FxOp80(void);
extern void asm_FxOp81(void), asm_FxOp82(void), asm_FxOp83(void), asm_FxOp84(void);
extern void asm_FxOp85(void), asm_FxOp86(void), asm_FxOp87(void), asm_FxOp88(void);
extern void asm_FxOp89(void), asm_FxOp8A(void), asm_FxOp8B(void), asm_FxOp8C(void);
extern void asm_FxOp8D(void), asm_FxOp8E(void), asm_FxOp80A1(void), asm_FxOp81A1(void);
extern void asm_FxOp82A1(void), asm_FxOp83A1(void), asm_FxOp84A1(void), asm_FxOp85A1(void);
extern void asm_FxOp86A1(void), asm_FxOp87A1(void), asm_FxOp88A1(void), asm_FxOp89A1(void);
extern void asm_FxOp8AA1(void), asm_FxOp8BA1(void), asm_FxOp8CA1(void), asm_FxOp8DA1(void);
extern void asm_FxOp8EA1(void), asm_FxOp80A2(void), asm_FxOp81A2(void), asm_FxOp82A2(void);
extern void asm_FxOp83A2(void), asm_FxOp84A2(void), asm_FxOp85A2(void), asm_FxOp86A2(void);
extern void asm_FxOp87A2(void), asm_FxOp88A2(void), asm_FxOp89A2(void), asm_FxOp8AA2(void);
extern void asm_FxOp8BA2(void), asm_FxOp8CA2(void), asm_FxOp8DA2(void), asm_FxOp8EA2(void);
extern void asm_FxOp8FA2(void), asm_FxOp80A3(void), asm_FxOp81A3(void), asm_FxOp82A3(void);
extern void asm_FxOp83A3(void), asm_FxOp84A3(void), asm_FxOp85A3(void), asm_FxOp86A3(void);
extern void asm_FxOp87A3(void), asm_FxOp88A3(void), asm_FxOp89A3(void), asm_FxOp8AA3(void);
extern void asm_FxOp8BA3(void), asm_FxOp8CA3(void), asm_FxOp8DA3(void), asm_FxOp8EA3(void);
extern void asm_FxOp8FA3(void), asm_FxOp10(void), asm_FxOp11(void), asm_FxOp12(void);
extern void asm_FxOp13(void), asm_FxOp14(void), asm_FxOp15(void), asm_FxOp16(void);
extern void asm_FxOp17(void), asm_FxOp18(void), asm_FxOp19(void), asm_FxOp1A(void);
extern void asm_FxOp1B(void), asm_FxOp1C(void), asm_FxOp1D(void), asm_FxOp20(void);
extern void asm_FxOp21(void), asm_FxOp22(void), asm_FxOp23(void), asm_FxOp24(void);
extern void asm_FxOp25(void), asm_FxOp26(void), asm_FxOp27(void), asm_FxOp28(void);
extern void asm_FxOp29(void), asm_FxOp2A(void), asm_FxOp2B(void), asm_FxOp2C(void);
extern void asm_FxOp2D(void), asm_FxOpB0(void), asm_FxOpB1(void), asm_FxOpB2(void);
extern void asm_FxOpB3(void), asm_FxOpB4(void), asm_FxOpB5(void), asm_FxOpB6(void);
extern void asm_FxOpB7(void), asm_FxOpB8(void), asm_FxOpB9(void), asm_FxOpBA(void);
extern void asm_FxOpBB(void), asm_FxOpBC(void), asm_FxOpBD(void), asm_FxOpBE(void);
extern void asm_FxOp30(void), asm_FxOp31(void), asm_FxOp32(void), asm_FxOp33(void);
extern void asm_FxOp34(void), asm_FxOp35(void), asm_FxOp36(void), asm_FxOp37(void);
extern void asm_FxOp38(void), asm_FxOp39(void), asm_FxOp3A(void), asm_FxOp3B(void);
extern void asm_FxOp30A1(void), asm_FxOp31A1(void), asm_FxOp32A1(void), asm_FxOp33A1(void);
extern void asm_FxOp34A1(void), asm_FxOp35A1(void), asm_FxOp36A1(void), asm_FxOp37A1(void);
extern void asm_FxOp38A1(void), asm_FxOp39A1(void), asm_FxOp3AA1(void), asm_FxOp3BA1(void);
extern void asm_FxOp40(void), asm_FxOp41(void), asm_FxOp42(void), asm_FxOp43(void);
extern void asm_FxOp44(void), asm_FxOp45(void), asm_FxOp46(void), asm_FxOp47(void);
extern void asm_FxOp48(void), asm_FxOp49(void), asm_FxOp4A(void), asm_FxOp4B(void);
extern void asm_FxOp40A1(void), asm_FxOp41A1(void), asm_FxOp42A1(void), asm_FxOp43A1(void);
extern void asm_FxOp44A1(void), asm_FxOp45A1(void), asm_FxOp46A1(void), asm_FxOp47A1(void);
extern void asm_FxOp48A1(void), asm_FxOp49A1(void), asm_FxOp4AA1(void), asm_FxOp4BA1(void);
extern void asm_FxOpA0(void), asm_FxOpA1(void), asm_FxOpA2(void), asm_FxOpA3(void);
extern void asm_FxOpA4(void), asm_FxOpA5(void), asm_FxOpA6(void), asm_FxOpA7(void);
extern void asm_FxOpA8(void), asm_FxOpA9(void), asm_FxOpAA(void), asm_FxOpAB(void);
extern void asm_FxOpAC(void), asm_FxOpAD(void), asm_FxOpA0A1(void), asm_FxOpA1A1(void);
extern void asm_FxOpA2A1(void), asm_FxOpA3A1(void), asm_FxOpA4A1(void), asm_FxOpA5A1(void);
extern void asm_FxOpA6A1(void), asm_FxOpA7A1(void), asm_FxOpA8A1(void), asm_FxOpA9A1(void);
extern void asm_FxOpAAA1(void), asm_FxOpABA1(void), asm_FxOpACA1(void), asm_FxOpADA1(void);
extern void asm_FxOpA0A2(void), asm_FxOpA1A2(void), asm_FxOpA2A2(void), asm_FxOpA3A2(void);
extern void asm_FxOpA4A2(void), asm_FxOpA5A2(void), asm_FxOpA6A2(void), asm_FxOpA7A2(void);
extern void asm_FxOpA8A2(void), asm_FxOpA9A2(void), asm_FxOpAAA2(void), asm_FxOpABA2(void);
extern void asm_FxOpACA2(void), asm_FxOpADA2(void), asm_FxOpAEA2(void), asm_FxOpF0(void);
extern void asm_FxOpF1(void), asm_FxOpF2(void), asm_FxOpF3(void), asm_FxOpF4(void);
extern void asm_FxOpF5(void), asm_FxOpF6(void), asm_FxOpF7(void), asm_FxOpF8(void);
extern void asm_FxOpF9(void), asm_FxOpFA(void), asm_FxOpFB(void), asm_FxOpFC(void);
extern void asm_FxOpFD(void), asm_FxOpF0A1(void), asm_FxOpF1A1(void), asm_FxOpF2A1(void);
extern void asm_FxOpF3A1(void), asm_FxOpF4A1(void), asm_FxOpF5A1(void), asm_FxOpF6A1(void);
extern void asm_FxOpF7A1(void), asm_FxOpF8A1(void), asm_FxOpF9A1(void), asm_FxOpFAA1(void);
extern void asm_FxOpFBA1(void), asm_FxOpFCA1(void), asm_FxOpFDA1(void), asm_FxOpF0A2(void);
extern void asm_FxOpF1A2(void), asm_FxOpF2A2(void), asm_FxOpF3A2(void), asm_FxOpF4A2(void);
extern void asm_FxOpF5A2(void), asm_FxOpF6A2(void), asm_FxOpF7A2(void), asm_FxOpF8A2(void);
extern void asm_FxOpF9A2(void), asm_FxOpFAA2(void), asm_FxOpFBA2(void), asm_FxOpFCA2(void);
extern void asm_FxOpFDA2(void), asm_FxOpFEA2(void), asm_FxOp91(void), asm_FxOp92(void);
extern void asm_FxOp93(void), asm_FxOp94(void), asm_FxOp98(void), asm_FxOp99(void);
extern void asm_FxOp9A(void), asm_FxOp9B(void), asm_FxOp9C(void), asm_FxOp9D(void);
extern void asm_FxOp98A1(void), asm_FxOp99A1(void), asm_FxOp9AA1(void), asm_FxOp9BA1(void);
extern void asm_FxOp9CA1(void), asm_FxOp9DA1(void), asm_FxOp02(void);
extern void asm_FxOp01(void), asm_FxOp4D(void), asm_FxOp4F(void), asm_FxOp95(void), asm_FxOp96(void), asm_FxOp96A1(void), asm_FxOp97(void), asm_FxOp9E(void), asm_FxOpC0(void);
extern void asm_FxOp03(void), asm_FxOp04(void), asm_FxOp3C(void), asm_FxOp9F(void), asm_FxOp9FA1(void), asm_FxOpAE(void), asm_FxOpAF(void), asm_FxOpDE(void), asm_FxOpEE(void);

typedef struct {
    char const* name;
    void (*asm_fn)(void);
    void (*c_fn)(void);
    /* Where the handler gets an address, so the difftest can keep it in
       bounds: 0 = neither, 1 = RAM via register addr_reg, 2 = RAM via a 16-bit
       immediate at the program counter, 3 = RAM via an 8-bit immediate scaled
       by two (always in range), 4 = new program counter from addr_reg,
       5 = new program counter from the source register via SfxMemTable. */
    int mem;
    u4 addr_reg;
} fxcase;

static fxcase const cases[] = {
    { "FxOpb05 BRA", asm_FxOpb05, c_FxOpb05, 0, 0 },
    { "FxOpb06 BGE", asm_FxOpb06, c_FxOpb06, 0, 0 },
    { "FxOpb07 BLT", asm_FxOpb07, c_FxOpb07, 0, 0 },
    { "FxOpb08 BNE", asm_FxOpb08, c_FxOpb08, 0, 0 },
    { "FxOpb09 BEQ", asm_FxOpb09, c_FxOpb09, 0, 0 },
    { "FxOpb0A BPL", asm_FxOpb0A, c_FxOpb0A, 0, 0 },
    { "FxOpb0B BMI", asm_FxOpb0B, c_FxOpb0B, 0, 0 },
    { "FxOpb0C BCC", asm_FxOpb0C, c_FxOpb0C, 0, 0 },
    { "FxOpb0D BCS", asm_FxOpb0D, c_FxOpb0D, 0, 0 },
    { "FxOpb0E BVC", asm_FxOpb0E, c_FxOpb0E, 0, 0 },
    { "FxOpb0F BVS", asm_FxOpb0F, c_FxOpb0F, 0, 0 },
    { "FxOpb10 TO r0", asm_FxOpb10, c_FxOpb10, 0, 0 },
    { "FxOpb11 TO r1", asm_FxOpb11, c_FxOpb11, 0, 0 },
    { "FxOpb12 TO r2", asm_FxOpb12, c_FxOpb12, 0, 0 },
    { "FxOpb13 TO r3", asm_FxOpb13, c_FxOpb13, 0, 0 },
    { "FxOpb14 TO r4", asm_FxOpb14, c_FxOpb14, 0, 0 },
    { "FxOpb15 TO r5", asm_FxOpb15, c_FxOpb15, 0, 0 },
    { "FxOpb16 TO r6", asm_FxOpb16, c_FxOpb16, 0, 0 },
    { "FxOpb17 TO r7", asm_FxOpb17, c_FxOpb17, 0, 0 },
    { "FxOpb18 TO r8", asm_FxOpb18, c_FxOpb18, 0, 0 },
    { "FxOpb19 TO r9", asm_FxOpb19, c_FxOpb19, 0, 0 },
    { "FxOpb1A TO r10", asm_FxOpb1A, c_FxOpb1A, 0, 0 },
    { "FxOpb1B TO r11", asm_FxOpb1B, c_FxOpb1B, 0, 0 },
    { "FxOpb1C TO r12", asm_FxOpb1C, c_FxOpb1C, 0, 0 },
    { "FxOpb1D TO r13", asm_FxOpb1D, c_FxOpb1D, 0, 0 },
    { "FxOpb1E TO r14", asm_FxOpb1E, c_FxOpb1E, 0, 0 },
    { "FxOpb1F TO r15", asm_FxOpb1F, c_FxOpb1F, 0, 0 },
    { "FxOpb3D ALT1", asm_FxOpb3D, c_FxOpb3D, 0, 0 },
    { "FxOpb3E ALT2", asm_FxOpb3E, c_FxOpb3E, 0, 0 },
    { "FxOpb3F ALT3", asm_FxOpb3F, c_FxOpb3F, 0, 0 },
    { "FxOpbB0 FROM r0", asm_FxOpbB0, c_FxOpbB0, 0, 0 },
    { "FxOpbB1 FROM r1", asm_FxOpbB1, c_FxOpbB1, 0, 0 },
    { "FxOpbB2 FROM r2", asm_FxOpbB2, c_FxOpbB2, 0, 0 },
    { "FxOpbB3 FROM r3", asm_FxOpbB3, c_FxOpbB3, 0, 0 },
    { "FxOpbB4 FROM r4", asm_FxOpbB4, c_FxOpbB4, 0, 0 },
    { "FxOpbB5 FROM r5", asm_FxOpbB5, c_FxOpbB5, 0, 0 },
    { "FxOpbB6 FROM r6", asm_FxOpbB6, c_FxOpbB6, 0, 0 },
    { "FxOpbB7 FROM r7", asm_FxOpbB7, c_FxOpbB7, 0, 0 },
    { "FxOpbB8 FROM r8", asm_FxOpbB8, c_FxOpbB8, 0, 0 },
    { "FxOpbB9 FROM r9", asm_FxOpbB9, c_FxOpbB9, 0, 0 },
    { "FxOpbBA FROM r10", asm_FxOpbBA, c_FxOpbBA, 0, 0 },
    { "FxOpbBB FROM r11", asm_FxOpbBB, c_FxOpbBB, 0, 0 },
    { "FxOpbBC FROM r12", asm_FxOpbBC, c_FxOpbBC, 0, 0 },
    { "FxOpbBD FROM r13", asm_FxOpbBD, c_FxOpbBD, 0, 0 },
    { "FxOpbBE FROM r14", asm_FxOpbBE, c_FxOpbBE, 0, 0 },
    { "FxOpbBF FROM r15", asm_FxOpbBF, c_FxOpbBF, 0, 0 },
    { "FxOpc05 BRA", asm_FxOpc05, c_FxOpc05, 0, 0 },
    { "FxOpc06 BGE", asm_FxOpc06, c_FxOpc06, 0, 0 },
    { "FxOpc07 BLT", asm_FxOpc07, c_FxOpc07, 0, 0 },
    { "FxOpc08 BNE", asm_FxOpc08, c_FxOpc08, 0, 0 },
    { "FxOpc09 BEQ", asm_FxOpc09, c_FxOpc09, 0, 0 },
    { "FxOpc0A BPL", asm_FxOpc0A, c_FxOpc0A, 0, 0 },
    { "FxOpc0B BMI", asm_FxOpc0B, c_FxOpc0B, 0, 0 },
    { "FxOpc0C BCC", asm_FxOpc0C, c_FxOpc0C, 0, 0 },
    { "FxOpc0D BCS", asm_FxOpc0D, c_FxOpc0D, 0, 0 },
    { "FxOpc0E BVC", asm_FxOpc0E, c_FxOpc0E, 0, 0 },
    { "FxOpc0F BVS", asm_FxOpc0F, c_FxOpc0F, 0, 0 },
    { "FxOpc10 TO r0", asm_FxOpc10, c_FxOpc10, 0, 0 },
    { "FxOpc11 TO r1", asm_FxOpc11, c_FxOpc11, 0, 0 },
    { "FxOpc12 TO r2", asm_FxOpc12, c_FxOpc12, 0, 0 },
    { "FxOpc13 TO r3", asm_FxOpc13, c_FxOpc13, 0, 0 },
    { "FxOpc14 TO r4", asm_FxOpc14, c_FxOpc14, 0, 0 },
    { "FxOpc15 TO r5", asm_FxOpc15, c_FxOpc15, 0, 0 },
    { "FxOpc16 TO r6", asm_FxOpc16, c_FxOpc16, 0, 0 },
    { "FxOpc17 TO r7", asm_FxOpc17, c_FxOpc17, 0, 0 },
    { "FxOpc18 TO r8", asm_FxOpc18, c_FxOpc18, 0, 0 },
    { "FxOpc19 TO r9", asm_FxOpc19, c_FxOpc19, 0, 0 },
    { "FxOpc1A TO r10", asm_FxOpc1A, c_FxOpc1A, 0, 0 },
    { "FxOpc1B TO r11", asm_FxOpc1B, c_FxOpc1B, 0, 0 },
    { "FxOpc1C TO r12", asm_FxOpc1C, c_FxOpc1C, 0, 0 },
    { "FxOpc1D TO r13", asm_FxOpc1D, c_FxOpc1D, 0, 0 },
    { "FxOpc1E TO r14", asm_FxOpc1E, c_FxOpc1E, 0, 0 },
    { "FxOpc1F TO r15", asm_FxOpc1F, c_FxOpc1F, 0, 0 },
    { "FxOpc3D ALT1", asm_FxOpc3D, c_FxOpc3D, 0, 0 },
    { "FxOpc3E ALT2", asm_FxOpc3E, c_FxOpc3E, 0, 0 },
    { "FxOpc3F ALT3", asm_FxOpc3F, c_FxOpc3F, 0, 0 },
    { "FxOpcB0 FROM r0", asm_FxOpcB0, c_FxOpcB0, 0, 0 },
    { "FxOpcB1 FROM r1", asm_FxOpcB1, c_FxOpcB1, 0, 0 },
    { "FxOpcB2 FROM r2", asm_FxOpcB2, c_FxOpcB2, 0, 0 },
    { "FxOpcB3 FROM r3", asm_FxOpcB3, c_FxOpcB3, 0, 0 },
    { "FxOpcB4 FROM r4", asm_FxOpcB4, c_FxOpcB4, 0, 0 },
    { "FxOpcB5 FROM r5", asm_FxOpcB5, c_FxOpcB5, 0, 0 },
    { "FxOpcB6 FROM r6", asm_FxOpcB6, c_FxOpcB6, 0, 0 },
    { "FxOpcB7 FROM r7", asm_FxOpcB7, c_FxOpcB7, 0, 0 },
    { "FxOpcB8 FROM r8", asm_FxOpcB8, c_FxOpcB8, 0, 0 },
    { "FxOpcB9 FROM r9", asm_FxOpcB9, c_FxOpcB9, 0, 0 },
    { "FxOpcBA FROM r10", asm_FxOpcBA, c_FxOpcBA, 0, 0 },
    { "FxOpcBB FROM r11", asm_FxOpcBB, c_FxOpcBB, 0, 0 },
    { "FxOpcBC FROM r12", asm_FxOpcBC, c_FxOpcBC, 0, 0 },
    { "FxOpcBD FROM r13", asm_FxOpcBD, c_FxOpcBD, 0, 0 },
    { "FxOpcBE FROM r14", asm_FxOpcBE, c_FxOpcBE, 0, 0 },
    { "FxOpcBF FROM r15", asm_FxOpcBF, c_FxOpcBF, 0, 0 },
    { "FxOp50 ADD r0", asm_FxOp50, c_FxOp50, 0, 0 },
    { "FxOp51 ADD r1", asm_FxOp51, c_FxOp51, 0, 0 },
    { "FxOp52 ADD r2", asm_FxOp52, c_FxOp52, 0, 0 },
    { "FxOp53 ADD r3", asm_FxOp53, c_FxOp53, 0, 0 },
    { "FxOp54 ADD r4", asm_FxOp54, c_FxOp54, 0, 0 },
    { "FxOp55 ADD r5", asm_FxOp55, c_FxOp55, 0, 0 },
    { "FxOp56 ADD r6", asm_FxOp56, c_FxOp56, 0, 0 },
    { "FxOp57 ADD r7", asm_FxOp57, c_FxOp57, 0, 0 },
    { "FxOp58 ADD r8", asm_FxOp58, c_FxOp58, 0, 0 },
    { "FxOp59 ADD r9", asm_FxOp59, c_FxOp59, 0, 0 },
    { "FxOp5A ADD r10", asm_FxOp5A, c_FxOp5A, 0, 0 },
    { "FxOp5B ADD r11", asm_FxOp5B, c_FxOp5B, 0, 0 },
    { "FxOp5C ADD r12", asm_FxOp5C, c_FxOp5C, 0, 0 },
    { "FxOp5D ADD r13", asm_FxOp5D, c_FxOp5D, 0, 0 },
    { "FxOp5E ADD r14", asm_FxOp5E, c_FxOp5E, 0, 0 },
    { "FxOp50A1 ADC r0", asm_FxOp50A1, c_FxOp50A1, 0, 0 },
    { "FxOp51A1 ADC r1", asm_FxOp51A1, c_FxOp51A1, 0, 0 },
    { "FxOp52A1 ADC r2", asm_FxOp52A1, c_FxOp52A1, 0, 0 },
    { "FxOp53A1 ADC r3", asm_FxOp53A1, c_FxOp53A1, 0, 0 },
    { "FxOp54A1 ADC r4", asm_FxOp54A1, c_FxOp54A1, 0, 0 },
    { "FxOp55A1 ADC r5", asm_FxOp55A1, c_FxOp55A1, 0, 0 },
    { "FxOp56A1 ADC r6", asm_FxOp56A1, c_FxOp56A1, 0, 0 },
    { "FxOp57A1 ADC r7", asm_FxOp57A1, c_FxOp57A1, 0, 0 },
    { "FxOp58A1 ADC r8", asm_FxOp58A1, c_FxOp58A1, 0, 0 },
    { "FxOp59A1 ADC r9", asm_FxOp59A1, c_FxOp59A1, 0, 0 },
    { "FxOp5AA1 ADC r10", asm_FxOp5AA1, c_FxOp5AA1, 0, 0 },
    { "FxOp5BA1 ADC r11", asm_FxOp5BA1, c_FxOp5BA1, 0, 0 },
    { "FxOp5CA1 ADC r12", asm_FxOp5CA1, c_FxOp5CA1, 0, 0 },
    { "FxOp5DA1 ADC r13", asm_FxOp5DA1, c_FxOp5DA1, 0, 0 },
    { "FxOp5EA1 ADC r14", asm_FxOp5EA1, c_FxOp5EA1, 0, 0 },
    { "FxOp50A2 ADD #0", asm_FxOp50A2, c_FxOp50A2, 0, 0 },
    { "FxOp51A2 ADD #1", asm_FxOp51A2, c_FxOp51A2, 0, 0 },
    { "FxOp52A2 ADD #2", asm_FxOp52A2, c_FxOp52A2, 0, 0 },
    { "FxOp53A2 ADD #3", asm_FxOp53A2, c_FxOp53A2, 0, 0 },
    { "FxOp54A2 ADD #4", asm_FxOp54A2, c_FxOp54A2, 0, 0 },
    { "FxOp55A2 ADD #5", asm_FxOp55A2, c_FxOp55A2, 0, 0 },
    { "FxOp56A2 ADD #6", asm_FxOp56A2, c_FxOp56A2, 0, 0 },
    { "FxOp57A2 ADD #7", asm_FxOp57A2, c_FxOp57A2, 0, 0 },
    { "FxOp58A2 ADD #8", asm_FxOp58A2, c_FxOp58A2, 0, 0 },
    { "FxOp59A2 ADD #9", asm_FxOp59A2, c_FxOp59A2, 0, 0 },
    { "FxOp5AA2 ADD #10", asm_FxOp5AA2, c_FxOp5AA2, 0, 0 },
    { "FxOp5BA2 ADD #11", asm_FxOp5BA2, c_FxOp5BA2, 0, 0 },
    { "FxOp5CA2 ADD #12", asm_FxOp5CA2, c_FxOp5CA2, 0, 0 },
    { "FxOp5DA2 ADD #13", asm_FxOp5DA2, c_FxOp5DA2, 0, 0 },
    { "FxOp5EA2 ADD #14", asm_FxOp5EA2, c_FxOp5EA2, 0, 0 },
    { "FxOp5FA2 ADD #15", asm_FxOp5FA2, c_FxOp5FA2, 0, 0 },
    { "FxOp50A3 ADC #0", asm_FxOp50A3, c_FxOp50A3, 0, 0 },
    { "FxOp51A3 ADC #1", asm_FxOp51A3, c_FxOp51A3, 0, 0 },
    { "FxOp52A3 ADC #2", asm_FxOp52A3, c_FxOp52A3, 0, 0 },
    { "FxOp53A3 ADC #3", asm_FxOp53A3, c_FxOp53A3, 0, 0 },
    { "FxOp54A3 ADC #4", asm_FxOp54A3, c_FxOp54A3, 0, 0 },
    { "FxOp55A3 ADC #5", asm_FxOp55A3, c_FxOp55A3, 0, 0 },
    { "FxOp56A3 ADC #6", asm_FxOp56A3, c_FxOp56A3, 0, 0 },
    { "FxOp57A3 ADC #7", asm_FxOp57A3, c_FxOp57A3, 0, 0 },
    { "FxOp58A3 ADC #8", asm_FxOp58A3, c_FxOp58A3, 0, 0 },
    { "FxOp59A3 ADC #9", asm_FxOp59A3, c_FxOp59A3, 0, 0 },
    { "FxOp5AA3 ADC #10", asm_FxOp5AA3, c_FxOp5AA3, 0, 0 },
    { "FxOp5BA3 ADC #11", asm_FxOp5BA3, c_FxOp5BA3, 0, 0 },
    { "FxOp5CA3 ADC #12", asm_FxOp5CA3, c_FxOp5CA3, 0, 0 },
    { "FxOp5DA3 ADC #13", asm_FxOp5DA3, c_FxOp5DA3, 0, 0 },
    { "FxOp5EA3 ADC #14", asm_FxOp5EA3, c_FxOp5EA3, 0, 0 },
    { "FxOp5FA3 ADC #15", asm_FxOp5FA3, c_FxOp5FA3, 0, 0 },
    { "FxOp60 SUB r0", asm_FxOp60, c_FxOp60, 0, 0 },
    { "FxOp61 SUB r1", asm_FxOp61, c_FxOp61, 0, 0 },
    { "FxOp62 SUB r2", asm_FxOp62, c_FxOp62, 0, 0 },
    { "FxOp63 SUB r3", asm_FxOp63, c_FxOp63, 0, 0 },
    { "FxOp64 SUB r4", asm_FxOp64, c_FxOp64, 0, 0 },
    { "FxOp65 SUB r5", asm_FxOp65, c_FxOp65, 0, 0 },
    { "FxOp66 SUB r6", asm_FxOp66, c_FxOp66, 0, 0 },
    { "FxOp67 SUB r7", asm_FxOp67, c_FxOp67, 0, 0 },
    { "FxOp68 SUB r8", asm_FxOp68, c_FxOp68, 0, 0 },
    { "FxOp69 SUB r9", asm_FxOp69, c_FxOp69, 0, 0 },
    { "FxOp6A SUB r10", asm_FxOp6A, c_FxOp6A, 0, 0 },
    { "FxOp6B SUB r11", asm_FxOp6B, c_FxOp6B, 0, 0 },
    { "FxOp6C SUB r12", asm_FxOp6C, c_FxOp6C, 0, 0 },
    { "FxOp6D SUB r13", asm_FxOp6D, c_FxOp6D, 0, 0 },
    { "FxOp6E SUB r14", asm_FxOp6E, c_FxOp6E, 0, 0 },
    { "FxOp60A1 SBC r0", asm_FxOp60A1, c_FxOp60A1, 0, 0 },
    { "FxOp61A1 SBC r1", asm_FxOp61A1, c_FxOp61A1, 0, 0 },
    { "FxOp62A1 SBC r2", asm_FxOp62A1, c_FxOp62A1, 0, 0 },
    { "FxOp63A1 SBC r3", asm_FxOp63A1, c_FxOp63A1, 0, 0 },
    { "FxOp64A1 SBC r4", asm_FxOp64A1, c_FxOp64A1, 0, 0 },
    { "FxOp65A1 SBC r5", asm_FxOp65A1, c_FxOp65A1, 0, 0 },
    { "FxOp66A1 SBC r6", asm_FxOp66A1, c_FxOp66A1, 0, 0 },
    { "FxOp67A1 SBC r7", asm_FxOp67A1, c_FxOp67A1, 0, 0 },
    { "FxOp68A1 SBC r8", asm_FxOp68A1, c_FxOp68A1, 0, 0 },
    { "FxOp69A1 SBC r9", asm_FxOp69A1, c_FxOp69A1, 0, 0 },
    { "FxOp6AA1 SBC r10", asm_FxOp6AA1, c_FxOp6AA1, 0, 0 },
    { "FxOp6BA1 SBC r11", asm_FxOp6BA1, c_FxOp6BA1, 0, 0 },
    { "FxOp6CA1 SBC r12", asm_FxOp6CA1, c_FxOp6CA1, 0, 0 },
    { "FxOp6DA1 SBC r13", asm_FxOp6DA1, c_FxOp6DA1, 0, 0 },
    { "FxOp6EA1 SBC r14", asm_FxOp6EA1, c_FxOp6EA1, 0, 0 },
    { "FxOp60A2 SUB #0", asm_FxOp60A2, c_FxOp60A2, 0, 0 },
    { "FxOp61A2 SUB #1", asm_FxOp61A2, c_FxOp61A2, 0, 0 },
    { "FxOp62A2 SUB #2", asm_FxOp62A2, c_FxOp62A2, 0, 0 },
    { "FxOp63A2 SUB #3", asm_FxOp63A2, c_FxOp63A2, 0, 0 },
    { "FxOp64A2 SUB #4", asm_FxOp64A2, c_FxOp64A2, 0, 0 },
    { "FxOp65A2 SUB #5", asm_FxOp65A2, c_FxOp65A2, 0, 0 },
    { "FxOp66A2 SUB #6", asm_FxOp66A2, c_FxOp66A2, 0, 0 },
    { "FxOp67A2 SUB #7", asm_FxOp67A2, c_FxOp67A2, 0, 0 },
    { "FxOp68A2 SUB #8", asm_FxOp68A2, c_FxOp68A2, 0, 0 },
    { "FxOp69A2 SUB #9", asm_FxOp69A2, c_FxOp69A2, 0, 0 },
    { "FxOp6AA2 SUB #10", asm_FxOp6AA2, c_FxOp6AA2, 0, 0 },
    { "FxOp6BA2 SUB #11", asm_FxOp6BA2, c_FxOp6BA2, 0, 0 },
    { "FxOp6CA2 SUB #12", asm_FxOp6CA2, c_FxOp6CA2, 0, 0 },
    { "FxOp6DA2 SUB #13", asm_FxOp6DA2, c_FxOp6DA2, 0, 0 },
    { "FxOp6EA2 SUB #14", asm_FxOp6EA2, c_FxOp6EA2, 0, 0 },
    { "FxOp6FA2 SUB #15", asm_FxOp6FA2, c_FxOp6FA2, 0, 0 },
    { "FxOp60A3 CMP r0", asm_FxOp60A3, c_FxOp60A3, 0, 0 },
    { "FxOp61A3 CMP r1", asm_FxOp61A3, c_FxOp61A3, 0, 0 },
    { "FxOp62A3 CMP r2", asm_FxOp62A3, c_FxOp62A3, 0, 0 },
    { "FxOp63A3 CMP r3", asm_FxOp63A3, c_FxOp63A3, 0, 0 },
    { "FxOp64A3 CMP r4", asm_FxOp64A3, c_FxOp64A3, 0, 0 },
    { "FxOp65A3 CMP r5", asm_FxOp65A3, c_FxOp65A3, 0, 0 },
    { "FxOp66A3 CMP r6", asm_FxOp66A3, c_FxOp66A3, 0, 0 },
    { "FxOp67A3 CMP r7", asm_FxOp67A3, c_FxOp67A3, 0, 0 },
    { "FxOp68A3 CMP r8", asm_FxOp68A3, c_FxOp68A3, 0, 0 },
    { "FxOp69A3 CMP r9", asm_FxOp69A3, c_FxOp69A3, 0, 0 },
    { "FxOp6AA3 CMP r10", asm_FxOp6AA3, c_FxOp6AA3, 0, 0 },
    { "FxOp6BA3 CMP r11", asm_FxOp6BA3, c_FxOp6BA3, 0, 0 },
    { "FxOp6CA3 CMP r12", asm_FxOp6CA3, c_FxOp6CA3, 0, 0 },
    { "FxOp6DA3 CMP r13", asm_FxOp6DA3, c_FxOp6DA3, 0, 0 },
    { "FxOp6EA3 CMP r14", asm_FxOp6EA3, c_FxOp6EA3, 0, 0 },
    { "FxOp71 AND r1", asm_FxOp71, c_FxOp71, 0, 0 },
    { "FxOp72 AND r2", asm_FxOp72, c_FxOp72, 0, 0 },
    { "FxOp73 AND r3", asm_FxOp73, c_FxOp73, 0, 0 },
    { "FxOp74 AND r4", asm_FxOp74, c_FxOp74, 0, 0 },
    { "FxOp75 AND r5", asm_FxOp75, c_FxOp75, 0, 0 },
    { "FxOp76 AND r6", asm_FxOp76, c_FxOp76, 0, 0 },
    { "FxOp77 AND r7", asm_FxOp77, c_FxOp77, 0, 0 },
    { "FxOp78 AND r8", asm_FxOp78, c_FxOp78, 0, 0 },
    { "FxOp79 AND r9", asm_FxOp79, c_FxOp79, 0, 0 },
    { "FxOp7A AND r10", asm_FxOp7A, c_FxOp7A, 0, 0 },
    { "FxOp7B AND r11", asm_FxOp7B, c_FxOp7B, 0, 0 },
    { "FxOp7C AND r12", asm_FxOp7C, c_FxOp7C, 0, 0 },
    { "FxOp7D AND r13", asm_FxOp7D, c_FxOp7D, 0, 0 },
    { "FxOp7E AND r14", asm_FxOp7E, c_FxOp7E, 0, 0 },
    { "FxOp71A1 BIC r1", asm_FxOp71A1, c_FxOp71A1, 0, 0 },
    { "FxOp72A1 BIC r2", asm_FxOp72A1, c_FxOp72A1, 0, 0 },
    { "FxOp73A1 BIC r3", asm_FxOp73A1, c_FxOp73A1, 0, 0 },
    { "FxOp74A1 BIC r4", asm_FxOp74A1, c_FxOp74A1, 0, 0 },
    { "FxOp75A1 BIC r5", asm_FxOp75A1, c_FxOp75A1, 0, 0 },
    { "FxOp76A1 BIC r6", asm_FxOp76A1, c_FxOp76A1, 0, 0 },
    { "FxOp77A1 BIC r7", asm_FxOp77A1, c_FxOp77A1, 0, 0 },
    { "FxOp78A1 BIC r8", asm_FxOp78A1, c_FxOp78A1, 0, 0 },
    { "FxOp79A1 BIC r9", asm_FxOp79A1, c_FxOp79A1, 0, 0 },
    { "FxOp7AA1 BIC r10", asm_FxOp7AA1, c_FxOp7AA1, 0, 0 },
    { "FxOp7BA1 BIC r11", asm_FxOp7BA1, c_FxOp7BA1, 0, 0 },
    { "FxOp7CA1 BIC r12", asm_FxOp7CA1, c_FxOp7CA1, 0, 0 },
    { "FxOp7DA1 BIC r13", asm_FxOp7DA1, c_FxOp7DA1, 0, 0 },
    { "FxOp7EA1 BIC r14", asm_FxOp7EA1, c_FxOp7EA1, 0, 0 },
    { "FxOp71A2 AND #1", asm_FxOp71A2, c_FxOp71A2, 0, 0 },
    { "FxOp72A2 AND #2", asm_FxOp72A2, c_FxOp72A2, 0, 0 },
    { "FxOp73A2 AND #3", asm_FxOp73A2, c_FxOp73A2, 0, 0 },
    { "FxOp74A2 AND #4", asm_FxOp74A2, c_FxOp74A2, 0, 0 },
    { "FxOp75A2 AND #5", asm_FxOp75A2, c_FxOp75A2, 0, 0 },
    { "FxOp76A2 AND #6", asm_FxOp76A2, c_FxOp76A2, 0, 0 },
    { "FxOp77A2 AND #7", asm_FxOp77A2, c_FxOp77A2, 0, 0 },
    { "FxOp78A2 AND #8", asm_FxOp78A2, c_FxOp78A2, 0, 0 },
    { "FxOp79A2 AND #9", asm_FxOp79A2, c_FxOp79A2, 0, 0 },
    { "FxOp7AA2 AND #10", asm_FxOp7AA2, c_FxOp7AA2, 0, 0 },
    { "FxOp7BA2 AND #11", asm_FxOp7BA2, c_FxOp7BA2, 0, 0 },
    { "FxOp7CA2 AND #12", asm_FxOp7CA2, c_FxOp7CA2, 0, 0 },
    { "FxOp7DA2 AND #13", asm_FxOp7DA2, c_FxOp7DA2, 0, 0 },
    { "FxOp7EA2 AND #14", asm_FxOp7EA2, c_FxOp7EA2, 0, 0 },
    { "FxOp7FA2 AND #15", asm_FxOp7FA2, c_FxOp7FA2, 0, 0 },
    { "FxOp71A3 BIC #1", asm_FxOp71A3, c_FxOp71A3, 0, 0 },
    { "FxOp72A3 BIC #2", asm_FxOp72A3, c_FxOp72A3, 0, 0 },
    { "FxOp73A3 BIC #3", asm_FxOp73A3, c_FxOp73A3, 0, 0 },
    { "FxOp74A3 BIC #4", asm_FxOp74A3, c_FxOp74A3, 0, 0 },
    { "FxOp75A3 BIC #5", asm_FxOp75A3, c_FxOp75A3, 0, 0 },
    { "FxOp76A3 BIC #6", asm_FxOp76A3, c_FxOp76A3, 0, 0 },
    { "FxOp77A3 BIC #7", asm_FxOp77A3, c_FxOp77A3, 0, 0 },
    { "FxOp78A3 BIC #8", asm_FxOp78A3, c_FxOp78A3, 0, 0 },
    { "FxOp79A3 BIC #9", asm_FxOp79A3, c_FxOp79A3, 0, 0 },
    { "FxOp7AA3 BIC #10", asm_FxOp7AA3, c_FxOp7AA3, 0, 0 },
    { "FxOp7BA3 BIC #11", asm_FxOp7BA3, c_FxOp7BA3, 0, 0 },
    { "FxOp7CA3 BIC #12", asm_FxOp7CA3, c_FxOp7CA3, 0, 0 },
    { "FxOp7DA3 BIC #13", asm_FxOp7DA3, c_FxOp7DA3, 0, 0 },
    { "FxOp7EA3 BIC #14", asm_FxOp7EA3, c_FxOp7EA3, 0, 0 },
    { "FxOp7FA3 BIC #15", asm_FxOp7FA3, c_FxOp7FA3, 0, 0 },
    { "FxOpC1 OR r1", asm_FxOpC1, c_FxOpC1, 0, 0 },
    { "FxOpC2 OR r2", asm_FxOpC2, c_FxOpC2, 0, 0 },
    { "FxOpC3 OR r3", asm_FxOpC3, c_FxOpC3, 0, 0 },
    { "FxOpC4 OR r4", asm_FxOpC4, c_FxOpC4, 0, 0 },
    { "FxOpC5 OR r5", asm_FxOpC5, c_FxOpC5, 0, 0 },
    { "FxOpC6 OR r6", asm_FxOpC6, c_FxOpC6, 0, 0 },
    { "FxOpC7 OR r7", asm_FxOpC7, c_FxOpC7, 0, 0 },
    { "FxOpC8 OR r8", asm_FxOpC8, c_FxOpC8, 0, 0 },
    { "FxOpC9 OR r9", asm_FxOpC9, c_FxOpC9, 0, 0 },
    { "FxOpCA OR r10", asm_FxOpCA, c_FxOpCA, 0, 0 },
    { "FxOpCB OR r11", asm_FxOpCB, c_FxOpCB, 0, 0 },
    { "FxOpCC OR r12", asm_FxOpCC, c_FxOpCC, 0, 0 },
    { "FxOpCD OR r13", asm_FxOpCD, c_FxOpCD, 0, 0 },
    { "FxOpCE OR r14", asm_FxOpCE, c_FxOpCE, 0, 0 },
    { "FxOpC1A1 XOR r1", asm_FxOpC1A1, c_FxOpC1A1, 0, 0 },
    { "FxOpC2A1 XOR r2", asm_FxOpC2A1, c_FxOpC2A1, 0, 0 },
    { "FxOpC3A1 XOR r3", asm_FxOpC3A1, c_FxOpC3A1, 0, 0 },
    { "FxOpC4A1 XOR r4", asm_FxOpC4A1, c_FxOpC4A1, 0, 0 },
    { "FxOpC5A1 XOR r5", asm_FxOpC5A1, c_FxOpC5A1, 0, 0 },
    { "FxOpC6A1 XOR r6", asm_FxOpC6A1, c_FxOpC6A1, 0, 0 },
    { "FxOpC7A1 XOR r7", asm_FxOpC7A1, c_FxOpC7A1, 0, 0 },
    { "FxOpC8A1 XOR r8", asm_FxOpC8A1, c_FxOpC8A1, 0, 0 },
    { "FxOpC9A1 XOR r9", asm_FxOpC9A1, c_FxOpC9A1, 0, 0 },
    { "FxOpCAA1 XOR r10", asm_FxOpCAA1, c_FxOpCAA1, 0, 0 },
    { "FxOpCBA1 XOR r11", asm_FxOpCBA1, c_FxOpCBA1, 0, 0 },
    { "FxOpCCA1 XOR r12", asm_FxOpCCA1, c_FxOpCCA1, 0, 0 },
    { "FxOpCDA1 XOR r13", asm_FxOpCDA1, c_FxOpCDA1, 0, 0 },
    { "FxOpCEA1 XOR r14", asm_FxOpCEA1, c_FxOpCEA1, 0, 0 },
    { "FxOpC1A2 OR #1", asm_FxOpC1A2, c_FxOpC1A2, 0, 0 },
    { "FxOpC2A2 OR #2", asm_FxOpC2A2, c_FxOpC2A2, 0, 0 },
    { "FxOpC3A2 OR #3", asm_FxOpC3A2, c_FxOpC3A2, 0, 0 },
    { "FxOpC4A2 OR #4", asm_FxOpC4A2, c_FxOpC4A2, 0, 0 },
    { "FxOpC5A2 OR #5", asm_FxOpC5A2, c_FxOpC5A2, 0, 0 },
    { "FxOpC6A2 OR #6", asm_FxOpC6A2, c_FxOpC6A2, 0, 0 },
    { "FxOpC7A2 OR #7", asm_FxOpC7A2, c_FxOpC7A2, 0, 0 },
    { "FxOpC8A2 OR #8", asm_FxOpC8A2, c_FxOpC8A2, 0, 0 },
    { "FxOpC9A2 OR #9", asm_FxOpC9A2, c_FxOpC9A2, 0, 0 },
    { "FxOpCAA2 OR #10", asm_FxOpCAA2, c_FxOpCAA2, 0, 0 },
    { "FxOpCBA2 OR #11", asm_FxOpCBA2, c_FxOpCBA2, 0, 0 },
    { "FxOpCCA2 OR #12", asm_FxOpCCA2, c_FxOpCCA2, 0, 0 },
    { "FxOpCDA2 OR #13", asm_FxOpCDA2, c_FxOpCDA2, 0, 0 },
    { "FxOpCEA2 OR #14", asm_FxOpCEA2, c_FxOpCEA2, 0, 0 },
    { "FxOpCFA2 OR #15", asm_FxOpCFA2, c_FxOpCFA2, 0, 0 },
    { "FxOpC1A3 XOR #1", asm_FxOpC1A3, c_FxOpC1A3, 0, 0 },
    { "FxOpC2A3 XOR #2", asm_FxOpC2A3, c_FxOpC2A3, 0, 0 },
    { "FxOpC3A3 XOR #3", asm_FxOpC3A3, c_FxOpC3A3, 0, 0 },
    { "FxOpC4A3 XOR #4", asm_FxOpC4A3, c_FxOpC4A3, 0, 0 },
    { "FxOpC5A3 XOR #5", asm_FxOpC5A3, c_FxOpC5A3, 0, 0 },
    { "FxOpC6A3 XOR #6", asm_FxOpC6A3, c_FxOpC6A3, 0, 0 },
    { "FxOpC7A3 XOR #7", asm_FxOpC7A3, c_FxOpC7A3, 0, 0 },
    { "FxOpC8A3 XOR #8", asm_FxOpC8A3, c_FxOpC8A3, 0, 0 },
    { "FxOpC9A3 XOR #9", asm_FxOpC9A3, c_FxOpC9A3, 0, 0 },
    { "FxOpCAA3 XOR #10", asm_FxOpCAA3, c_FxOpCAA3, 0, 0 },
    { "FxOpCBA3 XOR #11", asm_FxOpCBA3, c_FxOpCBA3, 0, 0 },
    { "FxOpCCA3 XOR #12", asm_FxOpCCA3, c_FxOpCCA3, 0, 0 },
    { "FxOpCDA3 XOR #13", asm_FxOpCDA3, c_FxOpCDA3, 0, 0 },
    { "FxOpCEA3 XOR #14", asm_FxOpCEA3, c_FxOpCEA3, 0, 0 },
    { "FxOpCFA3 XOR #15", asm_FxOpCFA3, c_FxOpCFA3, 0, 0 },
    { "FxOpD0 INC r0", asm_FxOpD0, c_FxOpD0, 0, 0 },
    { "FxOpD1 INC r1", asm_FxOpD1, c_FxOpD1, 0, 0 },
    { "FxOpD2 INC r2", asm_FxOpD2, c_FxOpD2, 0, 0 },
    { "FxOpD3 INC r3", asm_FxOpD3, c_FxOpD3, 0, 0 },
    { "FxOpD4 INC r4", asm_FxOpD4, c_FxOpD4, 0, 0 },
    { "FxOpD5 INC r5", asm_FxOpD5, c_FxOpD5, 0, 0 },
    { "FxOpD6 INC r6", asm_FxOpD6, c_FxOpD6, 0, 0 },
    { "FxOpD7 INC r7", asm_FxOpD7, c_FxOpD7, 0, 0 },
    { "FxOpD8 INC r8", asm_FxOpD8, c_FxOpD8, 0, 0 },
    { "FxOpD9 INC r9", asm_FxOpD9, c_FxOpD9, 0, 0 },
    { "FxOpDA INC r10", asm_FxOpDA, c_FxOpDA, 0, 0 },
    { "FxOpDB INC r11", asm_FxOpDB, c_FxOpDB, 0, 0 },
    { "FxOpDC INC r12", asm_FxOpDC, c_FxOpDC, 0, 0 },
    { "FxOpDD INC r13", asm_FxOpDD, c_FxOpDD, 0, 0 },
    { "FxOpE0 DEC r0", asm_FxOpE0, c_FxOpE0, 0, 0 },
    { "FxOpE1 DEC r1", asm_FxOpE1, c_FxOpE1, 0, 0 },
    { "FxOpE2 DEC r2", asm_FxOpE2, c_FxOpE2, 0, 0 },
    { "FxOpE3 DEC r3", asm_FxOpE3, c_FxOpE3, 0, 0 },
    { "FxOpE4 DEC r4", asm_FxOpE4, c_FxOpE4, 0, 0 },
    { "FxOpE5 DEC r5", asm_FxOpE5, c_FxOpE5, 0, 0 },
    { "FxOpE6 DEC r6", asm_FxOpE6, c_FxOpE6, 0, 0 },
    { "FxOpE7 DEC r7", asm_FxOpE7, c_FxOpE7, 0, 0 },
    { "FxOpE8 DEC r8", asm_FxOpE8, c_FxOpE8, 0, 0 },
    { "FxOpE9 DEC r9", asm_FxOpE9, c_FxOpE9, 0, 0 },
    { "FxOpEA DEC r10", asm_FxOpEA, c_FxOpEA, 0, 0 },
    { "FxOpEB DEC r11", asm_FxOpEB, c_FxOpEB, 0, 0 },
    { "FxOpEC DEC r12", asm_FxOpEC, c_FxOpEC, 0, 0 },
    { "FxOpED DEC r13", asm_FxOpED, c_FxOpED, 0, 0 },
    { "FxOp80 MULT r0", asm_FxOp80, c_FxOp80, 0, 0 },
    { "FxOp81 MULT r1", asm_FxOp81, c_FxOp81, 0, 0 },
    { "FxOp82 MULT r2", asm_FxOp82, c_FxOp82, 0, 0 },
    { "FxOp83 MULT r3", asm_FxOp83, c_FxOp83, 0, 0 },
    { "FxOp84 MULT r4", asm_FxOp84, c_FxOp84, 0, 0 },
    { "FxOp85 MULT r5", asm_FxOp85, c_FxOp85, 0, 0 },
    { "FxOp86 MULT r6", asm_FxOp86, c_FxOp86, 0, 0 },
    { "FxOp87 MULT r7", asm_FxOp87, c_FxOp87, 0, 0 },
    { "FxOp88 MULT r8", asm_FxOp88, c_FxOp88, 0, 0 },
    { "FxOp89 MULT r9", asm_FxOp89, c_FxOp89, 0, 0 },
    { "FxOp8A MULT r10", asm_FxOp8A, c_FxOp8A, 0, 0 },
    { "FxOp8B MULT r11", asm_FxOp8B, c_FxOp8B, 0, 0 },
    { "FxOp8C MULT r12", asm_FxOp8C, c_FxOp8C, 0, 0 },
    { "FxOp8D MULT r13", asm_FxOp8D, c_FxOp8D, 0, 0 },
    { "FxOp8E MULT r14", asm_FxOp8E, c_FxOp8E, 0, 0 },
    { "FxOp80A1 UMULT r0", asm_FxOp80A1, c_FxOp80A1, 0, 0 },
    { "FxOp81A1 UMULT r1", asm_FxOp81A1, c_FxOp81A1, 0, 0 },
    { "FxOp82A1 UMULT r2", asm_FxOp82A1, c_FxOp82A1, 0, 0 },
    { "FxOp83A1 UMULT r3", asm_FxOp83A1, c_FxOp83A1, 0, 0 },
    { "FxOp84A1 UMULT r4", asm_FxOp84A1, c_FxOp84A1, 0, 0 },
    { "FxOp85A1 UMULT r5", asm_FxOp85A1, c_FxOp85A1, 0, 0 },
    { "FxOp86A1 UMULT r6", asm_FxOp86A1, c_FxOp86A1, 0, 0 },
    { "FxOp87A1 UMULT r7", asm_FxOp87A1, c_FxOp87A1, 0, 0 },
    { "FxOp88A1 UMULT r8", asm_FxOp88A1, c_FxOp88A1, 0, 0 },
    { "FxOp89A1 UMULT r9", asm_FxOp89A1, c_FxOp89A1, 0, 0 },
    { "FxOp8AA1 UMULT r10", asm_FxOp8AA1, c_FxOp8AA1, 0, 0 },
    { "FxOp8BA1 UMULT r11", asm_FxOp8BA1, c_FxOp8BA1, 0, 0 },
    { "FxOp8CA1 UMULT r12", asm_FxOp8CA1, c_FxOp8CA1, 0, 0 },
    { "FxOp8DA1 UMULT r13", asm_FxOp8DA1, c_FxOp8DA1, 0, 0 },
    { "FxOp8EA1 UMULT r14", asm_FxOp8EA1, c_FxOp8EA1, 0, 0 },
    { "FxOp80A2 MULT #0", asm_FxOp80A2, c_FxOp80A2, 0, 0 },
    { "FxOp81A2 MULT #1", asm_FxOp81A2, c_FxOp81A2, 0, 0 },
    { "FxOp82A2 MULT #2", asm_FxOp82A2, c_FxOp82A2, 0, 0 },
    { "FxOp83A2 MULT #3", asm_FxOp83A2, c_FxOp83A2, 0, 0 },
    { "FxOp84A2 MULT #4", asm_FxOp84A2, c_FxOp84A2, 0, 0 },
    { "FxOp85A2 MULT #5", asm_FxOp85A2, c_FxOp85A2, 0, 0 },
    { "FxOp86A2 MULT #6", asm_FxOp86A2, c_FxOp86A2, 0, 0 },
    { "FxOp87A2 MULT #7", asm_FxOp87A2, c_FxOp87A2, 0, 0 },
    { "FxOp88A2 MULT #8", asm_FxOp88A2, c_FxOp88A2, 0, 0 },
    { "FxOp89A2 MULT #9", asm_FxOp89A2, c_FxOp89A2, 0, 0 },
    { "FxOp8AA2 MULT #10", asm_FxOp8AA2, c_FxOp8AA2, 0, 0 },
    { "FxOp8BA2 MULT #11", asm_FxOp8BA2, c_FxOp8BA2, 0, 0 },
    { "FxOp8CA2 MULT #12", asm_FxOp8CA2, c_FxOp8CA2, 0, 0 },
    { "FxOp8DA2 MULT #13", asm_FxOp8DA2, c_FxOp8DA2, 0, 0 },
    { "FxOp8EA2 MULT #14", asm_FxOp8EA2, c_FxOp8EA2, 0, 0 },
    { "FxOp8FA2 MULT #15", asm_FxOp8FA2, c_FxOp8FA2, 0, 0 },
    { "FxOp80A3 UMULT #0", asm_FxOp80A3, c_FxOp80A3, 0, 0 },
    { "FxOp81A3 UMULT #1", asm_FxOp81A3, c_FxOp81A3, 0, 0 },
    { "FxOp82A3 UMULT #2", asm_FxOp82A3, c_FxOp82A3, 0, 0 },
    { "FxOp83A3 UMULT #3", asm_FxOp83A3, c_FxOp83A3, 0, 0 },
    { "FxOp84A3 UMULT #4", asm_FxOp84A3, c_FxOp84A3, 0, 0 },
    { "FxOp85A3 UMULT #5", asm_FxOp85A3, c_FxOp85A3, 0, 0 },
    { "FxOp86A3 UMULT #6", asm_FxOp86A3, c_FxOp86A3, 0, 0 },
    { "FxOp87A3 UMULT #7", asm_FxOp87A3, c_FxOp87A3, 0, 0 },
    { "FxOp88A3 UMULT #8", asm_FxOp88A3, c_FxOp88A3, 0, 0 },
    { "FxOp89A3 UMULT #9", asm_FxOp89A3, c_FxOp89A3, 0, 0 },
    { "FxOp8AA3 UMULT #10", asm_FxOp8AA3, c_FxOp8AA3, 0, 0 },
    { "FxOp8BA3 UMULT #11", asm_FxOp8BA3, c_FxOp8BA3, 0, 0 },
    { "FxOp8CA3 UMULT #12", asm_FxOp8CA3, c_FxOp8CA3, 0, 0 },
    { "FxOp8DA3 UMULT #13", asm_FxOp8DA3, c_FxOp8DA3, 0, 0 },
    { "FxOp8EA3 UMULT #14", asm_FxOp8EA3, c_FxOp8EA3, 0, 0 },
    { "FxOp8FA3 UMULT #15", asm_FxOp8FA3, c_FxOp8FA3, 0, 0 },
    { "FxOp10 TO r0", asm_FxOp10, c_FxOp10, 0, 0 },
    { "FxOp11 TO r1", asm_FxOp11, c_FxOp11, 0, 0 },
    { "FxOp12 TO r2", asm_FxOp12, c_FxOp12, 0, 0 },
    { "FxOp13 TO r3", asm_FxOp13, c_FxOp13, 0, 0 },
    { "FxOp14 TO r4", asm_FxOp14, c_FxOp14, 0, 0 },
    { "FxOp15 TO r5", asm_FxOp15, c_FxOp15, 0, 0 },
    { "FxOp16 TO r6", asm_FxOp16, c_FxOp16, 0, 0 },
    { "FxOp17 TO r7", asm_FxOp17, c_FxOp17, 0, 0 },
    { "FxOp18 TO r8", asm_FxOp18, c_FxOp18, 0, 0 },
    { "FxOp19 TO r9", asm_FxOp19, c_FxOp19, 0, 0 },
    { "FxOp1A TO r10", asm_FxOp1A, c_FxOp1A, 0, 0 },
    { "FxOp1B TO r11", asm_FxOp1B, c_FxOp1B, 0, 0 },
    { "FxOp1C TO r12", asm_FxOp1C, c_FxOp1C, 0, 0 },
    { "FxOp1D TO r13", asm_FxOp1D, c_FxOp1D, 0, 0 },
    { "FxOp20 WITH r0", asm_FxOp20, c_FxOp20, 0, 0 },
    { "FxOp21 WITH r1", asm_FxOp21, c_FxOp21, 0, 0 },
    { "FxOp22 WITH r2", asm_FxOp22, c_FxOp22, 0, 0 },
    { "FxOp23 WITH r3", asm_FxOp23, c_FxOp23, 0, 0 },
    { "FxOp24 WITH r4", asm_FxOp24, c_FxOp24, 0, 0 },
    { "FxOp25 WITH r5", asm_FxOp25, c_FxOp25, 0, 0 },
    { "FxOp26 WITH r6", asm_FxOp26, c_FxOp26, 0, 0 },
    { "FxOp27 WITH r7", asm_FxOp27, c_FxOp27, 0, 0 },
    { "FxOp28 WITH r8", asm_FxOp28, c_FxOp28, 0, 0 },
    { "FxOp29 WITH r9", asm_FxOp29, c_FxOp29, 0, 0 },
    { "FxOp2A WITH r10", asm_FxOp2A, c_FxOp2A, 0, 0 },
    { "FxOp2B WITH r11", asm_FxOp2B, c_FxOp2B, 0, 0 },
    { "FxOp2C WITH r12", asm_FxOp2C, c_FxOp2C, 0, 0 },
    { "FxOp2D WITH r13", asm_FxOp2D, c_FxOp2D, 0, 0 },
    { "FxOpB0 FROM r0", asm_FxOpB0, c_FxOpB0, 0, 0 },
    { "FxOpB1 FROM r1", asm_FxOpB1, c_FxOpB1, 0, 0 },
    { "FxOpB2 FROM r2", asm_FxOpB2, c_FxOpB2, 0, 0 },
    { "FxOpB3 FROM r3", asm_FxOpB3, c_FxOpB3, 0, 0 },
    { "FxOpB4 FROM r4", asm_FxOpB4, c_FxOpB4, 0, 0 },
    { "FxOpB5 FROM r5", asm_FxOpB5, c_FxOpB5, 0, 0 },
    { "FxOpB6 FROM r6", asm_FxOpB6, c_FxOpB6, 0, 0 },
    { "FxOpB7 FROM r7", asm_FxOpB7, c_FxOpB7, 0, 0 },
    { "FxOpB8 FROM r8", asm_FxOpB8, c_FxOpB8, 0, 0 },
    { "FxOpB9 FROM r9", asm_FxOpB9, c_FxOpB9, 0, 0 },
    { "FxOpBA FROM r10", asm_FxOpBA, c_FxOpBA, 0, 0 },
    { "FxOpBB FROM r11", asm_FxOpBB, c_FxOpBB, 0, 0 },
    { "FxOpBC FROM r12", asm_FxOpBC, c_FxOpBC, 0, 0 },
    { "FxOpBD FROM r13", asm_FxOpBD, c_FxOpBD, 0, 0 },
    { "FxOpBE FROM r14", asm_FxOpBE, c_FxOpBE, 0, 0 },
    { "FxOp30 STW r0", asm_FxOp30, c_FxOp30, 1, 0 },
    { "FxOp31 STW r1", asm_FxOp31, c_FxOp31, 1, 1 },
    { "FxOp32 STW r2", asm_FxOp32, c_FxOp32, 1, 2 },
    { "FxOp33 STW r3", asm_FxOp33, c_FxOp33, 1, 3 },
    { "FxOp34 STW r4", asm_FxOp34, c_FxOp34, 1, 4 },
    { "FxOp35 STW r5", asm_FxOp35, c_FxOp35, 1, 5 },
    { "FxOp36 STW r6", asm_FxOp36, c_FxOp36, 1, 6 },
    { "FxOp37 STW r7", asm_FxOp37, c_FxOp37, 1, 7 },
    { "FxOp38 STW r8", asm_FxOp38, c_FxOp38, 1, 8 },
    { "FxOp39 STW r9", asm_FxOp39, c_FxOp39, 1, 9 },
    { "FxOp3A STW r10", asm_FxOp3A, c_FxOp3A, 1, 10 },
    { "FxOp3B STW r11", asm_FxOp3B, c_FxOp3B, 1, 11 },
    { "FxOp30A1 STB r0", asm_FxOp30A1, c_FxOp30A1, 1, 0 },
    { "FxOp31A1 STB r1", asm_FxOp31A1, c_FxOp31A1, 1, 1 },
    { "FxOp32A1 STB r2", asm_FxOp32A1, c_FxOp32A1, 1, 2 },
    { "FxOp33A1 STB r3", asm_FxOp33A1, c_FxOp33A1, 1, 3 },
    { "FxOp34A1 STB r4", asm_FxOp34A1, c_FxOp34A1, 1, 4 },
    { "FxOp35A1 STB r5", asm_FxOp35A1, c_FxOp35A1, 1, 5 },
    { "FxOp36A1 STB r6", asm_FxOp36A1, c_FxOp36A1, 1, 6 },
    { "FxOp37A1 STB r7", asm_FxOp37A1, c_FxOp37A1, 1, 7 },
    { "FxOp38A1 STB r8", asm_FxOp38A1, c_FxOp38A1, 1, 8 },
    { "FxOp39A1 STB r9", asm_FxOp39A1, c_FxOp39A1, 1, 9 },
    { "FxOp3AA1 STB r10", asm_FxOp3AA1, c_FxOp3AA1, 1, 10 },
    { "FxOp3BA1 STB r11", asm_FxOp3BA1, c_FxOp3BA1, 1, 11 },
    { "FxOp40 LDW r0", asm_FxOp40, c_FxOp40, 1, 0 },
    { "FxOp41 LDW r1", asm_FxOp41, c_FxOp41, 1, 1 },
    { "FxOp42 LDW r2", asm_FxOp42, c_FxOp42, 1, 2 },
    { "FxOp43 LDW r3", asm_FxOp43, c_FxOp43, 1, 3 },
    { "FxOp44 LDW r4", asm_FxOp44, c_FxOp44, 1, 4 },
    { "FxOp45 LDW r5", asm_FxOp45, c_FxOp45, 1, 5 },
    { "FxOp46 LDW r6", asm_FxOp46, c_FxOp46, 1, 6 },
    { "FxOp47 LDW r7", asm_FxOp47, c_FxOp47, 1, 7 },
    { "FxOp48 LDW r8", asm_FxOp48, c_FxOp48, 1, 8 },
    { "FxOp49 LDW r9", asm_FxOp49, c_FxOp49, 1, 9 },
    { "FxOp4A LDW r10", asm_FxOp4A, c_FxOp4A, 1, 10 },
    { "FxOp4B LDW r11", asm_FxOp4B, c_FxOp4B, 1, 11 },
    { "FxOp40A1 LDB r0", asm_FxOp40A1, c_FxOp40A1, 1, 0 },
    { "FxOp41A1 LDB r1", asm_FxOp41A1, c_FxOp41A1, 1, 1 },
    { "FxOp42A1 LDB r2", asm_FxOp42A1, c_FxOp42A1, 1, 2 },
    { "FxOp43A1 LDB r3", asm_FxOp43A1, c_FxOp43A1, 1, 3 },
    { "FxOp44A1 LDB r4", asm_FxOp44A1, c_FxOp44A1, 1, 4 },
    { "FxOp45A1 LDB r5", asm_FxOp45A1, c_FxOp45A1, 1, 5 },
    { "FxOp46A1 LDB r6", asm_FxOp46A1, c_FxOp46A1, 1, 6 },
    { "FxOp47A1 LDB r7", asm_FxOp47A1, c_FxOp47A1, 1, 7 },
    { "FxOp48A1 LDB r8", asm_FxOp48A1, c_FxOp48A1, 1, 8 },
    { "FxOp49A1 LDB r9", asm_FxOp49A1, c_FxOp49A1, 1, 9 },
    { "FxOp4AA1 LDB r10", asm_FxOp4AA1, c_FxOp4AA1, 1, 10 },
    { "FxOp4BA1 LDB r11", asm_FxOp4BA1, c_FxOp4BA1, 1, 11 },
    { "FxOpA0 IBT r0", asm_FxOpA0, c_FxOpA0, 0, 0 },
    { "FxOpA1 IBT r1", asm_FxOpA1, c_FxOpA1, 0, 0 },
    { "FxOpA2 IBT r2", asm_FxOpA2, c_FxOpA2, 0, 0 },
    { "FxOpA3 IBT r3", asm_FxOpA3, c_FxOpA3, 0, 0 },
    { "FxOpA4 IBT r4", asm_FxOpA4, c_FxOpA4, 0, 0 },
    { "FxOpA5 IBT r5", asm_FxOpA5, c_FxOpA5, 0, 0 },
    { "FxOpA6 IBT r6", asm_FxOpA6, c_FxOpA6, 0, 0 },
    { "FxOpA7 IBT r7", asm_FxOpA7, c_FxOpA7, 0, 0 },
    { "FxOpA8 IBT r8", asm_FxOpA8, c_FxOpA8, 0, 0 },
    { "FxOpA9 IBT r9", asm_FxOpA9, c_FxOpA9, 0, 0 },
    { "FxOpAA IBT r10", asm_FxOpAA, c_FxOpAA, 0, 0 },
    { "FxOpAB IBT r11", asm_FxOpAB, c_FxOpAB, 0, 0 },
    { "FxOpAC IBT r12", asm_FxOpAC, c_FxOpAC, 0, 0 },
    { "FxOpAD IBT r13", asm_FxOpAD, c_FxOpAD, 0, 0 },
    { "FxOpA0A1 LMS r0", asm_FxOpA0A1, c_FxOpA0A1, 3, 0 },
    { "FxOpA1A1 LMS r1", asm_FxOpA1A1, c_FxOpA1A1, 3, 0 },
    { "FxOpA2A1 LMS r2", asm_FxOpA2A1, c_FxOpA2A1, 3, 0 },
    { "FxOpA3A1 LMS r3", asm_FxOpA3A1, c_FxOpA3A1, 3, 0 },
    { "FxOpA4A1 LMS r4", asm_FxOpA4A1, c_FxOpA4A1, 3, 0 },
    { "FxOpA5A1 LMS r5", asm_FxOpA5A1, c_FxOpA5A1, 3, 0 },
    { "FxOpA6A1 LMS r6", asm_FxOpA6A1, c_FxOpA6A1, 3, 0 },
    { "FxOpA7A1 LMS r7", asm_FxOpA7A1, c_FxOpA7A1, 3, 0 },
    { "FxOpA8A1 LMS r8", asm_FxOpA8A1, c_FxOpA8A1, 3, 0 },
    { "FxOpA9A1 LMS r9", asm_FxOpA9A1, c_FxOpA9A1, 3, 0 },
    { "FxOpAAA1 LMS r10", asm_FxOpAAA1, c_FxOpAAA1, 3, 0 },
    { "FxOpABA1 LMS r11", asm_FxOpABA1, c_FxOpABA1, 3, 0 },
    { "FxOpACA1 LMS r12", asm_FxOpACA1, c_FxOpACA1, 3, 0 },
    { "FxOpADA1 LMS r13", asm_FxOpADA1, c_FxOpADA1, 3, 0 },
    { "FxOpA0A2 SMS r0", asm_FxOpA0A2, c_FxOpA0A2, 3, 0 },
    { "FxOpA1A2 SMS r1", asm_FxOpA1A2, c_FxOpA1A2, 3, 0 },
    { "FxOpA2A2 SMS r2", asm_FxOpA2A2, c_FxOpA2A2, 3, 0 },
    { "FxOpA3A2 SMS r3", asm_FxOpA3A2, c_FxOpA3A2, 3, 0 },
    { "FxOpA4A2 SMS r4", asm_FxOpA4A2, c_FxOpA4A2, 3, 0 },
    { "FxOpA5A2 SMS r5", asm_FxOpA5A2, c_FxOpA5A2, 3, 0 },
    { "FxOpA6A2 SMS r6", asm_FxOpA6A2, c_FxOpA6A2, 3, 0 },
    { "FxOpA7A2 SMS r7", asm_FxOpA7A2, c_FxOpA7A2, 3, 0 },
    { "FxOpA8A2 SMS r8", asm_FxOpA8A2, c_FxOpA8A2, 3, 0 },
    { "FxOpA9A2 SMS r9", asm_FxOpA9A2, c_FxOpA9A2, 3, 0 },
    { "FxOpAAA2 SMS r10", asm_FxOpAAA2, c_FxOpAAA2, 3, 0 },
    { "FxOpABA2 SMS r11", asm_FxOpABA2, c_FxOpABA2, 3, 0 },
    { "FxOpACA2 SMS r12", asm_FxOpACA2, c_FxOpACA2, 3, 0 },
    { "FxOpADA2 SMS r13", asm_FxOpADA2, c_FxOpADA2, 3, 0 },
    { "FxOpAEA2 SMS r14", asm_FxOpAEA2, c_FxOpAEA2, 3, 0 },
    { "FxOpF0 IWT r0", asm_FxOpF0, c_FxOpF0, 0, 0 },
    { "FxOpF1 IWT r1", asm_FxOpF1, c_FxOpF1, 0, 0 },
    { "FxOpF2 IWT r2", asm_FxOpF2, c_FxOpF2, 0, 0 },
    { "FxOpF3 IWT r3", asm_FxOpF3, c_FxOpF3, 0, 0 },
    { "FxOpF4 IWT r4", asm_FxOpF4, c_FxOpF4, 0, 0 },
    { "FxOpF5 IWT r5", asm_FxOpF5, c_FxOpF5, 0, 0 },
    { "FxOpF6 IWT r6", asm_FxOpF6, c_FxOpF6, 0, 0 },
    { "FxOpF7 IWT r7", asm_FxOpF7, c_FxOpF7, 0, 0 },
    { "FxOpF8 IWT r8", asm_FxOpF8, c_FxOpF8, 0, 0 },
    { "FxOpF9 IWT r9", asm_FxOpF9, c_FxOpF9, 0, 0 },
    { "FxOpFA IWT r10", asm_FxOpFA, c_FxOpFA, 0, 0 },
    { "FxOpFB IWT r11", asm_FxOpFB, c_FxOpFB, 0, 0 },
    { "FxOpFC IWT r12", asm_FxOpFC, c_FxOpFC, 0, 0 },
    { "FxOpFD IWT r13", asm_FxOpFD, c_FxOpFD, 0, 0 },
    { "FxOpF0A1 LM r0", asm_FxOpF0A1, c_FxOpF0A1, 2, 0 },
    { "FxOpF1A1 LM r1", asm_FxOpF1A1, c_FxOpF1A1, 2, 0 },
    { "FxOpF2A1 LM r2", asm_FxOpF2A1, c_FxOpF2A1, 2, 0 },
    { "FxOpF3A1 LM r3", asm_FxOpF3A1, c_FxOpF3A1, 2, 0 },
    { "FxOpF4A1 LM r4", asm_FxOpF4A1, c_FxOpF4A1, 2, 0 },
    { "FxOpF5A1 LM r5", asm_FxOpF5A1, c_FxOpF5A1, 2, 0 },
    { "FxOpF6A1 LM r6", asm_FxOpF6A1, c_FxOpF6A1, 2, 0 },
    { "FxOpF7A1 LM r7", asm_FxOpF7A1, c_FxOpF7A1, 2, 0 },
    { "FxOpF8A1 LM r8", asm_FxOpF8A1, c_FxOpF8A1, 2, 0 },
    { "FxOpF9A1 LM r9", asm_FxOpF9A1, c_FxOpF9A1, 2, 0 },
    { "FxOpFAA1 LM r10", asm_FxOpFAA1, c_FxOpFAA1, 2, 0 },
    { "FxOpFBA1 LM r11", asm_FxOpFBA1, c_FxOpFBA1, 2, 0 },
    { "FxOpFCA1 LM r12", asm_FxOpFCA1, c_FxOpFCA1, 2, 0 },
    { "FxOpFDA1 LM r13", asm_FxOpFDA1, c_FxOpFDA1, 2, 0 },
    { "FxOpF0A2 SM r0", asm_FxOpF0A2, c_FxOpF0A2, 2, 0 },
    { "FxOpF1A2 SM r1", asm_FxOpF1A2, c_FxOpF1A2, 2, 0 },
    { "FxOpF2A2 SM r2", asm_FxOpF2A2, c_FxOpF2A2, 2, 0 },
    { "FxOpF3A2 SM r3", asm_FxOpF3A2, c_FxOpF3A2, 2, 0 },
    { "FxOpF4A2 SM r4", asm_FxOpF4A2, c_FxOpF4A2, 2, 0 },
    { "FxOpF5A2 SM r5", asm_FxOpF5A2, c_FxOpF5A2, 2, 0 },
    { "FxOpF6A2 SM r6", asm_FxOpF6A2, c_FxOpF6A2, 2, 0 },
    { "FxOpF7A2 SM r7", asm_FxOpF7A2, c_FxOpF7A2, 2, 0 },
    { "FxOpF8A2 SM r8", asm_FxOpF8A2, c_FxOpF8A2, 2, 0 },
    { "FxOpF9A2 SM r9", asm_FxOpF9A2, c_FxOpF9A2, 2, 0 },
    { "FxOpFAA2 SM r10", asm_FxOpFAA2, c_FxOpFAA2, 2, 0 },
    { "FxOpFBA2 SM r11", asm_FxOpFBA2, c_FxOpFBA2, 2, 0 },
    { "FxOpFCA2 SM r12", asm_FxOpFCA2, c_FxOpFCA2, 2, 0 },
    { "FxOpFDA2 SM r13", asm_FxOpFDA2, c_FxOpFDA2, 2, 0 },
    { "FxOpFEA2 SM r14", asm_FxOpFEA2, c_FxOpFEA2, 2, 0 },
    { "FxOp91 LINK #1", asm_FxOp91, c_FxOp91, 0, 1 },
    { "FxOp92 LINK #2", asm_FxOp92, c_FxOp92, 0, 2 },
    { "FxOp93 LINK #3", asm_FxOp93, c_FxOp93, 0, 3 },
    { "FxOp94 LINK #4", asm_FxOp94, c_FxOp94, 0, 4 },
    { "FxOp98 JMP r8", asm_FxOp98, c_FxOp98, 4, 8 },
    { "FxOp99 JMP r9", asm_FxOp99, c_FxOp99, 4, 9 },
    { "FxOp9A JMP r10", asm_FxOp9A, c_FxOp9A, 4, 10 },
    { "FxOp9B JMP r11", asm_FxOp9B, c_FxOp9B, 4, 11 },
    { "FxOp9C JMP r12", asm_FxOp9C, c_FxOp9C, 4, 12 },
    { "FxOp9D JMP r13", asm_FxOp9D, c_FxOp9D, 4, 13 },
    { "FxOp98A1 LJMP r8", asm_FxOp98A1, c_FxOp98A1, 5, 8 },
    { "FxOp99A1 LJMP r9", asm_FxOp99A1, c_FxOp99A1, 5, 9 },
    { "FxOp9AA1 LJMP r10", asm_FxOp9AA1, c_FxOp9AA1, 5, 10 },
    { "FxOp9BA1 LJMP r11", asm_FxOp9BA1, c_FxOp9BA1, 5, 11 },
    { "FxOp9CA1 LJMP r12", asm_FxOp9CA1, c_FxOp9CA1, 5, 12 },
    { "FxOp9DA1 LJMP r13", asm_FxOp9DA1, c_FxOp9DA1, 5, 13 },
    { "FxOp02 CACHE", asm_FxOp02, c_FxOp02, 0, 0 },
    { "FxOp01 NOP", asm_FxOp01, c_FxOp01, 0, 0 },
    { "FxOp4D SWAP", asm_FxOp4D, c_FxOp4D, 0, 0 },
    { "FxOp4F NOT", asm_FxOp4F, c_FxOp4F, 0, 0 },
    { "FxOp95 SEX", asm_FxOp95, c_FxOp95, 0, 0 },
    { "FxOp96 ASR", asm_FxOp96, c_FxOp96, 0, 0 },
    { "FxOp96A1 DIV2", asm_FxOp96A1, c_FxOp96A1, 0, 0 },
    { "FxOp97 ROR", asm_FxOp97, c_FxOp97, 0, 0 },
    { "FxOp9E LOB", asm_FxOp9E, c_FxOp9E, 0, 0 },
    { "FxOpC0 HIB", asm_FxOpC0, c_FxOpC0, 0, 0 },
    { "FxOp03 LSR", asm_FxOp03, c_FxOp03, 0, 0 },
    { "FxOp04 ROL", asm_FxOp04, c_FxOp04, 0, 0 },
    { "FxOp3C LOOP", asm_FxOp3C, c_FxOp3C, 4, 13 }, /* branches to R13 */
    { "FxOp9F FMULT", asm_FxOp9F, c_FxOp9F, 0, 0 },
    { "FxOp9FA1 LMULT", asm_FxOp9FA1, c_FxOp9FA1, 0, 0 },
    { "FxOpAE IBT R14", asm_FxOpAE, c_FxOpAE, 0, 0 },
    { "FxOpAF JMP #d", asm_FxOpAF, c_FxOpAF, 0, 0 },
    { "FxOpDE INC R14", asm_FxOpDE, c_FxOpDE, 0, 0 },
    { "FxOpEE DEC R14", asm_FxOpEE, c_FxOpEE, 0, 0 },
};

/* A register value, biased towards the 16-bit boundaries. The arithmetic here
 * is 16-bit inside a 32-bit register, so plain random values almost never carry
 * or borrow out of the low half (1 in 65536) and would leave a wrong-width
 * INC/DEC or ADD/SUB indistinguishable from a correct one. */
static u4 dt_reg(void)
{
    u4 const hi = dt_u32() & 0xFFFF0000u;

    switch (dt_mod(8)) {
    case 0:
        return hi | 0xFFFFu;
    case 1:
        return hi;
    case 2:
        return hi | 0x8000u;
    case 3:
        return hi | 0x7FFFu;
    case 4:
        return hi | (dt_u32() & 0xFu); /* small, like the immediate forms */
    default:
        return dt_u32();
    }
}

/* SuperFX RAM for the load/store handlers. The address is the raw register
 * value added to SfxRAMMem, so the difftest masks the one register a given
 * handler uses as its address (fxcase.addr_reg) down to this window. A power of
 * two keeps STW's second byte, at addr^1, in range as well. */
#define FXRAM_SIZE 0x400
static u1 fxram[FXRAM_SIZE];
static u1 fxram_init[FXRAM_SIZE];

/* One instruction stream to branch around in. 0x100 of slack either side lets
 * a full -128..+127 displacement stay inside it. */
static u1 code[0x400];

/* Everything a handler is allowed to change. */
typedef struct {
    u4 pc, cx, src, dst;
    u4 spc, scx, ssrc, sdst, hits;
    u4 regs[16];
    u4 signzero, overflow, carry, b, rombuffer, r15sk;
    u4 table, stub_b;
    u4 cbr, pbr, cacheactive, cpb;
    u4 lastramadr;
    u1 ram[FXRAM_SIZE];
} snapshot;

/* The state both runs start from, so the second run cannot see the first's
 * leftovers. */
typedef struct {
    u4 pc_off, cx;
    u4 regs[16];
    u4 signzero, overflow, carry, b, rombuffer, r15sk, cpb, crom;
    u4 src_reg, dst_reg, lastramadr;
    u4 cbr, pbr, cacheactive;
    int mem;
} setup;

static void run(void (*fn)(void), setup const* in, int via_trampoline, snapshot* out)
{
    StubPC = StubCX = StubSrc = StubDst = StubHits = StubTable = StubB = 0;
    if (in->mem) {
        memcpy(fxram, fxram_init, sizeof fxram);
    }
    SfxRAMMem = (u4)(uintptr_t)fxram;
    SfxLastRamAdr = (u4)(uintptr_t)fxram + in->lastramadr;
    memcpy(SfxR0, in->regs, sizeof SfxR0);
    SfxSignZero = in->signzero;
    SfxOverflow = in->overflow;
    SfxCarry = in->carry;
    SfxB = in->b;
    SfxRomBuffer = in->rombuffer;
    withr15sk = in->r15sk;
    SfxCPB = in->cpb;
    SfxCROM = in->crom;
    SfxCBR = in->cbr;
    SfxPBR = in->pbr;
    SfxCacheActive = in->cacheactive;

    FxSeamPC = code + in->pc_off;
    FxSeamSrc = SfxR0 + in->src_reg;
    FxSeamDst = SfxR0 + in->dst_reg;
    FxSeamCX = in->cx;

    if (via_trampoline) {
        asm_fxcall((void*)fn);
    } else {
        fn();
    }

    out->pc = (u4)(FxSeamPC - code);
    out->cx = FxSeamCX;
    out->src = (u4)((u1*)FxSeamSrc - (u1*)SfxR0);
    out->dst = (u4)((u1*)FxSeamDst - (u1*)SfxR0);
    out->spc = StubPC ? StubPC - (u4)(uintptr_t)code : 0;
    out->scx = StubCX;
    out->ssrc = StubSrc ? StubSrc - (u4)(uintptr_t)SfxR0 : 0;
    out->sdst = StubDst ? StubDst - (u4)(uintptr_t)SfxR0 : 0;
    out->hits = StubHits;
    out->table = StubTable;
    out->stub_b = StubB;
    /* Store as an offset: the absolute pointer is the same for both runs, but
     * an offset is what a reader can actually interpret. */
    out->lastramadr = SfxLastRamAdr - (u4)(uintptr_t)fxram;
    if (in->mem) {
        memcpy(out->ram, fxram, sizeof out->ram);
    }
    memcpy(out->regs, SfxR0, sizeof out->regs);
    out->signzero = SfxSignZero;
    out->overflow = SfxOverflow;
    out->carry = SfxCarry;
    out->b = SfxB;
    out->rombuffer = SfxRomBuffer;
    out->r15sk = withr15sk;
    out->cbr = SfxCBR;
    out->pbr = SfxPBR;
    out->cacheactive = SfxCacheActive;
    out->cpb = SfxCPB;
}

int main(void)
{
    for (int i = 0; i < 1024; i++) {
        FxTable[i] = (u4)(uintptr_t)fxstub;
        FxTableb[i] = (u4)(uintptr_t)fxstubb;
        FxTablec[i] = (u4)(uintptr_t)fxstubc;
    }

    DT_MAIN(20260727, 1000000)
    {
        fxcase const* k = &cases[dt_mod(sizeof cases / sizeof *cases)];
        snapshot a, c;
        setup in;

        dt_fill(code, sizeof code);
        in.pc_off = 0x100 + dt_mod(0x200);
        in.cx = (dt_mod(4) << 8) | dt_mod(256);
        for (int i = 0; i < 16; i++) {
            in.regs[i] = dt_reg();
        }
        /* Bias the flags towards the interesting bits so both arms of every
         * condition get hit, not just random noise. */
        in.mem = k->mem;
        in.lastramadr = dt_mod(FXRAM_SIZE);
        in.cbr = dt_mod(2) ? dt_u32() & 0xFFF0u : dt_u32();
        in.pbr = dt_u32();
        in.cacheactive = dt_mod(2) ? dt_u32() : (dt_u32() & ~0xFFu) | 1u;
        in.signzero = dt_u32() & (dt_mod(2) ? 0x000FFFFFu : 0x0008F00Fu);
        /* Keep the high bytes live: FROM rN writes only the *low* byte of
         * SfxOverflow, so a mask here would hide a wrong-width store. */
        in.overflow = dt_u32() & (dt_mod(2) ? 0xFFFFFFFFu : 0x01u);
        in.carry = dt_u32() & (dt_mod(2) ? 0xFFFFFFFFu : 0x01u);
        /* SfxB picks between the two forms of TO/FROM, so hit both halves. */
        in.b = dt_mod(2) ? dt_u32() | 1u : dt_u32() & ~1u;
        in.rombuffer = dt_u32();
        in.r15sk = dt_u32();
        in.crom = dt_u32();
        in.src_reg = dt_mod(16);
        in.dst_reg = dt_mod(16);
        /* TO/FROM R15 rebuild the program counter as SfxCPB + R15, so CPB has
         * to be the real code base or the result lands outside the buffer. */
        in.cpb = (u4)(uintptr_t)code;
        in.regs[15] = dt_mod(sizeof code);

        /* Clamp anything the handler will use as an address. Done last: some of
           it depends on src_reg. */
        if (k->mem) {
            /* Fresh RAM so a load cannot pass by reading a stale value. */
            dt_fill(fxram_init, sizeof fxram_init);
        }
        switch (k->mem) {
        case 1:
            in.regs[k->addr_reg] &= FXRAM_SIZE - 1;
            break;
        case 2:
            /* Mask the immediate's high byte in the instruction stream. */
            code[in.pc_off + 1] &= (FXRAM_SIZE >> 8) - 1;
            break;
        case 4:
            /* JMP builds the program counter as SfxCPB + Rn. */
            in.regs[k->addr_reg] &= sizeof code - 1;
            break;
        case 5:
            /* LJMP takes the bank base from SfxMemTable and adds the source
               register, so point every bank at the instruction stream. */
            for (int b = 0; b < 256; b++) {
                SfxMemTable[b] = (u4)(uintptr_t)code;
            }
            in.regs[in.src_reg] &= sizeof code - 1;
            break;
        default:
            break;
        }

        run(k->asm_fn, &in, 1, &a);
        run(k->c_fn, &in, 0, &c);

        DT_EQ(k->name, a.pc, c.pc);
        DT_EQ("ecx", a.cx, c.cx);
        DT_EQ("esi", a.src, c.src);
        DT_EQ("edi", a.dst, c.dst);
        DT_EQ("next-opcode dispatch count", a.hits, c.hits);
        DT_EQ("next-opcode table", a.table, c.table);
        DT_EQ("next-opcode SfxB", a.stub_b, c.stub_b);
        DT_EQ("next-opcode pc", a.spc, c.spc);
        DT_EQ("next-opcode ecx", a.scx, c.scx);
        DT_EQ("next-opcode esi", a.ssrc, c.ssrc);
        DT_EQ("next-opcode edi", a.sdst, c.sdst);
        DT_MEM("SfxR0..R15", a.regs, c.regs, sizeof a.regs);
        DT_EQ("SfxSignZero", a.signzero, c.signzero);
        DT_EQ("SfxOverflow", a.overflow, c.overflow);
        DT_EQ("SfxCarry", a.carry, c.carry);
        DT_EQ("SfxB", a.b, c.b);
        DT_EQ("SfxRomBuffer", a.rombuffer, c.rombuffer);
        DT_EQ("withr15sk", a.r15sk, c.r15sk);
        DT_EQ("SfxLastRamAdr", a.lastramadr, c.lastramadr);
        DT_EQ("SfxCBR", a.cbr, c.cbr);
        DT_EQ("SfxPBR", a.pbr, c.pbr);
        DT_EQ("SfxCacheActive", a.cacheactive, c.cacheactive);
        DT_EQ("SfxCPB", a.cpb, c.cpb);
        if (k->mem) {
            DT_MEM("SuperFX RAM", a.ram, c.ram, sizeof a.ram);
        }
        if (dt_bad && DT_SHOW()) {
            printf("  ^ case %s\n", k->name);
        }
    }
    DT_DONE("SuperFX opcode handlers");
}
