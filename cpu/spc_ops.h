/*
 * cpu/spc_ops.h - SPC700 opcode handlers ported from cpu/spc700.asm.
 *
 * Textual include (cpu/c_spc700.c): the includer must first provide the u1/u2/
 * u4/s1 typedefs, SPCRAM[], spcextraram[], spcRamDP and SPCWriteReg/SPCReadReg.
 *
 * The assembly core keeps the SPC program counter in ebp. A ported handler is
 * therefore `u1* Op(u1* pc)`: it takes the PC just past the opcode byte and
 * returns the updated PC. cpu/spc700.asm keeps the OpXX entry point but reduces
 * its body to the `spccop` thunk, so both SPC dispatch sites (execute() in
 * cpu/c_execute.c and the `endloop` macro in the 65816 core) pick the port up
 * and opcodes can migrate one at a time.
 */
#ifndef SPC_OPS_H
#define SPC_OPS_H

/* $00F0-$00FF are the I/O registers and $FFC0-$FFFF is the IPL ROM window,
 * which shadows a writable 64-byte page. Everything else is plain RAM. These
 * mirror the ReadByte / WriteByte macros in cpu/spc700.asm. */
static inline u1 spc_read(u1 const* const p)
{
    u4 const a = (u4)(p - SPCRAM);
    return a >= 0xF0 && a <= 0xFF ? SPCReadReg(a) : *p;
}

static inline void spc_write(u1* const p, u1 const al)
{
    u4 const a = (u4)(p - SPCRAM);
    if (a >= 0xF0 && a <= 0xFF) {
        SPCWriteReg(a, al);
        return;
    }
    if (a >= 0xFFC0) {
        spcextraram[a - 0xFFC0] = al;
        if (SPCRAM[0xF1] & 0x80) return; /* IPL ROM paged in: RAM stays hidden */
    }
    *p = al;
}

/* The operand of a direct-page instruction, resolved through the DP pointer. */
static inline u1* spc_dp(u1 const* const pc)
{
    return spcRamDP + *pc;
}

u1* SpcOp00(u1* const pc) /* NOP */
{
    return pc;
}

/* SET1 / CLR1 dp.bit - read-modify-write of one direct-page bit. */
static inline u1* spc_set1(u1* const pc, u1 const mask)
{
    u1* const dp = spc_dp(pc);
    spc_write(dp, spc_read(dp) | mask);
    return pc + 1;
}

static inline u1* spc_clr1(u1* const pc, u1 const mask)
{
    u1* const dp = spc_dp(pc);
    spc_write(dp, spc_read(dp) & mask);
    return pc + 1;
}

/* BBS / BBC dp.bit,rel - branch on a direct-page bit. */
static inline u1* spc_bbs(u1* const pc, u1 const mask)
{
    if (spc_read(spc_dp(pc)) & mask) return pc + 2 + (s1)pc[1];
    return pc + 2;
}

static inline u1* spc_bbc(u1* const pc, u1 const mask)
{
    if (!(spc_read(spc_dp(pc)) & mask)) return pc + 2 + (s1)pc[1];
    return pc + 2;
}

#define SPC_BITOP(op, hex, fn, arg) \
    u1* SpcOp##hex(u1* const pc) { return fn(pc, arg); }

SPC_BITOP(SET1, 02, spc_set1, 0x01)
SPC_BITOP(SET1, 22, spc_set1, 0x02)
SPC_BITOP(SET1, 42, spc_set1, 0x04)
SPC_BITOP(SET1, 62, spc_set1, 0x08)
SPC_BITOP(SET1, 82, spc_set1, 0x10)
SPC_BITOP(SET1, A2, spc_set1, 0x20)
SPC_BITOP(SET1, C2, spc_set1, 0x40)
SPC_BITOP(SET1, E2, spc_set1, 0x80)

SPC_BITOP(CLR1, 12, spc_clr1, (u1)~0x01)
SPC_BITOP(CLR1, 32, spc_clr1, (u1)~0x02)
SPC_BITOP(CLR1, 52, spc_clr1, (u1)~0x04)
SPC_BITOP(CLR1, 72, spc_clr1, (u1)~0x08)
SPC_BITOP(CLR1, 92, spc_clr1, (u1)~0x10)
SPC_BITOP(CLR1, B2, spc_clr1, (u1)~0x20)
SPC_BITOP(CLR1, D2, spc_clr1, (u1)~0x40)
SPC_BITOP(CLR1, F2, spc_clr1, (u1)~0x80)

SPC_BITOP(BBS, 03, spc_bbs, 0x01)
SPC_BITOP(BBS, 23, spc_bbs, 0x02)
SPC_BITOP(BBS, 43, spc_bbs, 0x04)
SPC_BITOP(BBS, 63, spc_bbs, 0x08)
SPC_BITOP(BBS, 83, spc_bbs, 0x10)
SPC_BITOP(BBS, A3, spc_bbs, 0x20)
SPC_BITOP(BBS, C3, spc_bbs, 0x40)
SPC_BITOP(BBS, E3, spc_bbs, 0x80)

SPC_BITOP(BBC, 13, spc_bbc, 0x01)
SPC_BITOP(BBC, 33, spc_bbc, 0x02)
SPC_BITOP(BBC, 53, spc_bbc, 0x04)
SPC_BITOP(BBC, 73, spc_bbc, 0x08)
SPC_BITOP(BBC, 93, spc_bbc, 0x10)
SPC_BITOP(BBC, B3, spc_bbc, 0x20)
SPC_BITOP(BBC, D3, spc_bbc, 0x40)
SPC_BITOP(BBC, F3, spc_bbc, 0x80)

#undef SPC_BITOP

/* Branches: the operand is a signed displacement from the byte after it. */
static inline u1* spc_branch(u1* const pc, bool const taken)
{
    return taken ? pc + 1 + (s1)*pc : pc + 1;
}

