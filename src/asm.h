#ifndef ASM_H
#define ASM_H

static inline void cli(void) { asm volatile("cli"); }
static inline void sti(void) { asm volatile("sti"); }

#endif
