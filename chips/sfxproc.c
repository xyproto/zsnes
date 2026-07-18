/* C port of sfxproc.asm: the SuperFX (GSU) memory-mapped register handlers.
   The GSU core stays in fxemu2*.asm; these just read/write Sfx* state.
   Installed via REGPTR in c_sfxproc.c. */
#include "regabi.h"
#include <stdint.h>

/* SuperFX registers/state (defined in fxemu2.asm) */
extern uint32_t SfxR0, SfxR1, SfxR2, SfxR3, SfxR4, SfxR5, SfxR6, SfxR7, SfxR8, SfxR9, SfxR10, SfxR11, SfxR12, SfxR13, SfxR14, SfxR15;
extern uint32_t SfxSFR, SfxBRAMR, SfxPBR, SfxROMBR, SfxCFGR, SfxSCBR, SfxCLSR, SfxSCMR, SfxPOR, SfxCBR, SfxRAMBR, SfxCarry, SfxSignZero, SfxOverflow, SfxB, SfxAC, SfxCPB, SfxCROM, SfxRAMMem, SfxnRamBanks, SfxPIPE, SFXProc, sfx128lineloc, sfx160lineloc, sfx192lineloc, sfxobjlineloc, sfxclineloc;
extern uint32_t SfxMemTable[256];
extern uint8_t* sfxramdata;
void UpdatePORSCMR(void);
void UpdateSCBRCOLR(void);
void UpdateCLSR(void);

#define BYTE(v, n) (((uint8_t*)&(v))[n])

