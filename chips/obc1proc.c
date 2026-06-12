/*
 * OBC1 coprocessor bank-access functions
 *
 * Ported from chips/obc1proc.asm.  All four functions share the same
 * three-way address routing:
 *
 *   addr bit 15 set  → delegate to memaccessbank (ROM/WRAM mirror)
 *   addr < 0x6000    → delegate to regaccessbank (PPU/CPU registers)
 *   [0x6000, 0x7FFF] → OBC1 local access via GetOBC1 / SetOBC1
 *
 * 16-bit operations issue two consecutive GetOBC1 / SetOBC1 calls,
 * incrementing obc1_address between them.  Low byte is transferred first.
 *
 * The c_* functions are the portable implementations.  The OBC1* entry
 * points the memory map dispatches to are i386 trampolines that keep the
 * legacy register ABI (bank in EBX, address in ECX, value in AL/AX): they
 * tail-jump to the asm mem/regaccessbank handlers for routed addresses,
 * so the delegate branches in the c_* functions are only reachable from
 * unit tests until the 65816 core itself is C.
 */

#include <stdint.h>

extern uint16_t obc1_address;
extern uint8_t obc1_byte;
extern void GetOBC1(void);
extern void SetOBC1(void);

extern uint8_t regaccessbankr8(uint32_t addr);
extern void regaccessbankw8(uint32_t addr, uint8_t val);
extern uint16_t regaccessbankr16(uint32_t addr);
extern void regaccessbankw16(uint32_t addr, uint16_t val);
extern uint8_t memaccessbankr8(uint32_t addr);
extern void memaccessbankw8(uint32_t addr, uint8_t val);
extern uint16_t memaccessbankr16(uint32_t addr);
extern void memaccessbankw16(uint32_t addr, uint16_t val);

uint8_t c_OBC1Read8b(uint32_t addr)
{
    if (addr & 0x8000)
        return memaccessbankr8(addr);
    if (addr < 0x6000)
        return regaccessbankr8(addr);
    obc1_address = (uint16_t)addr;
    GetOBC1();
    return obc1_byte;
}

void c_OBC1Write8b(uint32_t addr, uint8_t val)
{
    if (addr & 0x8000) {
        memaccessbankw8(addr, val);
        return;
    }
    if (addr < 0x6000) {
        regaccessbankw8(addr, val);
        return;
    }
    obc1_address = (uint16_t)addr;
    obc1_byte = val;
    SetOBC1();
}

uint16_t c_OBC1Read16b(uint32_t addr)
{
    if (addr & 0x8000)
        return memaccessbankr16(addr);
    if (addr < 0x6000)
        return regaccessbankr16(addr);
    obc1_address = (uint16_t)addr;
    GetOBC1();
    uint8_t lo = obc1_byte;
    obc1_address++;
    GetOBC1();
    uint8_t hi = obc1_byte;
    return (uint16_t)(lo | ((uint16_t)hi << 8));
}

void c_OBC1Write16b(uint32_t addr, uint16_t val)
{
    if (addr & 0x8000) {
        memaccessbankw16(addr, val);
        return;
    }
    if (addr < 0x6000) {
        regaccessbankw16(addr, val);
        return;
    }
    obc1_address = (uint16_t)addr;
    obc1_byte = (uint8_t)val;
    SetOBC1();
    obc1_byte = (uint8_t)(val >> 8);
    obc1_address++;
    SetOBC1();
}

#if defined(__GNUC__) && defined(__i386__)

#if defined(__APPLE__) || defined(__MINGW32__)
#define CSYM(x) "_" #x
#else
#define CSYM(x) #x
#endif

__asm__(
    ".globl " CSYM(OBC1Read8b) "\n" CSYM(OBC1Read8b) ":\n"
                                                     "testw $0x8000, %cx\n"
                                                     "jnz " CSYM(memaccessbankr8) "\n"
                                                                                  "cmpl $0x6000, %ecx\n"
                                                                                  "jb " CSYM(regaccessbankr8) "\n"
                                                                                                              "pushl %ecx\n"
                                                                                                              "pushl %edx\n"
                                                                                                              "pushl %eax\n"
                                                                                                              "pushl %ecx\n"
                                                                                                              "call " CSYM(c_OBC1Read8b) "\n"
                                                                                                                                         "addl $4, %esp\n"
                                                                                                                                         "movb %al, (%esp)\n"
                                                                                                                                         "popl %eax\n"
                                                                                                                                         "popl %edx\n"
                                                                                                                                         "popl %ecx\n"
                                                                                                                                         "ret\n");

__asm__(
    ".globl " CSYM(OBC1Write8b) "\n" CSYM(OBC1Write8b) ":\n"
                                                       "testw $0x8000, %cx\n"
                                                       "jnz " CSYM(memaccessbankw8) "\n"
                                                                                    "cmpl $0x6000, %ecx\n"
                                                                                    "jb " CSYM(regaccessbankw8) "\n"
                                                                                                                "pushl %eax\n"
                                                                                                                "pushl %ecx\n"
                                                                                                                "pushl %edx\n"
                                                                                                                "pushl %eax\n"
                                                                                                                "pushl %ecx\n"
                                                                                                                "call " CSYM(c_OBC1Write8b) "\n"
                                                                                                                                            "addl $8, %esp\n"
                                                                                                                                            "popl %edx\n"
                                                                                                                                            "popl %ecx\n"
                                                                                                                                            "popl %eax\n"
                                                                                                                                            "ret\n");

__asm__(
    ".globl " CSYM(OBC1Read16b) "\n" CSYM(OBC1Read16b) ":\n"
                                                       "testw $0x8000, %cx\n"
                                                       "jnz " CSYM(memaccessbankr16) "\n"
                                                                                     "cmpl $0x6000, %ecx\n"
                                                                                     "jb " CSYM(regaccessbankr16) "\n"
                                                                                                                  "pushl %ecx\n"
                                                                                                                  "pushl %edx\n"
                                                                                                                  "pushl %eax\n"
                                                                                                                  "pushl %ecx\n"
                                                                                                                  "call " CSYM(c_OBC1Read16b) "\n"
                                                                                                                                              "addl $4, %esp\n"
                                                                                                                                              "movw %ax, (%esp)\n"
                                                                                                                                              "popl %eax\n"
                                                                                                                                              "popl %edx\n"
                                                                                                                                              "popl %ecx\n"
                                                                                                                                              "ret\n");

__asm__(
    ".globl " CSYM(OBC1Write16b) "\n" CSYM(OBC1Write16b) ":\n"
                                                         "testw $0x8000, %cx\n"
                                                         "jnz " CSYM(memaccessbankw16) "\n"
                                                                                       "cmpl $0x6000, %ecx\n"
                                                                                       "jb " CSYM(regaccessbankw16) "\n"
                                                                                                                    "pushl %eax\n"
                                                                                                                    "pushl %ecx\n"
                                                                                                                    "pushl %edx\n"
                                                                                                                    "pushl %eax\n"
                                                                                                                    "pushl %ecx\n"
                                                                                                                    "call " CSYM(c_OBC1Write16b) "\n"
                                                                                                                                                 "addl $8, %esp\n"
                                                                                                                                                 "popl %edx\n"
                                                                                                                                                 "popl %ecx\n"
                                                                                                                                                 "popl %eax\n"
                                                                                                                                                 "ret\n");

#endif
