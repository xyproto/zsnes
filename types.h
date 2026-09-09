#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

/* Deliberately ignore a result the compiler wants checked: the call happens,
   nothing acts on a short read or failed write. Mostly loaders that validate
   afterwards, or best-effort writes to a pipe. */
#define IGNORE_RESULT(call) \
    do {                    \
        if (call) { }       \
    } while (0)

/* Named for their width in bytes, and the whole tree means that literally: the
   asm-derived layouts in endmem.c and the *data.c files, the save-state format,
   and the difftests that compare against the original assembly all take u4 to
   be exactly four bytes. The fixed-width types say so, where `unsigned int`
   only promises at least sixteen bits and happens to be four bytes here. */
typedef int8_t s1;
typedef int16_t s2;
typedef int32_t s4;
typedef int64_t s8;

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;

/* The names are a promise; hold the compiler to it rather than finding out on
   a machine where it does not hold. */
_Static_assert(sizeof(s1) == 1 && sizeof(u1) == 1, "s1/u1 are one byte");
_Static_assert(sizeof(s2) == 2 && sizeof(u2) == 2, "s2/u2 are two bytes");
_Static_assert(sizeof(s4) == 4 && sizeof(u4) == 4, "s4/u4 are four bytes");
_Static_assert(sizeof(s8) == 8 && sizeof(u8) == 8, "s8/u8 are eight bytes");

/* u1 must stay a character type, not merely a one-byte one. The memory system
   reads and writes nearly everything through u1*, and only a character type
   may alias another object like that; the build does not pass
   -fno-strict-aliasing, so this is what keeps that legal. */
_Static_assert(_Generic((u1)0, unsigned char: 1, default: 0),
    "u1 must be unsigned char, for the aliasing the memory system relies on");

typedef void eop(void);

/* One slot of the 65816 core's register block: the x86 register file the
   assembly kept, in pushad order. Four slots hold 32-bit registers whose upper
   bits matter; the rest hold host pointers - esi the program counter, ebp the
   SPC700's, edi the opcode table, eax a RAM base through the stack macros - so
   a slot is pointer-wide, not u4. */
typedef uintptr_t zreg;

_Static_assert(sizeof(zreg) == sizeof(void*), "a register slot holds a host pointer");

/* A ported 65816 opcode body. It reads and writes the caller's register block,
   which the dispatch loop keeps in pushad order (see cpu/c_dispatch.h). */
typedef void opfn(zreg* r);

/* An SPC700 opcode handler: takes the program counter just past the opcode
   byte and returns the updated one. */
typedef u1* spcop(u1* pc);

#endif