u1* SpcOp10(u1* const pc) { return spc_branch(pc, !(spcNZ & 0x80)); } /* BPL */
u1* SpcOp30(u1* const pc) { return spc_branch(pc, spcNZ & 0x80); }    /* BMI */
u1* SpcOp50(u1* const pc) { return spc_branch(pc, !(spcP & 0x40)); }  /* BVC */
u1* SpcOp70(u1* const pc) { return spc_branch(pc, spcP & 0x40); }     /* BVS */
u1* SpcOp90(u1* const pc) { return spc_branch(pc, !(spcP & 0x01)); }  /* BCC */
u1* SpcOpB0(u1* const pc) { return spc_branch(pc, spcP & 0x01); }     /* BCS */
u1* SpcOpD0(u1* const pc) { return spc_branch(pc, spcNZ != 0); }      /* BNE */
u1* SpcOpF0(u1* const pc) { return spc_branch(pc, spcNZ == 0); }      /* BEQ */
u1* SpcOp2F(u1* const pc) { return spc_branch(pc, true); }            /* BRA */

/* The stack lives in page 1; only the low byte of spcS moves, and pushes go
 * straight to RAM rather than through the I/O-trapping write path. */
static inline void spc_push(u1 const val)
{
    SPCRAM[spcS] = val;
    spcS = (spcS & 0xFFFFFF00) | (u1)(spcS - 1);
}

/* TCALL n - push the return address and jump through the vector table at the
 * top of memory. The asm takes the vector from spcextraram while the IPL ROM is
 * paged in ($F1 bit 7) and from SPC RAM otherwise. */
static inline u1* spc_tcall(u1* const pc, u4 const off)
{
    u4 const ret = (u4)(pc - SPCRAM);
    spc_push((u1)(ret >> 8));
    spc_push((u1)ret);

    u1 const* const vec = SPCRAM[0xF1] & 0x80 ? spcextraram + off : SPCRAM + 0xFFC0 + off;
    return SPCRAM + (vec[0] | (u2)vec[1] << 8);
}

u1* SpcOp01(u1* const pc) { return spc_tcall(pc, 30); }
u1* SpcOp11(u1* const pc) { return spc_tcall(pc, 28); }
u1* SpcOp21(u1* const pc) { return spc_tcall(pc, 26); }
u1* SpcOp31(u1* const pc) { return spc_tcall(pc, 24); }
u1* SpcOp41(u1* const pc) { return spc_tcall(pc, 22); }
u1* SpcOp51(u1* const pc) { return spc_tcall(pc, 20); }
u1* SpcOp61(u1* const pc) { return spc_tcall(pc, 18); }
u1* SpcOp71(u1* const pc) { return spc_tcall(pc, 16); }
u1* SpcOp81(u1* const pc) { return spc_tcall(pc, 14); }
u1* SpcOp91(u1* const pc) { return spc_tcall(pc, 12); }
u1* SpcOpA1(u1* const pc) { return spc_tcall(pc, 10); }
u1* SpcOpB1(u1* const pc) { return spc_tcall(pc, 8); }
u1* SpcOpC1(u1* const pc) { return spc_tcall(pc, 6); }
u1* SpcOpD1(u1* const pc) { return spc_tcall(pc, 4); }
u1* SpcOpE1(u1* const pc) { return spc_tcall(pc, 2); }
u1* SpcOpF1(u1* const pc) { return spc_tcall(pc, 0); }

/*
 * Addressing modes (cpu/spcaddr.inc). Each fetches the operand byte and
 * advances the PC past its operand bytes.
 */
typedef struct {
    u1 val;
    u1* pc;
} spcaddr;

static inline spcaddr spc_a_imm(u1* const pc) /* #imm */
{
    return (spcaddr) { *pc, pc + 1 };
}
static inline spcaddr spc_a_dp(u1* const pc) /* dp */
{
    return (spcaddr) { spc_read(spcRamDP + *pc), pc + 1 };
}
static inline spcaddr spc_a_dp_x(u1* const pc) /* dp+X */
{
    return (spcaddr) { spc_read(spcRamDP + (u1)(*pc + spcX)), pc + 1 };
}
static inline spcaddr spc_a_abs(u1* const pc) /* !abs */
{
    return (spcaddr) { spc_read(SPCRAM + (pc[0] | (u2)pc[1] << 8)), pc + 2 };
}
static inline spcaddr spc_a_abs_x(u1* const pc) /* !abs+X */
{
    return (spcaddr) { spc_read(SPCRAM + (u2)((pc[0] | (u2)pc[1] << 8) + spcX)), pc + 2 };
}
static inline spcaddr spc_a_abs_y(u1* const pc) /* !abs+Y */
{
    return (spcaddr) { spc_read(SPCRAM + (u2)((pc[0] | (u2)pc[1] << 8) + spcY)), pc + 2 };
}
static inline spcaddr spc_a_ind_x(u1* const pc) /* (X) */
{
    return (spcaddr) { spc_read(spcRamDP + spcX), pc };
}
static inline spcaddr spc_a_dp_x_ind(u1* const pc) /* [dp+X] */
{
    u1 const* const ptr = spcRamDP + (u1)(*pc + spcX);
    return (spcaddr) { spc_read(SPCRAM + (ptr[0] | (u2)ptr[1] << 8)), pc + 1 };
}
static inline spcaddr spc_a_dp_ind_y(u1* const pc) /* [dp]+Y */
{
    u1 const* const ptr = spcRamDP + *pc;
    return (spcaddr) { spc_read(SPCRAM + (u2)((ptr[0] | (u2)ptr[1] << 8) + spcY)), pc + 1 };
}

/*
 * Operations on A (cpu/spcdef.inc). spcNZ carries both N (its sign bit) and
 * Z (whether it is zero), so the logical ops just store the result there.
 */
static inline void spc_or_a(u1 const m) { spcNZ = spcA |= m; }
static inline void spc_and_a(u1 const m) { spcNZ = spcA &= m; }
static inline void spc_eor_a(u1 const m) { spcNZ = spcA ^= m; }
static inline void spc_mov_a(u1 const m) { spcNZ = spcA = m; }

/* CMP leaves N/Z from the 8-bit difference and C from the *inverted* borrow
 * (the asm's `cmp` + `cmc`), and does not touch A. */
static inline void spc_cmp_a(u1 const m)
{
    u1 const r = (u1)(spcA - m);
    spcNZ = r & 0x80 ? 0x80 : r == 0 ? 0 : 1;
    if (spcA >= m)
        spcP |= 0x01;
    else
        spcP &= 0xFE;
}

