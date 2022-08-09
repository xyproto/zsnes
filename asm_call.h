/*
 * This file provides the DJGPP/MinGW/MSVC/GCC on *nix compatible
 * "call asm function safely from C/C++" macro function.
 *
 * Usage:
 * asm_call( func_name );
 *
 * NOTE: It might not work with GCC and the -MASM=intel flag.
 */

#ifndef ASM_CALL_H
#define ASM_CALL_H

#ifdef __GNUC__
#if defined __x86_64__
#define asm_call(func) asm volatile("push %%rbx; call %P0; pop %%rbx" ::"X"(func) \
                                    : "cc", "memory", "rax", "rcx", "rdx", "rbp", "rsi", "rdi")
#elif defined __i386__
#define asm_call(func) asm volatile("push %%ebp; call %P0; pop %%ebp" ::"X"(func) \
                                    : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi")
#else
#error unknown architecture
#endif
#elif defined _MSC_VER
#define asm_call(func) __asm { __asm pushad  __asm call func  __asm popad }
#else
#error unknown compiler
#endif

#endif
