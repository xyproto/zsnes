#include <stdint.h>

#if !defined(__GNUC__) || !defined(__i386__)
#error "chips/dsp4proc.c requires GCC-compatible inline assembly on i386"
#endif

#if defined(__APPLE__) || defined(__MINGW32__)
#define CSYM(x) "_" #x
#else
#define CSYM(x) #x
#endif

extern uint8_t dsp4_byte;
extern uint16_t dsp4_address;
extern void DSP4GetByte(void);
extern void DSP4SetByte(void);
extern void regaccessbankr8(void);
extern void regaccessbankw8(void);
extern void regaccessbankr16(void);
extern void regaccessbankw16(void);

/*
 * Preserve register-based ABI used by eop handlers:
 * - address in ECX
 * - write value in AL (or AX for 16-bit)
 * Route behavior matches legacy asm:
 * - 0x0000-0x7FFF: jump to regaccessbank*
 * - 0x8000-0xBFFF: DSP4 handling
 * - 0xC000-0xFFFF: immediate return
 */
__asm__(
    ".globl " CSYM(DSP4Read8b) "\n" CSYM(DSP4Read8b) ":\n"
                                                     "testw $0x8000, %cx\n"
                                                     "jz " CSYM(regaccessbankr8) "\n"
                                                                                 "testw $0x4000, %cx\n"
                                                                                 "jnz 1f\n"
                                                                                 "movw %cx, " CSYM(dsp4_address) "\n"
                                                                                                                 "pushl %eax\n"
                                                                                                                 "pushl %ecx\n"
                                                                                                                 "pushl %edx\n"
                                                                                                                 "call " CSYM(DSP4GetByte) "\n"
                                                                                                                                           "popl %edx\n"
                                                                                                                                           "popl %ecx\n"
                                                                                                                                           "popl %eax\n"
                                                                                                                                           "movb " CSYM(dsp4_byte) ", %al\n"
                                                                                                                                                                   "1:\n"
                                                                                                                                                                   "ret\n");

__asm__(
    ".globl " CSYM(DSP4Write8b) "\n" CSYM(DSP4Write8b) ":\n"
                                                       "testw $0x8000, %cx\n"
                                                       "jz " CSYM(regaccessbankw8) "\n"
                                                                                   "testw $0x4000, %cx\n"
                                                                                   "jnz 1f\n"
                                                                                   "movw %cx, " CSYM(dsp4_address) "\n"
                                                                                                                   "movb %al, " CSYM(dsp4_byte) "\n"
                                                                                                                                                "pushl %eax\n"
                                                                                                                                                "pushl %ecx\n"
                                                                                                                                                "pushl %edx\n"
                                                                                                                                                "call " CSYM(DSP4SetByte) "\n"
                                                                                                                                                                          "popl %edx\n"
                                                                                                                                                                          "popl %ecx\n"
                                                                                                                                                                          "popl %eax\n"
                                                                                                                                                                          "1:\n"
                                                                                                                                                                          "ret\n");

__asm__(
    ".globl " CSYM(DSP4Read16b) "\n" CSYM(DSP4Read16b) ":\n"
                                                       "testw $0x8000, %cx\n"
                                                       "jz " CSYM(regaccessbankr16) "\n"
                                                                                    "testw $0x4000, %cx\n"
                                                                                    "jnz 1f\n"
                                                                                    "movw %cx, " CSYM(dsp4_address) "\n"
                                                                                                                    "pushl %eax\n"
                                                                                                                    "pushl %ecx\n"
                                                                                                                    "pushl %edx\n"
                                                                                                                    "call " CSYM(DSP4GetByte) "\n"
                                                                                                                                              "popl %edx\n"
                                                                                                                                              "popl %ecx\n"
                                                                                                                                              "popl %eax\n"
                                                                                                                                              "movb " CSYM(dsp4_byte) ", %al\n"
                                                                                                                                                                      "incw " CSYM(dsp4_address) "\n"
                                                                                                                                                                                                 "pushl %eax\n"
                                                                                                                                                                                                 "pushl %ecx\n"
                                                                                                                                                                                                 "pushl %edx\n"
                                                                                                                                                                                                 "call " CSYM(DSP4GetByte) "\n"
                                                                                                                                                                                                                           "popl %edx\n"
                                                                                                                                                                                                                           "popl %ecx\n"
                                                                                                                                                                                                                           "popl %eax\n"
                                                                                                                                                                                                                           "movb " CSYM(dsp4_byte) ", %ah\n"
                                                                                                                                                                                                                                                   "1:\n"
                                                                                                                                                                                                                                                   "ret\n");

__asm__(
    ".globl " CSYM(DSP4Write16b) "\n" CSYM(DSP4Write16b) ":\n"
                                                         "testw $0x8000, %cx\n"
                                                         "jz " CSYM(regaccessbankw16) "\n"
                                                                                      "testw $0x4000, %cx\n"
                                                                                      "jnz 1f\n"
                                                                                      "movw %cx, " CSYM(dsp4_address) "\n"
                                                                                                                      "movb %al, " CSYM(dsp4_byte) "\n"
                                                                                                                                                   "pushl %eax\n"
                                                                                                                                                   "pushl %ecx\n"
                                                                                                                                                   "pushl %edx\n"
                                                                                                                                                   "call " CSYM(DSP4SetByte) "\n"
                                                                                                                                                                             "popl %edx\n"
                                                                                                                                                                             "popl %ecx\n"
                                                                                                                                                                             "popl %eax\n"
                                                                                                                                                                             "movb %ah, " CSYM(dsp4_byte) "\n"
                                                                                                                                                                                                          "incw " CSYM(dsp4_address) "\n"
                                                                                                                                                                                                                                     "pushl %eax\n"
                                                                                                                                                                                                                                     "pushl %ecx\n"
                                                                                                                                                                                                                                     "pushl %edx\n"
                                                                                                                                                                                                                                     "call " CSYM(DSP4SetByte) "\n"
                                                                                                                                                                                                                                                               "popl %edx\n"
                                                                                                                                                                                                                                                               "popl %ecx\n"
                                                                                                                                                                                                                                                               "popl %eax\n"
                                                                                                                                                                                                                                                               "1:\n"
                                                                                                                                                                                                                                                               "ret\n");
