#include "c_dma.h"

#if !defined(__GNUC__) || !defined(__i386__)
#error "cpu/dma.c requires GCC-compatible inline assembly on i386"
#endif

#if defined(__APPLE__)
#define CSYM(x) "_" #x
#else
#define CSYM(x) #x
#endif

/*
 * Legacy register handlers are called with value in EAX and address in ECX.
 * Keep the original trampoline behavior: preserve EAX/ECX/EDX around the C call.
 */
__asm__(
    ".globl " CSYM(reg420Bw) "\n"
    CSYM(reg420Bw) ":\n"
    "pushl %eax\n"
    "pushl %ecx\n"
    "pushl %edx\n"
    "pushl %eax\n"
    "call " CSYM(c_reg420Bw) "\n"
    "addl $4, %esp\n"
    "popl %edx\n"
    "popl %ecx\n"
    "popl %eax\n"
    "ret\n");

__asm__(
    ".globl " CSYM(reg420Cw) "\n"
    CSYM(reg420Cw) ":\n"
    "pushl %eax\n"
    "pushl %ecx\n"
    "pushl %edx\n"
    "pushl %eax\n"
    "call " CSYM(c_reg420Cw) "\n"
    "addl $4, %esp\n"
    "popl %edx\n"
    "popl %ecx\n"
    "popl %eax\n"
    "ret\n");
