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

/* --- TO rN / FROM rN, and the register-select opcodes ---------------------
 *
 * These come in two flavours. Outside a WITH block (SfxB clear, "version A")
 * TO/FROM only *retarget* the destination or source register and then run the
 * next opcode with that retargeting in place, restoring it afterwards. Inside a
 * WITH block (SfxB set, "version B") the same opcode instead performs an
 * immediate register-to-register move.
 *
 * Note which table each one chains through: TO uses the b table, FROM uses the
 * base table, and the c-group opcodes have no version A at all. */

/* FETCHPIPE: load the next opcode byte into cl, leaving the ALT mode in ch. */
static inline void fx_fetchpipe(void)
{
    FxSeamCX = (FxSeamCX & ~0xFFu) | *FxSeamPC;
}

/* R15 mirrors the program counter as a bank-relative address. */
static inline u4 fx_pc_rel(void)
{
    return (u4)(uintptr_t)FxSeamPC - SfxCPB;
}

/* UpdateR14: recompute the ROM pointer R14 reads through. */
static inline void fx_update_r14(void)
{
    SfxRomBuffer = SfxCROM + SfxR0[14];
}

/* The destination write shared by every FROM rN: the value sets sign/zero, and
 * `shr al,7` followed by a *byte* store means only the low byte of SfxOverflow
 * is touched, the upper three keep whatever they held. */
static inline void fx_from_write(u4 const v)
{
    *FxSeamDst = v;
    SfxSignZero = v;
    *(u1*)&SfxOverflow = (u1)((v & 0xFF) >> 7);
}

static inline void fx_torn_b(u4 const n)
{
    fx_fetchpipe();
    if (SfxB & 1) {
        u4 const v = *FxSeamSrc;
        withr15sk = 1;
        FxSeamPC++;
        SfxR0[n] = v;
        return;
    }
    FxSeamDst = SfxR0 + n;
    FxSeamPC++;
    withr15sk = 1;
    SfxR0[15] = fx_pc_rel();
    FxDispatch(FxTableb);
    FxSeamDst = SfxR0;
}

static inline void fx_torn_c(u4 const n)
{
    fx_fetchpipe();
    u4 const v = *FxSeamSrc;
    FxSeamPC++;
    SfxR0[n] = v;
}

static inline void fx_fromrn_b(u4 const n)
{
    fx_fetchpipe();
    if (SfxB & 1) {
        u4 const v = SfxR0[n];
        FxSeamPC++;
        fx_from_write(v);
        return;
    }
    FxSeamSrc = SfxR0 + n;
    FxSeamPC++;
    FxDispatch(FxTable);
    FxSeamSrc = SfxR0;
}

static inline void fx_fromrn_c(u4 const n)
{
    fx_fetchpipe();
    u4 const v = SfxR0[n];
    FxSeamPC++;
    fx_from_write(v);
}

/* ALT1/ALT2/ALT3 set the mode bits in ch for exactly one instruction, which
 * steers the dispatch into the matching ALT sub-table, then clear them. */
static inline void fx_alt_b(u4 const mode, u4 const* const table)
{
    fx_fetchpipe();
    SfxB = 0;
    FxSeamCX |= mode << 8;
    FxSeamPC++;
    SfxR0[15] = fx_pc_rel();
    FxDispatch(table);
    FxSeamCX &= 0xFFu;
}

static inline void fx_alt_c(u4 const mode)
{
    fx_fetchpipe();
    SfxB = 0;
    FxSeamCX |= mode << 8;
    FxSeamPC++;
    FxDispatch(FxTablec);
    FxSeamCX &= 0xFFu;
}

#define FX_TORN(suffix, kind)                             \
    void c_FxOp##suffix##10(void) { fx_torn_##kind(0); }  \
    void c_FxOp##suffix##11(void) { fx_torn_##kind(1); }  \
    void c_FxOp##suffix##12(void) { fx_torn_##kind(2); }  \
    void c_FxOp##suffix##13(void) { fx_torn_##kind(3); }  \
    void c_FxOp##suffix##14(void) { fx_torn_##kind(4); }  \
    void c_FxOp##suffix##15(void) { fx_torn_##kind(5); }  \
    void c_FxOp##suffix##16(void) { fx_torn_##kind(6); }  \
    void c_FxOp##suffix##17(void) { fx_torn_##kind(7); }  \
    void c_FxOp##suffix##18(void) { fx_torn_##kind(8); }  \
    void c_FxOp##suffix##19(void) { fx_torn_##kind(9); }  \
    void c_FxOp##suffix##1A(void) { fx_torn_##kind(10); } \
    void c_FxOp##suffix##1B(void) { fx_torn_##kind(11); } \
    void c_FxOp##suffix##1C(void) { fx_torn_##kind(12); } \
    void c_FxOp##suffix##1D(void) { fx_torn_##kind(13); }

