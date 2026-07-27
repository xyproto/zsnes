/*
 * test/difftest_spcdec.c - proves the C ports of the SPC700 DAA/DAS opcodes
 * ($DF/$BE in cpu/spc_ops.h) bit-identical to the assembly they replaced.
 *
 * These two are the only SPC opcodes the whole-emulator A/B cannot verify: SNES
 * sound drivers do not use BCD, so they simply never execute. The assembly
 * built them out of x86 `sahf` + `daa`/`das`, which the C reimplements, so an
 * exhaustive comparison over every input is the only honest check.
 *
 * Build/run:  make -C test spcdec
 */
#include "../types.h"
#include "difftest.h"

u1 spcA, spcNZ, spcP;

/* The handlers below only touch A and the flags; the rest of the SPC state the
 * header names is unused here but has to exist to compile. */
u1 SPCRAM[0x10000];
u1 spcextraram[64];
u1 spcX, spcY;
u4 spcS, spc700read;
u1* spcRamDP = SPCRAM;
void SPCWriteReg(u4 reg, u1 val)
{
    (void)reg;
    (void)val;
}
u1 SPCReadReg(u4 reg) { return (u1)reg; }

#include <stdbool.h>

#include "../cpu/spc_ops.h"

extern void asm_OpBE(void);
extern void asm_OpDF(void);

static void call_asm(void (*fn)(void))
{
    __asm__ volatile("call *%0" ::"r"(fn) : "eax", "ebx", "ecx", "edx", "cc", "memory");
}

int main(void)
{
    /* DAA/DAS depend only on A, the carry and the half-carry, so every input
     * combination fits in 8 x 2 x 2 x 2 = 2048 cases - test them all. */
    dt_fails = 0;
    dt_iters = 0;
    for (u4 a = 0; a < 256; a++) {
        for (u4 c = 0; c < 2; c++) {
            for (u4 h = 0; h < 2; h++) {
                for (u4 nz = 0; nz < 3; nz++) {
                    static u1 const nzv[] = { 0x00, 0x01, 0x80 };
                    for (int op = 0; op < 2; op++) {
                        dt_iters++;
                        dt_bad = 0;
                        u1 const p0 = (u1)(0x24 | c | h << 3);

                        spcA = (u1)a;
                        spcP = p0;
                        spcNZ = nzv[nz];
                        call_asm(op ? asm_OpDF : asm_OpBE);
                        u1 const aA = spcA, aP = spcP, aNZ = spcNZ;

                        spcA = (u1)a;
                        spcP = p0;
                        spcNZ = nzv[nz];
                        (op ? SpcOpDF : SpcOpBE)(NULL);
                        DT_EQ("A", aA, spcA);
                        DT_EQ("P", aP, spcP);
                        DT_EQ("NZ", aNZ, spcNZ);
                        if (dt_bad) {
                            if (DT_SHOW())
                                printf("  %s A=%02x C=%u H=%u NZ=%02x\n",
                                    op ? "DAA" : "DAS", a, c, h, nzv[nz]);
                            dt_fails++;
                        }
                    }
                }
            }
        }
    }
    DT_DONE("SPC700 DAA/DAS");
}
