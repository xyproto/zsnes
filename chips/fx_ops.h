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

/* --- The 16-bit ALU group (chips/fxemu2.asm, base table) ------------------
 *
 * ADD/ADC/SUB/SBC/CMP/AND/BIC, register and immediate forms. Two things about
 * these are easy to get wrong:
 *
 *  - the arithmetic is `add ax,bx`, i.e. 16-bit, but the *whole* 32-bit
 *    register is then written to the destination and to SfxSignZero, so the
 *    upper half of the source value survives untouched;
 *  - `seto`/`setc` store a single byte into SfxOverflow/SfxCarry, leaving the
 *    upper three bytes of each alone.
 */

/* Flag writes are byte-wide, exactly as seto/setc are. */
static inline void fx_set_overflow(int const v) { *(u1*)&SfxOverflow = (u1) !!v; }
static inline void fx_set_carry(int const v) { *(u1*)&SfxCarry = (u1) !!v; }

/* Splice a 16-bit result back into the upper half of the original value. */
static inline u4 fx_lo16(u4 const orig, u4 const res)
{
    return (orig & 0xFFFF0000u) | (res & 0xFFFFu);
}

/* ADD / ADC: carry-in is bit 0 of the SfxCarry byte (`shr byte[SfxCarry],1`). */
static inline void fx_add(u4 const rhs, u4 const carry_in)
{
    u4 const a = *FxSeamSrc;
    u4 const sum = (a & 0xFFFFu) + (rhs & 0xFFFFu) + carry_in;
    u4 const v = fx_lo16(a, sum);

    fx_set_overflow((~(a ^ rhs) & (a ^ sum) & 0x8000u) != 0);
    fx_set_carry((sum & 0x10000u) != 0);
    SfxSignZero = v;
    FxSeamPC++;
    *FxSeamDst = v;
}

/* SUB / SBC / CMP: x86 leaves CF set on borrow, and the asm inverts it, so the
 * SuperFX carry is "no borrow". CMP is the same minus the destination write. */
static inline u4 fx_sub_flags(u4 const rhs, u4 const borrow_in)
{
    u4 const a = *FxSeamSrc;
    u4 const diff = (a & 0xFFFFu) - (rhs & 0xFFFFu) - borrow_in;
    u4 const v = fx_lo16(a, diff);

    fx_set_overflow(((a ^ rhs) & (a ^ diff) & 0x8000u) != 0);
    fx_set_carry((diff & 0x10000u) == 0); /* setc then xor 1 */
    return v;
}

static inline void fx_sub(u4 const rhs, u4 const borrow_in)
{
    u4 const v = fx_sub_flags(rhs, borrow_in);

    FxSeamPC++;
    *FxSeamDst = v;
    SfxSignZero = v;
}

static inline void fx_cmp(u4 const rhs)
{
    SfxSignZero = fx_sub_flags(rhs, 0);
    FxSeamPC++;
}

/* AND / BIC operate on the full 32 bits. */
static inline void fx_and(u4 const rhs)
{
    u4 const v = *FxSeamSrc & rhs;

    FxSeamPC++;
    SfxSignZero = v;
    *FxSeamDst = v;
}

/* Register-operand forms read Rn; immediate forms use the opcode's low nibble.
 * ALT2 gives the immediate ADD/SUB, ALT3 the immediate ADC/CMP, and the AND
 * block follows the same shape one opcode row down. */
#define FX_ALU(name, expr)       \
    static void name(u4 const n) \
    {                            \
        fx_fetchpipe();          \
        expr;                    \
    }

FX_ALU(fx_addrn, fx_add(SfxR0[n], 0))
FX_ALU(fx_adcrn, fx_add(SfxR0[n], SfxCarry & 1))
FX_ALU(fx_adirn, fx_add(n, 0))
FX_ALU(fx_adcirn, fx_add(n, SfxCarry & 1))
FX_ALU(fx_subrn, fx_sub(SfxR0[n], 0))
/* `cmp byte[SfxCarry],1` sets the borrow when the carry byte is zero. */
FX_ALU(fx_sbcrn, fx_sub(SfxR0[n], (SfxCarry & 0xFF) == 0))
FX_ALU(fx_subirn, fx_sub(n, 0))
FX_ALU(fx_cmprn, fx_cmp(SfxR0[n]))
FX_ALU(fx_andrn, fx_and(SfxR0[n]))
/* BIC rN inverts only the low 16 bits of the operand (`xor ebx,0FFFFh`). */
FX_ALU(fx_bicrn, fx_and(SfxR0[n] ^ 0xFFFFu))
FX_ALU(fx_andirn, fx_and(n))
/* BIC #n complements the immediate over 16 bits, like BIC rN does its operand
 * (the asm passes the macro `n ^ 0FFFFh` rather than n). */
FX_ALU(fx_bicirn, fx_and(n ^ 0xFFFFu))

/* ADDRN */
void c_FxOp50(void) { fx_addrn(0); }
void c_FxOp51(void) { fx_addrn(1); }
void c_FxOp52(void) { fx_addrn(2); }
void c_FxOp53(void) { fx_addrn(3); }
void c_FxOp54(void) { fx_addrn(4); }
void c_FxOp55(void) { fx_addrn(5); }
void c_FxOp56(void) { fx_addrn(6); }
void c_FxOp57(void) { fx_addrn(7); }
void c_FxOp58(void) { fx_addrn(8); }
void c_FxOp59(void) { fx_addrn(9); }
void c_FxOp5A(void) { fx_addrn(10); }
void c_FxOp5B(void) { fx_addrn(11); }
void c_FxOp5C(void) { fx_addrn(12); }
void c_FxOp5D(void) { fx_addrn(13); }
void c_FxOp5E(void) { fx_addrn(14); }

/* ADCRN */
void c_FxOp50A1(void) { fx_adcrn(0); }
void c_FxOp51A1(void) { fx_adcrn(1); }
void c_FxOp52A1(void) { fx_adcrn(2); }
void c_FxOp53A1(void) { fx_adcrn(3); }
void c_FxOp54A1(void) { fx_adcrn(4); }
void c_FxOp55A1(void) { fx_adcrn(5); }
void c_FxOp56A1(void) { fx_adcrn(6); }
void c_FxOp57A1(void) { fx_adcrn(7); }
void c_FxOp58A1(void) { fx_adcrn(8); }
void c_FxOp59A1(void) { fx_adcrn(9); }
void c_FxOp5AA1(void) { fx_adcrn(10); }
void c_FxOp5BA1(void) { fx_adcrn(11); }
void c_FxOp5CA1(void) { fx_adcrn(12); }
void c_FxOp5DA1(void) { fx_adcrn(13); }
void c_FxOp5EA1(void) { fx_adcrn(14); }

/* ADIRN */
void c_FxOp50A2(void) { fx_adirn(0); }
void c_FxOp51A2(void) { fx_adirn(1); }
void c_FxOp52A2(void) { fx_adirn(2); }
void c_FxOp53A2(void) { fx_adirn(3); }
void c_FxOp54A2(void) { fx_adirn(4); }
void c_FxOp55A2(void) { fx_adirn(5); }
void c_FxOp56A2(void) { fx_adirn(6); }
void c_FxOp57A2(void) { fx_adirn(7); }
void c_FxOp58A2(void) { fx_adirn(8); }
void c_FxOp59A2(void) { fx_adirn(9); }
void c_FxOp5AA2(void) { fx_adirn(10); }
void c_FxOp5BA2(void) { fx_adirn(11); }
void c_FxOp5CA2(void) { fx_adirn(12); }
void c_FxOp5DA2(void) { fx_adirn(13); }
void c_FxOp5EA2(void) { fx_adirn(14); }
void c_FxOp5FA2(void) { fx_adirn(15); }

/* ADCIRN */
void c_FxOp50A3(void) { fx_adcirn(0); }
void c_FxOp51A3(void) { fx_adcirn(1); }
void c_FxOp52A3(void) { fx_adcirn(2); }
void c_FxOp53A3(void) { fx_adcirn(3); }
void c_FxOp54A3(void) { fx_adcirn(4); }
void c_FxOp55A3(void) { fx_adcirn(5); }
void c_FxOp56A3(void) { fx_adcirn(6); }
void c_FxOp57A3(void) { fx_adcirn(7); }
void c_FxOp58A3(void) { fx_adcirn(8); }
void c_FxOp59A3(void) { fx_adcirn(9); }
void c_FxOp5AA3(void) { fx_adcirn(10); }
void c_FxOp5BA3(void) { fx_adcirn(11); }
void c_FxOp5CA3(void) { fx_adcirn(12); }
void c_FxOp5DA3(void) { fx_adcirn(13); }
void c_FxOp5EA3(void) { fx_adcirn(14); }
void c_FxOp5FA3(void) { fx_adcirn(15); }

/* SUBRN */
void c_FxOp60(void) { fx_subrn(0); }
void c_FxOp61(void) { fx_subrn(1); }
void c_FxOp62(void) { fx_subrn(2); }
void c_FxOp63(void) { fx_subrn(3); }
void c_FxOp64(void) { fx_subrn(4); }
void c_FxOp65(void) { fx_subrn(5); }
void c_FxOp66(void) { fx_subrn(6); }
void c_FxOp67(void) { fx_subrn(7); }
void c_FxOp68(void) { fx_subrn(8); }
void c_FxOp69(void) { fx_subrn(9); }
void c_FxOp6A(void) { fx_subrn(10); }
void c_FxOp6B(void) { fx_subrn(11); }
void c_FxOp6C(void) { fx_subrn(12); }
void c_FxOp6D(void) { fx_subrn(13); }
void c_FxOp6E(void) { fx_subrn(14); }

