/*
 * DSP3 bank access, ported from the i386 trampolines that stood in for
 * chips/dsp3proc.asm.
 *
 * The DSP3 answers in the top half of the bank; anything below $8000 belongs
 * to the I/O registers, which the assembly reached by tail-jumping into
 * regaccessbank*.  A 16-bit access is two byte transfers with the address
 * stepped in between, low byte first.
 */

#include <stdint.h>

#include "../cpu/memseam.h"

extern uint8_t dsp3_byte;
extern uint16_t dsp3_address;
extern void DSP3GetByte(void);
extern void DSP3SetByte(void);
extern memfn regaccessbankr8, regaccessbankw8, regaccessbankr16, regaccessbankw16;

void DSP3Read8b(void)
{
    if (!(MemSeamC & 0x8000)) {
        regaccessbankr8();
        return;
    }
    dsp3_address = (uint16_t)MemSeamC;
    DSP3GetByte();
    mem_set_al(dsp3_byte);
}

void DSP3Write8b(void)
{
    if (!(MemSeamC & 0x8000)) {
        regaccessbankw8();
        return;
    }
    dsp3_address = (uint16_t)MemSeamC;
    dsp3_byte = (uint8_t)MemSeamA;
    DSP3SetByte();
}

void DSP3Read16b(void)
{
    uint8_t lo;

    if (!(MemSeamC & 0x8000)) {
        regaccessbankr16();
        return;
    }
    dsp3_address = (uint16_t)MemSeamC;
    DSP3GetByte();
    lo = dsp3_byte;
    dsp3_address++;
    DSP3GetByte();
    mem_set_ax((uint16_t)(lo | dsp3_byte << 8));
}

void DSP3Write16b(void)
{
    if (!(MemSeamC & 0x8000)) {
        regaccessbankw16();
        return;
    }
    dsp3_address = (uint16_t)MemSeamC;
    dsp3_byte = (uint8_t)MemSeamA;
    DSP3SetByte();
    dsp3_byte = (uint8_t)(MemSeamA >> 8);
    dsp3_address++;
    DSP3SetByte();
}
