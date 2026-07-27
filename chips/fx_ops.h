/*
 * chips/fx_ops.h - SuperFX opcode handlers ported from chips/fxemu2b.asm.
 *
 * Textual include (chips/c_fxemu2b.c): the includer provides the u1/u2/u4/s1
 * typedefs and the seam block declared below.
 *
 * The assembly core runs the SuperFX with its state in registers:
 *
 *     ebp  program counter, a host pointer into the current code bank
 *     esi  source register pointer      (&SfxR0 + n*4)
 *     edi  destination register pointer (&SfxR0 + n*4)
 *     ecx  cl = the next opcode byte, ch = the ALT1/ALT2/ALT3 mode
 *
 * ch is not just a flag: the dispatch is `call [FxTable + ecx*4]`, and
 * FxTable/FxTableA1/FxTableA2/FxTableA3 are laid out adjacently (endmem.c), so
 * ch selects the ALT table and cl the opcode within it. The same holds for the
 * b and c table groups.
 *
 * A ported handler cannot take those in registers, so chips/fxemu2b.asm keeps
 * the public FxOpXX entry point and reduces its body to the `fxcop` thunk,
 * which spills the four live registers to the seam block, calls the C body,
 * and reloads them. Handlers that chain into the next opcode (nearly all of
 * them do; a SuperFX branch executes its delay slot) re-enter the dispatch
 * table through FxDispatch, which does the same in reverse. Opcodes can
 * therefore migrate one at a time.
 */
#ifndef FX_OPS_H
#define FX_OPS_H

/* The branch condition tests below are deliberately bit-for-bit what the asm
 * did, quirks included: BPL/BMI test SfxSignZero against 0x88000, not 0x8000,
 * and the sign/overflow XOR only ever looks at bit 0. */

static inline int fx_cond_ge(void)
{
    return (((SfxSignZero >> 15) ^ SfxOverflow) & 1) == 0;
}

static inline int fx_cond_zero(void)
{
    return (SfxSignZero & 0xFFFF) == 0;
}

static inline int fx_cond_minus(void)
{
    return (SfxSignZero & 0x88000) != 0;
}

static inline int fx_cond_carry(void)
{
    return (SfxCarry & 1) != 0;
}

static inline int fx_cond_overflow(void)
{
    return (SfxOverflow & 1) != 0;
}

/* Shared body of the eleven conditional branches. The displacement is the
 * signed byte at the program counter; the opcode byte that follows it is the
 * delay slot, which runs either way. */
static inline void fx_branch(u4 const* const table, int const taken)
{
    s1 const disp = (s1)*FxSeamPC;

    FxSeamPC++;
    FxSeamCX = (FxSeamCX & ~0xFFu) | *FxSeamPC;
    FxSeamPC += taken ? disp : 1;
    FxDispatch(table);
}

/* BRA reads the delay slot at ebp+1 before advancing, which lands on the same
 * byte the conditional branches read after their `inc ebp`. */
static inline void fx_branch_always(u4 const* const table)
{
    s1 const disp = (s1)*FxSeamPC;

    FxSeamCX = (FxSeamCX & ~0xFFu) | FxSeamPC[1];
    FxSeamPC++;
    FxSeamPC += disp;
    FxDispatch(table);
}

#define FX_BRANCHES(suffix, table)                                           \
    void c_FxOp##suffix##05(void) { fx_branch_always(table); }               \
    void c_FxOp##suffix##06(void) { fx_branch(table, fx_cond_ge()); }        \
    void c_FxOp##suffix##07(void) { fx_branch(table, !fx_cond_ge()); }       \
    void c_FxOp##suffix##08(void) { fx_branch(table, !fx_cond_zero()); }     \
    void c_FxOp##suffix##09(void) { fx_branch(table, fx_cond_zero()); }      \
    void c_FxOp##suffix##0A(void) { fx_branch(table, !fx_cond_minus()); }    \
    void c_FxOp##suffix##0B(void) { fx_branch(table, fx_cond_minus()); }     \
    void c_FxOp##suffix##0C(void) { fx_branch(table, !fx_cond_carry()); }    \
    void c_FxOp##suffix##0D(void) { fx_branch(table, fx_cond_carry()); }     \
    void c_FxOp##suffix##0E(void) { fx_branch(table, !fx_cond_overflow()); } \
    void c_FxOp##suffix##0F(void) { fx_branch(table, fx_cond_overflow()); }

FX_BRANCHES(b, FxTableb)
FX_BRANCHES(c, FxTablec)

#endif
