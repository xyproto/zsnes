/*
 * Differential test: every SuperFX opcode handler ported into chips/fx_ops.h -
 * all 92 from chips/fxemu2b.asm (branches, TO rN / FROM rN in the b and c
 * table groups, ALT1/ALT2/ALT3) and the 267-strong ALU group from
 * chips/fxemu2.asm (ADD/ADC/SUB/SBC/CMP/AND/BIC/OR/XOR/INC/DEC, register
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

/* Shared SuperFX state the handlers read (normally chips/fxemu2.asm). */
u4 SfxCarry, SfxSignZero, SfxOverflow;
u4 SfxB, SfxCPB, SfxCROM, SfxRomBuffer, withr15sk;
u4 SfxR0[16];
u4 FxTable[1024];

/* The assembly names R14 and R15 individually; they are just the last two
 * slots of the contiguous register file. */
__asm__(".globl SfxR14\n.set SfxR14, SfxR0+56\n"
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
u4 StubPC, StubCX, StubSrc, StubDst, StubHits, StubTable;

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
extern void asm_FxOpEB(void), asm_FxOpEC(void), asm_FxOpED(void);

typedef struct {
    char const* name;
    void (*asm_fn)(void);
    void (*c_fn)(void);
} fxcase;

static fxcase const cases[] = {
    { "FxOpb05 BRA", asm_FxOpb05, c_FxOpb05 },
    { "FxOpb06 BGE", asm_FxOpb06, c_FxOpb06 },
    { "FxOpb07 BLT", asm_FxOpb07, c_FxOpb07 },
    { "FxOpb08 BNE", asm_FxOpb08, c_FxOpb08 },
    { "FxOpb09 BEQ", asm_FxOpb09, c_FxOpb09 },
    { "FxOpb0A BPL", asm_FxOpb0A, c_FxOpb0A },
    { "FxOpb0B BMI", asm_FxOpb0B, c_FxOpb0B },
    { "FxOpb0C BCC", asm_FxOpb0C, c_FxOpb0C },
    { "FxOpb0D BCS", asm_FxOpb0D, c_FxOpb0D },
    { "FxOpb0E BVC", asm_FxOpb0E, c_FxOpb0E },
    { "FxOpb0F BVS", asm_FxOpb0F, c_FxOpb0F },
    { "FxOpb10 TO r0", asm_FxOpb10, c_FxOpb10 },
    { "FxOpb11 TO r1", asm_FxOpb11, c_FxOpb11 },
    { "FxOpb12 TO r2", asm_FxOpb12, c_FxOpb12 },
    { "FxOpb13 TO r3", asm_FxOpb13, c_FxOpb13 },
    { "FxOpb14 TO r4", asm_FxOpb14, c_FxOpb14 },
    { "FxOpb15 TO r5", asm_FxOpb15, c_FxOpb15 },
    { "FxOpb16 TO r6", asm_FxOpb16, c_FxOpb16 },
    { "FxOpb17 TO r7", asm_FxOpb17, c_FxOpb17 },
    { "FxOpb18 TO r8", asm_FxOpb18, c_FxOpb18 },
    { "FxOpb19 TO r9", asm_FxOpb19, c_FxOpb19 },
    { "FxOpb1A TO r10", asm_FxOpb1A, c_FxOpb1A },
    { "FxOpb1B TO r11", asm_FxOpb1B, c_FxOpb1B },
    { "FxOpb1C TO r12", asm_FxOpb1C, c_FxOpb1C },
    { "FxOpb1D TO r13", asm_FxOpb1D, c_FxOpb1D },
    { "FxOpb1E TO r14", asm_FxOpb1E, c_FxOpb1E },
    { "FxOpb1F TO r15", asm_FxOpb1F, c_FxOpb1F },
    { "FxOpb3D ALT1", asm_FxOpb3D, c_FxOpb3D },
    { "FxOpb3E ALT2", asm_FxOpb3E, c_FxOpb3E },
    { "FxOpb3F ALT3", asm_FxOpb3F, c_FxOpb3F },
    { "FxOpbB0 FROM r0", asm_FxOpbB0, c_FxOpbB0 },
    { "FxOpbB1 FROM r1", asm_FxOpbB1, c_FxOpbB1 },
    { "FxOpbB2 FROM r2", asm_FxOpbB2, c_FxOpbB2 },
    { "FxOpbB3 FROM r3", asm_FxOpbB3, c_FxOpbB3 },
    { "FxOpbB4 FROM r4", asm_FxOpbB4, c_FxOpbB4 },
    { "FxOpbB5 FROM r5", asm_FxOpbB5, c_FxOpbB5 },
    { "FxOpbB6 FROM r6", asm_FxOpbB6, c_FxOpbB6 },
    { "FxOpbB7 FROM r7", asm_FxOpbB7, c_FxOpbB7 },
    { "FxOpbB8 FROM r8", asm_FxOpbB8, c_FxOpbB8 },
    { "FxOpbB9 FROM r9", asm_FxOpbB9, c_FxOpbB9 },
    { "FxOpbBA FROM r10", asm_FxOpbBA, c_FxOpbBA },
    { "FxOpbBB FROM r11", asm_FxOpbBB, c_FxOpbBB },
    { "FxOpbBC FROM r12", asm_FxOpbBC, c_FxOpbBC },
    { "FxOpbBD FROM r13", asm_FxOpbBD, c_FxOpbBD },
    { "FxOpbBE FROM r14", asm_FxOpbBE, c_FxOpbBE },
    { "FxOpbBF FROM r15", asm_FxOpbBF, c_FxOpbBF },
    { "FxOpc05 BRA", asm_FxOpc05, c_FxOpc05 },
    { "FxOpc06 BGE", asm_FxOpc06, c_FxOpc06 },
    { "FxOpc07 BLT", asm_FxOpc07, c_FxOpc07 },
    { "FxOpc08 BNE", asm_FxOpc08, c_FxOpc08 },
    { "FxOpc09 BEQ", asm_FxOpc09, c_FxOpc09 },
    { "FxOpc0A BPL", asm_FxOpc0A, c_FxOpc0A },
    { "FxOpc0B BMI", asm_FxOpc0B, c_FxOpc0B },
    { "FxOpc0C BCC", asm_FxOpc0C, c_FxOpc0C },
    { "FxOpc0D BCS", asm_FxOpc0D, c_FxOpc0D },
    { "FxOpc0E BVC", asm_FxOpc0E, c_FxOpc0E },
    { "FxOpc0F BVS", asm_FxOpc0F, c_FxOpc0F },
    { "FxOpc10 TO r0", asm_FxOpc10, c_FxOpc10 },
    { "FxOpc11 TO r1", asm_FxOpc11, c_FxOpc11 },
    { "FxOpc12 TO r2", asm_FxOpc12, c_FxOpc12 },
    { "FxOpc13 TO r3", asm_FxOpc13, c_FxOpc13 },
    { "FxOpc14 TO r4", asm_FxOpc14, c_FxOpc14 },
    { "FxOpc15 TO r5", asm_FxOpc15, c_FxOpc15 },
    { "FxOpc16 TO r6", asm_FxOpc16, c_FxOpc16 },
    { "FxOpc17 TO r7", asm_FxOpc17, c_FxOpc17 },
    { "FxOpc18 TO r8", asm_FxOpc18, c_FxOpc18 },
    { "FxOpc19 TO r9", asm_FxOpc19, c_FxOpc19 },
    { "FxOpc1A TO r10", asm_FxOpc1A, c_FxOpc1A },
    { "FxOpc1B TO r11", asm_FxOpc1B, c_FxOpc1B },
    { "FxOpc1C TO r12", asm_FxOpc1C, c_FxOpc1C },
    { "FxOpc1D TO r13", asm_FxOpc1D, c_FxOpc1D },
    { "FxOpc1E TO r14", asm_FxOpc1E, c_FxOpc1E },
    { "FxOpc1F TO r15", asm_FxOpc1F, c_FxOpc1F },
    { "FxOpc3D ALT1", asm_FxOpc3D, c_FxOpc3D },
    { "FxOpc3E ALT2", asm_FxOpc3E, c_FxOpc3E },
    { "FxOpc3F ALT3", asm_FxOpc3F, c_FxOpc3F },
    { "FxOpcB0 FROM r0", asm_FxOpcB0, c_FxOpcB0 },
    { "FxOpcB1 FROM r1", asm_FxOpcB1, c_FxOpcB1 },
    { "FxOpcB2 FROM r2", asm_FxOpcB2, c_FxOpcB2 },
    { "FxOpcB3 FROM r3", asm_FxOpcB3, c_FxOpcB3 },
    { "FxOpcB4 FROM r4", asm_FxOpcB4, c_FxOpcB4 },
    { "FxOpcB5 FROM r5", asm_FxOpcB5, c_FxOpcB5 },
    { "FxOpcB6 FROM r6", asm_FxOpcB6, c_FxOpcB6 },
    { "FxOpcB7 FROM r7", asm_FxOpcB7, c_FxOpcB7 },
    { "FxOpcB8 FROM r8", asm_FxOpcB8, c_FxOpcB8 },
    { "FxOpcB9 FROM r9", asm_FxOpcB9, c_FxOpcB9 },
    { "FxOpcBA FROM r10", asm_FxOpcBA, c_FxOpcBA },
    { "FxOpcBB FROM r11", asm_FxOpcBB, c_FxOpcBB },
    { "FxOpcBC FROM r12", asm_FxOpcBC, c_FxOpcBC },
    { "FxOpcBD FROM r13", asm_FxOpcBD, c_FxOpcBD },
    { "FxOpcBE FROM r14", asm_FxOpcBE, c_FxOpcBE },
    { "FxOpcBF FROM r15", asm_FxOpcBF, c_FxOpcBF },
    { "FxOp50 ADD r0", asm_FxOp50, c_FxOp50 },
    { "FxOp51 ADD r1", asm_FxOp51, c_FxOp51 },
    { "FxOp52 ADD r2", asm_FxOp52, c_FxOp52 },
    { "FxOp53 ADD r3", asm_FxOp53, c_FxOp53 },
    { "FxOp54 ADD r4", asm_FxOp54, c_FxOp54 },
    { "FxOp55 ADD r5", asm_FxOp55, c_FxOp55 },
    { "FxOp56 ADD r6", asm_FxOp56, c_FxOp56 },
    { "FxOp57 ADD r7", asm_FxOp57, c_FxOp57 },
    { "FxOp58 ADD r8", asm_FxOp58, c_FxOp58 },
    { "FxOp59 ADD r9", asm_FxOp59, c_FxOp59 },
    { "FxOp5A ADD r10", asm_FxOp5A, c_FxOp5A },
    { "FxOp5B ADD r11", asm_FxOp5B, c_FxOp5B },
    { "FxOp5C ADD r12", asm_FxOp5C, c_FxOp5C },
    { "FxOp5D ADD r13", asm_FxOp5D, c_FxOp5D },
    { "FxOp5E ADD r14", asm_FxOp5E, c_FxOp5E },
    { "FxOp50A1 ADC r0", asm_FxOp50A1, c_FxOp50A1 },
    { "FxOp51A1 ADC r1", asm_FxOp51A1, c_FxOp51A1 },
    { "FxOp52A1 ADC r2", asm_FxOp52A1, c_FxOp52A1 },
    { "FxOp53A1 ADC r3", asm_FxOp53A1, c_FxOp53A1 },
    { "FxOp54A1 ADC r4", asm_FxOp54A1, c_FxOp54A1 },
    { "FxOp55A1 ADC r5", asm_FxOp55A1, c_FxOp55A1 },
    { "FxOp56A1 ADC r6", asm_FxOp56A1, c_FxOp56A1 },
    { "FxOp57A1 ADC r7", asm_FxOp57A1, c_FxOp57A1 },
    { "FxOp58A1 ADC r8", asm_FxOp58A1, c_FxOp58A1 },
    { "FxOp59A1 ADC r9", asm_FxOp59A1, c_FxOp59A1 },
    { "FxOp5AA1 ADC r10", asm_FxOp5AA1, c_FxOp5AA1 },
    { "FxOp5BA1 ADC r11", asm_FxOp5BA1, c_FxOp5BA1 },
    { "FxOp5CA1 ADC r12", asm_FxOp5CA1, c_FxOp5CA1 },
    { "FxOp5DA1 ADC r13", asm_FxOp5DA1, c_FxOp5DA1 },
    { "FxOp5EA1 ADC r14", asm_FxOp5EA1, c_FxOp5EA1 },
    { "FxOp50A2 ADD #0", asm_FxOp50A2, c_FxOp50A2 },
    { "FxOp51A2 ADD #1", asm_FxOp51A2, c_FxOp51A2 },
    { "FxOp52A2 ADD #2", asm_FxOp52A2, c_FxOp52A2 },
    { "FxOp53A2 ADD #3", asm_FxOp53A2, c_FxOp53A2 },
    { "FxOp54A2 ADD #4", asm_FxOp54A2, c_FxOp54A2 },
    { "FxOp55A2 ADD #5", asm_FxOp55A2, c_FxOp55A2 },
    { "FxOp56A2 ADD #6", asm_FxOp56A2, c_FxOp56A2 },
    { "FxOp57A2 ADD #7", asm_FxOp57A2, c_FxOp57A2 },
    { "FxOp58A2 ADD #8", asm_FxOp58A2, c_FxOp58A2 },
    { "FxOp59A2 ADD #9", asm_FxOp59A2, c_FxOp59A2 },
    { "FxOp5AA2 ADD #10", asm_FxOp5AA2, c_FxOp5AA2 },
    { "FxOp5BA2 ADD #11", asm_FxOp5BA2, c_FxOp5BA2 },
    { "FxOp5CA2 ADD #12", asm_FxOp5CA2, c_FxOp5CA2 },
    { "FxOp5DA2 ADD #13", asm_FxOp5DA2, c_FxOp5DA2 },
    { "FxOp5EA2 ADD #14", asm_FxOp5EA2, c_FxOp5EA2 },
    { "FxOp5FA2 ADD #15", asm_FxOp5FA2, c_FxOp5FA2 },
    { "FxOp50A3 ADC #0", asm_FxOp50A3, c_FxOp50A3 },
    { "FxOp51A3 ADC #1", asm_FxOp51A3, c_FxOp51A3 },
    { "FxOp52A3 ADC #2", asm_FxOp52A3, c_FxOp52A3 },
    { "FxOp53A3 ADC #3", asm_FxOp53A3, c_FxOp53A3 },
    { "FxOp54A3 ADC #4", asm_FxOp54A3, c_FxOp54A3 },
    { "FxOp55A3 ADC #5", asm_FxOp55A3, c_FxOp55A3 },
    { "FxOp56A3 ADC #6", asm_FxOp56A3, c_FxOp56A3 },
    { "FxOp57A3 ADC #7", asm_FxOp57A3, c_FxOp57A3 },
    { "FxOp58A3 ADC #8", asm_FxOp58A3, c_FxOp58A3 },
    { "FxOp59A3 ADC #9", asm_FxOp59A3, c_FxOp59A3 },
    { "FxOp5AA3 ADC #10", asm_FxOp5AA3, c_FxOp5AA3 },
    { "FxOp5BA3 ADC #11", asm_FxOp5BA3, c_FxOp5BA3 },
    { "FxOp5CA3 ADC #12", asm_FxOp5CA3, c_FxOp5CA3 },
    { "FxOp5DA3 ADC #13", asm_FxOp5DA3, c_FxOp5DA3 },
    { "FxOp5EA3 ADC #14", asm_FxOp5EA3, c_FxOp5EA3 },
    { "FxOp5FA3 ADC #15", asm_FxOp5FA3, c_FxOp5FA3 },
    { "FxOp60 SUB r0", asm_FxOp60, c_FxOp60 },
    { "FxOp61 SUB r1", asm_FxOp61, c_FxOp61 },
    { "FxOp62 SUB r2", asm_FxOp62, c_FxOp62 },
    { "FxOp63 SUB r3", asm_FxOp63, c_FxOp63 },
    { "FxOp64 SUB r4", asm_FxOp64, c_FxOp64 },
    { "FxOp65 SUB r5", asm_FxOp65, c_FxOp65 },
    { "FxOp66 SUB r6", asm_FxOp66, c_FxOp66 },
    { "FxOp67 SUB r7", asm_FxOp67, c_FxOp67 },
    { "FxOp68 SUB r8", asm_FxOp68, c_FxOp68 },
    { "FxOp69 SUB r9", asm_FxOp69, c_FxOp69 },
    { "FxOp6A SUB r10", asm_FxOp6A, c_FxOp6A },
    { "FxOp6B SUB r11", asm_FxOp6B, c_FxOp6B },
    { "FxOp6C SUB r12", asm_FxOp6C, c_FxOp6C },
    { "FxOp6D SUB r13", asm_FxOp6D, c_FxOp6D },
    { "FxOp6E SUB r14", asm_FxOp6E, c_FxOp6E },
    { "FxOp60A1 SBC r0", asm_FxOp60A1, c_FxOp60A1 },
    { "FxOp61A1 SBC r1", asm_FxOp61A1, c_FxOp61A1 },
    { "FxOp62A1 SBC r2", asm_FxOp62A1, c_FxOp62A1 },
    { "FxOp63A1 SBC r3", asm_FxOp63A1, c_FxOp63A1 },
    { "FxOp64A1 SBC r4", asm_FxOp64A1, c_FxOp64A1 },
    { "FxOp65A1 SBC r5", asm_FxOp65A1, c_FxOp65A1 },
    { "FxOp66A1 SBC r6", asm_FxOp66A1, c_FxOp66A1 },
    { "FxOp67A1 SBC r7", asm_FxOp67A1, c_FxOp67A1 },
    { "FxOp68A1 SBC r8", asm_FxOp68A1, c_FxOp68A1 },
    { "FxOp69A1 SBC r9", asm_FxOp69A1, c_FxOp69A1 },
    { "FxOp6AA1 SBC r10", asm_FxOp6AA1, c_FxOp6AA1 },
    { "FxOp6BA1 SBC r11", asm_FxOp6BA1, c_FxOp6BA1 },
    { "FxOp6CA1 SBC r12", asm_FxOp6CA1, c_FxOp6CA1 },
    { "FxOp6DA1 SBC r13", asm_FxOp6DA1, c_FxOp6DA1 },
    { "FxOp6EA1 SBC r14", asm_FxOp6EA1, c_FxOp6EA1 },
    { "FxOp60A2 SUB #0", asm_FxOp60A2, c_FxOp60A2 },
    { "FxOp61A2 SUB #1", asm_FxOp61A2, c_FxOp61A2 },
    { "FxOp62A2 SUB #2", asm_FxOp62A2, c_FxOp62A2 },
    { "FxOp63A2 SUB #3", asm_FxOp63A2, c_FxOp63A2 },
    { "FxOp64A2 SUB #4", asm_FxOp64A2, c_FxOp64A2 },
    { "FxOp65A2 SUB #5", asm_FxOp65A2, c_FxOp65A2 },
    { "FxOp66A2 SUB #6", asm_FxOp66A2, c_FxOp66A2 },
    { "FxOp67A2 SUB #7", asm_FxOp67A2, c_FxOp67A2 },
    { "FxOp68A2 SUB #8", asm_FxOp68A2, c_FxOp68A2 },
    { "FxOp69A2 SUB #9", asm_FxOp69A2, c_FxOp69A2 },
    { "FxOp6AA2 SUB #10", asm_FxOp6AA2, c_FxOp6AA2 },
    { "FxOp6BA2 SUB #11", asm_FxOp6BA2, c_FxOp6BA2 },
    { "FxOp6CA2 SUB #12", asm_FxOp6CA2, c_FxOp6CA2 },
    { "FxOp6DA2 SUB #13", asm_FxOp6DA2, c_FxOp6DA2 },
    { "FxOp6EA2 SUB #14", asm_FxOp6EA2, c_FxOp6EA2 },
    { "FxOp6FA2 SUB #15", asm_FxOp6FA2, c_FxOp6FA2 },
    { "FxOp60A3 CMP r0", asm_FxOp60A3, c_FxOp60A3 },
    { "FxOp61A3 CMP r1", asm_FxOp61A3, c_FxOp61A3 },
    { "FxOp62A3 CMP r2", asm_FxOp62A3, c_FxOp62A3 },
    { "FxOp63A3 CMP r3", asm_FxOp63A3, c_FxOp63A3 },
    { "FxOp64A3 CMP r4", asm_FxOp64A3, c_FxOp64A3 },
    { "FxOp65A3 CMP r5", asm_FxOp65A3, c_FxOp65A3 },
    { "FxOp66A3 CMP r6", asm_FxOp66A3, c_FxOp66A3 },
    { "FxOp67A3 CMP r7", asm_FxOp67A3, c_FxOp67A3 },
    { "FxOp68A3 CMP r8", asm_FxOp68A3, c_FxOp68A3 },
    { "FxOp69A3 CMP r9", asm_FxOp69A3, c_FxOp69A3 },
    { "FxOp6AA3 CMP r10", asm_FxOp6AA3, c_FxOp6AA3 },
    { "FxOp6BA3 CMP r11", asm_FxOp6BA3, c_FxOp6BA3 },
    { "FxOp6CA3 CMP r12", asm_FxOp6CA3, c_FxOp6CA3 },
    { "FxOp6DA3 CMP r13", asm_FxOp6DA3, c_FxOp6DA3 },
    { "FxOp6EA3 CMP r14", asm_FxOp6EA3, c_FxOp6EA3 },
    { "FxOp71 AND r1", asm_FxOp71, c_FxOp71 },
    { "FxOp72 AND r2", asm_FxOp72, c_FxOp72 },
    { "FxOp73 AND r3", asm_FxOp73, c_FxOp73 },
    { "FxOp74 AND r4", asm_FxOp74, c_FxOp74 },
    { "FxOp75 AND r5", asm_FxOp75, c_FxOp75 },
    { "FxOp76 AND r6", asm_FxOp76, c_FxOp76 },
    { "FxOp77 AND r7", asm_FxOp77, c_FxOp77 },
    { "FxOp78 AND r8", asm_FxOp78, c_FxOp78 },
    { "FxOp79 AND r9", asm_FxOp79, c_FxOp79 },
    { "FxOp7A AND r10", asm_FxOp7A, c_FxOp7A },
    { "FxOp7B AND r11", asm_FxOp7B, c_FxOp7B },
    { "FxOp7C AND r12", asm_FxOp7C, c_FxOp7C },
    { "FxOp7D AND r13", asm_FxOp7D, c_FxOp7D },
    { "FxOp7E AND r14", asm_FxOp7E, c_FxOp7E },
    { "FxOp71A1 BIC r1", asm_FxOp71A1, c_FxOp71A1 },
    { "FxOp72A1 BIC r2", asm_FxOp72A1, c_FxOp72A1 },
    { "FxOp73A1 BIC r3", asm_FxOp73A1, c_FxOp73A1 },
    { "FxOp74A1 BIC r4", asm_FxOp74A1, c_FxOp74A1 },
    { "FxOp75A1 BIC r5", asm_FxOp75A1, c_FxOp75A1 },
    { "FxOp76A1 BIC r6", asm_FxOp76A1, c_FxOp76A1 },
    { "FxOp77A1 BIC r7", asm_FxOp77A1, c_FxOp77A1 },
    { "FxOp78A1 BIC r8", asm_FxOp78A1, c_FxOp78A1 },
    { "FxOp79A1 BIC r9", asm_FxOp79A1, c_FxOp79A1 },
    { "FxOp7AA1 BIC r10", asm_FxOp7AA1, c_FxOp7AA1 },
    { "FxOp7BA1 BIC r11", asm_FxOp7BA1, c_FxOp7BA1 },
    { "FxOp7CA1 BIC r12", asm_FxOp7CA1, c_FxOp7CA1 },
    { "FxOp7DA1 BIC r13", asm_FxOp7DA1, c_FxOp7DA1 },
    { "FxOp7EA1 BIC r14", asm_FxOp7EA1, c_FxOp7EA1 },
    { "FxOp71A2 AND #1", asm_FxOp71A2, c_FxOp71A2 },
    { "FxOp72A2 AND #2", asm_FxOp72A2, c_FxOp72A2 },
    { "FxOp73A2 AND #3", asm_FxOp73A2, c_FxOp73A2 },
    { "FxOp74A2 AND #4", asm_FxOp74A2, c_FxOp74A2 },
    { "FxOp75A2 AND #5", asm_FxOp75A2, c_FxOp75A2 },
    { "FxOp76A2 AND #6", asm_FxOp76A2, c_FxOp76A2 },
    { "FxOp77A2 AND #7", asm_FxOp77A2, c_FxOp77A2 },
    { "FxOp78A2 AND #8", asm_FxOp78A2, c_FxOp78A2 },
    { "FxOp79A2 AND #9", asm_FxOp79A2, c_FxOp79A2 },
    { "FxOp7AA2 AND #10", asm_FxOp7AA2, c_FxOp7AA2 },
    { "FxOp7BA2 AND #11", asm_FxOp7BA2, c_FxOp7BA2 },
    { "FxOp7CA2 AND #12", asm_FxOp7CA2, c_FxOp7CA2 },
    { "FxOp7DA2 AND #13", asm_FxOp7DA2, c_FxOp7DA2 },
    { "FxOp7EA2 AND #14", asm_FxOp7EA2, c_FxOp7EA2 },
    { "FxOp7FA2 AND #15", asm_FxOp7FA2, c_FxOp7FA2 },
    { "FxOp71A3 BIC #1", asm_FxOp71A3, c_FxOp71A3 },
    { "FxOp72A3 BIC #2", asm_FxOp72A3, c_FxOp72A3 },
    { "FxOp73A3 BIC #3", asm_FxOp73A3, c_FxOp73A3 },
    { "FxOp74A3 BIC #4", asm_FxOp74A3, c_FxOp74A3 },
    { "FxOp75A3 BIC #5", asm_FxOp75A3, c_FxOp75A3 },
    { "FxOp76A3 BIC #6", asm_FxOp76A3, c_FxOp76A3 },
    { "FxOp77A3 BIC #7", asm_FxOp77A3, c_FxOp77A3 },
    { "FxOp78A3 BIC #8", asm_FxOp78A3, c_FxOp78A3 },
    { "FxOp79A3 BIC #9", asm_FxOp79A3, c_FxOp79A3 },
    { "FxOp7AA3 BIC #10", asm_FxOp7AA3, c_FxOp7AA3 },
    { "FxOp7BA3 BIC #11", asm_FxOp7BA3, c_FxOp7BA3 },
    { "FxOp7CA3 BIC #12", asm_FxOp7CA3, c_FxOp7CA3 },
    { "FxOp7DA3 BIC #13", asm_FxOp7DA3, c_FxOp7DA3 },
    { "FxOp7EA3 BIC #14", asm_FxOp7EA3, c_FxOp7EA3 },
    { "FxOp7FA3 BIC #15", asm_FxOp7FA3, c_FxOp7FA3 },
    { "FxOpC1 OR r1", asm_FxOpC1, c_FxOpC1 },
    { "FxOpC2 OR r2", asm_FxOpC2, c_FxOpC2 },
    { "FxOpC3 OR r3", asm_FxOpC3, c_FxOpC3 },
    { "FxOpC4 OR r4", asm_FxOpC4, c_FxOpC4 },
    { "FxOpC5 OR r5", asm_FxOpC5, c_FxOpC5 },
    { "FxOpC6 OR r6", asm_FxOpC6, c_FxOpC6 },
    { "FxOpC7 OR r7", asm_FxOpC7, c_FxOpC7 },
    { "FxOpC8 OR r8", asm_FxOpC8, c_FxOpC8 },
    { "FxOpC9 OR r9", asm_FxOpC9, c_FxOpC9 },
    { "FxOpCA OR r10", asm_FxOpCA, c_FxOpCA },
    { "FxOpCB OR r11", asm_FxOpCB, c_FxOpCB },
    { "FxOpCC OR r12", asm_FxOpCC, c_FxOpCC },
    { "FxOpCD OR r13", asm_FxOpCD, c_FxOpCD },
    { "FxOpCE OR r14", asm_FxOpCE, c_FxOpCE },
    { "FxOpC1A1 XOR r1", asm_FxOpC1A1, c_FxOpC1A1 },
    { "FxOpC2A1 XOR r2", asm_FxOpC2A1, c_FxOpC2A1 },
    { "FxOpC3A1 XOR r3", asm_FxOpC3A1, c_FxOpC3A1 },
    { "FxOpC4A1 XOR r4", asm_FxOpC4A1, c_FxOpC4A1 },
    { "FxOpC5A1 XOR r5", asm_FxOpC5A1, c_FxOpC5A1 },
    { "FxOpC6A1 XOR r6", asm_FxOpC6A1, c_FxOpC6A1 },
    { "FxOpC7A1 XOR r7", asm_FxOpC7A1, c_FxOpC7A1 },
    { "FxOpC8A1 XOR r8", asm_FxOpC8A1, c_FxOpC8A1 },
    { "FxOpC9A1 XOR r9", asm_FxOpC9A1, c_FxOpC9A1 },
    { "FxOpCAA1 XOR r10", asm_FxOpCAA1, c_FxOpCAA1 },
    { "FxOpCBA1 XOR r11", asm_FxOpCBA1, c_FxOpCBA1 },
    { "FxOpCCA1 XOR r12", asm_FxOpCCA1, c_FxOpCCA1 },
    { "FxOpCDA1 XOR r13", asm_FxOpCDA1, c_FxOpCDA1 },
    { "FxOpCEA1 XOR r14", asm_FxOpCEA1, c_FxOpCEA1 },
    { "FxOpC1A2 OR #1", asm_FxOpC1A2, c_FxOpC1A2 },
    { "FxOpC2A2 OR #2", asm_FxOpC2A2, c_FxOpC2A2 },
    { "FxOpC3A2 OR #3", asm_FxOpC3A2, c_FxOpC3A2 },
    { "FxOpC4A2 OR #4", asm_FxOpC4A2, c_FxOpC4A2 },
    { "FxOpC5A2 OR #5", asm_FxOpC5A2, c_FxOpC5A2 },
    { "FxOpC6A2 OR #6", asm_FxOpC6A2, c_FxOpC6A2 },
    { "FxOpC7A2 OR #7", asm_FxOpC7A2, c_FxOpC7A2 },
    { "FxOpC8A2 OR #8", asm_FxOpC8A2, c_FxOpC8A2 },
    { "FxOpC9A2 OR #9", asm_FxOpC9A2, c_FxOpC9A2 },
    { "FxOpCAA2 OR #10", asm_FxOpCAA2, c_FxOpCAA2 },
    { "FxOpCBA2 OR #11", asm_FxOpCBA2, c_FxOpCBA2 },
    { "FxOpCCA2 OR #12", asm_FxOpCCA2, c_FxOpCCA2 },
    { "FxOpCDA2 OR #13", asm_FxOpCDA2, c_FxOpCDA2 },
    { "FxOpCEA2 OR #14", asm_FxOpCEA2, c_FxOpCEA2 },
    { "FxOpCFA2 OR #15", asm_FxOpCFA2, c_FxOpCFA2 },
    { "FxOpC1A3 XOR #1", asm_FxOpC1A3, c_FxOpC1A3 },
    { "FxOpC2A3 XOR #2", asm_FxOpC2A3, c_FxOpC2A3 },
    { "FxOpC3A3 XOR #3", asm_FxOpC3A3, c_FxOpC3A3 },
    { "FxOpC4A3 XOR #4", asm_FxOpC4A3, c_FxOpC4A3 },
    { "FxOpC5A3 XOR #5", asm_FxOpC5A3, c_FxOpC5A3 },
    { "FxOpC6A3 XOR #6", asm_FxOpC6A3, c_FxOpC6A3 },
    { "FxOpC7A3 XOR #7", asm_FxOpC7A3, c_FxOpC7A3 },
    { "FxOpC8A3 XOR #8", asm_FxOpC8A3, c_FxOpC8A3 },
    { "FxOpC9A3 XOR #9", asm_FxOpC9A3, c_FxOpC9A3 },
    { "FxOpCAA3 XOR #10", asm_FxOpCAA3, c_FxOpCAA3 },
    { "FxOpCBA3 XOR #11", asm_FxOpCBA3, c_FxOpCBA3 },
    { "FxOpCCA3 XOR #12", asm_FxOpCCA3, c_FxOpCCA3 },
    { "FxOpCDA3 XOR #13", asm_FxOpCDA3, c_FxOpCDA3 },
    { "FxOpCEA3 XOR #14", asm_FxOpCEA3, c_FxOpCEA3 },
    { "FxOpCFA3 XOR #15", asm_FxOpCFA3, c_FxOpCFA3 },
    { "FxOpD0 INC r0", asm_FxOpD0, c_FxOpD0 },
    { "FxOpD1 INC r1", asm_FxOpD1, c_FxOpD1 },
    { "FxOpD2 INC r2", asm_FxOpD2, c_FxOpD2 },
    { "FxOpD3 INC r3", asm_FxOpD3, c_FxOpD3 },
    { "FxOpD4 INC r4", asm_FxOpD4, c_FxOpD4 },
    { "FxOpD5 INC r5", asm_FxOpD5, c_FxOpD5 },
    { "FxOpD6 INC r6", asm_FxOpD6, c_FxOpD6 },
    { "FxOpD7 INC r7", asm_FxOpD7, c_FxOpD7 },
    { "FxOpD8 INC r8", asm_FxOpD8, c_FxOpD8 },
    { "FxOpD9 INC r9", asm_FxOpD9, c_FxOpD9 },
    { "FxOpDA INC r10", asm_FxOpDA, c_FxOpDA },
    { "FxOpDB INC r11", asm_FxOpDB, c_FxOpDB },
    { "FxOpDC INC r12", asm_FxOpDC, c_FxOpDC },
    { "FxOpDD INC r13", asm_FxOpDD, c_FxOpDD },
    { "FxOpE0 DEC r0", asm_FxOpE0, c_FxOpE0 },
    { "FxOpE1 DEC r1", asm_FxOpE1, c_FxOpE1 },
    { "FxOpE2 DEC r2", asm_FxOpE2, c_FxOpE2 },
    { "FxOpE3 DEC r3", asm_FxOpE3, c_FxOpE3 },
    { "FxOpE4 DEC r4", asm_FxOpE4, c_FxOpE4 },
    { "FxOpE5 DEC r5", asm_FxOpE5, c_FxOpE5 },
    { "FxOpE6 DEC r6", asm_FxOpE6, c_FxOpE6 },
    { "FxOpE7 DEC r7", asm_FxOpE7, c_FxOpE7 },
    { "FxOpE8 DEC r8", asm_FxOpE8, c_FxOpE8 },
    { "FxOpE9 DEC r9", asm_FxOpE9, c_FxOpE9 },
    { "FxOpEA DEC r10", asm_FxOpEA, c_FxOpEA },
    { "FxOpEB DEC r11", asm_FxOpEB, c_FxOpEB },
    { "FxOpEC DEC r12", asm_FxOpEC, c_FxOpEC },
    { "FxOpED DEC r13", asm_FxOpED, c_FxOpED },
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

/* One instruction stream to branch around in. 0x100 of slack either side lets
 * a full -128..+127 displacement stay inside it. */
static u1 code[0x400];

/* Everything a handler is allowed to change. */
typedef struct {
    u4 pc, cx, src, dst;
    u4 spc, scx, ssrc, sdst, hits;
    u4 regs[16];
    u4 signzero, overflow, carry, b, rombuffer, r15sk;
    u4 table;
} snapshot;

/* The state both runs start from, so the second run cannot see the first's
 * leftovers. */
typedef struct {
    u4 pc_off, cx;
    u4 regs[16];
    u4 signzero, overflow, carry, b, rombuffer, r15sk, cpb, crom;
    u4 src_reg, dst_reg;
} setup;

static void run(void (*fn)(void), setup const* in, int via_trampoline, snapshot* out)
{
    StubPC = StubCX = StubSrc = StubDst = StubHits = StubTable = 0;
    memcpy(SfxR0, in->regs, sizeof SfxR0);
    SfxSignZero = in->signzero;
    SfxOverflow = in->overflow;
    SfxCarry = in->carry;
    SfxB = in->b;
    SfxRomBuffer = in->rombuffer;
    withr15sk = in->r15sk;
    SfxCPB = in->cpb;
    SfxCROM = in->crom;

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
    memcpy(out->regs, SfxR0, sizeof out->regs);
    out->signzero = SfxSignZero;
    out->overflow = SfxOverflow;
    out->carry = SfxCarry;
    out->b = SfxB;
    out->rombuffer = SfxRomBuffer;
    out->r15sk = withr15sk;
}

int main(void)
{
    for (int i = 0; i < 1024; i++) {
        FxTable[i] = (u4)(uintptr_t)fxstub;
        FxTableb[i] = (u4)(uintptr_t)fxstubb;
        FxTablec[i] = (u4)(uintptr_t)fxstubc;
    }

    DT_MAIN(20260727, 800000)
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

        run(k->asm_fn, &in, 1, &a);
        run(k->c_fn, &in, 0, &c);

        DT_EQ(k->name, a.pc, c.pc);
        DT_EQ("ecx", a.cx, c.cx);
        DT_EQ("esi", a.src, c.src);
        DT_EQ("edi", a.dst, c.dst);
        DT_EQ("next-opcode dispatch count", a.hits, c.hits);
        DT_EQ("next-opcode table", a.table, c.table);
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
        if (dt_bad && DT_SHOW()) {
            printf("  ^ case %s\n", k->name);
        }
    }
    DT_DONE("SuperFX opcode handlers");
}
