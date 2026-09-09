/* spc64_test - the SPC700 core run identically at 32 and 64 bits.
 *
 * cpu/spc_ops.h is a textual include with a small, self-contained set of
 * requirements, so it can be driven straight from here. Every opcode is run
 * over deterministic pseudo-random state and the resulting machine state is
 * hashed; the same build at -m32 and -m64 has to print the same digest.
 *
 * The difftests cannot cover this: they assemble the original with nasm and so
 * only ever build -m32, where zreg and u4 are the same type and a width bug is
 * invisible. This compares the port against itself across word sizes instead.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../types.h"

/* The state cpu/spc_ops.h expects its includer to provide. */
u1 SPCRAM[65536 + 64];
u1 spcextraram[64];
u1 spcA, spcNZ, spcP, spcX, spcY;
u1* spcRamDP;
u4 spcS;
u4 spc700read; /* the idle detector counts reads; unused here */

/* The I/O window at $F0-$FF, kept deterministic and side-effect free so the
   comparison is about the opcode bodies, not the sound hardware. */
static u1 ioreg[16];
static u1 SPCReadReg(u4 const reg) { return ioreg[reg & 0x0F]; }
static void SPCWriteReg(u4 const reg, u1 const al) { ioreg[reg & 0x0F] = al; }

#include "../cpu/spc_ops.h"

/* The dispatch table cpu/c_dspproc.c builds at start-up, rebuilt here so the
   core can be driven without the rest of the emulator. */
static u1* (*const spc_table[256])(u1*) = {
    SpcOp00,
    SpcOp01,
    SpcOp02,
    SpcOp03,
    SpcOp04,
    SpcOp05,
    SpcOp06,
    SpcOp07,
    SpcOp08,
    SpcOp09,
    SpcOp0A,
    SpcOp0B,
    SpcOp0C,
    SpcOp0D,
    SpcOp0E,
    SpcOp0F,
    SpcOp10,
    SpcOp11,
    SpcOp12,
    SpcOp13,
    SpcOp14,
    SpcOp15,
    SpcOp16,
    SpcOp17,
    SpcOp18,
    SpcOp19,
    SpcOp1A,
    SpcOp1B,
    SpcOp1C,
    SpcOp1D,
    SpcOp1E,
    SpcOp1F,
    SpcOp20,
    SpcOp21,
    SpcOp22,
    SpcOp23,
    SpcOp24,
    SpcOp25,
    SpcOp26,
    SpcOp27,
    SpcOp28,
    SpcOp29,
    SpcOp2A,
    SpcOp2B,
    SpcOp2C,
    SpcOp2D,
    SpcOp2E,
    SpcOp2F,
    SpcOp30,
    SpcOp31,
    SpcOp32,
    SpcOp33,
    SpcOp34,
    SpcOp35,
    SpcOp36,
    SpcOp37,
    SpcOp38,
    SpcOp39,
    SpcOp3A,
    SpcOp3B,
    SpcOp3C,
    SpcOp3D,
    SpcOp3E,
    SpcOp3F,
    SpcOp40,
    SpcOp41,
    SpcOp42,
    SpcOp43,
    SpcOp44,
    SpcOp45,
    SpcOp46,
    SpcOp47,
    SpcOp48,
    SpcOp49,
    SpcOp4A,
    SpcOp4B,
    SpcOp4C,
    SpcOp4D,
    SpcOp4E,
    SpcOp4F,
    SpcOp50,
    SpcOp51,
    SpcOp52,
    SpcOp53,
    SpcOp54,
    SpcOp55,
    SpcOp56,
    SpcOp57,
    SpcOp58,
    SpcOp59,
    SpcOp5A,
    SpcOp5B,
    SpcOp5C,
    SpcOp5D,
    SpcOp5E,
    SpcOp5F,
    SpcOp60,
    SpcOp61,
    SpcOp62,
    SpcOp63,
    SpcOp64,
    SpcOp65,
    SpcOp66,
    SpcOp67,
    SpcOp68,
    SpcOp69,
    SpcOp6A,
    SpcOp6B,
    SpcOp6C,
    SpcOp6D,
    SpcOp6E,
    SpcOp6F,
    SpcOp70,
    SpcOp71,
    SpcOp72,
    SpcOp73,
    SpcOp74,
    SpcOp75,
    SpcOp76,
    SpcOp77,
    SpcOp78,
    SpcOp79,
    SpcOp7A,
    SpcOp7B,
    SpcOp7C,
    SpcOp7D,
    SpcOp7E,
    SpcOp7F,
    SpcOp80,
    SpcOp81,
    SpcOp82,
    SpcOp83,
    SpcOp84,
    SpcOp85,
    SpcOp86,
    SpcOp87,
    SpcOp88,
    SpcOp89,
    SpcOp8A,
    SpcOp8B,
    SpcOp8C,
    SpcOp8D,
    SpcOp8E,
    SpcOp8F,
    SpcOp90,
    SpcOp91,
    SpcOp92,
    SpcOp93,
    SpcOp94,
    SpcOp95,
    SpcOp96,
    SpcOp97,
    SpcOp98,
    SpcOp99,
    SpcOp9A,
    SpcOp9B,
    SpcOp9C,
    SpcOp9D,
    SpcOp9E,
    SpcOp9F,
    SpcOpA0,
    SpcOpA1,
    SpcOpA2,
    SpcOpA3,
    SpcOpA4,
    SpcOpA5,
    SpcOpA6,
    SpcOpA7,
    SpcOpA8,
    SpcOpA9,
    SpcOpAA,
    SpcOpAB,
    SpcOpAC,
    SpcOpAD,
    SpcOpAE,
    SpcOpAF,
    SpcOpB0,
    SpcOpB1,
    SpcOpB2,
    SpcOpB3,
    SpcOpB4,
    SpcOpB5,
    SpcOpB6,
    SpcOpB7,
    SpcOpB8,
    SpcOpB9,
    SpcOpBA,
    SpcOpBB,
    SpcOpBC,
    SpcOpBD,
    SpcOpBE,
    SpcOpBF,
    SpcOpC0,
    SpcOpC1,
    SpcOpC2,
    SpcOpC3,
    SpcOpC4,
    SpcOpC5,
    SpcOpC6,
    SpcOpC7,
    SpcOpC8,
    SpcOpC9,
    SpcOpCA,
    SpcOpCB,
    SpcOpCC,
    SpcOpCD,
    SpcOpCE,
    SpcOpCF,
    SpcOpD0,
    SpcOpD1,
    SpcOpD2,
    SpcOpD3,
    SpcOpD4,
    SpcOpD5,
    SpcOpD6,
    SpcOpD7,
    SpcOpD8,
    SpcOpD9,
    SpcOpDA,
    SpcOpDB,
    SpcOpDC,
    SpcOpDD,
    SpcOpDE,
    SpcOpDF,
    SpcOpE0,
    SpcOpE1,
    SpcOpE2,
    SpcOpE3,
    SpcOpE4,
    SpcOpE5,
    SpcOpE6,
    SpcOpE7,
    SpcOpE8,
    SpcOpE9,
    SpcOpEA,
    SpcOpEB,
    SpcOpEC,
    SpcOpED,
    SpcOpEE,
    SpcOpEF,
    SpcOpF0,
    SpcOpF1,
    SpcOpF2,
    SpcOpF3,
    SpcOpF4,
    SpcOpF5,
    SpcOpF6,
    SpcOpF7,
    SpcOpF8,
    SpcOpF9,
    SpcOpFA,
    SpcOpFB,
    SpcOpFC,
    SpcOpFD,
    SpcOpFE,
    SpcOpFF,
};

