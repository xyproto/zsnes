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