#define SPC_ALU(hex, mode, op)                    \
    u1* SpcOp##hex(u1* const pc)                  \
    {                                             \
        spcaddr const a = mode(pc);               \
        op(a.val);                                \
        return a.pc;                              \
    }

SPC_ALU(04, spc_a_dp, spc_or_a)
SPC_ALU(14, spc_a_dp_x, spc_or_a)
SPC_ALU(05, spc_a_abs, spc_or_a)
SPC_ALU(15, spc_a_abs_x, spc_or_a)
SPC_ALU(06, spc_a_ind_x, spc_or_a)
SPC_ALU(16, spc_a_abs_y, spc_or_a)
SPC_ALU(07, spc_a_dp_x_ind, spc_or_a)
SPC_ALU(17, spc_a_dp_ind_y, spc_or_a)
SPC_ALU(08, spc_a_imm, spc_or_a)

SPC_ALU(24, spc_a_dp, spc_and_a)
SPC_ALU(34, spc_a_dp_x, spc_and_a)
SPC_ALU(25, spc_a_abs, spc_and_a)
SPC_ALU(35, spc_a_abs_x, spc_and_a)
SPC_ALU(26, spc_a_ind_x, spc_and_a)
SPC_ALU(36, spc_a_abs_y, spc_and_a)
SPC_ALU(27, spc_a_dp_x_ind, spc_and_a)
SPC_ALU(37, spc_a_dp_ind_y, spc_and_a)
SPC_ALU(28, spc_a_imm, spc_and_a)

SPC_ALU(44, spc_a_dp, spc_eor_a)
SPC_ALU(54, spc_a_dp_x, spc_eor_a)
SPC_ALU(45, spc_a_abs, spc_eor_a)
SPC_ALU(55, spc_a_abs_x, spc_eor_a)
SPC_ALU(46, spc_a_ind_x, spc_eor_a)
SPC_ALU(56, spc_a_abs_y, spc_eor_a)
SPC_ALU(47, spc_a_dp_x_ind, spc_eor_a)
SPC_ALU(57, spc_a_dp_ind_y, spc_eor_a)
SPC_ALU(48, spc_a_imm, spc_eor_a)

SPC_ALU(64, spc_a_dp, spc_cmp_a)
SPC_ALU(74, spc_a_dp_x, spc_cmp_a)
SPC_ALU(65, spc_a_abs, spc_cmp_a)
SPC_ALU(75, spc_a_abs_x, spc_cmp_a)
SPC_ALU(66, spc_a_ind_x, spc_cmp_a)
SPC_ALU(76, spc_a_abs_y, spc_cmp_a)
SPC_ALU(67, spc_a_dp_x_ind, spc_cmp_a)
SPC_ALU(77, spc_a_dp_ind_y, spc_cmp_a)
SPC_ALU(68, spc_a_imm, spc_cmp_a)

SPC_ALU(E4, spc_a_dp, spc_mov_a)
SPC_ALU(F4, spc_a_dp_x, spc_mov_a)
SPC_ALU(E5, spc_a_abs, spc_mov_a)
SPC_ALU(F5, spc_a_abs_x, spc_mov_a)
SPC_ALU(E6, spc_a_ind_x, spc_mov_a)
SPC_ALU(F6, spc_a_abs_y, spc_mov_a)
SPC_ALU(E7, spc_a_dp_x_ind, spc_mov_a)
SPC_ALU(F7, spc_a_dp_ind_y, spc_mov_a)
SPC_ALU(E8, spc_a_imm, spc_mov_a)

#undef SPC_ALU

/* Flag setters. CLRP/SETP also move the direct-page base. */
u1* SpcOp20(u1* const pc) { spcP &= 0xDF; spcRamDP = SPCRAM; return pc; }        /* CLRP */
u1* SpcOp40(u1* const pc) { spcP = (spcP & 0xFB) | 0x20; spcRamDP = SPCRAM + 0x100; return pc; } /* SETP */
u1* SpcOp60(u1* const pc) { spcP &= 0xFE; return pc; }                           /* CLRC */
u1* SpcOp80(u1* const pc) { spcP |= 0x01; return pc; }                           /* SETC */
u1* SpcOpA0(u1* const pc) { spcP |= 0x04; return pc; }                           /* EI   */
u1* SpcOpC0(u1* const pc) { spcP &= 0xFB; return pc; }                           /* DI   */
u1* SpcOpE0(u1* const pc) { spcP &= 0xB7; return pc; }                           /* CLRV */
u1* SpcOpED(u1* const pc) { spcP ^= 0x01; return pc; }                           /* NOTC */
u1* SpcOpBD(u1* const pc) { spcS = (spcS & 0xFFFFFF00) | spcX; return pc; }        /* MOV SP,X */

/* --- 8-bit arithmetic ------------------------------------------------------
 * The asm gets N/Z/V/C/H straight out of x86 `adc`/`sbb` + `lahf`, so the C
 * has to reconstruct each flag: V is the signed overflow of the 8-bit
 * operation, H is the carry/borrow out of bit 3 (x86's AF), and SBC reports
 * carry inverted (its `cmc`), i.e. C means "no borrow".
 */
static inline void spc_setnz(u1 const r)
{
    spcNZ = r & 0x80 ? 0x80 : r == 0 ? 0 : 1;
}

static inline void spc_setflags_nvhzc(u1 const r, bool const v, bool const h, bool const c)
{
    spc_setnz(r);
    u1 p = spcP & 0xBF;
    if (v) p |= 0x40;
    p &= 0xF6;
    if (c) p |= 0x01;
    if (h) p |= 0x08;
    spcP = p;
}

static inline u1 spc_adc(u1 const x, u1 const y)
{
    u1 const cin = spcP & 0x01;
    u4 const sum = (u4)x + y + cin;
    u1 const r = (u1)sum;
    spc_setflags_nvhzc(r, (~(x ^ y) & (x ^ r) & 0x80) != 0,
        (((x & 0x0F) + (y & 0x0F) + cin) & 0x10) != 0, sum > 0xFF);
    return r;
}

