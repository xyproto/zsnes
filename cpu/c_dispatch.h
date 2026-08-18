#ifndef C_DISPATCH_H
#define C_DISPATCH_H

#include "../endmem.h"
#include "../types.h"

/*
 * The one x86 seam left in the 65816 path.
 *
 * Opcode bodies are C, but the tail every one of them ends with (the `endloop`
 * macro) still dispatches the next opcode by jumping rather than returning, so
 * a whole run of instructions executes with the core's state in real registers:
 * esi is the program counter, dl the flags, dh the scanline cycle counter, edi
 * the opcode table for the current flag state and ebp the SPC program counter.
 * run_chain hands that register file to such a run and takes it back when the
 * run returns, which it does once dh underflows.
 *
 * ebp travels in eax because GCC will not give up the frame pointer. The real
 * eax is dead across the call: the assembly loop reloaded it too.
 */
static inline void run_chain(u4* const r, u4 const op)
{
    u4 eax = r[2]; /* R_EBP */
    u4 ecx = r[6], edx = r[5], ebx = op, esi = r[1], edi = r[0];

    __asm__ volatile("push %%ebp\n\t"
                     "mov %0, %%ebp\n\t"
                     "call *(%5, %3, 4)\n\t"
                     "mov %%ebp, %0\n\t"
                     "pop %%ebp"
                     : "+a"(eax), "+c"(ecx), "+d"(edx), "+b"(ebx), "+S"(esi), "+D"(edi)
                     :
                     : "cc", "memory");

    r[2] = eax;
    r[6] = ecx;
    r[5] = edx;
    r[4] = ebx;
    r[1] = esi;
    r[0] = edi;
}


/* One SPC700 opcode. Its handlers keep the SPC program counter in ebp too. */
static inline void spc_step(u4* const r, u4 const op)
{
    u4 eax = r[2], ecx = (u4)opcjmptab[op], ebx = 0;
    u4 esi = r[1], edi = r[0];

    __asm__ volatile("push %%ebp\n\t"
                     "mov %0, %%ebp\n\t"
                     "call *%1\n\t"
                     "mov %%ebp, %0\n\t"
                     "pop %%ebp"
                     : "+a"(eax), "+c"(ecx), "+b"(ebx), "+S"(esi), "+D"(edi)
                     :
                     : "cc", "memory");

    r[2] = eax;
    r[1] = esi;
    r[0] = edi;
}

#endif