REGABI_REG_READ8(reg3000r);
uint8_t c_reg3000r(void) { return BYTE(SfxR0, 0); }
REGABI_REG_READ8(reg3001r);
uint8_t c_reg3001r(void) { return BYTE(SfxR0, 1); }
REGABI_REG_WRITE8(reg3000w);
void c_reg3000w(uint8_t v) { BYTE(SfxR0, 0) = v; }
REGABI_REG_WRITE8(reg3001w);
void c_reg3001w(uint8_t v) { BYTE(SfxR0, 1) = v; }
REGABI_REG_READ8(reg3002r);
uint8_t c_reg3002r(void) { return BYTE(SfxR1, 0); }
REGABI_REG_READ8(reg3003r);
uint8_t c_reg3003r(void) { return BYTE(SfxR1, 1); }
REGABI_REG_WRITE8(reg3002w);
void c_reg3002w(uint8_t v) { BYTE(SfxR1, 0) = v; }
REGABI_REG_WRITE8(reg3003w);
void c_reg3003w(uint8_t v) { BYTE(SfxR1, 1) = v; }
REGABI_REG_READ8(reg3004r);
uint8_t c_reg3004r(void) { return BYTE(SfxR2, 0); }
REGABI_REG_READ8(reg3005r);
uint8_t c_reg3005r(void) { return BYTE(SfxR2, 1); }
REGABI_REG_WRITE8(reg3004w);
void c_reg3004w(uint8_t v) { BYTE(SfxR2, 0) = v; }
REGABI_REG_WRITE8(reg3005w);
void c_reg3005w(uint8_t v) { BYTE(SfxR2, 1) = v; }
REGABI_REG_READ8(reg3006r);
uint8_t c_reg3006r(void) { return BYTE(SfxR3, 0); }
REGABI_REG_READ8(reg3007r);
uint8_t c_reg3007r(void) { return BYTE(SfxR3, 1); }
REGABI_REG_WRITE8(reg3006w);
void c_reg3006w(uint8_t v) { BYTE(SfxR3, 0) = v; }
REGABI_REG_WRITE8(reg3007w);
void c_reg3007w(uint8_t v) { BYTE(SfxR3, 1) = v; }
REGABI_REG_READ8(reg3008r);
uint8_t c_reg3008r(void) { return BYTE(SfxR4, 0); }
REGABI_REG_READ8(reg3009r);
uint8_t c_reg3009r(void) { return BYTE(SfxR4, 1); }
REGABI_REG_WRITE8(reg3008w);
void c_reg3008w(uint8_t v) { BYTE(SfxR4, 0) = v; }
REGABI_REG_WRITE8(reg3009w);
void c_reg3009w(uint8_t v) { BYTE(SfxR4, 1) = v; }
REGABI_REG_READ8(reg300Ar);
uint8_t c_reg300Ar(void) { return BYTE(SfxR5, 0); }
REGABI_REG_READ8(reg300Br);
uint8_t c_reg300Br(void) { return BYTE(SfxR5, 1); }
REGABI_REG_WRITE8(reg300Aw);
void c_reg300Aw(uint8_t v) { BYTE(SfxR5, 0) = v; }
REGABI_REG_WRITE8(reg300Bw);
void c_reg300Bw(uint8_t v) { BYTE(SfxR5, 1) = v; }
REGABI_REG_READ8(reg300Cr);
uint8_t c_reg300Cr(void) { return BYTE(SfxR6, 0); }
REGABI_REG_READ8(reg300Dr);
uint8_t c_reg300Dr(void) { return BYTE(SfxR6, 1); }
REGABI_REG_WRITE8(reg300Cw);
void c_reg300Cw(uint8_t v) { BYTE(SfxR6, 0) = v; }
REGABI_REG_WRITE8(reg300Dw);
void c_reg300Dw(uint8_t v) { BYTE(SfxR6, 1) = v; }
REGABI_REG_READ8(reg300Er);
uint8_t c_reg300Er(void) { return BYTE(SfxR7, 0); }
REGABI_REG_READ8(reg300Fr);
uint8_t c_reg300Fr(void) { return BYTE(SfxR7, 1); }
REGABI_REG_WRITE8(reg300Ew);
void c_reg300Ew(uint8_t v) { BYTE(SfxR7, 0) = v; }
REGABI_REG_WRITE8(reg300Fw);
void c_reg300Fw(uint8_t v) { BYTE(SfxR7, 1) = v; }
REGABI_REG_READ8(reg3010r);
uint8_t c_reg3010r(void) { return BYTE(SfxR8, 0); }
REGABI_REG_READ8(reg3011r);
uint8_t c_reg3011r(void) { return BYTE(SfxR8, 1); }
REGABI_REG_WRITE8(reg3010w);
void c_reg3010w(uint8_t v) { BYTE(SfxR8, 0) = v; }
REGABI_REG_WRITE8(reg3011w);
void c_reg3011w(uint8_t v) { BYTE(SfxR8, 1) = v; }
REGABI_REG_READ8(reg3012r);
uint8_t c_reg3012r(void) { return BYTE(SfxR9, 0); }
REGABI_REG_READ8(reg3013r);
uint8_t c_reg3013r(void) { return BYTE(SfxR9, 1); }
REGABI_REG_WRITE8(reg3012w);
void c_reg3012w(uint8_t v) { BYTE(SfxR9, 0) = v; }
REGABI_REG_WRITE8(reg3013w);
void c_reg3013w(uint8_t v) { BYTE(SfxR9, 1) = v; }
REGABI_REG_READ8(reg3014r);
uint8_t c_reg3014r(void) { return BYTE(SfxR10, 0); }
REGABI_REG_READ8(reg3015r);
uint8_t c_reg3015r(void) { return BYTE(SfxR10, 1); }
REGABI_REG_WRITE8(reg3014w);
void c_reg3014w(uint8_t v) { BYTE(SfxR10, 0) = v; }
REGABI_REG_WRITE8(reg3015w);
void c_reg3015w(uint8_t v) { BYTE(SfxR10, 1) = v; }
REGABI_REG_READ8(reg3016r);
uint8_t c_reg3016r(void) { return BYTE(SfxR11, 0); }
REGABI_REG_READ8(reg3017r);
uint8_t c_reg3017r(void) { return BYTE(SfxR11, 1); }
REGABI_REG_WRITE8(reg3016w);
void c_reg3016w(uint8_t v) { BYTE(SfxR11, 0) = v; }
REGABI_REG_WRITE8(reg3017w);
void c_reg3017w(uint8_t v) { BYTE(SfxR11, 1) = v; }
REGABI_REG_READ8(reg3018r);
uint8_t c_reg3018r(void) { return BYTE(SfxR12, 0); }
REGABI_REG_READ8(reg3019r);
uint8_t c_reg3019r(void) { return BYTE(SfxR12, 1); }
REGABI_REG_WRITE8(reg3018w);
void c_reg3018w(uint8_t v) { BYTE(SfxR12, 0) = v; }
REGABI_REG_WRITE8(reg3019w);
void c_reg3019w(uint8_t v) { BYTE(SfxR12, 1) = v; }
REGABI_REG_READ8(reg301Ar);
uint8_t c_reg301Ar(void) { return BYTE(SfxR13, 0); }
REGABI_REG_READ8(reg301Br);
uint8_t c_reg301Br(void) { return BYTE(SfxR13, 1); }
REGABI_REG_WRITE8(reg301Aw);
void c_reg301Aw(uint8_t v) { BYTE(SfxR13, 0) = v; }
REGABI_REG_WRITE8(reg301Bw);
void c_reg301Bw(uint8_t v) { BYTE(SfxR13, 1) = v; }
REGABI_REG_READ8(reg301Cr);
uint8_t c_reg301Cr(void) { return BYTE(SfxR14, 0); }
REGABI_REG_READ8(reg301Dr);
uint8_t c_reg301Dr(void) { return BYTE(SfxR14, 1); }
REGABI_REG_WRITE8(reg301Cw);
void c_reg301Cw(uint8_t v) { BYTE(SfxR14, 0) = v; }
REGABI_REG_WRITE8(reg301Dw);
void c_reg301Dw(uint8_t v) { BYTE(SfxR14, 1) = v; }
REGABI_REG_READ8(reg301Er);
uint8_t c_reg301Er(void) { return BYTE(SfxR15, 0); }
REGABI_REG_READ8(reg301Fr);
uint8_t c_reg301Fr(void) { return BYTE(SfxR15, 1); }
REGABI_REG_WRITE8(reg301Ew);
void c_reg301Ew(uint8_t v) { BYTE(SfxR15, 0) = v; }
REGABI_REG_WRITE8(reg301Fw);
void c_reg301Fw(uint8_t v)
{
    BYTE(SfxR15, 1) = v;
    /* start execution; the IRQ flag is only set when the GSU stops */
    uint8_t* p = (uint8_t*)(uintptr_t)SfxMemTable[(uint8_t)SfxPBR] + (uint16_t)SfxR15;
    BYTE(SfxPIPE, 0) = *p;
    *(uint16_t*)&SfxR15 += 1;
    BYTE(SfxSFR, 0) |= 0x20;
    SFXProc = 1;
}
REGABI_REG_READ8(reg3030r);
uint8_t c_reg3030r(void)
{
    uint16_t* sfr = (uint16_t*)&SfxSFR;
    *sfr &= 0x8F60;
    if (SfxCarry & 1)
        *sfr |= 0x04;
    if ((uint16_t)SfxSignZero == 0)
        *sfr |= 0x02;
    if (SfxSignZero & 0x8000)
        *sfr |= 0x08;
    if ((uint8_t)SfxOverflow)
        *sfr |= 0x10;
    if ((uint8_t)SfxB)
        *sfr |= 0x1000;
    return BYTE(SfxSFR, 0);
}
REGABI_REG_READ8(reg3031r);
uint8_t c_reg3031r(void)
{
    // Reading clears the IRQ flag (bit 7), but must return the old value
    // so IRQ handlers can identify the interrupt source.
    uint8_t v = BYTE(SfxSFR, 1);
    BYTE(SfxSFR, 1) = v & 0x7F;
    return v;
}
REGABI_REG_READ8(reg3032r);
uint8_t c_reg3032r(void)
{
    return 0;
}
REGABI_REG_READ8(reg3033r);
uint8_t c_reg3033r(void)
{
    return BYTE(SfxBRAMR, 0);
}
REGABI_REG_READ8(reg3034r);
uint8_t c_reg3034r(void)
{
    return BYTE(SfxPBR, 0);
}
REGABI_REG_READ8(reg3035r);
uint8_t c_reg3035r(void)
{
    return 0;
}
REGABI_REG_READ8(reg3036r);
uint8_t c_reg3036r(void)
{
    return BYTE(SfxROMBR, 0);
}
REGABI_REG_READ8(reg3037r);
uint8_t c_reg3037r(void)
{
    return BYTE(SfxCFGR, 0);
}
REGABI_REG_READ8(reg3038r);
uint8_t c_reg3038r(void)
{
    return BYTE(SfxSCBR, 0);
}
REGABI_REG_READ8(reg3039r);
uint8_t c_reg3039r(void)
{
    return BYTE(SfxCLSR, 0);
}
REGABI_REG_READ8(reg303Ar);
uint8_t c_reg303Ar(void)
{
    uint8_t r = BYTE(SfxSCMR, 0);
    uint32_t loc;
    if (SfxPOR & 0x10)
        loc = sfxobjlineloc;
    else
        switch (((uint8_t)SfxSCMR) & 0x24) {
        case 0x04:
            loc = sfx160lineloc;
            break;
        case 0x20:
            loc = sfx192lineloc;
            break;
        case 0x24:
            loc = sfxobjlineloc;
            break;
        default:
            loc = sfx128lineloc;
            break;
        }
    sfxclineloc = loc;
    (void)r;
    return (uint8_t)loc; /* asm returns low byte of the line location */
}
REGABI_REG_READ8(reg303Br);
uint8_t c_reg303Br(void)
{
    return 0x20; /* VCR */
}
REGABI_REG_READ8(reg303Cr);
uint8_t c_reg303Cr(void)
{
    return BYTE(SfxRAMBR, 0);
}
REGABI_REG_READ8(reg303Dr);
uint8_t c_reg303Dr(void)
{
    return 0;
}
REGABI_REG_READ8(reg303Er);
uint8_t c_reg303Er(void)
{
    return BYTE(SfxCBR, 0);
}
REGABI_REG_READ8(reg303Fr);
uint8_t c_reg303Fr(void)
{
    return BYTE(SfxCBR, 1);
}
REGABI_REG_WRITE8(reg3030w);
void c_reg3030w(uint8_t v)
{
    BYTE(SfxSFR, 0) = v;
    BYTE(SfxAC, 0) = 1;
}
REGABI_REG_WRITE8(reg3031w);
void c_reg3031w(uint8_t v)
{
    BYTE(SfxSFR, 1) = v;
}
REGABI_REG_WRITE8(reg3032w);
void c_reg3032w(uint8_t v)
{
    (void)v;
}
REGABI_REG_WRITE8(reg3033w);
void c_reg3033w(uint8_t v)
{
    BYTE(SfxBRAMR, 0) = v & 0xFE;
}
REGABI_REG_WRITE8(reg3034w);
void c_reg3034w(uint8_t v)
{
    v &= 0x7F;
    BYTE(SfxPBR, 0) = v;
    SfxCPB = SfxMemTable[v];
}
REGABI_REG_WRITE8(reg3035w);
void c_reg3035w(uint8_t v)
{
    (void)v;
}
REGABI_REG_WRITE8(reg3036w);
void c_reg3036w(uint8_t v)
{
    v &= 0x7F;
    BYTE(SfxROMBR, 0) = v;
    SfxCROM = SfxMemTable[v];
}
REGABI_REG_WRITE8(reg3037w);
void c_reg3037w(uint8_t v)
{
    BYTE(SfxCFGR, 0) = v;
}
REGABI_REG_WRITE8(reg3038w);
void c_reg3038w(uint8_t v)
{
    BYTE(SfxSCBR, 0) = v;
    UpdateSCBRCOLR();
}
REGABI_REG_WRITE8(reg3039w);
void c_reg3039w(uint8_t v)
{
    BYTE(SfxCLSR, 0) = v & 0x01; // bit 0 selects 10.7/21.4 MHz
    UpdateCLSR();
}
REGABI_REG_WRITE8(reg303Aw);
void c_reg303Aw(uint8_t v)
{
    BYTE(SfxSCMR, 0) = v;
    UpdatePORSCMR();
}
REGABI_REG_WRITE8(reg303Bw);
void c_reg303Bw(uint8_t v)
{
    (void)v; /* VCR */
}
REGABI_REG_WRITE8(reg303Cw);
void c_reg303Cw(uint8_t v)
{
    uint32_t mask = SfxnRamBanks - 1;
    SfxRAMBR = v & mask;
    SfxRAMMem = (uintptr_t)sfxramdata + ((SfxRAMBR & 0xFF) << 16);
}
REGABI_REG_WRITE8(reg303Dw);
void c_reg303Dw(uint8_t v)
{
    (void)v;
}
REGABI_REG_WRITE8(reg303Ew);
void c_reg303Ew(uint8_t v)
{
    BYTE(SfxCBR, 0) = v;
}
REGABI_REG_WRITE8(reg303Fw);
void c_reg303Fw(uint8_t v)
{
    BYTE(SfxCBR, 1) = v;
}

