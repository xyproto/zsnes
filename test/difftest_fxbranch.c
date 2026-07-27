/*
 * Differential test: the 22 SuperFX branch handlers (BRA/BGE/BLT/BNE/BEQ/BPL/
 * BMI/BCC/BCS/BVC/BVS, in both the b and c table groups) ported out of
 * chips/fxemu2b.asm into chips/fx_ops.h.
 *
 * Run `make fxbranch` in this directory. Not part of `all`: the oracle is the
 * original assembly, pulled out of git by mkfxbranch.sh.
 *
 * Both sides run against the same random flags, program counter and delay-slot
 * byte, and both dispatch the delay slot into the same recording stub, so the
 * comparison covers the resulting program counter, the ALT-mode/opcode word the
 * next instruction was dispatched with, and the source/destination pointers.
 */
#include "difftest.h"

typedef uint8_t u1;
typedef int8_t s1;
typedef uint32_t u4;

/* Shared SuperFX state the handlers read (normally chips/fxemu2.asm). */
u4 SfxCarry, SfxSignZero, SfxOverflow;

/* ecx indexes (ALT mode << 8) | opcode, so the b and c tables need all four
 * ALT sub-tables' worth of entries laid out adjacently, as in endmem.c. */
u4 FxTableb[1024];
u4 FxTablec[1024];

/* The seam block (normally chips/c_fxemu2b.c). */
u1* FxSeamPC;
u4* FxSeamSrc;
u4* FxSeamDst;
u4 FxSeamCX;

/* What the delay-slot dispatch saw. */
u4 StubPC, StubCX, StubSrc, StubDst, StubHits;

void FxDispatch(u4 const* table); /* _fxbranch.o */
void asm_fxcall(void* fn); /* _fxbranch.o */
void fxstub(void); /* _fxbranch.o */

#include "../chips/fx_ops.h"

extern void asm_FxOpb05(void), asm_FxOpb06(void), asm_FxOpb07(void), asm_FxOpb08(void);
extern void asm_FxOpb09(void), asm_FxOpb0A(void), asm_FxOpb0B(void), asm_FxOpb0C(void);
extern void asm_FxOpb0D(void), asm_FxOpb0E(void), asm_FxOpb0F(void);
extern void asm_FxOpc05(void), asm_FxOpc06(void), asm_FxOpc07(void), asm_FxOpc08(void);
extern void asm_FxOpc09(void), asm_FxOpc0A(void), asm_FxOpc0B(void), asm_FxOpc0C(void);
extern void asm_FxOpc0D(void), asm_FxOpc0E(void), asm_FxOpc0F(void);

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
};

/* One instruction stream to branch around in. 0x100 of slack either side lets
 * a full -128..+127 displacement stay inside it. */
static u1 code[0x400];
static u4 sfxregs[16];

typedef struct {
    u4 pc, cx, src, dst;
    u4 spc, scx, ssrc, sdst, hits;
} snapshot;

static void run(void (*fn)(void), u4 pc_off, u4 cx, int via_trampoline, snapshot* out)
{
    StubPC = StubCX = StubSrc = StubDst = StubHits = 0;
    FxSeamPC = code + pc_off;
    FxSeamSrc = sfxregs;
    FxSeamDst = sfxregs + 1;
    FxSeamCX = cx;

    if (via_trampoline) {
        asm_fxcall((void*)fn);
    } else {
        fn();
    }

    out->pc = (u4)(FxSeamPC - code);
    out->cx = FxSeamCX;
    out->src = (u4)((u1*)FxSeamSrc - (u1*)sfxregs);
    out->dst = (u4)((u1*)FxSeamDst - (u1*)sfxregs);
    out->spc = StubPC ? StubPC - (u4)(uintptr_t)code : 0;
    out->scx = StubCX;
    out->ssrc = StubSrc;
    out->sdst = StubDst;
    out->hits = StubHits;
}

int main(void)
{
    for (int i = 0; i < 1024; i++) {
        FxTableb[i] = (u4)(uintptr_t)fxstub;
        FxTablec[i] = (u4)(uintptr_t)fxstub;
    }

    DT_MAIN(20260727, 200000)
    {
        fxcase const* k = &cases[dt_mod(sizeof cases / sizeof *cases)];
        u4 const pc_off = 0x100 + dt_mod(0x200);
        u4 const cx = (dt_mod(4) << 8) | dt_mod(256);
        snapshot a, c;

        dt_fill(code, sizeof code);
        /* Bias the flags towards the interesting bits so both arms of every
         * condition get hit, not just random noise. */
        SfxSignZero = dt_u32() & (dt_mod(2) ? 0x000FFFFFu : 0x0008F00Fu);
        SfxOverflow = dt_u32() & (dt_mod(2) ? 0xFFu : 0x01u);
        SfxCarry = dt_u32() & (dt_mod(2) ? 0xFFu : 0x01u);

        run(k->asm_fn, pc_off, cx, 1, &a);
        run(k->c_fn, pc_off, cx, 0, &c);

        DT_EQ(k->name, a.pc, c.pc);
        DT_EQ("ecx", a.cx, c.cx);
        DT_EQ("esi", a.src, c.src);
        DT_EQ("edi", a.dst, c.dst);
        DT_EQ("delay-slot dispatch count", a.hits, c.hits);
        DT_EQ("delay-slot pc", a.spc, c.spc);
        DT_EQ("delay-slot ecx", a.scx, c.scx);
        DT_EQ("delay-slot esi", a.ssrc, c.ssrc);
        DT_EQ("delay-slot edi", a.sdst, c.sdst);
    }
    DT_DONE("SuperFX branch handlers");
}
