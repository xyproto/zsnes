#ifndef REGABI_H
#define REGABI_H

#include <stdint.h>

/*
 * Trampolines bridging the legacy register ABI of the 65816 asm core to
 * portable cdecl implementations named with a c_ prefix.
 *
 * Legacy ABI: address in ECX, value in AL (8-bit) or AX (16-bit); handlers
 * must preserve ECX, EDX and the unused upper bits of EAX.  BANK handlers
 * receive an address, REG handlers (I/O register table entries) do not.
 *
 * Each macro emits the trampoline under the public name plus a prototype
 * for the cdecl implementation.  On non-i386 builds only the prototype
 * remains.  Delete this header once the 65816 core itself is C.
 */

#if defined(__GNUC__) && defined(__i386__)

#if defined(__APPLE__) || defined(__MINGW32__)
#define REGABI_SYM(x) "_" #x
#else
#define REGABI_SYM(x) #x
#endif

#define REGABI_ENTRY(name) \
    ".globl " REGABI_SYM(name) "\n" REGABI_SYM(name) ":\n"

#define REGABI_BANK_READ8(name)                                                  \
    __asm__(REGABI_ENTRY(name) "pushl %ecx\n"                                    \
                               "pushl %edx\n"                                    \
                               "pushl %eax\n"                                    \
                               "pushl %ecx\n"                                    \
                               "call " REGABI_SYM(c_##name) "\n"                 \
                                                            "addl $4, %esp\n"    \
                                                            "movb %al, (%esp)\n" \
                                                            "popl %eax\n"        \
                                                            "popl %edx\n"        \
                                                            "popl %ecx\n"        \
                                                            "ret\n");            \
    uint8_t c_##name(uint32_t)

#define REGABI_BANK_READ16(name)                                                 \
    __asm__(REGABI_ENTRY(name) "pushl %ecx\n"                                    \
                               "pushl %edx\n"                                    \
                               "pushl %eax\n"                                    \
                               "pushl %ecx\n"                                    \
                               "call " REGABI_SYM(c_##name) "\n"                 \
                                                            "addl $4, %esp\n"    \
                                                            "movw %ax, (%esp)\n" \
                                                            "popl %eax\n"        \
                                                            "popl %edx\n"        \
                                                            "popl %ecx\n"        \
                                                            "ret\n");            \
    uint16_t c_##name(uint32_t)

#define REGABI_BANK_WRITE_BODY(name)                                          \
    __asm__(REGABI_ENTRY(name) "pushl %eax\n"                                 \
                               "pushl %ecx\n"                                 \
                               "pushl %edx\n"                                 \
                               "pushl %eax\n"                                 \
                               "pushl %ecx\n"                                 \
                               "call " REGABI_SYM(c_##name) "\n"              \
                                                            "addl $8, %esp\n" \
                                                            "popl %edx\n"     \
                                                            "popl %ecx\n"     \
                                                            "popl %eax\n"     \
                                                            "ret\n")

#define REGABI_BANK_WRITE8(name)  \
    REGABI_BANK_WRITE_BODY(name); \
    void c_##name(uint32_t, uint8_t)

#define REGABI_BANK_WRITE16(name) \
    REGABI_BANK_WRITE_BODY(name); \
    void c_##name(uint32_t, uint16_t)

#define REGABI_REG_READ8(name)                                                   \
    __asm__(REGABI_ENTRY(name) "pushl %ecx\n"                                    \
                               "pushl %edx\n"                                    \
                               "pushl %eax\n"                                    \
                               "call " REGABI_SYM(c_##name) "\n"                 \
                                                            "movb %al, (%esp)\n" \
                                                            "popl %eax\n"        \
                                                            "popl %edx\n"        \
                                                            "popl %ecx\n"        \
                                                            "ret\n");            \
    uint8_t c_##name(void)

#define REGABI_REG_WRITE8(name)                                               \
    __asm__(REGABI_ENTRY(name) "pushl %eax\n"                                 \
                               "pushl %ecx\n"                                 \
                               "pushl %edx\n"                                 \
                               "pushl %eax\n"                                 \
                               "call " REGABI_SYM(c_##name) "\n"              \
                                                            "addl $4, %esp\n" \
                                                            "popl %edx\n"     \
                                                            "popl %ecx\n"     \
                                                            "popl %eax\n"     \
                                                            "ret\n");         \
    void c_##name(uint8_t)

#else

#define REGABI_BANK_READ8(name) uint8_t c_##name(uint32_t)
#define REGABI_BANK_READ16(name) uint16_t c_##name(uint32_t)
#define REGABI_BANK_WRITE8(name) void c_##name(uint32_t, uint8_t)
#define REGABI_BANK_WRITE16(name) void c_##name(uint32_t, uint16_t)
#define REGABI_REG_READ8(name) uint8_t c_##name(void)
#define REGABI_REG_WRITE8(name) void c_##name(uint8_t)

#endif

#endif