#define FX_FROMRN(suffix, kind)                             \
    void c_FxOp##suffix##B0(void) { fx_fromrn_##kind(0); }  \
    void c_FxOp##suffix##B1(void) { fx_fromrn_##kind(1); }  \
    void c_FxOp##suffix##B2(void) { fx_fromrn_##kind(2); }  \
    void c_FxOp##suffix##B3(void) { fx_fromrn_##kind(3); }  \
    void c_FxOp##suffix##B4(void) { fx_fromrn_##kind(4); }  \
    void c_FxOp##suffix##B5(void) { fx_fromrn_##kind(5); }  \
    void c_FxOp##suffix##B6(void) { fx_fromrn_##kind(6); }  \
    void c_FxOp##suffix##B7(void) { fx_fromrn_##kind(7); }  \
    void c_FxOp##suffix##B8(void) { fx_fromrn_##kind(8); }  \
    void c_FxOp##suffix##B9(void) { fx_fromrn_##kind(9); }  \
    void c_FxOp##suffix##BA(void) { fx_fromrn_##kind(10); } \
    void c_FxOp##suffix##BB(void) { fx_fromrn_##kind(11); } \
    void c_FxOp##suffix##BC(void) { fx_fromrn_##kind(12); } \
    void c_FxOp##suffix##BD(void) { fx_fromrn_##kind(13); } \
    void c_FxOp##suffix##BE(void) { fx_fromrn_##kind(14); }

FX_TORN(b, b)
FX_TORN(c, c)
FX_FROMRN(b, b)
FX_FROMRN(c, c)

/* TO R14 also refreshes the ROM read pointer. */
void c_FxOpb1E(void)
{
    fx_fetchpipe();
    if (SfxB & 1) {
        u4 const v = *FxSeamSrc;
        withr15sk = 1;
        SfxR0[14] = v;
        fx_update_r14();
        FxSeamPC++;
        return;
    }
    FxSeamDst = SfxR0 + 14;
    FxSeamPC++;
    withr15sk = 1;
    SfxR0[15] = fx_pc_rel();
    FxDispatch(FxTableb);
    FxSeamDst = SfxR0;
    fx_update_r14();
}

/* TO R15 is a jump: the program counter is rebuilt from R15 afterwards. The
   WITH-block form takes the source value directly and, unlike FxOpc1F, does
   not write R15 back. */
void c_FxOpb1F(void)
{
    fx_fetchpipe();
    if (SfxB & 1) {
        u4 const v = *FxSeamSrc;
        withr15sk = 1;
        FxSeamPC = (u1*)(uintptr_t)(SfxCPB + v);
        return;
    }
    FxSeamDst = SfxR0 + 15;
    FxSeamPC++;
    SfxR0[15] = fx_pc_rel();
    FxDispatch(FxTableb);
    withr15sk = 1;
    FxSeamPC = (u1*)(uintptr_t)(SfxCPB + SfxR0[15]);
    FxSeamDst = SfxR0;
}

/* FROM R15 yields the program counter itself. The non-WITH path deliberately
   has no FETCHPIPE, so it dispatches on whatever opcode byte cl already held;
   that is what the assembly did. */
void c_FxOpbBF(void)
{
    if (SfxB & 1) {
        fx_fetchpipe();
        u4 const v = fx_pc_rel();
        FxSeamPC++;
        fx_from_write(v);
        return;
    }
    FxSeamSrc = SfxR0 + 15;
    FxSeamPC++;
    SfxR0[15] = fx_pc_rel();
    FxDispatch(FxTableb);
    FxSeamSrc = SfxR0;
}

void c_FxOpb3D(void) { fx_alt_b(1, FxTableb); }
void c_FxOpb3E(void) { fx_alt_b(2, FxTable); }
void c_FxOpb3F(void) { fx_alt_b(3, FxTable); }

void c_FxOpc1E(void)
{
    fx_fetchpipe();
    SfxR0[14] = *FxSeamSrc;
    fx_update_r14();
    FxSeamPC++;
}

void c_FxOpc1F(void)
{
    fx_fetchpipe();
    u4 const v = *FxSeamSrc;
    SfxR0[15] = v;
    FxSeamPC = (u1*)(uintptr_t)(SfxCPB + v);
}

void c_FxOpcBF(void)
{
    fx_fetchpipe();
    u4 const v = fx_pc_rel();
    FxSeamPC++;
    fx_from_write(v);
}

void c_FxOpc3D(void) { fx_alt_c(1); }
void c_FxOpc3E(void) { fx_alt_c(2); }
void c_FxOpc3F(void) { fx_alt_c(3); }

#endif
