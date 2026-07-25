/*
 * cpu/c_spc700.c - entry points for the SPC700 I/O register ($00F0-$00FF)
 * dispatch. The handlers themselves live in cpu/spc_ioregs.h; this file only
 * declares the state they touch and exposes the two functions that the SPC
 * core's WriteByte / ReadByte macros in cpu/spc700.asm call.
 */
#include "../types.h"
#include "../ui.h" /* dspWptr */

/* SPCRAM is 0xFFC0 bytes of RAM immediately followed, in cpu/spc700.asm's data
 * section, by the 64-byte IPL ROM window at $FFC0 - hence the unsized array. */
extern u1 SPCRAM[];
extern u1 DSPMem[256];
extern u1 SPCROM[64], spcextraram[64];
extern u1 disablespcclr, SPCSkipXtraROM;
extern u1 reg1read, reg2read, reg3read, reg4read;
extern u4 spc700read;
extern u1 timeron, timincr0, timincr1, timincr2, timinl0, timinl1, timinl2;
extern u1 spcnumread;

#include "spc_ioregs.h"

u4 SPCWriteReg(u4 reg, u4 eax)
{
    return spc_write_reg(reg, eax);
}

u1 SPCReadReg(u4 reg)
{
    return spc_read_reg(reg);
}