static inline u1 spc_sbc(u1 const x, u1 const y)
{
    u1 const bin = (spcP & 0x01) ^ 1;
    u4 const diff = (u4)x - y - bin;
    u1 const r = (u1)diff;
    spc_setflags_nvhzc(r, ((x ^ y) & (x ^ r) & 0x80) != 0,
        (((u4)(x & 0x0F) - (y & 0x0F) - bin) & 0x10) != 0, diff <= 0xFF);
    return r;
}

/* CMP: N/Z from the difference, C = no borrow, and no writeback. */
static inline void spc_cmp(u1 const x, u1 const y)
{
    spc_setnz((u1)(x - y));
    if (x >= y)
        spcP |= 0x01;
    else
        spcP &= 0xFE;
}

static inline void spc_adc_a(u1 const m) { spcA = spc_adc(spcA, m); }
static inline void spc_sbc_a(u1 const m) { spcA = spc_sbc(spcA, m); }

#define SPC_ALU2(hex, mode, op)      \
    u1* SpcOp##hex(u1* const pc)     \
    {                                \
        spcaddr const a = mode(pc);  \
        op(a.val);                   \
        return a.pc;                 \
    }

SPC_ALU2(84, spc_a_dp, spc_adc_a)
SPC_ALU2(94, spc_a_dp_x, spc_adc_a)
SPC_ALU2(85, spc_a_abs, spc_adc_a)
SPC_ALU2(95, spc_a_abs_x, spc_adc_a)
SPC_ALU2(86, spc_a_ind_x, spc_adc_a)
SPC_ALU2(96, spc_a_abs_y, spc_adc_a)
SPC_ALU2(87, spc_a_dp_x_ind, spc_adc_a)
SPC_ALU2(97, spc_a_dp_ind_y, spc_adc_a)
SPC_ALU2(88, spc_a_imm, spc_adc_a)

SPC_ALU2(A4, spc_a_dp, spc_sbc_a)
SPC_ALU2(B4, spc_a_dp_x, spc_sbc_a)
SPC_ALU2(A5, spc_a_abs, spc_sbc_a)
SPC_ALU2(B5, spc_a_abs_x, spc_sbc_a)
SPC_ALU2(A6, spc_a_ind_x, spc_sbc_a)
SPC_ALU2(B6, spc_a_abs_y, spc_sbc_a)
SPC_ALU2(A7, spc_a_dp_x_ind, spc_sbc_a)
SPC_ALU2(B7, spc_a_dp_ind_y, spc_sbc_a)
SPC_ALU2(A8, spc_a_imm, spc_sbc_a)

#undef SPC_ALU2

/* --- read-modify-write forms ----------------------------------------------
 * dp,#imm and dp(dest),dp(src): the destination is read, combined, written
 * back. `(X),(Y)` is the same with the pointers coming from X and Y.
 */
#define SPC_RMW(hex, get, comb)                     \
    u1* SpcOp##hex(u1* const pc)                    \
    {                                               \
        get;                                        \
        u1 const r = comb;                          \
        spc_write(dst, r);                          \
        return npc;                                 \
    }

/* dp,#imm - note the assembly takes the dp operand from pc[1], the immediate
 * from pc[0]. */
#define SPC_GET_DP_IMM                     \
    u1* const dst = spcRamDP + pc[1];      \
    u1 const src = pc[0];                  \
    u1 const dv = spc_read(dst);           \
    u1* const npc = pc + 2

/* dp(dest),dp(src) */
#define SPC_GET_DP_DP                      \
    u1* const dst = spcRamDP + pc[1];      \
    u1 const dv = spc_read(dst);           \
    u1 const src = spc_read(spcRamDP + pc[0]); \
    u1* const npc = pc + 2

/* (X),(Y) - the (Y) operand is read first, as in the assembly. */
#define SPC_GET_X_Y                              \
    u1 const src = spc_read(spcRamDP + spcY);    \
    u1* const dst = spcRamDP + spcX;             \
    u1 const dv = spc_read(dst);                 \
    u1* const npc = pc

SPC_RMW(18, SPC_GET_DP_IMM, (spcNZ = dv | src))
SPC_RMW(38, SPC_GET_DP_IMM, (spcNZ = dv & src))
SPC_RMW(58, SPC_GET_DP_IMM, (spcNZ = dv ^ src))
SPC_RMW(98, SPC_GET_DP_IMM, spc_adc(dv, src))
SPC_RMW(B8, SPC_GET_DP_IMM, spc_sbc(dv, src))

SPC_RMW(09, SPC_GET_DP_DP, (spcNZ = dv | src))
SPC_RMW(29, SPC_GET_DP_DP, (spcNZ = dv & src))
SPC_RMW(49, SPC_GET_DP_DP, (spcNZ = dv ^ src))
SPC_RMW(89, SPC_GET_DP_DP, spc_adc(dv, src))
SPC_RMW(A9, SPC_GET_DP_DP, spc_sbc(dv, src))

SPC_RMW(19, SPC_GET_X_Y, (spcNZ = dv | src))
SPC_RMW(39, SPC_GET_X_Y, (spcNZ = dv & src))
SPC_RMW(59, SPC_GET_X_Y, (spcNZ = dv ^ src))
SPC_RMW(99, SPC_GET_X_Y, spc_adc(dv, src))
SPC_RMW(B9, SPC_GET_X_Y, spc_sbc(dv, src))

/* The CMP variants compare without writing back. */
u1* SpcOp78(u1* const pc) { SPC_GET_DP_IMM; spc_cmp(dv, src); return npc; }
u1* SpcOp69(u1* const pc) { SPC_GET_DP_DP;  spc_cmp(dv, src); return npc; }
u1* SpcOp79(u1* const pc) { SPC_GET_X_Y;    spc_cmp(dv, src); return npc; }

#undef SPC_RMW

/* MOV dp,dp - a straight byte copy, destination operand second. */
u1* SpcOpFA(u1* const pc)
{
    spc_write(spcRamDP + pc[1], spc_read(spcRamDP + pc[0]));
    return pc + 2;
}