/* SBCRN */
void c_FxOp60A1(void) { fx_sbcrn(0); }
void c_FxOp61A1(void) { fx_sbcrn(1); }
void c_FxOp62A1(void) { fx_sbcrn(2); }
void c_FxOp63A1(void) { fx_sbcrn(3); }
void c_FxOp64A1(void) { fx_sbcrn(4); }
void c_FxOp65A1(void) { fx_sbcrn(5); }
void c_FxOp66A1(void) { fx_sbcrn(6); }
void c_FxOp67A1(void) { fx_sbcrn(7); }
void c_FxOp68A1(void) { fx_sbcrn(8); }
void c_FxOp69A1(void) { fx_sbcrn(9); }
void c_FxOp6AA1(void) { fx_sbcrn(10); }
void c_FxOp6BA1(void) { fx_sbcrn(11); }
void c_FxOp6CA1(void) { fx_sbcrn(12); }
void c_FxOp6DA1(void) { fx_sbcrn(13); }
void c_FxOp6EA1(void) { fx_sbcrn(14); }

/* SUBIRN */
void c_FxOp60A2(void) { fx_subirn(0); }
void c_FxOp61A2(void) { fx_subirn(1); }
void c_FxOp62A2(void) { fx_subirn(2); }
void c_FxOp63A2(void) { fx_subirn(3); }
void c_FxOp64A2(void) { fx_subirn(4); }
void c_FxOp65A2(void) { fx_subirn(5); }
void c_FxOp66A2(void) { fx_subirn(6); }
void c_FxOp67A2(void) { fx_subirn(7); }
void c_FxOp68A2(void) { fx_subirn(8); }
void c_FxOp69A2(void) { fx_subirn(9); }
void c_FxOp6AA2(void) { fx_subirn(10); }
void c_FxOp6BA2(void) { fx_subirn(11); }
void c_FxOp6CA2(void) { fx_subirn(12); }
void c_FxOp6DA2(void) { fx_subirn(13); }
void c_FxOp6EA2(void) { fx_subirn(14); }
void c_FxOp6FA2(void) { fx_subirn(15); }

/* CMPRN */
void c_FxOp60A3(void) { fx_cmprn(0); }
void c_FxOp61A3(void) { fx_cmprn(1); }
void c_FxOp62A3(void) { fx_cmprn(2); }
void c_FxOp63A3(void) { fx_cmprn(3); }
void c_FxOp64A3(void) { fx_cmprn(4); }
void c_FxOp65A3(void) { fx_cmprn(5); }
void c_FxOp66A3(void) { fx_cmprn(6); }
void c_FxOp67A3(void) { fx_cmprn(7); }
void c_FxOp68A3(void) { fx_cmprn(8); }
void c_FxOp69A3(void) { fx_cmprn(9); }
void c_FxOp6AA3(void) { fx_cmprn(10); }
void c_FxOp6BA3(void) { fx_cmprn(11); }
void c_FxOp6CA3(void) { fx_cmprn(12); }
void c_FxOp6DA3(void) { fx_cmprn(13); }
void c_FxOp6EA3(void) { fx_cmprn(14); }

/* ANDRN */
void c_FxOp71(void) { fx_andrn(1); }
void c_FxOp72(void) { fx_andrn(2); }
void c_FxOp73(void) { fx_andrn(3); }
void c_FxOp74(void) { fx_andrn(4); }
void c_FxOp75(void) { fx_andrn(5); }
void c_FxOp76(void) { fx_andrn(6); }
void c_FxOp77(void) { fx_andrn(7); }
void c_FxOp78(void) { fx_andrn(8); }
void c_FxOp79(void) { fx_andrn(9); }
void c_FxOp7A(void) { fx_andrn(10); }
void c_FxOp7B(void) { fx_andrn(11); }
void c_FxOp7C(void) { fx_andrn(12); }
void c_FxOp7D(void) { fx_andrn(13); }
void c_FxOp7E(void) { fx_andrn(14); }

/* BICRN */
void c_FxOp71A1(void) { fx_bicrn(1); }
void c_FxOp72A1(void) { fx_bicrn(2); }
void c_FxOp73A1(void) { fx_bicrn(3); }
void c_FxOp74A1(void) { fx_bicrn(4); }
void c_FxOp75A1(void) { fx_bicrn(5); }
void c_FxOp76A1(void) { fx_bicrn(6); }
void c_FxOp77A1(void) { fx_bicrn(7); }
void c_FxOp78A1(void) { fx_bicrn(8); }
void c_FxOp79A1(void) { fx_bicrn(9); }
void c_FxOp7AA1(void) { fx_bicrn(10); }
void c_FxOp7BA1(void) { fx_bicrn(11); }
void c_FxOp7CA1(void) { fx_bicrn(12); }
void c_FxOp7DA1(void) { fx_bicrn(13); }
void c_FxOp7EA1(void) { fx_bicrn(14); }

/* ANDIRN */
void c_FxOp71A2(void) { fx_andirn(1); }
void c_FxOp72A2(void) { fx_andirn(2); }
void c_FxOp73A2(void) { fx_andirn(3); }
void c_FxOp74A2(void) { fx_andirn(4); }
void c_FxOp75A2(void) { fx_andirn(5); }
void c_FxOp76A2(void) { fx_andirn(6); }
void c_FxOp77A2(void) { fx_andirn(7); }
void c_FxOp78A2(void) { fx_andirn(8); }
void c_FxOp79A2(void) { fx_andirn(9); }
void c_FxOp7AA2(void) { fx_andirn(10); }
void c_FxOp7BA2(void) { fx_andirn(11); }
void c_FxOp7CA2(void) { fx_andirn(12); }
void c_FxOp7DA2(void) { fx_andirn(13); }
void c_FxOp7EA2(void) { fx_andirn(14); }
void c_FxOp7FA2(void) { fx_andirn(15); }

/* BICIRN */
void c_FxOp71A3(void) { fx_bicirn(1); }
void c_FxOp72A3(void) { fx_bicirn(2); }
void c_FxOp73A3(void) { fx_bicirn(3); }
void c_FxOp74A3(void) { fx_bicirn(4); }
void c_FxOp75A3(void) { fx_bicirn(5); }
void c_FxOp76A3(void) { fx_bicirn(6); }
void c_FxOp77A3(void) { fx_bicirn(7); }
void c_FxOp78A3(void) { fx_bicirn(8); }
void c_FxOp79A3(void) { fx_bicirn(9); }
void c_FxOp7AA3(void) { fx_bicirn(10); }
void c_FxOp7BA3(void) { fx_bicirn(11); }
void c_FxOp7CA3(void) { fx_bicirn(12); }
void c_FxOp7DA3(void) { fx_bicirn(13); }
void c_FxOp7EA3(void) { fx_bicirn(14); }
void c_FxOp7FA3(void) { fx_bicirn(15); }

/* --- OR / XOR / INC / DEC (chips/fxemu2.asm, base table) ------------------
 *
 * OR and XOR are plain 32-bit logic, like AND. INC and DEC are the odd ones:
 * they are `inc word[SfxR0+n*4]`, so they work 16-bit and *in place* on the
 * register, never through the esi/edi source/destination pointers, and the
 * upper half of the register survives the wrap. Neither touches carry or
 * overflow; x86 `inc`/`dec` leave CF alone and the asm has no seto/setc.
 */

static inline void fx_or(u4 const rhs)
{
    u4 const v = *FxSeamSrc | rhs;

    FxSeamPC++;
    *FxSeamDst = v;
    SfxSignZero = v;
}

static inline void fx_xor(u4 const rhs)
{
    u4 const v = *FxSeamSrc ^ rhs;

    FxSeamPC++;
    *FxSeamDst = v;
    SfxSignZero = v;
}

static inline void fx_incdec(u4 const n, u4 const delta)
{
    SfxR0[n] = fx_lo16(SfxR0[n], SfxR0[n] + delta);
    fx_fetchpipe();
    SfxSignZero = SfxR0[n];
    FxSeamPC++;
}

FX_ALU(fx_orrn, fx_or(SfxR0[n]))
FX_ALU(fx_ori, fx_or(n))
FX_ALU(fx_xorrn, fx_xor(SfxR0[n]))
FX_ALU(fx_xori, fx_xor(n))

/* INC/DEC do their own fetch, so they cannot go through FX_ALU. */
static void fx_incrn(u4 const n) { fx_incdec(n, 1); }
static void fx_decrn(u4 const n) { fx_incdec(n, (u4)-1); }

/* ORRN */
void c_FxOpC1(void) { fx_orrn(1); }
void c_FxOpC2(void) { fx_orrn(2); }
void c_FxOpC3(void) { fx_orrn(3); }
void c_FxOpC4(void) { fx_orrn(4); }
void c_FxOpC5(void) { fx_orrn(5); }
void c_FxOpC6(void) { fx_orrn(6); }
void c_FxOpC7(void) { fx_orrn(7); }
void c_FxOpC8(void) { fx_orrn(8); }
void c_FxOpC9(void) { fx_orrn(9); }
void c_FxOpCA(void) { fx_orrn(10); }
void c_FxOpCB(void) { fx_orrn(11); }
void c_FxOpCC(void) { fx_orrn(12); }
void c_FxOpCD(void) { fx_orrn(13); }
void c_FxOpCE(void) { fx_orrn(14); }

/* XORRN */
void c_FxOpC1A1(void) { fx_xorrn(1); }
void c_FxOpC2A1(void) { fx_xorrn(2); }
void c_FxOpC3A1(void) { fx_xorrn(3); }
void c_FxOpC4A1(void) { fx_xorrn(4); }
void c_FxOpC5A1(void) { fx_xorrn(5); }
void c_FxOpC6A1(void) { fx_xorrn(6); }
void c_FxOpC7A1(void) { fx_xorrn(7); }
void c_FxOpC8A1(void) { fx_xorrn(8); }
void c_FxOpC9A1(void) { fx_xorrn(9); }
void c_FxOpCAA1(void) { fx_xorrn(10); }
void c_FxOpCBA1(void) { fx_xorrn(11); }
void c_FxOpCCA1(void) { fx_xorrn(12); }
void c_FxOpCDA1(void) { fx_xorrn(13); }
void c_FxOpCEA1(void) { fx_xorrn(14); }

