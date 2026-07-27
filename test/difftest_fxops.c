/*
 * Differential test: all 92 SuperFX opcode handlers ported out of
 * chips/fxemu2b.asm into chips/fx_ops.h - the branches, TO rN / FROM rN in
 * both the b and c table groups, and ALT1/ALT2/ALT3.
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
};

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

    DT_MAIN(20260727, 400000)
    {
        fxcase const* k = &cases[dt_mod(sizeof cases / sizeof *cases)];
        snapshot a, c;
        setup in;

        dt_fill(code, sizeof code);
        in.pc_off = 0x100 + dt_mod(0x200);
        in.cx = (dt_mod(4) << 8) | dt_mod(256);
        for (int i = 0; i < 16; i++) {
            in.regs[i] = dt_u32();
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
    }
    DT_DONE("SuperFX opcode handlers (fxemu2b)");
}
