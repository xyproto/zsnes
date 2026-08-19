#ifndef TYPES_H
#define TYPES_H

typedef signed char s1;
typedef signed short s2;
typedef signed int s4;
typedef signed long long s8;

typedef unsigned char u1;
typedef unsigned short u2;
typedef unsigned int u4;
typedef unsigned long long u8;

typedef void eop();

/* A ported 65816 opcode body. It reads and writes the caller's register block,
   which the dispatch loop keeps in pushad order (see cpu/c_dispatch.h). */
typedef void opfn(u4* r);

/* An SPC700 opcode handler: takes the program counter just past the opcode
   byte and returns the updated one. */
typedef u1* spcop(u1* pc);

#endif
