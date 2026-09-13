/* Sizes the SA-1 handlers and the memory accessors both need, kept apart from
   chips/sa1regs.h so cpu/mem_ops.h can take them without the register file. */
#ifndef SA1CONST_H
#define SA1CONST_H

enum { SA1_BWRAM_BYTES = 131072 }; /* the most an SA-1 cart carries */

#endif
