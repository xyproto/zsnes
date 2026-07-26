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
    spcS = spcS & 0xFFFFFF00 | (u1)(spcS - 1);
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

#endif /* SPC_OPS_H */
