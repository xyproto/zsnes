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

#endif /* SPC_OPS_H */
