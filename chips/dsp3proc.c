/*
 * Pure-C port of dsp3proc.c (formerly inline asm).
 *
 * Routing: addr & 0x8000 → DSP3 chip; else → regaccessbank*
 */

#include "../cpu/memory.h"
#include "../types.h"

extern u1 dsp3_byte;
extern u2 dsp3_address;
extern void DSP3GetByte(void);
extern void DSP3SetByte(void);

u1 __attribute__((fastcall)) DSP3Read8b(u2 addr)
{
    if (!(addr & 0x8000))
        return regaccessbankr8(addr);
    dsp3_address = addr;
    DSP3GetByte();
    return dsp3_byte;
}

void __attribute__((fastcall)) DSP3Write8b(u2 addr, u1 val)
{
    if (!(addr & 0x8000)) {
        regaccessbankw8(addr, val);
        return;
    }
    dsp3_address = addr;
    dsp3_byte = val;
    DSP3SetByte();
}

u2 __attribute__((fastcall)) DSP3Read16b(u2 addr)
{
    if (!(addr & 0x8000))
        return regaccessbankr16(addr);
    dsp3_address = addr;
    DSP3GetByte();
    u1 lo = dsp3_byte;
    dsp3_address++;
    DSP3GetByte();
    return (u2)lo | ((u2)dsp3_byte << 8);
}

void __attribute__((fastcall)) DSP3Write16b(u2 addr, u2 val)
{
    if (!(addr & 0x8000)) {
        regaccessbankw16(addr, val);
        return;
    }
    dsp3_address = addr;
    dsp3_byte = (u1)val;
    DSP3SetByte();
    dsp3_address++;
    dsp3_byte = (u1)(val >> 8);
    DSP3SetByte();
}
