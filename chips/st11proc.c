/*
 * ST011 coprocessor bank access, from chips/st11proc.asm. Two regions:
 *
 *   Seta11*_68  reads come straight from setaramdata and also update ST011_DR;
 *               writes go through ST011_MapW_68 via the seta11_address /
 *               seta11_byte handshake, and are ignored when address bit 15 is
 *               set (ROM guard)
 *   Seta11*_60  everything through the ST011_MapR_60 / ST011_MapW_60
 *               callbacks, address masked to 2 bits, >= 0x4000 ignored; a
 *               16-bit access is two callbacks with the address wrapping mod 4
 *
 * One departure: Seta11Write16_68 called ST011_MapW_68 twice without updating
 * seta11_byte, so both calls wrote the low byte. Fixed here.
 */

#include <stdint.h>

#include "../cpu/memseam.h"
#include "regabi.h"

extern uint8_t* setaramdata;
extern uint8_t ST011_DR;
extern uint16_t seta11_address;
extern uint8_t seta11_byte;
extern void ST011_MapW_68(void);
extern void ST011_MapR_60(void);
extern void ST011_MapW_60(void);

/* -----------------------------------------------------------------------
 * Region 68 — setaramdata buffer
 * ----------------------------------------------------------------------- */

uint8_t c_Seta11Read8_68(uint32_t addr)
{
    uint8_t val = setaramdata[addr & 0xfff];
    ST011_DR = val;
    return val;
}

void c_Seta11Write8_68(uint32_t addr, uint8_t val)
{
    if (addr & 0x8000)
        return;
    seta11_address = (uint16_t)addr;
    seta11_byte = val;
    ST011_MapW_68();
}

uint16_t c_Seta11Read16_68(uint32_t addr)
{
    uint32_t a = addr & 0xfff;
    uint16_t val = (uint16_t)(setaramdata[a] | ((uint16_t)setaramdata[a + 1] << 8));
    ST011_DR = (uint8_t)(val >> 8);
    return val;
}

void c_Seta11Write16_68(uint32_t addr, uint16_t val)
{
    if (addr & 0x8000)
        return;
    seta11_address = (uint16_t)addr;
    seta11_byte = (uint8_t)val;
    ST011_MapW_68();
    seta11_byte = (uint8_t)(val >> 8);
    seta11_address++;
    ST011_MapW_68();
}

/* -----------------------------------------------------------------------
 * Region 60 — command/status port via callbacks
 * ----------------------------------------------------------------------- */

uint8_t c_Seta11Read8_60(uint32_t addr)
{
    if (addr >= 0x4000)
        return 0;
    seta11_address = (uint16_t)(addr & 3);
    ST011_MapR_60();
    return seta11_byte;
}

void c_Seta11Write8_60(uint32_t addr, uint8_t val)
{
    if (addr >= 0x4000)
        return;
    seta11_address = (uint16_t)(addr & 3);
    seta11_byte = val;
    ST011_MapW_60();
}

uint16_t c_Seta11Read16_60(uint32_t addr)
{
    if (addr >= 0x4000)
        return 0;
    seta11_address = (uint16_t)(addr & 3);
    ST011_MapR_60();
    uint8_t lo = seta11_byte;
    seta11_address = (uint16_t)((seta11_address + 1) & 3);
    ST011_MapR_60();
    uint8_t hi = seta11_byte;
    return (uint16_t)(lo | ((uint16_t)hi << 8));
}

void c_Seta11Write16_60(uint32_t addr, uint16_t val)
{
    if (addr >= 0x4000)
        return;
    seta11_address = (uint16_t)(addr & 3);
    seta11_byte = (uint8_t)val;
    ST011_MapW_60();
    seta11_byte = (uint8_t)(val >> 8);
    seta11_address = (uint16_t)((seta11_address + 1) & 3);
    ST011_MapW_60();
}

MEMBANK_READ8(Seta11Read8_68);
MEMBANK_WRITE8(Seta11Write8_68);
MEMBANK_READ16(Seta11Read16_68);
MEMBANK_WRITE16(Seta11Write16_68);
MEMBANK_READ8(Seta11Read8_60);
MEMBANK_WRITE8(Seta11Write8_60);
MEMBANK_READ16(Seta11Read16_60);
MEMBANK_WRITE16(Seta11Write16_60);