/* --- stores (no flags) ----------------------------------------------------- */
u1* SpcOpC4(u1* const pc) { spc_write(spcRamDP + *pc, spcA); return pc + 1; }
u1* SpcOpD4(u1* const pc) { spc_write(spcRamDP + (u1)(*pc + spcX), spcA); return pc + 1; }
u1* SpcOpC5(u1* const pc) { spc_write(SPCRAM + (pc[0] | (u2)pc[1] << 8), spcA); return pc + 2; }
u1* SpcOpD5(u1* const pc) { spc_write(SPCRAM + (u2)((pc[0] | (u2)pc[1] << 8) + spcX), spcA); return pc + 2; }
u1* SpcOpD6(u1* const pc) { spc_write(SPCRAM + (u2)((pc[0] | (u2)pc[1] << 8) + spcY), spcA); return pc + 2; }
u1* SpcOpC6(u1* const pc) { spc_write(spcRamDP + spcX, spcA); return pc; }
u1* SpcOpD8(u1* const pc) { spc_write(spcRamDP + *pc, spcX); return pc + 1; }
u1* SpcOpD9(u1* const pc) { spc_write(spcRamDP + (u1)(*pc + spcY), spcX); return pc + 1; }
u1* SpcOpC9(u1* const pc) { spc_write(SPCRAM + (pc[0] | (u2)pc[1] << 8), spcX); return pc + 2; }
u1* SpcOpCB(u1* const pc) { spc_write(spcRamDP + *pc, spcY); return pc + 1; }
u1* SpcOpDB(u1* const pc) { spc_write(spcRamDP + (u1)(*pc + spcX), spcY); return pc + 1; }
u1* SpcOpCC(u1* const pc) { spc_write(SPCRAM + (pc[0] | (u2)pc[1] << 8), spcY); return pc + 2; }

u1* SpcOpC7(u1* const pc) /* MOV [dp+X],A */
{
    u1 const* const ptr = spcRamDP + (u1)(*pc + spcX);
    spc_write(SPCRAM + (ptr[0] | (u2)ptr[1] << 8), spcA);
    return pc + 1;
}
u1* SpcOpD7(u1* const pc) /* MOV [dp]+Y,A */
{
    u1 const* const ptr = spcRamDP + *pc;
    spc_write(SPCRAM + (u2)((ptr[0] | (u2)ptr[1] << 8) + spcY), spcA);
    return pc + 1;
}

/* --- loads into X / Y ------------------------------------------------------ */
u1* SpcOpF8(u1* const pc) { spcNZ = spcX = spc_read(spcRamDP + *pc); return pc + 1; }
u1* SpcOpF9(u1* const pc) { spcNZ = spcX = spc_read(spcRamDP + (u1)(*pc + spcY)); return pc + 1; }
u1* SpcOpE9(u1* const pc) { spcNZ = spcX = spc_read(SPCRAM + (pc[0] | (u2)pc[1] << 8)); return pc + 2; }
u1* SpcOpEB(u1* const pc) { spcNZ = spcY = spc_read(spcRamDP + *pc); return pc + 1; }
u1* SpcOpFB(u1* const pc) { spcNZ = spcY = spc_read(spcRamDP + (u1)(*pc + spcX)); return pc + 1; }
u1* SpcOpEC(u1* const pc) { spcNZ = spcY = spc_read(SPCRAM + (pc[0] | (u2)pc[1] << 8)); return pc + 2; }

u1* SpcOpDD(u1* const pc) { spcNZ = spcA = spcY; return pc; } /* MOV A,Y */
u1* SpcOpFD(u1* const pc) { spcNZ = spcY = spcA; return pc; } /* MOV Y,A */

u1* SpcOpAF(u1* const pc) /* MOV (X)+,A */
{
    u1* const dst = spcRamDP + spcX;
    spcX++;
    spc_write(dst, spcA);
    return pc;
}
u1* SpcOpBF(u1* const pc) /* MOV A,(X)+ */
{
    u1 const v = spc_read(spcRamDP + spcX);
    spcX++;
    spcNZ = spcA = v;
    return pc;
}

/* DBNZ Y,rel */
u1* SpcOpFE(u1* const pc) { return spc_branch(pc, --spcY != 0); }

/* XCN - swap the nibbles of A. */
u1* SpcOp9F(u1* const pc) { spcNZ = spcA = (u1)(spcA >> 4 | spcA << 4); return pc; }

/* SLEEP / STOP - park the PC on the opcode itself. */
u1* SpcOpEF(u1* const pc) { return pc - 1; }
u1* SpcOpFF(u1* const pc) { spc700read++; return pc - 1; }
u1* SpcOp0F(u1* const pc) { spc700read++; return pc - 1; }

/* --- register moves, INC/DEC ----------------------------------------------- */
u1* SpcOp5D(u1* const pc) { spcNZ = spcX = spcA; return pc; }             /* MOV X,A */
u1* SpcOp7D(u1* const pc) { spcNZ = spcA = spcX; return pc; }             /* MOV A,X */
u1* SpcOp9D(u1* const pc) { spcNZ = spcX = (u1)spcS; return pc; }         /* MOV X,SP */
u1* SpcOp8D(u1* const pc) { spcNZ = spcY = *pc; return pc + 1; }          /* MOV Y,#i */
u1* SpcOpCD(u1* const pc) { spcNZ = spcX = *pc; return pc + 1; }          /* MOV X,#i */
u1* SpcOp9C(u1* const pc) { spcNZ = --spcA; return pc; }                  /* DEC A */
u1* SpcOpBC(u1* const pc) { spcNZ = ++spcA; return pc; }                  /* INC A */
u1* SpcOpDC(u1* const pc) { spcNZ = --spcY; return pc; }                  /* DEC Y */
u1* SpcOpFC(u1* const pc) { spcNZ = ++spcY; return pc; }                  /* INC Y */
u1* SpcOp1D(u1* const pc) { spcNZ = --spcX; return pc; }                  /* DEC X */
u1* SpcOp3D(u1* const pc) { spcNZ = ++spcX; return pc; }                  /* INC X */

/* MOV dp,#imm - like the other dp,#imm forms the dp operand is the second byte. */
u1* SpcOp8F(u1* const pc) { spc_write(spcRamDP + pc[1], pc[0]); return pc + 2; }