/* ORI */
void c_FxOpC1A2(void) { fx_ori(1); }
void c_FxOpC2A2(void) { fx_ori(2); }
void c_FxOpC3A2(void) { fx_ori(3); }
void c_FxOpC4A2(void) { fx_ori(4); }
void c_FxOpC5A2(void) { fx_ori(5); }
void c_FxOpC6A2(void) { fx_ori(6); }
void c_FxOpC7A2(void) { fx_ori(7); }
void c_FxOpC8A2(void) { fx_ori(8); }
void c_FxOpC9A2(void) { fx_ori(9); }
void c_FxOpCAA2(void) { fx_ori(10); }
void c_FxOpCBA2(void) { fx_ori(11); }
void c_FxOpCCA2(void) { fx_ori(12); }
void c_FxOpCDA2(void) { fx_ori(13); }
void c_FxOpCEA2(void) { fx_ori(14); }
void c_FxOpCFA2(void) { fx_ori(15); }

/* XORI */
void c_FxOpC1A3(void) { fx_xori(1); }
void c_FxOpC2A3(void) { fx_xori(2); }
void c_FxOpC3A3(void) { fx_xori(3); }
void c_FxOpC4A3(void) { fx_xori(4); }
void c_FxOpC5A3(void) { fx_xori(5); }
void c_FxOpC6A3(void) { fx_xori(6); }
void c_FxOpC7A3(void) { fx_xori(7); }
void c_FxOpC8A3(void) { fx_xori(8); }
void c_FxOpC9A3(void) { fx_xori(9); }
void c_FxOpCAA3(void) { fx_xori(10); }
void c_FxOpCBA3(void) { fx_xori(11); }
void c_FxOpCCA3(void) { fx_xori(12); }
void c_FxOpCDA3(void) { fx_xori(13); }
void c_FxOpCEA3(void) { fx_xori(14); }
void c_FxOpCFA3(void) { fx_xori(15); }

/* INCRN */
void c_FxOpD0(void) { fx_incrn(0); }
void c_FxOpD1(void) { fx_incrn(1); }
void c_FxOpD2(void) { fx_incrn(2); }
void c_FxOpD3(void) { fx_incrn(3); }
void c_FxOpD4(void) { fx_incrn(4); }
void c_FxOpD5(void) { fx_incrn(5); }
void c_FxOpD6(void) { fx_incrn(6); }
void c_FxOpD7(void) { fx_incrn(7); }
void c_FxOpD8(void) { fx_incrn(8); }
void c_FxOpD9(void) { fx_incrn(9); }
void c_FxOpDA(void) { fx_incrn(10); }
void c_FxOpDB(void) { fx_incrn(11); }
void c_FxOpDC(void) { fx_incrn(12); }
void c_FxOpDD(void) { fx_incrn(13); }

/* DECRN */
void c_FxOpE0(void) { fx_decrn(0); }
void c_FxOpE1(void) { fx_decrn(1); }
void c_FxOpE2(void) { fx_decrn(2); }
void c_FxOpE3(void) { fx_decrn(3); }
void c_FxOpE4(void) { fx_decrn(4); }
void c_FxOpE5(void) { fx_decrn(5); }
void c_FxOpE6(void) { fx_decrn(6); }
void c_FxOpE7(void) { fx_decrn(7); }
void c_FxOpE8(void) { fx_decrn(8); }
void c_FxOpE9(void) { fx_decrn(9); }
void c_FxOpEA(void) { fx_decrn(10); }
void c_FxOpEB(void) { fx_decrn(11); }
void c_FxOpEC(void) { fx_decrn(12); }
void c_FxOpED(void) { fx_decrn(13); }

/* --- MULT / UMULT (chips/fxemu2.asm, base table) --------------------------
 *
 * 8x8 multiplies: only the low byte of each operand takes part (`mov al,[esi]`
 * / `mov bl,...`), MULT is signed (imul) and UMULT unsigned (mul). The 16-bit
 * product is masked with `and eax,0FFFFh` before it is stored, so unlike the
 * add/subtract ops these zero the upper half of the destination rather than
 * preserving it. No carry or overflow is written.
 */

static inline void fx_mult(u4 const rhs, int const sign)
{
    u4 const a = *FxSeamSrc & 0xFFu;
    u4 const b = rhs & 0xFFu;
    u4 const v = (sign ? (u4)((s4)(s1)a * (s4)(s1)b) : a * b) & 0xFFFFu;

    FxSeamPC++;
    SfxSignZero = v;
    *FxSeamDst = v;
}

FX_ALU(fx_multrn, fx_mult(SfxR0[n], 1))
FX_ALU(fx_umultrn, fx_mult(SfxR0[n], 0))
FX_ALU(fx_multirn, fx_mult(n, 1))
FX_ALU(fx_umultirn, fx_mult(n, 0))

/* MULTRN */
void c_FxOp80(void) { fx_multrn(0); }
void c_FxOp81(void) { fx_multrn(1); }
void c_FxOp82(void) { fx_multrn(2); }
void c_FxOp83(void) { fx_multrn(3); }
void c_FxOp84(void) { fx_multrn(4); }
void c_FxOp85(void) { fx_multrn(5); }
void c_FxOp86(void) { fx_multrn(6); }
void c_FxOp87(void) { fx_multrn(7); }
void c_FxOp88(void) { fx_multrn(8); }
void c_FxOp89(void) { fx_multrn(9); }
void c_FxOp8A(void) { fx_multrn(10); }
void c_FxOp8B(void) { fx_multrn(11); }
void c_FxOp8C(void) { fx_multrn(12); }
void c_FxOp8D(void) { fx_multrn(13); }
void c_FxOp8E(void) { fx_multrn(14); }

/* UMULTRN */
void c_FxOp80A1(void) { fx_umultrn(0); }
void c_FxOp81A1(void) { fx_umultrn(1); }
void c_FxOp82A1(void) { fx_umultrn(2); }
void c_FxOp83A1(void) { fx_umultrn(3); }
void c_FxOp84A1(void) { fx_umultrn(4); }
void c_FxOp85A1(void) { fx_umultrn(5); }
void c_FxOp86A1(void) { fx_umultrn(6); }
void c_FxOp87A1(void) { fx_umultrn(7); }
void c_FxOp88A1(void) { fx_umultrn(8); }
void c_FxOp89A1(void) { fx_umultrn(9); }
void c_FxOp8AA1(void) { fx_umultrn(10); }
void c_FxOp8BA1(void) { fx_umultrn(11); }
void c_FxOp8CA1(void) { fx_umultrn(12); }
void c_FxOp8DA1(void) { fx_umultrn(13); }
void c_FxOp8EA1(void) { fx_umultrn(14); }

/* MULTIRN */
void c_FxOp80A2(void) { fx_multirn(0); }
void c_FxOp81A2(void) { fx_multirn(1); }
void c_FxOp82A2(void) { fx_multirn(2); }
void c_FxOp83A2(void) { fx_multirn(3); }
void c_FxOp84A2(void) { fx_multirn(4); }
void c_FxOp85A2(void) { fx_multirn(5); }
void c_FxOp86A2(void) { fx_multirn(6); }
void c_FxOp87A2(void) { fx_multirn(7); }
void c_FxOp88A2(void) { fx_multirn(8); }
void c_FxOp89A2(void) { fx_multirn(9); }
void c_FxOp8AA2(void) { fx_multirn(10); }
void c_FxOp8BA2(void) { fx_multirn(11); }
void c_FxOp8CA2(void) { fx_multirn(12); }
void c_FxOp8DA2(void) { fx_multirn(13); }
void c_FxOp8EA2(void) { fx_multirn(14); }
void c_FxOp8FA2(void) { fx_multirn(15); }

/* UMULTIRN */
void c_FxOp80A3(void) { fx_umultirn(0); }
void c_FxOp81A3(void) { fx_umultirn(1); }
void c_FxOp82A3(void) { fx_umultirn(2); }
void c_FxOp83A3(void) { fx_umultirn(3); }
void c_FxOp84A3(void) { fx_umultirn(4); }
void c_FxOp85A3(void) { fx_umultirn(5); }
void c_FxOp86A3(void) { fx_umultirn(6); }
void c_FxOp87A3(void) { fx_umultirn(7); }
void c_FxOp88A3(void) { fx_umultirn(8); }
void c_FxOp89A3(void) { fx_umultirn(9); }
void c_FxOp8AA3(void) { fx_umultirn(10); }
void c_FxOp8BA3(void) { fx_umultirn(11); }
void c_FxOp8CA3(void) { fx_umultirn(12); }
void c_FxOp8DA3(void) { fx_umultirn(13); }
void c_FxOp8EA3(void) { fx_umultirn(14); }
void c_FxOp8FA3(void) { fx_umultirn(15); }

/* --- TO rN / FROM rN / WITH rN (chips/fxemu2.asm, base table) -------------
 *
 * The plain forms: retarget the destination (TO), the source (FROM), or both
 * plus the B flag (WITH), run the next opcode with that in place, then put the
 * pointers back. WITH chains through the c table, the others through the base
 * table. Compare the b-group versions above, which additionally have a
 * version-B immediate-move path and maintain R15.
 */

static inline void fx_torn(u4 const n)
{
    fx_fetchpipe();
    FxSeamDst = SfxR0 + n;
    FxSeamPC++;
    FxDispatch(FxTable);
    FxSeamDst = SfxR0;
}

static inline void fx_fromrn(u4 const n)
{
    fx_fetchpipe();
    FxSeamSrc = SfxR0 + n;
    FxSeamPC++;
    FxDispatch(FxTable);
    FxSeamSrc = SfxR0;
}

static inline void fx_with(u4 const n)
{
    fx_fetchpipe();
    FxSeamSrc = SfxR0 + n;
    FxSeamDst = SfxR0 + n;
    SfxB = 1;
    FxSeamPC++;
    FxDispatch(FxTablec);
    FxSeamSrc = SfxR0;
    FxSeamDst = SfxR0;
    SfxB = 0;
}

/* TORN */
void c_FxOp10(void) { fx_torn(0); }
void c_FxOp11(void) { fx_torn(1); }
void c_FxOp12(void) { fx_torn(2); }
void c_FxOp13(void) { fx_torn(3); }
void c_FxOp14(void) { fx_torn(4); }
void c_FxOp15(void) { fx_torn(5); }
void c_FxOp16(void) { fx_torn(6); }
void c_FxOp17(void) { fx_torn(7); }
void c_FxOp18(void) { fx_torn(8); }
void c_FxOp19(void) { fx_torn(9); }
void c_FxOp1A(void) { fx_torn(10); }
void c_FxOp1B(void) { fx_torn(11); }
void c_FxOp1C(void) { fx_torn(12); }
void c_FxOp1D(void) { fx_torn(13); }