/* xorshift32: same sequence at both widths. */
static u4 rs = 0x2026u;
static u4 rnd(void)
{
    rs ^= rs << 13;
    rs ^= rs >> 17;
    rs ^= rs << 5;
    return rs;
}

static u4 digest;
static void mix(u4 const v) { digest = digest * 16777619u ^ v; }

static int check_div(
    u1 const a, u1 const y, u1 const x, u1 const want_a, u1 const want_y, u1 const vh)
{
    spcA = a;
    spcY = y;
    spcX = x;
    spcP = 0xFF;
    SpcOp9E(SPCRAM);
    u1 const nz = want_a & 0x80u ? 0x80u : want_a == 0 ? 0
                                                       : 1;
    return spcA != want_a || spcY != want_y || spcNZ != nz
        || spcP != (u1)(0xB7u | vh);
}

int main(void)
{
    u4 op;

    if (check_div(0x34, 0x12, 0x20, 0x91, 0x14, 0x08)
        || check_div(0xA0, 0x11, 0x12, 0xFA, 0x0C, 0x00)
        || check_div(0x10, 0x40, 0x20, 0xFF, 0x30, 0x48)
        || check_div(0x34, 0x12, 0x00, 0xED, 0x34, 0x48)) {
        return 1;
    }

    digest = 2166136261u;
    for (op = 0; op < 256; op++) {
        u4 iter;
        for (iter = 0; iter < 64; iter++) {
            u4 i;
            u1* pc;

            /* Seed the whole machine, not just the registers: an opcode that
               reads memory has to see the same bytes on both builds. */
            rs = 0x2026u + op * 1013u + iter;
            for (i = 0; i < sizeof SPCRAM; i++)
                SPCRAM[i] = (u1)rnd();
            for (i = 0; i < sizeof spcextraram; i++)
                spcextraram[i] = (u1)rnd();
            for (i = 0; i < sizeof ioreg; i++)
                ioreg[i] = (u1)rnd();
            spcA = (u1)rnd();
            spcX = (u1)rnd();
            spcY = (u1)rnd();
            spcP = (u1)rnd();
            spcNZ = (u1)rnd();
            spcS = 0x0100u | (rnd() & 0xFFu);
            spcRamDP = SPCRAM + ((rnd() & 1u) ? 0x100 : 0);

            /* Start well away from the register and ROM windows so the opcode
               reads plain RAM unless it addresses elsewhere itself. */
            pc = SPCRAM + 0x2000u + (rnd() & 0x0FFFu);
            pc = spc_table[op](pc);

            mix((u4)(pc - SPCRAM));
            mix(spcA | (u4)spcX << 8 | (u4)spcY << 16 | (u4)spcP << 24);
            mix(spcNZ | spcS << 8);
            mix((u4)(spcRamDP - SPCRAM));
            for (i = 0; i < sizeof SPCRAM; i += 64)
                mix(SPCRAM[i]);
            for (i = 0; i < sizeof ioreg; i++)
                mix(ioreg[i]);
        }
    }
    printf("spc700 state digest: %08x\n", digest);
    return 0;
}
