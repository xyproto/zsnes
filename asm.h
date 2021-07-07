#ifndef ASM_H
#define ASM_H

#include "types.h"

static inline void cli(void) { asm volatile("cli"); }
static inline void sti(void) { asm volatile("sti"); }

static inline u1 inb(u2 const port)
{
    u1 val;
    asm volatile("inb %1, %0"
                 : "=a"(val)
                 : "Nd"(port));
    return val;
}

static inline void outb(u2 const port, u1 const val)
{
    asm volatile("outb %0, %1" ::"a"(val), "dN"(port));
}

#endif