/* WITH */
void c_FxOp20(void) { fx_with(0); }
void c_FxOp21(void) { fx_with(1); }
void c_FxOp22(void) { fx_with(2); }
void c_FxOp23(void) { fx_with(3); }
void c_FxOp24(void) { fx_with(4); }
void c_FxOp25(void) { fx_with(5); }
void c_FxOp26(void) { fx_with(6); }
void c_FxOp27(void) { fx_with(7); }
void c_FxOp28(void) { fx_with(8); }
void c_FxOp29(void) { fx_with(9); }
void c_FxOp2A(void) { fx_with(10); }
void c_FxOp2B(void) { fx_with(11); }
void c_FxOp2C(void) { fx_with(12); }
void c_FxOp2D(void) { fx_with(13); }

/* FROMRN */
void c_FxOpB0(void) { fx_fromrn(0); }
void c_FxOpB1(void) { fx_fromrn(1); }
void c_FxOpB2(void) { fx_fromrn(2); }
void c_FxOpB3(void) { fx_fromrn(3); }
void c_FxOpB4(void) { fx_fromrn(4); }
void c_FxOpB5(void) { fx_fromrn(5); }
void c_FxOpB6(void) { fx_fromrn(6); }
void c_FxOpB7(void) { fx_fromrn(7); }
void c_FxOpB8(void) { fx_fromrn(8); }
void c_FxOpB9(void) { fx_fromrn(9); }
void c_FxOpBA(void) { fx_fromrn(10); }
void c_FxOpBB(void) { fx_fromrn(11); }
void c_FxOpBC(void) { fx_fromrn(12); }
void c_FxOpBD(void) { fx_fromrn(13); }
void c_FxOpBE(void) { fx_fromrn(14); }

/* --- Load / store (chips/fxemu2.asm, base table) --------------------------
 *
 * The address is the raw register value added to SfxRAMMem; SfxLastRamAdr
 * records the absolute address for the caching logic elsewhere. None of these
 * touch SfxSignZero or any flag.
 *
 * The word forms address the second byte as addr^1, not addr+1: SuperFX RAM is
 * word-interleaved, so the high byte lives at the sibling even/odd offset.
 */

static inline u1* fx_ram(u4 const addr)
{
    return (u1*)(uintptr_t)(SfxRAMMem + addr);
}

static inline void fx_stw(u4 const n)
{
    u4 const addr = SfxR0[n];
    u4 const val = *FxSeamSrc;

    SfxLastRamAdr = SfxRAMMem + addr;
    fx_fetchpipe();
    *fx_ram(addr) = (u1)val;
    FxSeamPC++;
    *fx_ram(addr ^ 1) = (u1)(val >> 8);
}

static inline void fx_stb(u4 const n)
{
    u4 const addr = SfxR0[n];

    fx_fetchpipe();
    SfxLastRamAdr = SfxRAMMem + addr;
    *fx_ram(addr) = (u1)*FxSeamSrc;
    FxSeamPC++;
}

static inline void fx_ldw(u4 const n)
{
    u4 const addr = SfxR0[n];

    SfxLastRamAdr = SfxRAMMem + addr;
    fx_fetchpipe();
    /* `and edx,0FFFFh` lands between the two byte loads, so the result is the
       two bytes zero-extended, whatever edx held before. */
    FxSeamPC++;
    *FxSeamDst = (u4)*fx_ram(addr) | ((u4)*fx_ram(addr ^ 1) << 8);
}

static inline void fx_ldb(u4 const n)
{
    u4 const addr = SfxR0[n];

    fx_fetchpipe();
    SfxLastRamAdr = SfxRAMMem + addr;
    FxSeamPC++;
    *FxSeamDst = *fx_ram(addr);
}

/* Each of these does its own fetch at the point the macro had it, so they
   cannot go through FX_ALU (which fetches first). */
static void fx_stwrn(u4 const n) { fx_stw(n); }
static void fx_stbrn(u4 const n) { fx_stb(n); }
static void fx_ldwrn(u4 const n) { fx_ldw(n); }
static void fx_ldbrn(u4 const n) { fx_ldb(n); }

/* STWRN */
void c_FxOp30(void) { fx_stwrn(0); }
void c_FxOp31(void) { fx_stwrn(1); }
void c_FxOp32(void) { fx_stwrn(2); }
void c_FxOp33(void) { fx_stwrn(3); }
void c_FxOp34(void) { fx_stwrn(4); }
void c_FxOp35(void) { fx_stwrn(5); }
void c_FxOp36(void) { fx_stwrn(6); }
void c_FxOp37(void) { fx_stwrn(7); }
void c_FxOp38(void) { fx_stwrn(8); }
void c_FxOp39(void) { fx_stwrn(9); }
void c_FxOp3A(void) { fx_stwrn(10); }
void c_FxOp3B(void) { fx_stwrn(11); }

/* STBRN */
void c_FxOp30A1(void) { fx_stbrn(0); }
void c_FxOp31A1(void) { fx_stbrn(1); }
void c_FxOp32A1(void) { fx_stbrn(2); }
void c_FxOp33A1(void) { fx_stbrn(3); }
void c_FxOp34A1(void) { fx_stbrn(4); }
void c_FxOp35A1(void) { fx_stbrn(5); }
void c_FxOp36A1(void) { fx_stbrn(6); }
void c_FxOp37A1(void) { fx_stbrn(7); }
void c_FxOp38A1(void) { fx_stbrn(8); }
void c_FxOp39A1(void) { fx_stbrn(9); }
void c_FxOp3AA1(void) { fx_stbrn(10); }
void c_FxOp3BA1(void) { fx_stbrn(11); }

/* LDWRN */
void c_FxOp40(void) { fx_ldwrn(0); }
void c_FxOp41(void) { fx_ldwrn(1); }
void c_FxOp42(void) { fx_ldwrn(2); }
void c_FxOp43(void) { fx_ldwrn(3); }
void c_FxOp44(void) { fx_ldwrn(4); }
void c_FxOp45(void) { fx_ldwrn(5); }
void c_FxOp46(void) { fx_ldwrn(6); }
void c_FxOp47(void) { fx_ldwrn(7); }
void c_FxOp48(void) { fx_ldwrn(8); }
void c_FxOp49(void) { fx_ldwrn(9); }
void c_FxOp4A(void) { fx_ldwrn(10); }
void c_FxOp4B(void) { fx_ldwrn(11); }

/* LDBRN */
void c_FxOp40A1(void) { fx_ldbrn(0); }
void c_FxOp41A1(void) { fx_ldbrn(1); }
void c_FxOp42A1(void) { fx_ldbrn(2); }
void c_FxOp43A1(void) { fx_ldbrn(3); }
void c_FxOp44A1(void) { fx_ldbrn(4); }
void c_FxOp45A1(void) { fx_ldbrn(5); }
void c_FxOp46A1(void) { fx_ldbrn(6); }
void c_FxOp47A1(void) { fx_ldbrn(7); }
void c_FxOp48A1(void) { fx_ldbrn(8); }
void c_FxOp49A1(void) { fx_ldbrn(9); }
void c_FxOp4AA1(void) { fx_ldbrn(10); }
void c_FxOp4BA1(void) { fx_ldbrn(11); }

/* --- Immediate loads and short/long memory moves --------------------------
 *
 * These take their operand from the instruction stream rather than a register,
 * so they advance the program counter by more than one and prefetch the next
 * opcode from past the immediate.
 *
 * Watch the store widths, which differ across the group: IBT and the LM forms
 * write `ax`/`dx`/`bx`, i.e. 16-bit, leaving the register's upper half intact,
 * while IWT writes the full `eax` and so zeroes it. LM/SM address RAM
 * word-interleaved (addr^1) like LDW/STW; LMS/SMS instead do one plain 16-bit
 * access at 2*imm8.
 */

/* Write only the low half of a register, as `mov [SfxR0+n*4],ax` does. */
static inline void fx_set_lo16(u4 const n, u4 const v)
{
    SfxR0[n] = fx_lo16(SfxR0[n], v);
}

static inline void fx_ibt(u4 const n)
{
    s1 const imm = (s1)*FxSeamPC;

    FxSeamCX = (FxSeamCX & ~0xFFu) | FxSeamPC[1];
    FxSeamPC += 2;
    fx_set_lo16(n, (u4)(s4)imm);
}

static inline void fx_iwt(u4 const n)
{
    u4 const imm = (u4)FxSeamPC[0] | ((u4)FxSeamPC[1] << 8);

    FxSeamCX = (FxSeamCX & ~0xFFu) | FxSeamPC[2];
    FxSeamPC += 3;
    SfxR0[n] = imm; /* full 32-bit store: the upper half is zeroed */
}

static inline void fx_lm(u4 const n)
{
    u4 const addr = (u4)FxSeamPC[0] | ((u4)FxSeamPC[1] << 8);

    FxSeamCX = (FxSeamCX & ~0xFFu) | FxSeamPC[2];
    SfxLastRamAdr = SfxRAMMem + addr;
    FxSeamPC += 3;
    fx_set_lo16(n, (u4)*fx_ram(addr) | ((u4)*fx_ram(addr ^ 1) << 8));
}

static inline void fx_sm(u4 const n)
{
    u4 const val = SfxR0[n];
    u4 const addr = (u4)FxSeamPC[0] | ((u4)FxSeamPC[1] << 8);

    FxSeamCX = (FxSeamCX & ~0xFFu) | FxSeamPC[2];
    SfxLastRamAdr = SfxRAMMem + addr;
    *fx_ram(addr) = (u1)val;
    FxSeamPC += 3;
    *fx_ram(addr ^ 1) = (u1)(val >> 8);
}

/* The short forms take an 8-bit offset scaled by two, and access RAM as one
   16-bit word rather than through the addr^1 interleave. The address is always
   even, so addr^1 and addr+1 coincide here and either spelling is correct. */
