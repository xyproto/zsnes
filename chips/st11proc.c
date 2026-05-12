/*
 * ST011 coprocessor bank-access functions
 *
 * Ported from chips/st11proc.asm.  Two memory regions:
 *
 *   Region 68 (Seta11Read8_68 / Write8_68 / Read16_68 / Write16_68):
 *     Reads access setaramdata directly.  Writes delegate to ST011_MapW_68
 *     via the seta11_address / seta11_byte handshake variables.  Read8 and
 *     Read16 also update ST011_DR as a side effect.  Writes with bit 15 set
 *     in the address are silently ignored (ROM-area guard).
 *
 *   Region 60 (Seta11Read8_60 / Write8_60 / Read16_60 / Write16_60):
 *     All access goes through ST011_MapR_60 / ST011_MapW_60 callbacks.
 *     Address is masked to 2 bits; addresses >= 0x4000 are silently ignored
 *     (reads return 0).  16-bit operations issue two consecutive callbacks
 *     with the address wrapping modulo 4.
 *
 * Asm bug fixed — Seta11Write16_68:
 *   The original asm called ST011_MapW_68 twice but never updated seta11_byte
 *   to the high byte before the second call, so both calls wrote the low byte.
 *   Fixed to write seta11_byte = high byte before the second call, matching
 *   the correct pattern used in Seta11Write16_60.
 */

#include <stdint.h>

extern uint8_t  *setaramdata;
extern uint8_t   ST011_DR;
extern uint16_t  seta11_address;
extern uint8_t   seta11_byte;
extern void ST011_MapW_68(void);
extern void ST011_MapR_60(void);
extern void ST011_MapW_60(void);

/* -----------------------------------------------------------------------
 * Region 68 — setaramdata buffer
 * ----------------------------------------------------------------------- */

uint8_t Seta11Read8_68(uint32_t addr)
{
    uint8_t val = setaramdata[addr & 0xfff];
    ST011_DR = val;
    return val;
}

void Seta11Write8_68(uint32_t addr, uint8_t val)
{
    if (addr & 0x8000)
        return;
    seta11_address = (uint16_t)addr;
    seta11_byte    = val;
    ST011_MapW_68();
}

uint16_t Seta11Read16_68(uint32_t addr)
{
    uint32_t a   = addr & 0xfff;
    uint16_t val = (uint16_t)(setaramdata[a] | ((uint16_t)setaramdata[a + 1] << 8));
    ST011_DR = (uint8_t)(val >> 8);
    return val;
}

void Seta11Write16_68(uint32_t addr, uint16_t val)
{
    if (addr & 0x8000)
        return;
    seta11_address = (uint16_t)addr;
    seta11_byte    = (uint8_t)val;
    ST011_MapW_68();
    seta11_byte = (uint8_t)(val >> 8);
    seta11_address++;
    ST011_MapW_68();
}

/* -----------------------------------------------------------------------
 * Region 60 — command/status port via callbacks
 * ----------------------------------------------------------------------- */

uint8_t Seta11Read8_60(uint32_t addr)
{
    if (addr >= 0x4000)
        return 0;
    seta11_address = (uint16_t)(addr & 3);
    ST011_MapR_60();
    return seta11_byte;
}

void Seta11Write8_60(uint32_t addr, uint8_t val)
{
    if (addr >= 0x4000)
        return;
    seta11_address = (uint16_t)(addr & 3);
    seta11_byte    = val;
    ST011_MapW_60();
}

uint16_t Seta11Read16_60(uint32_t addr)
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

void Seta11Write16_60(uint32_t addr, uint16_t val)
{
    if (addr >= 0x4000)
        return;
    seta11_address = (uint16_t)(addr & 3);
    seta11_byte    = (uint8_t)val;
    ST011_MapW_60();
    seta11_byte    = (uint8_t)(val >> 8);
    seta11_address = (uint16_t)((seta11_address + 1) & 3);
    ST011_MapW_60();
}
