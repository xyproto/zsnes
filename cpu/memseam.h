#ifndef MEMSEAM_H
#define MEMSEAM_H

#include <stdint.h>

/*
 * The memtable/Bank0dat calling convention, formerly the x86 register ABI of
 * cpu/memory.asm.
 *
 * A handler takes nothing and returns nothing.  The caller leaves the bank in
 * MemSeamB, the address in MemSeamC, the value in MemSeamA (al on an 8-bit
 * write, ax on a 16-bit one) and the core's edx in MemSeamD, and reads all
 * four back afterwards: handlers advance the address, clear the bank, or hand
 * a nested access to another handler, and the caller keeps whatever they left.
 *
 * Call sites (cpu/ops65816.h mem_call, cpu/memtable.h) save and restore the
 * seam around the call, so it behaves like the callee-saved register set it
 * replaces even when a register write starts a DMA that reenters here.
 */
/* Pointer-wide, not uint32_t: these stand in for the assembly's ebx/ecx/eax/
   edx, and ebx in particular carries a host RAM base on the bank paths. On
   i386 uintptr_t is the same type and nothing changes. */
extern uintptr_t MemSeamB, MemSeamC, MemSeamA, MemSeamD;

/* The core's esi, i.e. the 65816 program counter. Only the sound-skip hack in
   the $2140-$2143 handlers reads it, and only when SPC emulation is off; the
   assembly took it straight out of the register, which stopped holding the PC
   once the opcode core became C. mem_call and bank0_call publish it here. */
/* Pointer-wide: this is the PC, a host pointer, not a 32-bit register. */
extern uintptr_t MemSeamS;

typedef void memfn(void);

/* Reads land in al/ax and leave the rest of eax alone: the 65816 core keeps
   live values in the upper half. */
static inline void mem_set_al(uint8_t const v)
{
    MemSeamA = (MemSeamA & ~(uintptr_t)0xFFu) | v;
}

static inline void mem_set_ax(uint16_t const v)
{
    MemSeamA = (MemSeamA & ~(uintptr_t)0xFFFFu) | v;
}

/* Hand an access on to another memtable handler, the way the assembly's
   tail-jump did: the bank the caller left in MemSeamB stays where it is. */
static inline uint8_t mem_bank_read8(memfn* const fn, uint32_t const addr)
{
    MemSeamC = addr;
    fn();
    return (uint8_t)MemSeamA;
}

static inline uint16_t mem_bank_read16(memfn* const fn, uint32_t const addr)
{
    MemSeamC = addr;
    fn();
    return (uint16_t)MemSeamA;
}

static inline void mem_bank_write8(memfn* const fn, uint32_t const addr, uint8_t const val)
{
    MemSeamC = addr;
    mem_set_al(val);
    fn();
}

static inline void mem_bank_write16(memfn* const fn, uint32_t const addr, uint16_t const val)
{
    MemSeamC = addr;
    mem_set_ax(val);
    fn();
}

/* Wrap a cdecl c_<name> body in the convention.  These replace the
   REGABI_BANK_* trampolines in chips/regabi.h, which are still needed for the
   I/O register tables that cpu/table.asm indexes but no longer for anything
   the memtable holds. */
#define MEMBANK_READ8(name)               \
    uint8_t c_##name(uint32_t);           \
    void name(void)                       \
    {                                     \
        mem_set_al(c_##name(MemSeamC));   \
    }                                     \
    uint8_t c_##name(uint32_t)

#define MEMBANK_READ16(name)              \
    uint16_t c_##name(uint32_t);          \
    void name(void)                       \
    {                                     \
        mem_set_ax(c_##name(MemSeamC));   \
    }                                     \
    uint16_t c_##name(uint32_t)

#define MEMBANK_WRITE8(name)                          \
    void c_##name(uint32_t, uint8_t);                 \
    void name(void)                                   \
    {                                                 \
        c_##name(MemSeamC, (uint8_t)MemSeamA);        \
    }                                                 \
    void c_##name(uint32_t, uint8_t)

#define MEMBANK_WRITE16(name)                         \
    void c_##name(uint32_t, uint16_t);                \
    void name(void)                                   \
    {                                                 \
        c_##name(MemSeamC, (uint16_t)MemSeamA);       \
    }                                                 \
    void c_##name(uint32_t, uint16_t)

#endif