static inline void fx_lms(u4 const n)
{
    u4 const addr = (u4)*FxSeamPC * 2u;

    FxSeamCX = (FxSeamCX & ~0xFFu) | FxSeamPC[1];
    SfxLastRamAdr = SfxRAMMem + addr;
    FxSeamPC += 2;
    fx_set_lo16(n, (u4)fx_ram(addr)[0] | ((u4)fx_ram(addr)[1] << 8));
}

static inline void fx_sms(u4 const n)
{
    u4 const addr = (u4)*FxSeamPC * 2u;
    u4 const val = SfxR0[n];

    FxSeamCX = (FxSeamCX & ~0xFFu) | FxSeamPC[1];
    SfxLastRamAdr = SfxRAMMem + addr;
    FxSeamPC += 2;
    fx_ram(addr)[0] = (u1)val;
    fx_ram(addr)[1] = (u1)(val >> 8);
}

/* Their own fetch is at a stream offset, so FX_ALU does not apply. */
static void fx_ibtrn(u4 const n) { fx_ibt(n); }
static void fx_iwtrn(u4 const n) { fx_iwt(n); }
static void fx_lmrn(u4 const n) { fx_lm(n); }
static void fx_smrn(u4 const n) { fx_sm(n); }
static void fx_lmsrn(u4 const n) { fx_lms(n); }
static void fx_smsrn(u4 const n) { fx_sms(n); }

/* IBTRN */
void c_FxOpA0(void) { fx_ibtrn(0); }
void c_FxOpA1(void) { fx_ibtrn(1); }
void c_FxOpA2(void) { fx_ibtrn(2); }
void c_FxOpA3(void) { fx_ibtrn(3); }
void c_FxOpA4(void) { fx_ibtrn(4); }
void c_FxOpA5(void) { fx_ibtrn(5); }
void c_FxOpA6(void) { fx_ibtrn(6); }
void c_FxOpA7(void) { fx_ibtrn(7); }
void c_FxOpA8(void) { fx_ibtrn(8); }
void c_FxOpA9(void) { fx_ibtrn(9); }
void c_FxOpAA(void) { fx_ibtrn(10); }
void c_FxOpAB(void) { fx_ibtrn(11); }
void c_FxOpAC(void) { fx_ibtrn(12); }
void c_FxOpAD(void) { fx_ibtrn(13); }

/* LMSRN */
void c_FxOpA0A1(void) { fx_lmsrn(0); }
void c_FxOpA1A1(void) { fx_lmsrn(1); }
void c_FxOpA2A1(void) { fx_lmsrn(2); }
void c_FxOpA3A1(void) { fx_lmsrn(3); }
void c_FxOpA4A1(void) { fx_lmsrn(4); }
void c_FxOpA5A1(void) { fx_lmsrn(5); }
void c_FxOpA6A1(void) { fx_lmsrn(6); }
void c_FxOpA7A1(void) { fx_lmsrn(7); }
void c_FxOpA8A1(void) { fx_lmsrn(8); }
void c_FxOpA9A1(void) { fx_lmsrn(9); }
void c_FxOpAAA1(void) { fx_lmsrn(10); }
void c_FxOpABA1(void) { fx_lmsrn(11); }
void c_FxOpACA1(void) { fx_lmsrn(12); }
void c_FxOpADA1(void) { fx_lmsrn(13); }

/* SMSRN */
void c_FxOpA0A2(void) { fx_smsrn(0); }
void c_FxOpA1A2(void) { fx_smsrn(1); }
void c_FxOpA2A2(void) { fx_smsrn(2); }
void c_FxOpA3A2(void) { fx_smsrn(3); }
void c_FxOpA4A2(void) { fx_smsrn(4); }
void c_FxOpA5A2(void) { fx_smsrn(5); }
void c_FxOpA6A2(void) { fx_smsrn(6); }
void c_FxOpA7A2(void) { fx_smsrn(7); }
void c_FxOpA8A2(void) { fx_smsrn(8); }
void c_FxOpA9A2(void) { fx_smsrn(9); }
void c_FxOpAAA2(void) { fx_smsrn(10); }
void c_FxOpABA2(void) { fx_smsrn(11); }
void c_FxOpACA2(void) { fx_smsrn(12); }
void c_FxOpADA2(void) { fx_smsrn(13); }
void c_FxOpAEA2(void) { fx_smsrn(14); }

/* IWTRN */
void c_FxOpF0(void) { fx_iwtrn(0); }
void c_FxOpF1(void) { fx_iwtrn(1); }
void c_FxOpF2(void) { fx_iwtrn(2); }
void c_FxOpF3(void) { fx_iwtrn(3); }
void c_FxOpF4(void) { fx_iwtrn(4); }
void c_FxOpF5(void) { fx_iwtrn(5); }
void c_FxOpF6(void) { fx_iwtrn(6); }
void c_FxOpF7(void) { fx_iwtrn(7); }
void c_FxOpF8(void) { fx_iwtrn(8); }
void c_FxOpF9(void) { fx_iwtrn(9); }
void c_FxOpFA(void) { fx_iwtrn(10); }
void c_FxOpFB(void) { fx_iwtrn(11); }
void c_FxOpFC(void) { fx_iwtrn(12); }
void c_FxOpFD(void) { fx_iwtrn(13); }

/* LMRN */
void c_FxOpF0A1(void) { fx_lmrn(0); }
void c_FxOpF1A1(void) { fx_lmrn(1); }
void c_FxOpF2A1(void) { fx_lmrn(2); }
void c_FxOpF3A1(void) { fx_lmrn(3); }
void c_FxOpF4A1(void) { fx_lmrn(4); }
void c_FxOpF5A1(void) { fx_lmrn(5); }
void c_FxOpF6A1(void) { fx_lmrn(6); }
void c_FxOpF7A1(void) { fx_lmrn(7); }
void c_FxOpF8A1(void) { fx_lmrn(8); }
void c_FxOpF9A1(void) { fx_lmrn(9); }
void c_FxOpFAA1(void) { fx_lmrn(10); }
void c_FxOpFBA1(void) { fx_lmrn(11); }
void c_FxOpFCA1(void) { fx_lmrn(12); }
void c_FxOpFDA1(void) { fx_lmrn(13); }

/* SMRN */
void c_FxOpF0A2(void) { fx_smrn(0); }
void c_FxOpF1A2(void) { fx_smrn(1); }
void c_FxOpF2A2(void) { fx_smrn(2); }
void c_FxOpF3A2(void) { fx_smrn(3); }
void c_FxOpF4A2(void) { fx_smrn(4); }
void c_FxOpF5A2(void) { fx_smrn(5); }
void c_FxOpF6A2(void) { fx_smrn(6); }
void c_FxOpF7A2(void) { fx_smrn(7); }
void c_FxOpF8A2(void) { fx_smrn(8); }
void c_FxOpF9A2(void) { fx_smrn(9); }
void c_FxOpFAA2(void) { fx_smrn(10); }
void c_FxOpFBA2(void) { fx_smrn(11); }
void c_FxOpFCA2(void) { fx_smrn(12); }
void c_FxOpFDA2(void) { fx_smrn(13); }
void c_FxOpFEA2(void) { fx_smrn(14); }

/* --- CACHE, LINK and the jumps (chips/fxemu2.asm, base table) -------------
 *
 * LJMP switches code bank: it indexes SfxMemTable with the low 7 bits of the
 * register to get the new bank base, then re-runs the CACHE opcode for its
 * cache-invalidate side effect. The assembly did that with a literal
 * `push ecx / call FxOp02 / pop ecx / dec ebp`, so the opcode byte and the
 * program counter CACHE leaves behind are both discarded and only SfxCBR /
 * SfxCacheActive survive.
 */

void FlushCache(void); /* chips/fxemu2.asm; currently a stub */

/* CACHE: point the cache at the 16-byte-aligned block holding the program
   counter, unless it is already there or a cache load is in progress. */
static inline void fx_cache(void)
{
    u4 const base = (fx_pc_rel()) & 0xFFF0u;

    fx_fetchpipe();
    if (SfxCBR != base && (SfxCacheActive & 0xFFu) != 1) {
        SfxCBR = base;
        SfxCacheActive = 1;
        FlushCache();
    }
    FxSeamPC++;
}

void c_FxOp02(void) { fx_cache(); }

/* LINK: stash the return address, PC-relative plus the operand, in R11. The
   store is 16-bit, so R11's upper half is left alone. */
static inline void fx_link(u4 const n)
{
    u4 const ret = fx_pc_rel() + n;

    fx_fetchpipe();
    fx_set_lo16(11, ret);
    FxSeamPC++;
}

static inline void fx_jmp(u4 const n)
{
    fx_fetchpipe();
    FxSeamPC = (u1*)(uintptr_t)(SfxCPB + SfxR0[n]);
}

static inline void fx_ljmp(u4 const n)
{
    u4 const bank = SfxR0[n] & 0x7Fu;
    u4 saved_cx;

    fx_fetchpipe();
    *(u1*)&SfxPBR = (u1)bank; /* byte store: the upper three are kept */
    SfxCPB = SfxMemTable[bank];
    FxSeamPC = (u1*)(uintptr_t)(SfxCPB + *FxSeamSrc);
    SfxCacheActive = 0;

    saved_cx = FxSeamCX;
    fx_cache();
    FxSeamCX = saved_cx; /* pop ecx */
    FxSeamPC--; /* dec ebp */
}

static void fx_linkn(u4 const n) { fx_link(n); }
static void fx_jmprn(u4 const n) { fx_jmp(n); }
static void fx_ljmprn(u4 const n) { fx_ljmp(n); }

/* LINK */
void c_FxOp91(void) { fx_linkn(1); }
void c_FxOp92(void) { fx_linkn(2); }
void c_FxOp93(void) { fx_linkn(3); }
void c_FxOp94(void) { fx_linkn(4); }

/* JMPRN */
void c_FxOp98(void) { fx_jmprn(8); }
void c_FxOp99(void) { fx_jmprn(9); }
void c_FxOp9A(void) { fx_jmprn(10); }
void c_FxOp9B(void) { fx_jmprn(11); }
void c_FxOp9C(void) { fx_jmprn(12); }
void c_FxOp9D(void) { fx_jmprn(13); }

