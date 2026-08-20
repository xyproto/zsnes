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
 *
 * The c_* functions are the portable implementations; the OBC1* entry points
 * the memory map dispatches to wrap them in the seam convention (cpu/memseam.h).
 * A routed address is handed to another memtable handler, which reads the bank
 * straight out of MemSeamB, exactly as the assembly's tail-jump did.
 */

#include <stdint.h>

#include "../cpu/memseam.h"

extern uint16_t obc1_address;
extern uint8_t obc1_byte;
extern void GetOBC1(void);
extern void SetOBC1(void);

extern memfn regaccessbankr8, regaccessbankw8, regaccessbankr16, regaccessbankw16;
extern memfn memaccessbankr8, memaccessbankw8, memaccessbankr16, memaccessbankw16;

uint8_t c_OBC1Read8b(uint32_t addr)
{
    if (addr & 0x8000)
        return mem_bank_read8(memaccessbankr8, addr);
    if (addr < 0x6000)
        return mem_bank_read8(regaccessbankr8, addr);
    obc1_address = (uint16_t)addr;
    GetOBC1();
    return obc1_byte;
}

void c_OBC1Write8b(uint32_t addr, uint8_t val)
{
    if (addr & 0x8000) {
        mem_bank_write8(memaccessbankw8, addr, val);
        return;
    }
    if (addr < 0x6000) {
        mem_bank_write8(regaccessbankw8, addr, val);
        return;
    }
    obc1_address = (uint16_t)addr;
    obc1_byte = val;
    SetOBC1();
}

uint16_t c_OBC1Read16b(uint32_t addr)
{
    if (addr & 0x8000)
        return mem_bank_read16(memaccessbankr16, addr);
    if (addr < 0x6000)
        return mem_bank_read16(regaccessbankr16, addr);
    obc1_address = (uint16_t)addr;
    GetOBC1();
    uint8_t lo = obc1_byte;
    obc1_address++;
    GetOBC1();
    uint8_t hi = obc1_byte;
    return (uint16_t)(lo | ((uint16_t)hi << 8));
}

void c_OBC1Write16b(uint32_t addr, uint16_t val)
{
    if (addr & 0x8000) {
        mem_bank_write16(memaccessbankw16, addr, val);
        return;
    }
    if (addr < 0x6000) {
        mem_bank_write16(regaccessbankw16, addr, val);
        return;
    }
    obc1_address = (uint16_t)addr;
    obc1_byte = (uint8_t)val;
    SetOBC1();
    obc1_byte = (uint8_t)(val >> 8);
    obc1_address++;
    SetOBC1();
}

/* The memtable entry points. */
void OBC1Read8b(void)
{
    mem_set_al(c_OBC1Read8b(MemSeamC));
}

void OBC1Write8b(void)
{
    c_OBC1Write8b(MemSeamC, (uint8_t)MemSeamA);
}

void OBC1Read16b(void)
{
    mem_set_ax(c_OBC1Read16b(MemSeamC));
}

void OBC1Write16b(void)
{
    c_OBC1Write16b(MemSeamC, (uint16_t)MemSeamA);
}
