/*
 * DSP4 bank access, ported from the i386 trampolines that stood in for
 * chips/dsp4proc.asm.
 *
 * Address routing, as in the legacy assembly:
 *   $0000-$7FFF  tail into regaccessbank*
 *   $8000-$BFFF  DSP4 transfer
 *   $C000-$FFFF  nothing at all, the value is left as the caller had it
 *
 * A 16-bit access is two byte transfers with the address stepped in between,
 * low byte first.
 */

#include <stdint.h>

#include "../cpu/memseam.h"

extern uint8_t dsp4_byte;
extern uint16_t dsp4_address;
extern void DSP4GetByte(void);
extern void DSP4SetByte(void);
extern memfn regaccessbankr8, regaccessbankw8, regaccessbankr16, regaccessbankw16;

void DSP4Read8b(void)
{
    if (!(MemSeamC & 0x8000)) {
        regaccessbankr8();
        return;
    }
    if (MemSeamC & 0x4000)
        return;
    dsp4_address = (uint16_t)MemSeamC;
    DSP4GetByte();
    mem_set_al(dsp4_byte);
}

void DSP4Write8b(void)
{
    if (!(MemSeamC & 0x8000)) {
        regaccessbankw8();
        return;
    }
    if (MemSeamC & 0x4000)
        return;
    dsp4_address = (uint16_t)MemSeamC;
    dsp4_byte = (uint8_t)MemSeamA;
    DSP4SetByte();
}

void DSP4Read16b(void)
{
    uint8_t lo;

    if (!(MemSeamC & 0x8000)) {
        regaccessbankr16();
        return;
    }
    if (MemSeamC & 0x4000)
        return;
    dsp4_address = (uint16_t)MemSeamC;
    DSP4GetByte();
    lo = dsp4_byte;
    dsp4_address++;
    DSP4GetByte();
    mem_set_ax((uint16_t)(lo | dsp4_byte << 8));
}

void DSP4Write16b(void)
{
    if (!(MemSeamC & 0x8000)) {
        regaccessbankw16();
        return;
    }
    if (MemSeamC & 0x4000)
        return;
    dsp4_address = (uint16_t)MemSeamC;
    dsp4_byte = (uint8_t)MemSeamA;
    DSP4SetByte();
    dsp4_byte = (uint8_t)(MemSeamA >> 8);
    dsp4_address++;
    DSP4SetByte();
}