/* --- CMP X / CMP Y --------------------------------------------------------- */
u1* SpcOpC8(u1* const pc) { spc_cmp(spcX, *pc); return pc + 1; }
u1* SpcOpAD(u1* const pc) { spc_cmp(spcY, *pc); return pc + 1; }
u1* SpcOp3E(u1* const pc) { spc_cmp(spcX, spc_read(spcRamDP + *pc)); return pc + 1; }
u1* SpcOp7E(u1* const pc) { spc_cmp(spcY, spc_read(spcRamDP + *pc)); return pc + 1; }
u1* SpcOp1E(u1* const pc) { spc_cmp(spcX, spc_read(SPCRAM + (pc[0] | (u2)pc[1] << 8))); return pc + 2; }
u1* SpcOp5E(u1* const pc) { spc_cmp(spcY, spc_read(SPCRAM + (pc[0] | (u2)pc[1] << 8))); return pc + 2; }

/* --- read-modify-write on memory ------------------------------------------- */
#define SPC_MEM_RMW(hex, addr, adv, expr)      \
    u1* SpcOp##hex(u1* const pc)               \
    {                                          \
        u1* const m = addr;                    \
        u1 al = spc_read(m);                   \
        expr;                                  \
        spc_write(m, al);                      \
        return pc + adv;                       \
    }

#define SPC_DP    (spcRamDP + *pc)
#define SPC_DP_X  (spcRamDP + (u1)(*pc + spcX))
#define SPC_ABS   (SPCRAM + (pc[0] | (u2)pc[1] << 8))

/* INC/DEC set N/Z from the result only. */
SPC_MEM_RMW(8B, SPC_DP, 1, spcNZ = --al)
SPC_MEM_RMW(AB, SPC_DP, 1, spcNZ = ++al)
SPC_MEM_RMW(9B, SPC_DP_X, 1, spcNZ = --al)
SPC_MEM_RMW(BB, SPC_DP_X, 1, spcNZ = ++al)
SPC_MEM_RMW(8C, SPC_ABS, 2, spcNZ = --al)
SPC_MEM_RMW(AC, SPC_ABS, 2, spcNZ = ++al)

/* ASL/LSR also push the shifted-out bit into C (the asm's SPCSetFlagnzc). */
static inline u1 spc_asl(u1 const v)
{
    u1 const r = (u1)(v << 1);
    spc_setnz(r);
    spcP = v & 0x80 ? spcP | 0x01 : spcP & 0xFE;
    return r;
}
static inline u1 spc_lsr(u1 const v)
{
    u1 const r = (u1)(v >> 1);
    spc_setnz(r);
    spcP = v & 0x01 ? spcP | 0x01 : spcP & 0xFE;
    return r;
}

SPC_MEM_RMW(0B, SPC_DP, 1, al = spc_asl(al))
SPC_MEM_RMW(4B, SPC_DP, 1, al = spc_lsr(al))
SPC_MEM_RMW(1B, SPC_DP_X, 1, al = spc_asl(al))
SPC_MEM_RMW(5B, SPC_DP_X, 1, al = spc_lsr(al))
SPC_MEM_RMW(0C, SPC_ABS, 2, al = spc_asl(al))
SPC_MEM_RMW(4C, SPC_ABS, 2, al = spc_lsr(al))

u1* SpcOp1C(u1* const pc) { spcA = spc_asl(spcA); return pc; } /* ASL A */
u1* SpcOp5C(u1* const pc) { spcA = spc_lsr(spcA); return pc; } /* LSR A */

/* TSET1 / TCLR1 - N/Z come from A AND mem, and the stored value differs.
 * Note the assembly's TCLR1 adds the *direct page* base to a 16-bit operand;
 * reproduced as-is. */
SPC_MEM_RMW(0E, SPC_ABS, 2, spcNZ = al & spcA; al |= spcA)
SPC_MEM_RMW(4E, (spcRamDP + (pc[0] | (u2)pc[1] << 8)), 2, spcNZ = al & spcA; al &= (u1)~spcA)

#undef SPC_MEM_RMW

/* --- stack ----------------------------------------------------------------- */
static inline u1 spc_pop(void)
{
    spcS = (spcS & 0xFFFFFF00) | (u1)(spcS + 1);
    return SPCRAM[spcS];
}

u1* SpcOp2D(u1* const pc) { spc_push(spcA); return pc; } /* PUSH A */
u1* SpcOp4D(u1* const pc) { spc_push(spcX); return pc; } /* PUSH X */
u1* SpcOp6D(u1* const pc) { spc_push(spcY); return pc; } /* PUSH Y */
u1* SpcOpAE(u1* const pc) { spcA = spc_pop(); return pc; } /* POP A - no flags */
u1* SpcOpCE(u1* const pc) { spcX = spc_pop(); return pc; } /* POP X */
u1* SpcOpEE(u1* const pc) { spcY = spc_pop(); return pc; } /* POP Y */

/* PUSH P - rebuild the real N and Z bits from spcNZ first. */
u1* SpcOp0D(u1* const pc)
{
    u1 p = spcP & 0x7D;
    if (spcNZ & 0x80)
        p |= 0x80;
    else if (spcNZ == 0)
        p |= 0x02;
    spc_push(p);
    return pc;
}

/* --- jumps and calls ------------------------------------------------------- */
u1* SpcOp5F(u1* const pc) { return SPCRAM + (pc[0] | (u2)pc[1] << 8); } /* JMP !abs */

u1* SpcOp1F(u1* const pc) /* JMP [!abs+X] */
{
    u1 const* const ptr = SPCRAM + (u2)((pc[0] | (u2)pc[1] << 8) + spcX);
    return SPCRAM + (ptr[0] | (u2)ptr[1] << 8);
}

u1* SpcOp3F(u1* const pc) /* CALL !abs */
{
    u4 const ret = (u4)(pc + 2 - SPCRAM);
    spc_push((u1)(ret >> 8));
    spc_push((u1)ret);
    return SPCRAM + (pc[0] | (u2)pc[1] << 8);
}

u1* SpcOp4F(u1* const pc) /* PCALL up - target is in page $FF */
{
    u4 const ret = (u4)(pc + 1 - SPCRAM);
    spc_push((u1)(ret >> 8));
    spc_push((u1)ret);
    return SPCRAM + 0xFF00 + *pc;
}

