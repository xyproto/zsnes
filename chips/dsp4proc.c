/*
 * Pure-C port of dsp4proc.c (formerly inline asm).
 *
 * Routing:
 *   addr < 0x8000  → regaccessbank* (bit 15 clear)
 *   0xC000-0xFFFF  → no-op / return 0 (bits 15+14 both set)
 *   0x8000-0xBFFF  → DSP4 chip registers
 */

#include "../types.h"
#include "../cpu/memory.h"

extern u1   dsp4_byte;
extern u2   dsp4_address;
extern void DSP4GetByte(void);
extern void DSP4SetByte(void);

u1 __attribute__((fastcall)) DSP4Read8b(u2 addr)
{
    if (!(addr & 0x8000))
        return regaccessbankr8(addr);
    if (addr & 0x4000)
        return 0;
    dsp4_address = addr;
    DSP4GetByte();
    return dsp4_byte;
}

void __attribute__((fastcall)) DSP4Write8b(u2 addr, u1 val)
{
    if (!(addr & 0x8000)) { regaccessbankw8(addr, val); return; }
    if (addr & 0x4000) return;
    dsp4_address = addr;
    dsp4_byte    = val;
    DSP4SetByte();
}

u2 __attribute__((fastcall)) DSP4Read16b(u2 addr)
{
    if (!(addr & 0x8000))
        return regaccessbankr16(addr);
    if (addr & 0x4000)
        return 0;
    dsp4_address = addr;
    DSP4GetByte();
    u1 lo = dsp4_byte;
    dsp4_address++;
    DSP4GetByte();
    return (u2)lo | ((u2)dsp4_byte << 8);
}

void __attribute__((fastcall)) DSP4Write16b(u2 addr, u2 val)
{
    if (!(addr & 0x8000)) { regaccessbankw16(addr, val); return; }
    if (addr & 0x4000) return;
    dsp4_address = addr;
    dsp4_byte    = (u1)val;
    DSP4SetByte();
    dsp4_address++;
    dsp4_byte = (u1)(val >> 8);
    DSP4SetByte();
}
