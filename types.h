#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

/* Deliberately ignoring a result the compiler wants checked.
 *
 * The call still happens; this only records that nothing here acts on a short
 * read or a failed write. Most of these are loaders that validate the data
 * afterwards, or best-effort writes to a pipe. Grep for IGNORE_RESULT to find
 * the places that should grow real error handling. */
#define IGNORE_RESULT(call) \
    do {                    \
        if (call) { }       \
    } while (0)

typedef signed char s1;
typedef signed short s2;
typedef signed int s4;
typedef signed long long s8;

typedef unsigned char u1;
typedef unsigned short u2;
typedef unsigned int u4;
typedef unsigned long long u8;

typedef void eop(void);

/* One slot of the 65816 core's register block.
 *
 * The block is the x86 register file the assembly kept, in pushad order. Four
 * slots hold 32-bit registers whose upper bits are part of the behaviour, and
 * the rest hold host pointers: esi is the program counter, ebp the SPC700's,
 * edi the current opcode table, and eax carries a RAM base through the stack
 * macros. So a slot has to be pointer-wide, not u4 - on i386, where all of this
 * was written, the two are the same type and nothing changes. */
typedef uintptr_t zreg;

/* A ported 65816 opcode body. It reads and writes the caller's register block,
   which the dispatch loop keeps in pushad order (see cpu/c_dispatch.h). */
typedef void opfn(zreg* r);

/* An SPC700 opcode handler: takes the program counter just past the opcode
   byte and returns the updated one. */
typedef u1* spcop(u1* pc);

#endif
