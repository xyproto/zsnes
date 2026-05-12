/*
 * OBC1 coprocessor bank-access functions
 *
 * Ported from chips/obc1proc.asm.  All four functions share the same
 * three-way address routing:
 *
 *   addr bit 15 set  → delegate to memaccessbank (ROM/WRAM mirror)
 *   addr < 0x6000    → delegate to regaccessbank (PPU/CPU registers)
 *   [0x6000, 0x7FFF] → OBC1 local access via GetOBC1 / SetOBC1
 *
 * 16-bit operations issue two consecutive GetOBC1 / SetOBC1 calls,
 * incrementing obc1_address between them.  Low byte is transferred first.
 */

#include <stdint.h>

extern uint16_t obc1_address;
extern uint8_t obc1_byte;
extern void GetOBC1(void);
extern void SetOBC1(void);

extern uint8_t regaccessbankr8(uint32_t addr);
extern void regaccessbankw8(uint32_t addr, uint8_t val);
extern uint16_t regaccessbankr16(uint32_t addr);
extern void regaccessbankw16(uint32_t addr, uint16_t val);
extern uint8_t memaccessbankr8(uint32_t addr);
extern void memaccessbankw8(uint32_t addr, uint8_t val);
extern uint16_t memaccessbankr16(uint32_t addr);
extern void memaccessbankw16(uint32_t addr, uint16_t val);

uint8_t OBC1Read8b(uint32_t addr)
{
    if (addr & 0x8000)
        return memaccessbankr8(addr);
    if (addr < 0x6000)
        return regaccessbankr8(addr);
    obc1_address = (uint16_t)addr;
    GetOBC1();
    return obc1_byte;
}

void OBC1Write8b(uint32_t addr, uint8_t val)
{
    if (addr & 0x8000) {
        memaccessbankw8(addr, val);
        return;
    }
    if (addr < 0x6000) {
        regaccessbankw8(addr, val);
        return;
    }
    obc1_address = (uint16_t)addr;
    obc1_byte = val;
    SetOBC1();
}

uint16_t OBC1Read16b(uint32_t addr)
{
    if (addr & 0x8000)
        return memaccessbankr16(addr);
    if (addr < 0x6000)
        return regaccessbankr16(addr);
    obc1_address = (uint16_t)addr;
    GetOBC1();
    uint8_t lo = obc1_byte;
    obc1_address++;
    GetOBC1();
    uint8_t hi = obc1_byte;
    return (uint16_t)(lo | ((uint16_t)hi << 8));
}

void OBC1Write16b(uint32_t addr, uint16_t val)
{
    if (addr & 0x8000) {
        memaccessbankw16(addr, val);
        return;
    }
    if (addr < 0x6000) {
        regaccessbankw16(addr, val);
        return;
    }
    obc1_address = (uint16_t)addr;
    obc1_byte = (uint8_t)val;
    SetOBC1();
    obc1_byte = (uint8_t)(val >> 8);
    obc1_address++;
    SetOBC1();
}
