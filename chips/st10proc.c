/*
 * ST010 / ST011 coprocessor bank access, from chips/st10proc.asm. Two regions:
 *
 *   setaaccessbank*    the 4 KiB setaramdata buffer; a write runs
 *                      ST010DoCommand when setaramdata[0x21] == 0x80
 *   setaaccessbank*a   the 4-byte SetaCmdEnable register, 2-bit address mask
 *                      with a 0x4000 guard
 *
 * Two departures from the assembly. setaaccessbankw16 at 0x7FFF: NASM's
 * `mov [setaramdata+0fffh], al` addressed the *pointer* plus 0xFFF rather than
 * the buffer, so this writes buffer[0xFFF] as intended. And setaaccessbankw16a
 * wrote setaramdata instead of SetaCmdEnable, which made read and write
 * asymmetric; fixed here.
 */

#include <stdint.h>

#include "../cpu/memseam.h"
#include "regabi.h"

extern uint8_t* setaramdata;
extern void ST010DoCommand(void);

uint8_t SetaCmdEnable[4];

/* -----------------------------------------------------------------------
 * Region A — setaramdata buffer
 * ----------------------------------------------------------------------- */

uint8_t c_setaaccessbankr8(uint32_t addr)
{
    return setaramdata[addr & 0xfff];
}

void c_setaaccessbankw8(uint32_t addr, uint8_t val)
{
    if (addr & 0x8000)
        return;
    setaramdata[addr & 0xfff] = val;
    if (setaramdata[0x21] == 0x80)
        ST010DoCommand();
}

uint16_t c_setaaccessbankr16(uint32_t addr)
{
    uint32_t a = addr & 0xfff;
    return (uint16_t)(setaramdata[a] | ((uint16_t)setaramdata[a + 1] << 8));
}

void c_setaaccessbankw16(uint32_t addr, uint16_t val)
{
    if (addr & 0x8000)
        return;

    /* Only the low byte reaches the RAM side of a ROM/RAM boundary word. */
    if (addr == 0x7fff) {
        setaramdata[0xfff] = (uint8_t)val;
        return;
    }

    uint32_t a = addr & 0xfff;

    /* Word straddles the end of the 4 KiB window; high byte wraps to [0]. */
    if (a == 0xfff) {
        setaramdata[0xfff] = (uint8_t)val;
        setaramdata[0] = (uint8_t)(val >> 8);
        return;
    }

    setaramdata[a] = (uint8_t)val;
    setaramdata[a + 1] = (uint8_t)(val >> 8);
    if (setaramdata[0x21] == 0x80)
        ST010DoCommand();
}

/* -----------------------------------------------------------------------
 * Region B — SetaCmdEnable register
 * ----------------------------------------------------------------------- */

uint8_t c_setaaccessbankr8a(uint32_t addr)
{
    if (addr >= 0x4000)
        return 0;
    return SetaCmdEnable[addr & 3];
}

void c_setaaccessbankw8a(uint32_t addr, uint8_t val)
{
    if (addr >= 0x4000)
        return;
    SetaCmdEnable[addr & 3] = val;
}

uint16_t c_setaaccessbankr16a(uint32_t addr)
{
    if (addr >= 0x4000)
        return 0;
    uint32_t a = addr & 3;
    return (uint16_t)((SetaCmdEnable[a] << 8) | SetaCmdEnable[(a + 1) & 3]);
}

void c_setaaccessbankw16a(uint32_t addr, uint16_t val)
{
    if (addr >= 0x4000)
        return;
    uint32_t a = addr & 3;
    SetaCmdEnable[a] = (uint8_t)(val >> 8);
    SetaCmdEnable[(a + 1) & 3] = (uint8_t)val;
}

MEMBANK_READ8(setaaccessbankr8);
MEMBANK_WRITE8(setaaccessbankw8);
MEMBANK_READ16(setaaccessbankr16);
MEMBANK_WRITE16(setaaccessbankw16);
MEMBANK_READ8(setaaccessbankr8a);
MEMBANK_WRITE8(setaaccessbankw8a);
MEMBANK_READ16(setaaccessbankr16a);
MEMBANK_WRITE16(setaaccessbankw16a);