/* LJMPRN */
void c_FxOp98A1(void) { fx_ljmprn(8); }
void c_FxOp99A1(void) { fx_ljmprn(9); }
void c_FxOp9AA1(void) { fx_ljmprn(10); }
void c_FxOp9BA1(void) { fx_ljmprn(11); }
void c_FxOp9CA1(void) { fx_ljmprn(12); }
void c_FxOp9DA1(void) { fx_ljmprn(13); }

/* --- Single-register bit and shift ops (chips/fxemu2.asm, base table) -----
 *
 * All of these read the source register and write the destination, with no
 * operand. Note how much the write widths vary: the shifts work on `ax` and so
 * keep the register's upper half, while SEX, LOB and HIB write the full `eax`
 * and clear it. The flag stores are byte-wide (`mov [SfxCarry],al`), leaving
 * the upper three bytes of SfxCarry alone.
 */

void c_FxOp01(void) /* NOP */
{
    fx_fetchpipe();
    FxSeamPC++;
}

void c_FxOp4D(void) /* SWAP: exchange the two bytes of the low half */
{
    u4 const a = *FxSeamSrc;
    u4 const v = fx_lo16(a, ((a & 0xFFu) << 8) | ((a >> 8) & 0xFFu));

    fx_fetchpipe();
    FxSeamPC++;
    SfxSignZero = v;
    *FxSeamDst = v;
}

void c_FxOp4F(void) /* NOT: invert the low 16 bits only */
{
    u4 const v = *FxSeamSrc ^ 0xFFFFu;

    fx_fetchpipe();
    FxSeamPC++;
    SfxSignZero = v;
    *FxSeamDst = v;
}

void c_FxOp95(void) /* SEX: sign-extend the low byte to 16 bits */
{
    u4 const v = (u4)(s4)(s1)(u1)*FxSeamSrc & 0xFFFFu;

    fx_fetchpipe();
    FxSeamPC++;
    *FxSeamDst = v;
    SfxSignZero = v;
}

/* Arithmetic shift right of the low half; the bit shifted out becomes carry.
   Replicating bit 15 is what `sar ax,1` does. */
static inline void fx_asr(void)
{
    u4 const lo = *FxSeamSrc & 0xFFFFu;
    u4 const v = fx_lo16(*FxSeamSrc, (lo >> 1) | (lo & 0x8000u));

    fx_set_carry(lo & 1);
    FxSeamPC++;
    *FxSeamDst = v;
    SfxSignZero = v;
}

void c_FxOp96(void) /* ASR */
{
    fx_fetchpipe();
    fx_asr();
}

void c_FxOp96A1(void) /* DIV2: ASR, except -1 divides to 0 */
{
    fx_fetchpipe();
    if ((*FxSeamSrc & 0xFFFFu) != 0xFFFFu) {
        fx_asr();
        return;
    }
    fx_set_carry(1);
    FxSeamPC++;
    *FxSeamDst = 0; /* `xor eax,eax`: the whole register, not just the low half */
    SfxSignZero = 0;
}

void c_FxOp97(void) /* ROR: rotate the low half right through carry */
{
    u4 const a = *FxSeamSrc;
    u4 const v = fx_lo16(a, ((SfxCarry & 1u) << 15) | ((a & 0xFFFFu) >> 1));

    fx_fetchpipe();
    fx_set_carry(a & 1);
    FxSeamPC++;
    *FxSeamDst = v;
    SfxSignZero = v;
}

void c_FxOp9E(void) /* LOB: keep the low byte */
{
    u4 const v = *FxSeamSrc & 0xFFu;

    fx_fetchpipe();
    FxSeamPC++;
    *FxSeamDst = v;
    /* The flag word takes the byte shifted up, so the sign test still looks at
       bit 15. */
    SfxSignZero = v << 8;
}

void c_FxOpC0(void) /* HIB: move the high byte down */
{
    u4 const hi = *FxSeamSrc & 0xFF00u;

    fx_fetchpipe();
    SfxSignZero = hi; /* set before the shift, so it keeps bit 15 */
    FxSeamPC++;
    *FxSeamDst = hi >> 8;
}

/* --- Shifts, LOOP, the 16x16 multiplies and the R14 forms -----------------
 */

void c_FxOp03(void) /* LSR: logical shift right of the low half */
{
    u4 const lo = *FxSeamSrc & 0xFFFFu;
    u4 const v = fx_lo16(*FxSeamSrc, lo >> 1);

    fx_fetchpipe();
    fx_set_carry(lo & 1);
    FxSeamPC++;
    *FxSeamDst = v;
    SfxSignZero = v;
}

void c_FxOp04(void) /* ROL: rotate the low half left through carry */
{
    u4 const carry = SfxCarry;
    u4 const a = *FxSeamSrc;
    u4 const v = fx_lo16(a, ((a & 0xFFFFu) << 1) | (carry & 1u));

    fx_fetchpipe();
    /* `shr byte[SfxCarry],1` then `rcl byte[SfxCarry],1` puts the bit shifted
       out of ax into bit 0 and leaves the rest of the byte as it was. */
    *(u1*)&SfxCarry = (u1)((carry & 0xFEu) | ((a >> 15) & 1u));
    FxSeamPC++;
    *FxSeamDst = v;
    SfxSignZero = v;
}

void c_FxOp3C(void) /* LOOP: decrement R12, branch to R13 while non-zero */
{
    u4 v;

    SfxR0[12] = fx_lo16(SfxR0[12], SfxR0[12] - 1);
    fx_fetchpipe();
    v = SfxR0[12];
    SfxSignZero = v;
    /* `or eax,eax` tests all 32 bits, so a non-zero upper half keeps looping
       even once the counter's low half reaches zero. */
    if (v != 0) {
        FxSeamPC = (u1*)(uintptr_t)(SfxCPB + SfxR0[13]);
        return;
    }
    FxSeamPC++;
}

/* FMULT/LMULT: signed 16x16 giving a 32-bit product. The destination takes the
   upper half; carry is bit 15 of the lower half. */
static inline void fx_fmult(int const keep_low)
{
    u4 const prod = (u4)((s4)(s2)(u2)*FxSeamSrc * (s4)(s2)(u2)SfxR0[6]);
    u4 const hi = (prod >> 16) & 0xFFFFu;

    fx_fetchpipe();
    FxSeamPC++;
    *FxSeamDst = hi;
    if (keep_low) {
        fx_set_lo16(4, prod); /* LMULT also keeps the low half in R4 */
    }
    SfxSignZero = hi;
    fx_set_carry((prod >> 15) & 1u);
}

void c_FxOp9F(void) { fx_fmult(0); }
void c_FxOp9FA1(void) { fx_fmult(1); }

void c_FxOpAE(void) /* IBT R14: immediate byte into R14, then refresh the pointer */
{
    s1 const imm = (s1)*FxSeamPC;

    FxSeamCX = (FxSeamCX & ~0xFFu) | FxSeamPC[1];
    FxSeamPC += 2;
    fx_set_lo16(14, (u4)(s4)imm);
    fx_update_r14();
}

void c_FxOpAF(void) /* branch to a sign-extended byte displacement from the bank base */
{
    u4 const imm = (u4)(s4)(s1)*FxSeamPC & 0xFFFFu;

    FxSeamCX = (FxSeamCX & ~0xFFu) | FxSeamPC[1];
    FxSeamPC = (u1*)(uintptr_t)(SfxCPB + imm);
}

void c_FxOpDE(void) /* INC R14 */
{
    u4 v;

    fx_fetchpipe();
    v = fx_lo16(SfxR0[14], SfxR0[14] + 1);
    SfxR0[14] = v;
    SfxSignZero = v;
    FxSeamPC++;
    fx_update_r14();
}

void c_FxOpEE(void) /* DEC R14 */
{
    SfxR0[14] = fx_lo16(SfxR0[14], SfxR0[14] - 1);
    fx_fetchpipe();
    SfxSignZero = SfxR0[14];
    fx_update_r14();
    FxSeamPC++;
}

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

