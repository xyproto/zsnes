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

typedef void eop(); /* opcode/dispatch handler – register-based, no C args */

/* Memory bank handler types – fastcall: addr in ECX, val in DL/DX for writes */
#ifdef __GNUC__
typedef u1 __attribute__((fastcall)) mr8(u2 addr);
typedef void __attribute__((fastcall)) mw8(u2 addr, u1 val);
typedef u2 __attribute__((fastcall)) mr16(u2 addr);
typedef void __attribute__((fastcall)) mw16(u2 addr, u2 val);
#else
#error "memory handler ABI requires GCC __attribute__((fastcall)) on i386"
#endif

#endif