u1* SpcOp6F(u1* const pc) /* RET */
{
    u1 const lo = spc_pop();
    u1 const hi = spc_pop();
    return SPCRAM + (lo | (u2)hi << 8);
}

/* RETI: the assembly returns immediately with the PC parked on the opcode -
 * everything after its `ret` is unreachable. Kept as-is. */
u1* SpcOp7F(u1* const pc) { return pc - 1; }

/* --- compare-and-branch ---------------------------------------------------- */
u1* SpcOp2E(u1* const pc) /* CBNE dp,rel */
{
    return spcA != spc_read(spcRamDP + pc[0]) ? pc + 2 + (s1)pc[1] : pc + 2;
}
u1* SpcOpDE(u1* const pc) /* CBNE dp+X,rel */
{
    return spcA != spc_read(spcRamDP + (u1)(pc[0] + spcX)) ? pc + 2 + (s1)pc[1] : pc + 2;
}
u1* SpcOp6E(u1* const pc) /* DBNZ dp,rel */
{
    u1* const m = spcRamDP + pc[0];
    u1 const v = (u1)(spc_read(m) - 1);
    u1* const npc = v != 0 ? pc + 2 + (s1)pc[1] : pc + 2;
    spc_write(m, v);
    return npc;
}

/* MUL YA,#0 - Y:A = Y * A, flags from the 16-bit product. */
u1* SpcOpCF(u1* const pc)
{
    u2 const r = (u2)((u2)spcY * spcA);
    spcA = (u1)r;
    spcY = (u1)(r >> 8);
    spcNZ = r & 0x8000 ? 0x80 : r == 0 ? 0 : 1;
    return pc;
}

/* --- 16-bit (word) direct-page operations ----------------------------------
 * The operand is the dp byte pair (dp) = low, (dp+1) = high. The assembly
 * reads the high byte first (via ReadByte2, so the I/O trap still applies) and
 * derives N/Z from the full 16-bit result.
 */
static inline void spc_setnz16(u2 const r)
{
    spcNZ = r & 0x8000 ? 0x80 : r == 0 ? 0 : 1;
}

static inline u2 spc_readw(u1* const lo)
{
    u1 const hi = spc_read(lo + 1);
    return (u2)(spc_read(lo) | (u2)hi << 8);
}

u1* SpcOp1A(u1* const pc) /* DECW dp */
{
    u1* const m = spcRamDP + *pc;
    u2 const r = (u2)(spc_readw(m) - 1);
    spc_setnz16(r);
    spc_write(m, (u1)r);
    spc_write(m + 1, (u1)(r >> 8));
    return pc + 1;
}

u1* SpcOp3A(u1* const pc) /* INCW dp */
{
    u1* const m = spcRamDP + *pc;
    u2 const r = (u2)(spc_readw(m) + 1);
    spc_setnz16(r);
    spc_write(m, (u1)r);
    spc_write(m + 1, (u1)(r >> 8));
    return pc + 1;
}

u1* SpcOp5A(u1* const pc) /* CMPW YA,dp */
{
    u2 const m = spc_readw(spcRamDP + *pc);
    u2 const ya = (u2)(spcA | (u2)spcY << 8);
    spc_setnz16((u2)(ya - m));
    if (ya >= m)
        spcP |= 0x01;
    else
        spcP &= 0xFE;
    return pc + 1;
}

u1* SpcOp7A(u1* const pc) /* ADDW YA,dp */
{
    u2 const m = spc_readw(spcRamDP + *pc);
    u2 const ya = (u2)(spcA | (u2)spcY << 8);
    u4 const sum = (u4)ya + m;
    u2 const r = (u2)sum;
    spcA = (u1)r;
    spcY = (u1)(r >> 8);
    spc_setnz16(r);
    u1 p = spcP & 0xBF;
    if ((~(ya ^ m) & (ya ^ r) & 0x8000) != 0) p |= 0x40;
    p &= 0xF6;
    if (sum > 0xFFFF) p |= 0x01;
    if ((((ya & 0x000F) + (m & 0x000F)) & 0x10) != 0) p |= 0x08; /* x86 AF: bit 3 */
    spcP = p;
    return pc + 1;
}

u1* SpcOp9A(u1* const pc) /* SUBW YA,dp */
{
    u2 const m = spc_readw(spcRamDP + *pc);
    u2 const ya = (u2)(spcA | (u2)spcY << 8);
    u4 const diff = (u4)ya - m;
    u2 const r = (u2)diff;
    spcA = (u1)r;
    spcY = (u1)(r >> 8);
    spc_setnz16(r);
    u1 p = spcP & 0xBF;
    if (((ya ^ m) & (ya ^ r) & 0x8000) != 0) p |= 0x40;
    p &= 0xF6;
    if (diff <= 0xFFFF) p |= 0x01; /* the asm's cmc: C means "no borrow" */
    if ((((u4)(ya & 0x000F) - (m & 0x000F)) & 0x10) != 0) p |= 0x08; /* x86 AF: bit 3 */
    spcP = p;
    return pc + 1;
}

u1* SpcOpBA(u1* const pc) /* MOVW YA,dp */
{
    u2 const r = spc_readw(spcRamDP + *pc);
    spcA = (u1)r;
    spcY = (u1)(r >> 8);
    spc_setnz16(r);
    return pc + 1;
}

u1* SpcOpDA(u1* const pc) /* MOVW dp,YA - no flags */
{
    u1* const m = spcRamDP + *pc;
    spc_write(m, spcA);
    spc_write(m + 1, spcY);
    return pc + 1;
}

/* POP P - restores the status byte, rebuilds spcNZ from its N/Z bits and moves
 * the direct-page base to match bit 5. */
u1* SpcOp8E(u1* const pc)
{
    u1 const p = spc_pop();
    spcP = p;
    spcNZ = p & 0x02 ? 0 : (u1)(1 | (p & 0x80));
    spcRamDP = SPCRAM + (p & 0x20 ? 0x100 : 0);
    return pc;
}

/* --- membit ops ------------------------------------------------------------
 * The 16-bit operand packs a 13-bit address in the low bits and the bit index
 * in the top three, so `mem.bit` addresses any bit of $0000-$1FFF.
 */
typedef struct {
    u1* addr;
    u1 bit;
} spcmembit;