/* The plain form. Only the b group additionally stamps R15 (fx_alt_b). */
static inline void fx_alt(u4 const mode, u4 const* const table)
{
    fx_fetchpipe();
    SfxB = 0;
    FxSeamCX |= mode << 8;
    FxSeamPC++;
    FxDispatch(table);
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

void c_FxOpc3D(void) { fx_alt(1, FxTablec); }
void c_FxOpc3E(void) { fx_alt(2, FxTablec); }
void c_FxOpc3F(void) { fx_alt(3, FxTablec); }

/* --- Base-table branches and register-select edge cases -------------------
 *
 * The same shapes as the b and c groups above, but chaining through the base
 * table. The R14 and R15 forms are spelled out because each has a tail the
 * plain TO/FROM/WITH do not: R14 refreshes the ROM pointer, R15 rebuilds the
 * program counter from the register the nested opcode may have moved.
 */

FX_BRANCHES(, FxTable)

void c_FxOp1E(void) /* TO R14 */
{
    fx_fetchpipe();
    FxSeamDst = SfxR0 + 14;
    FxSeamPC++;
    FxDispatch(FxTable);
    FxSeamDst = SfxR0;
    fx_update_r14();
}

void c_FxOp1F(void) /* TO R15: the nested opcode's write to R15 is the jump */
{
    fx_fetchpipe();
    FxSeamDst = SfxR0 + 15;
    FxSeamPC++;
    FxDispatch(FxTable);
    FxSeamPC = (u1*)(uintptr_t)(SfxCPB + SfxR0[15]);
    FxSeamDst = SfxR0;
}

void c_FxOp2E(void) /* WITH R14 */
{
    fx_fetchpipe();
    FxSeamSrc = SfxR0 + 14;
    FxSeamDst = SfxR0 + 14;
    SfxB = 1;
    FxSeamPC++;
    FxDispatch(FxTablec);
    SfxB = 0;
    FxSeamSrc = SfxR0;
    FxSeamDst = SfxR0;
    fx_update_r14();
}

void c_FxOp2F(void) /* WITH R15 */
{
    fx_fetchpipe();
    FxSeamSrc = SfxR0 + 15;
    FxSeamDst = SfxR0 + 15;
    SfxB = 1;
    FxSeamPC++;
    SfxR0[15] = fx_pc_rel();
    /* withr15sk lets the nested opcode say it already set the program counter
       itself, in which case R15 must not be applied a second time. */
    withr15sk = 0;
    FxDispatch(FxTableb);
    if (withr15sk != 1) {
        FxSeamPC = (u1*)(uintptr_t)(SfxCPB + SfxR0[15]);
    }
    SfxB = 0;
    FxSeamSrc = SfxR0;
    FxSeamDst = SfxR0;
}

/* All three chain through the base table here, and none of them stamps R15. */
void c_FxOp3D(void) { fx_alt(1, FxTable); }
void c_FxOp3E(void) { fx_alt(2, FxTable); }
void c_FxOp3F(void) { fx_alt(3, FxTable); }

void c_FxOpBF(void) /* FROM R15 */
{
    fx_fetchpipe();
    FxSeamSrc = SfxR0 + 15;
    FxSeamPC++;
    SfxR0[15] = fx_pc_rel();
    FxDispatch(FxTableb);
    FxSeamSrc = SfxR0;
}

/* --- The R15 operand forms and SBK ---------------------------------------
 *
 * $x F with the register field at 15: the second operand is the *live*
 * program counter (`mov ebx,ebp / sub ebx,[SfxCPB]`), taken before the counter
 * advances, rather than the stored R15.
 */

void c_FxOp5F(void) /* ADD R15 */
{
    fx_fetchpipe();
    fx_add(fx_pc_rel(), 0);
}

void c_FxOp5FA1(void) /* ADC R15 */
{
    fx_fetchpipe();
    fx_add(fx_pc_rel(), SfxCarry & 1);
}

void c_FxOp6F(void) /* SUB R15 */
{
    fx_fetchpipe();
    fx_sub(fx_pc_rel(), 0);
}

void c_FxOp6FA1(void) /* SBC R15 */
{
    fx_fetchpipe();
    fx_sub(fx_pc_rel(), (SfxCarry & 0xFFu) == 0);
}

void c_FxOp6FA3(void) /* CMP R15 */
{
    fx_fetchpipe();
    fx_cmp(fx_pc_rel());
}

void c_FxOp7F(void) /* AND R15 */
{
    fx_fetchpipe();
    fx_and(fx_pc_rel());
}

void c_FxOp7FA1(void) /* BIC R15 */
{
    fx_fetchpipe();
    fx_and(fx_pc_rel() ^ 0xFFFFu);
}

void c_FxOp8F(void) /* MULT R15 */
{
    fx_fetchpipe();
    fx_mult(fx_pc_rel(), 1);
}

void c_FxOp8FA1(void) /* UMULT R15 */
{
    fx_fetchpipe();
    fx_mult(fx_pc_rel(), 0);
}

void c_FxOp90(void) /* SBK: store a word back to the last RAM address used */
{
    u4 const addr = SfxLastRamAdr - SfxRAMMem;
    u4 const val = *FxSeamSrc;

    fx_fetchpipe();
    *fx_ram(addr) = (u1)val;
    FxSeamPC++;
    *fx_ram(addr ^ 1) = (u1)(val >> 8);
}

/* --- ROM reads, bank registers and the remaining R14/R15 forms ------------
 */

/* The byte R14 currently points at. The assembly reads a dword and masks it,
   which on little-endian x86 is the same byte. */
static inline u1 fx_rom_byte(void)
{
    return *(u1 const*)(uintptr_t)SfxRomBuffer;
}

void c_FxOpEF(void) /* GETB: zero-extended byte from ROM */
{
    fx_fetchpipe();
    FxSeamPC++;
    *FxSeamDst = fx_rom_byte();
}

void c_FxOpEFA1(void) /* GETBH: the byte becomes the high half */
{
    u4 const v = (*FxSeamSrc & 0xFFu) | ((u4)fx_rom_byte() << 8);

    fx_fetchpipe();
    FxSeamPC++;
    *FxSeamDst = v;
}

void c_FxOpEFA2(void) /* GETBL: the byte becomes the low half */
{
    u4 const v = (*FxSeamSrc & 0xFF00u) | fx_rom_byte();

    fx_fetchpipe();
    FxSeamPC++;
    *FxSeamDst = v;
}

void c_FxOpEFA3(void) /* GETBS: sign-extended, stored 16-bit */
{
    u4 const v = (u4)(s4)(s1)fx_rom_byte();

    fx_fetchpipe();
    FxSeamPC++;
    *FxSeamDst = fx_lo16(*FxSeamDst, v); /* `mov [edi],ax` keeps the upper half */
}

void c_FxOpDFA2(void) /* RAMB: select the RAM bank */
{
    u4 const bank = *FxSeamSrc & (SfxnRamBanks - 1);

    fx_fetchpipe();
    SfxRAMBR = bank;
    SfxRAMMem = (bank << 16) + (u4)(uintptr_t)sfxramdata;
    FxSeamPC++;
}

void c_FxOpDFA3(void) /* ROMB: select the ROM bank */
{
    u4 const bank = *FxSeamSrc & 0x7Fu;

    fx_fetchpipe();
    SfxROMBR = bank;
    SfxCROM = SfxMemTable[bank];
    FxSeamPC++;
}

void c_FxOpCF(void) /* OR R15 */
{
    fx_fetchpipe();
    fx_or(fx_pc_rel());
}

void c_FxOpCFA1(void) /* XOR R15 */
{
    fx_fetchpipe();
    fx_xor(fx_pc_rel());
}

void c_FxOpAEA1(void) /* LMS R14 */
{
    fx_lms(14);
    fx_update_r14();
}

void c_FxOpAFA1(void) /* LMS R15: the loaded word is the jump target */
{
    u4 const addr = (u4)*FxSeamPC * 2u;
    u4 target;

    FxSeamPC++;
    FxSeamCX = (FxSeamCX & ~0xFFu) | *FxSeamPC;
    SfxLastRamAdr = SfxRAMMem + addr;
    target = ((u4)fx_ram(addr)[0] | ((u4)fx_ram(addr)[1] << 8)) & 0xFFFFu;
    FxSeamPC = (u1*)(uintptr_t)(SfxCPB + target);
}

void c_FxOpAFA2(void) /* SMS R15: store the program counter, short address */
{
    u4 const val = fx_pc_rel(); /* taken before either increment */
    u4 const addr = (u4)*FxSeamPC * 2u;

    FxSeamPC++;
    fx_fetchpipe();
    SfxLastRamAdr = SfxRAMMem + addr;
    FxSeamPC++;
    fx_ram(addr)[0] = (u1)val;
    fx_ram(addr)[1] = (u1)(val >> 8);
}

void c_FxOpFE(void) /* IWT R14 */
{
    u4 const imm = (u4)FxSeamPC[0] | ((u4)FxSeamPC[1] << 8);

    FxSeamCX = (FxSeamCX & ~0xFFu) | FxSeamPC[2];
    FxSeamPC += 3;
    SfxR0[14] = imm;
    fx_update_r14();
}

void c_FxOpFF(void) /* IWT R15: an absolute jump within the bank */
{
    u4 const imm = (u4)FxSeamPC[0] | ((u4)FxSeamPC[1] << 8);

    FxSeamCX = (FxSeamCX & ~0xFFu) | FxSeamPC[2];
    FxSeamPC = (u1*)(uintptr_t)(SfxCPB + imm);
}

/* --- COLOR / GETC / CMODE / MERGE and the 16-bit-address R14/R15 forms ----
 */

/* Setting the colour register also refreshes the four per-bitplane lookups the
   plot routines use, but only when the value actually changes. */
static inline void fx_set_color(u1 const col)
{
    if ((u1)SfxCOLR == col) {
        return;
    }
    *(u1*)&SfxCOLR = col; /* byte store: the upper three are kept */
    fxbit01pcal = fxbit01[col];
    fxbit23pcal = fxbit23[col];
    fxbit45pcal = fxbit45[col];
    fxbit67pcal = fxbit67[col];
}

/* POR bit 2 duplicates the high nibble down; bit 3 keeps the colour register's
   existing high nibble instead. */
static inline void fx_color(u4 const raw)
{
    u1 col = (u1)raw;

    if (SfxPOR & 0x04u) {
        col = (u1)((col & 0xF0u) | (col >> 4));
    }
    if (SfxPOR & 0x08u) {
        col = (u1)((col & 0x0Fu) | ((u1)SfxCOLR & 0xF0u));
    }
    fx_set_color(col);
}

void c_FxOp4E(void) /* COLOR */
{
    fx_fetchpipe();
    fx_color(*FxSeamSrc);
    FxSeamPC++;
}

void c_FxOpDF(void) /* GETC: colour from the ROM buffer */
{
    u4 const raw = fx_rom_byte();

    fx_fetchpipe();
    fx_color(raw);
    FxSeamPC++;
}

/* Which line-location table the current screen mode uses. POR bit 4 forces
   object mode; otherwise SCMR bits 2 and 5 give the screen height. */
static inline u4 fx_lineloc(void)
{
    if (SfxPOR & 0x10u) {
        return sfxobjlineloc;
    }
    switch (SfxSCMR & 0x24u) {
    case 4:
        return sfx160lineloc;
    case 32:
        return sfx192lineloc;
    case 36:
        return sfxobjlineloc;
    default:
        return sfx128lineloc;
    }
}

void c_FxOp4EA1(void) /* CMODE: plot options, screen height and plot variant */
{
    u4 mode;

    fx_fetchpipe();
    FxSeamPC++;
    SfxPOR = *FxSeamSrc;
    sfxclineloc = fx_lineloc();

    /* Screen mode and plot options together pick the PLOT variant, which is
       patched straight into the dispatch tables at $4C. */
    mode = (SfxSCMR & 3u) | ((SfxPOR & 0x0Fu) << 2);
    FxTable[0x4C] = PLOTJmpa[mode];
    FxTableb[0x4C] = PLOTJmpa[mode];
    FxTablec[0x4C] = PLOTJmpa[mode];
    FxTabled[0x4C] = PLOTJmpb[mode];
}

void c_FxOp70(void) /* MERGE: R7 and R8's high bytes, with hand-rolled flags */
{
    u4 const v = ((SfxR0[7] & 0xFF00u)) | ((SfxR0[8] >> 8) & 0xFFu);

    fx_fetchpipe();
    FxSeamPC++;
    *FxSeamDst = v;
    SfxSignZero = (v & 0xF0F0u) ? 0 : 1;
    if (v & 0x8080u) {
        SfxSignZero |= 0x80000u;
    }
    /* These two are full dword stores here, unlike the byte stores elsewhere. */
    SfxOverflow = (v & 0xC0C0u) ? 1 : 0;
    SfxCarry = (v & 0xE0E0u) ? 1 : 0;
}

void c_FxOpFEA1(void) /* LM R14 */
{
    fx_lm(14);
    fx_update_r14();
}

/* The $FFA1/$FFA2 pair build their 16-bit address one opcode fetch at a time,
   so the address inherits ecx's upper half. That is zero in the core, but keep
   it so the port matches the assembly bit for bit. */
static inline u4 fx_stream_addr16(void)
{
    u4 addr;

    fx_fetchpipe();
    addr = FxSeamCX;
    FxSeamPC++;
    fx_fetchpipe();
    FxSeamPC++;
    addr = (addr & ~0xFF00u) | ((FxSeamCX & 0xFFu) << 8);
    fx_fetchpipe();
    return addr;
}

void c_FxOpFFA1(void) /* LM R15: the loaded word is the jump target */
{
    u4 const addr = fx_stream_addr16();
    u4 const target = ((u4)*fx_ram(addr) | ((u4)*fx_ram(addr ^ 1) << 8)) & 0xFFFFu;

    SfxLastRamAdr = SfxRAMMem + addr;
    FxSeamPC = (u1*)(uintptr_t)(SfxCPB + target);
}

void c_FxOpFFA2(void) /* SM R15: store the program counter at a 16-bit address */
{
    u4 const val = fx_pc_rel(); /* taken before any increment */
    u4 const addr = fx_stream_addr16();

    SfxLastRamAdr = SfxRAMMem + addr;
    *fx_ram(addr) = (u1)val;
    FxSeamPC++;
    *fx_ram(addr ^ 1) = (u1)(val >> 8);
}

/* --- STOP ----------------------------------------------------------------
 */

void c_FxOp00(void) /* STOP: halt the GSU, optionally raising an IRQ */
{
    fx_fetchpipe();
    *(u1*)&SfxPIPE = (u1)(FxSeamCX & 0xFFu);
    SfxSFR &= 0xFFFFu - 32u; /* clear the Go flag */
    if (!(SfxCFGR & 0x80u)) {
        SfxSFR |= 0x8000u; /* IRQ not masked, so flag it */
    }
    FxSeamPC++;
    /* Give back the opcode budget this line did not use, then stop. */
    ChangeOps += NumberOfOpcodes + 0xF0000000u;
    NumberOfOpcodes = 1;
    SFXProc = 0;
    FxSeamCX &= ~0xFFu; /* xor cl,cl */
}

/* --- PLOT and RPIX -------------------------------------------------------
 *
 * PLOT writes the colour register to the pixel at (R1, R2) and advances R1.
 * CMODE chooses one of sixteen specialisations up front (see c_FxOp4EA1) and
 * patches it into the dispatch tables, so the depth, the zero check and the
 * dither are all decided before a pixel is ever drawn.
 *
 * The line-location table maps a packed (x, y) to a tile number, or to
 * 0xFFFFFFFF for coordinates off the right-hand edge of the screen. The tile
 * number scales by the depth's bits-per-tile-row shift, plus two bytes for
 * each of the eight rows in a tile.
 *
 * The bitplane pairs are 16 bytes apart, and each write is 32-bit: one access
 * covers both bytes of a pair. fxxand[x] clears the destination bit in both,
 * and the fxbitNNpcal values (refreshed by COLOR) carry the colour already
 * expanded to that layout.
 */

enum { FX_PLOT_2BPP,
    FX_PLOT_4BPP,
    FX_PLOT_8BPP };

static inline u4* fx_plane(u4 const addr, u4 const n)
{
    return (u4*)(uintptr_t)(addr + n * 16u);
}

/* Write one pixel. The dithered form just uses the other pair of colour
   lookups, which COLOR loaded with the high nibble. */
static inline void fx_drawpix(u4 const addr, u4 mask, int const depth, int const dither)
{
    u4 const b01 = dither ? fxbit45pcal : fxbit01pcal;
    u4 const b23 = dither ? fxbit67pcal : fxbit23pcal;
    u4 const b45 = dither ? fxbit01pcal : fxbit45pcal;
    u4 const b67 = dither ? fxbit23pcal : fxbit67pcal;
    u4 const planes = depth == FX_PLOT_2BPP ? 1 : depth == FX_PLOT_4BPP ? 2
                                                                        : 4;
    u4 const col[4] = { b01, b23, b45, b67 };

    for (u4 i = 0; i < planes; i++) {
        *fx_plane(addr, i) &= mask;
    }
    mask ^= 0xFFFFFFFFu;
    for (u4 i = 0; i < planes; i++) {
        *fx_plane(addr, i) |= col[i] & mask;
    }
}

static inline void fx_plot(int const depth, u4 const zmask, int const zcheck, int const dither)
{
    u4 const shift = depth == FX_PLOT_2BPP ? 4 : depth == FX_PLOT_4BPP ? 5
                                                                       : 6;
    u4 const index = (SfxR0[2] & 0xFFFF00FFu) | ((SfxR0[1] & 0xFFu) << 8);
    u4 tile;

    fx_fetchpipe();
    FxSeamPC++;
    tile = ((u4 const*)(uintptr_t)sfxclineloc)[index];
    if (tile != 0xFFFFFFFFu && (!zcheck || ((u1)SfxCOLR & zmask))) {
        u4 const addr = (tile << shift) + ((index & 7u) * 2u) + SCBRrel;
        u4 const mask = fxxand[SfxR0[1] & 0xFFu];

        fx_drawpix(addr, mask, depth,
            dither && (((SfxR0[1] ^ SfxR0[2]) & 1u) != 0));
    }
    fx_set_lo16(1, SfxR0[1] + 1); /* `inc word[SfxR1]` on every path */
}

#define FX_PLOTS(name, depth, zmask)                                  \
    void c_FxOp4C128##name##b(void) { fx_plot(depth, zmask, 0, 0); }  \
    void c_FxOp4C128##name##bz(void) { fx_plot(depth, zmask, 1, 0); } \
    void c_FxOp4C128##name##bd(void) { fx_plot(depth, zmask, 0, 1); } \
    void c_FxOp4C128##name##bzd(void) { fx_plot(depth, zmask, 1, 1); }

FX_PLOTS(2, FX_PLOT_2BPP, 0x03u)
FX_PLOTS(4, FX_PLOT_4BPP, 0x0Fu)
FX_PLOTS(8, FX_PLOT_8BPP, 0xFFu)

/* The "l" set is 8bpp with the 4bpp zero-check mask. */
void c_FxOp4C1288bl(void) { fx_plot(FX_PLOT_8BPP, 0x0Fu, 0, 0); }
void c_FxOp4C1288bzl(void) { fx_plot(FX_PLOT_8BPP, 0x0Fu, 1, 0); }
void c_FxOp4C1288bdl(void) { fx_plot(FX_PLOT_8BPP, 0x0Fu, 0, 1); }
void c_FxOp4C1288bzdl(void) { fx_plot(FX_PLOT_8BPP, 0x0Fu, 1, 1); }

/* The unspecialised entry, used until CMODE runs: the assembly is a bare
   `jmp FxOp4C1284b` with a long-dead copy of the plot code behind it. */
void c_FxOp4C(void) { c_FxOp4C1284b(); }

/* RPIX reads a pixel back. It picks the line table itself rather than using
   the one CMODE cached, and reports 0xFF for an off-screen coordinate. */
void c_FxOp4CA1(void)
{
    u4 const index = (SfxR0[2] & 0xFFFF00FFu) | ((SfxR0[1] & 0xFFu) << 8);
    u4 const lineloc = fx_lineloc();
    u4 const tile = ((u4 const*)(uintptr_t)lineloc)[index];
    u4 res = 0xFFu;

    fx_fetchpipe();
    if (tile != 0xFFFFFFFFu) {
        u4 const depth = SfxSCMR & 3u;
        u4 const shift = depth == 0 ? 4 : depth == 3 ? 6
                                                     : 5;
        u4 const planes = depth == 0 ? 2 : depth == 3 ? 8
                                                      : 4;
        u4 const addr = (SfxSCBR << 10) + (tile << shift)
            + ((index & 7u) * 2u) + (u4)(uintptr_t)sfxramdata;
        u1 const bit = (u1)(1u << ((SfxR0[1] & 7u) ^ 7u));

        res = 0;
        for (u4 i = 0; i < planes; i++) {
            /* The planes alternate between the two bytes of each 16-byte pair. */
            if (*(u1 const*)(uintptr_t)(addr + (i >> 1) * 16u + (i & 1u)) & bit) {
                res |= 1u << i;
            }
        }
    }
    FxSeamPC++;
    *FxSeamDst = res;
    flagnz = res;
}

/* --- The d table's one divergent handler ---------------------------------
 *
 * Every other d-table opcode shares its body with the base-table one and
 * differs only in the tail (FXReturn instead of ret), which lives in the asm
 * thunk. STOP is the exception, in two ways: it leaves the loop directly, so it
 * skips the SFXProc and opcode-byte clears the base-table version does, and its
 * IRQ-flag store is commented out in chips/fxemu2c.asm, so this copy never
 * raises the interrupt. Kept as-is; c_FxOp00 has the line the other one runs.
 */
void c_FxOpd00(void)
{
    fx_fetchpipe();
    *(u1*)&SfxPIPE = (u1)(FxSeamCX & 0xFFu);
    SfxSFR &= 0xFFFFu - 32u;
    FxSeamPC++;
    ChangeOps += NumberOfOpcodes + 0xF0000000u;
    NumberOfOpcodes = 1;
}

#endif
