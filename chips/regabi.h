#ifndef REGABI_H
#define REGABI_H

#include <stdint.h>

#include "../cpu/memseam.h"

/*
 * The I/O register handlers ($2100-$437F, reached through regptra/regptwa).
 * These were trampolines while the core was assembly; now they are plain C
 * over the seam in cpu/memseam.h, the convention the memtable uses. The macro
 * names and c_<name> signatures are unchanged, so the ~300 handlers did not
 * have to move.
 *
 *   REG   no address; the table index already is one
 *   BANK  takes the address out of MemSeamC
 *   _DX   passes MemSeamD through, for the beam and hblank registers
 */

#if defined(__APPLE__) || defined(__MINGW32__)
#define REGABI_SYM(x) "_" #x
#else
#define REGABI_SYM(x) #x
#endif

#define REGABI_ENTRY(name) \
    ".globl " REGABI_SYM(name) "\n" REGABI_SYM(name) ":\n"

#define REGABI_BANK_READ8(name)         \
    uint8_t c_##name(uint32_t);         \
    void name(void)                     \
    {                                   \
        mem_set_al(c_##name(MemSeamC)); \
    }                                   \
    uint8_t c_##name(uint32_t)

#define REGABI_BANK_READ16(name)        \
    uint16_t c_##name(uint32_t);        \
    void name(void)                     \
    {                                   \
        mem_set_ax(c_##name(MemSeamC)); \
    }                                   \
    uint16_t c_##name(uint32_t)

#define REGABI_BANK_WRITE8(name)               \
    void c_##name(uint32_t, uint8_t);          \
    void name(void)                            \
    {                                          \
        c_##name(MemSeamC, (uint8_t)MemSeamA); \
    }                                          \
    void c_##name(uint32_t, uint8_t)

#define REGABI_BANK_WRITE16(name)               \
    void c_##name(uint32_t, uint16_t);          \
    void name(void)                             \
    {                                           \
        c_##name(MemSeamC, (uint16_t)MemSeamA); \
    }                                           \
    void c_##name(uint32_t, uint16_t)

#define REGABI_REG_READ8(name)  \
    uint8_t c_##name(void);     \
    void name(void)             \
    {                           \
        mem_set_al(c_##name()); \
    }                           \
    uint8_t c_##name(void)

/* $4212's hblank flag compares DH against the cycles-per-hblank count. */
#define REGABI_REG_READ8_DX(name)       \
    uint8_t c_##name(uint32_t);         \
    void name(void)                     \
    {                                   \
        mem_set_al(c_##name(MemSeamD)); \
    }                                   \
    uint8_t c_##name(uint32_t)

#define REGABI_REG_WRITE8(name)      \
    void c_##name(uint8_t);          \
    void name(void)                  \
    {                                \
        c_##name((uint8_t)MemSeamA); \
    }                                \
    void c_##name(uint8_t)

/* The IRQ beam registers carry the running cycle count in DH, in and out. */
#define REGABI_REG_WRITE8_DX(name)                        \
    uint32_t c_##name(uint8_t, uint32_t);                 \
    void name(void)                                       \
    {                                                     \
        MemSeamD = c_##name((uint8_t)MemSeamA, MemSeamD); \
    }                                                     \
    uint32_t c_##name(uint8_t, uint32_t)

#endif