static inline spcmembit spc_membit(u1 const* const pc)
{
    u2 const w = (u2)(pc[0] | (u2)pc[1] << 8);
    return (spcmembit) { SPCRAM + (w & 0x1FFF), (u1)(w >> 13) };
}

static inline u1 spc_getbit(u1 const* const pc)
{
    spcmembit const m = spc_membit(pc);
    return (u1)(spc_read(m.addr) >> m.bit & 1);
}

u1* SpcOp0A(u1* const pc) { spcP |= spc_getbit(pc); return pc + 2; }              /* OR1  C,m.b  */
u1* SpcOp2A(u1* const pc) { spcP |= spc_getbit(pc) ^ 1; return pc + 2; }          /* OR1  C,/m.b */
u1* SpcOp4A(u1* const pc) { spcP &= spc_getbit(pc) | 0xFE; return pc + 2; }       /* AND1 C,m.b  */
u1* SpcOp6A(u1* const pc) { spcP &= (spc_getbit(pc) | 0xFE) ^ 1; return pc + 2; } /* AND1 C,/m.b */
u1* SpcOp8A(u1* const pc) { spcP ^= spc_getbit(pc); return pc + 2; }              /* EOR1 C,m.b  */
u1* SpcOpAA(u1* const pc) { spcP = (spcP & 0xFE) | spc_getbit(pc); return pc + 2; } /* MOV1 C,m.b  */

u1* SpcOpCA(u1* const pc) /* MOV1 m.b,C */
{
    spcmembit const m = spc_membit(pc);
    u1 const mask = (u1)(1 << m.bit);
    spc_write(m.addr, (u1)((spc_read(m.addr) & ~mask) | (spcP & 1) << m.bit));
    return pc + 2;
}

u1* SpcOpEA(u1* const pc) /* NOT1 m.b */
{
    spcmembit const m = spc_membit(pc);
    spc_write(m.addr, (u1)(spc_read(m.addr) ^ 1 << m.bit));
    return pc + 2;
}

/* --- ROL / ROR -------------------------------------------------------------
 * Rotate through carry. The carry-in is sampled before the read (the assembly
 * branches on it to pick a `clc`/`stc` variant), and spcNZ takes the raw
 * result byte.
 */
static inline u1 spc_rol(u1 const v)
{
    u1 const r = (u1)(v << 1 | (spcP & 1));
    spcP = v & 0x80 ? spcP | 0x01 : spcP & 0xFE;
    spcNZ = r;
    return r;
}
static inline u1 spc_ror(u1 const v)
{
    u1 const r = (u1)(v >> 1 | (spcP & 1) << 7);
    spcP = v & 0x01 ? spcP | 0x01 : spcP & 0xFE;
    spcNZ = r;
    return r;
}

#define SPC_ROT(hex, addr, adv, fn)                     \
    u1* SpcOp##hex(u1* const pc)                        \
    {                                                   \
        u1* const m = addr;                             \
        spc_write(m, fn(spc_read(m)));                  \
        return pc + adv;                                \
    }

SPC_ROT(2B, SPC_DP, 1, spc_rol)
SPC_ROT(3B, SPC_DP_X, 1, spc_rol)
SPC_ROT(2C, SPC_ABS, 2, spc_rol)
SPC_ROT(6B, SPC_DP, 1, spc_ror)
SPC_ROT(7B, SPC_DP_X, 1, spc_ror)
SPC_ROT(6C, SPC_ABS, 2, spc_ror)

#undef SPC_ROT

u1* SpcOp3C(u1* const pc) { spcA = spc_rol(spcA); return pc; } /* ROL A */
u1* SpcOp7C(u1* const pc) { spcA = spc_ror(spcA); return pc; } /* ROR A */

/* --- DIV YA,X --------------------------------------------------------------
 * A 16-bit divide (dx:ax / bx with dx cleared), so the quotient always fits.
 * X = 0 is caught before the divide; a quotient wider than 8 bits takes the
 * "overflow" path, which keeps the truncated result but sets V and clears H.
 */
u1* SpcOp9E(u1* const pc)
{
    if (spcX == 0) { /* NoDiv */
        spcA = 0xFF;
        spcY = 0xFF;
        spcP = spcP | 0x10;
        spcP = spcP & ~0x40;
        return pc;
    }
    u2 const ya = (u2)(spcA | (u2)spcY << 8);
    u2 const q = (u2)(ya / spcX);
    spcA = (u1)q;
    spcY = (u1)(ya % spcX);
    if (q >> 8) { /* Over */
        spcP = spcP | 0x40;
        spcP = spcP & ~0x10;
    } else {
        spcP &= 0xAF; /* clear V and H */
    }
    spcNZ = (u1)q;
    return pc;
}

/* --- DAA / DAS -------------------------------------------------------------
 * The assembly rebuilds an x86 flags byte from spcNZ/spcP with `sahf`, runs
 * `daa`/`das`, then re-derives N/Z/C. These reimplement the two instructions;
 * only C and the result feed back, so AF's output value is not needed.
 */
static inline void spc_decadj(bool const sub)
{
    u1 const old = spcA;
    bool const oldc = (spcP & 0x01) != 0;
    bool const af = (spcP & 0x08) != 0;
    u1 a = old;
    bool c = false;

    if ((a & 0x0F) > 9 || af) {
        u4 const t = sub ? (u4)a - 6 : (u4)a + 6;
        c = oldc || (sub ? t > 0xFF : t > 0xFF);
        a = (u1)t;
    } else {
        c = oldc;
    }
    if (old > 0x99 || oldc) {
        a = (u1)(sub ? a - 0x60 : a + 0x60);
        c = true;
    } else if (!((old & 0x0F) > 9 || af)) {
        c = false;
    }

    spcA = a;
    spc_setnz(a);
    spcP = c ? spcP | 0x01 : spcP & 0xFE;
}

u1* SpcOpBE(u1* const pc) { spc_decadj(true); return pc; }  /* DAS */
u1* SpcOpDF(u1* const pc) { spc_decadj(false); return pc; } /* DAA */

/* Invalid opcodes park the PC on the opcode, like SLEEP. */
u1* SpcOpInvalid(u1* const pc) { return pc - 1; }

#endif /* SPC_OPS_H */