/* cache region (0x3100-0x32FF): writes set a warning flag */
uint8_t cachewarning;
REGABI_REG_READ8(cacheregr);
uint8_t c_cacheregr(void)
{
    cachewarning |= 1;
    return 0;
}
REGABI_REG_WRITE8(cacheregw);
void c_cacheregw(uint8_t v)
{
    (void)v;
    cachewarning |= 2;
}

/* SuperFX RAM access: sfxramdata[addr + bank*64K], banks 0..3 = ""/b/c/d */
REGABI_BANK_READ8(sfxaccessbankr8);
uint8_t c_sfxaccessbankr8(uint32_t a) { return sfxramdata[a + 0]; }
REGABI_BANK_WRITE8(sfxaccessbankw8);
void c_sfxaccessbankw8(uint32_t a, uint8_t v) { sfxramdata[a + 0] = v; }
REGABI_BANK_READ16(sfxaccessbankr16);
uint16_t c_sfxaccessbankr16(uint32_t a) { return *(uint16_t*)(sfxramdata + a + 0); }
REGABI_BANK_WRITE16(sfxaccessbankw16);
void c_sfxaccessbankw16(uint32_t a, uint16_t v) { *(uint16_t*)(sfxramdata + a + 0) = v; }
REGABI_BANK_READ8(sfxaccessbankr8b);
uint8_t c_sfxaccessbankr8b(uint32_t a) { return sfxramdata[a + 65536]; }
REGABI_BANK_WRITE8(sfxaccessbankw8b);
void c_sfxaccessbankw8b(uint32_t a, uint8_t v) { sfxramdata[a + 65536] = v; }
REGABI_BANK_READ16(sfxaccessbankr16b);
uint16_t c_sfxaccessbankr16b(uint32_t a) { return *(uint16_t*)(sfxramdata + a + 65536); }
REGABI_BANK_WRITE16(sfxaccessbankw16b);
void c_sfxaccessbankw16b(uint32_t a, uint16_t v) { *(uint16_t*)(sfxramdata + a + 65536) = v; }
REGABI_BANK_READ8(sfxaccessbankr8c);
uint8_t c_sfxaccessbankr8c(uint32_t a) { return sfxramdata[a + 131072]; }
REGABI_BANK_WRITE8(sfxaccessbankw8c);
void c_sfxaccessbankw8c(uint32_t a, uint8_t v) { sfxramdata[a + 131072] = v; }
REGABI_BANK_READ16(sfxaccessbankr16c);
uint16_t c_sfxaccessbankr16c(uint32_t a) { return *(uint16_t*)(sfxramdata + a + 131072); }
REGABI_BANK_WRITE16(sfxaccessbankw16c);
void c_sfxaccessbankw16c(uint32_t a, uint16_t v) { *(uint16_t*)(sfxramdata + a + 131072) = v; }
REGABI_BANK_READ8(sfxaccessbankr8d);
uint8_t c_sfxaccessbankr8d(uint32_t a) { return sfxramdata[a + 196608]; }
REGABI_BANK_WRITE8(sfxaccessbankw8d);
void c_sfxaccessbankw8d(uint32_t a, uint8_t v) { sfxramdata[a + 196608] = v; }
REGABI_BANK_READ16(sfxaccessbankr16d);
uint16_t c_sfxaccessbankr16d(uint32_t a) { return *(uint16_t*)(sfxramdata + a + 196608); }
REGABI_BANK_WRITE16(sfxaccessbankw16d);
void c_sfxaccessbankw16d(uint32_t a, uint16_t v) { *(uint16_t*)(sfxramdata + a + 196608) = v; }
