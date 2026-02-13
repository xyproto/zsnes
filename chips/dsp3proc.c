#include <stdint.h>

#if !defined(__GNUC__) || !defined(__i386__)
#error "chips/dsp3proc.c requires GCC-compatible inline assembly on i386"
#endif

#if defined(__APPLE__)
#define CSYM(x) "_" #x
#else
#define CSYM(x) #x
#endif

extern uint8_t dsp3_byte;
extern uint16_t dsp3_address;
extern void DSP3GetByte(void);
extern void DSP3SetByte(void);
extern void regaccessbankr8(void);
extern void regaccessbankw8(void);
extern void regaccessbankr16(void);
extern void regaccessbankw16(void);

/*
 * Preserve register-based ABI used by eop handlers:
 * - address in ECX
 * - write value in AL (or AX for 16-bit)
 */
__asm__(
    ".globl " CSYM(DSP3Read8b) "\n" CSYM(DSP3Read8b) ":\n"
                                                     "testw $0x8000, %cx\n"
                                                     "jz " CSYM(regaccessbankr8) "\n"
                                                                                 "movw %cx, " CSYM(dsp3_address) "\n"
                                                                                                                 "pushl %eax\n"
                                                                                                                 "pushl %ecx\n"
                                                                                                                 "pushl %edx\n"
                                                                                                                 "call " CSYM(DSP3GetByte) "\n"
                                                                                                                                           "popl %edx\n"
                                                                                                                                           "popl %ecx\n"
                                                                                                                                           "popl %eax\n"
                                                                                                                                           "movb " CSYM(dsp3_byte) ", %al\n"
                                                                                                                                                                   "ret\n");

__asm__(
    ".globl " CSYM(DSP3Write8b) "\n" CSYM(DSP3Write8b) ":\n"
                                                       "testw $0x8000, %cx\n"
                                                       "jz " CSYM(regaccessbankw8) "\n"
                                                                                   "movw %cx, " CSYM(dsp3_address) "\n"
                                                                                                                   "movb %al, " CSYM(dsp3_byte) "\n"
                                                                                                                                                "pushl %eax\n"
                                                                                                                                                "pushl %ecx\n"
                                                                                                                                                "pushl %edx\n"
                                                                                                                                                "call " CSYM(DSP3SetByte) "\n"
                                                                                                                                                                          "popl %edx\n"
                                                                                                                                                                          "popl %ecx\n"
                                                                                                                                                                          "popl %eax\n"
                                                                                                                                                                          "ret\n");

__asm__(
    ".globl " CSYM(DSP3Read16b) "\n" CSYM(DSP3Read16b) ":\n"
                                                       "testw $0x8000, %cx\n"
                                                       "jz " CSYM(regaccessbankr16) "\n"
                                                                                    "movw %cx, " CSYM(dsp3_address) "\n"
                                                                                                                    "pushl %eax\n"
                                                                                                                    "pushl %ecx\n"
                                                                                                                    "pushl %edx\n"
                                                                                                                    "call " CSYM(DSP3GetByte) "\n"
                                                                                                                                              "popl %edx\n"
                                                                                                                                              "popl %ecx\n"
                                                                                                                                              "popl %eax\n"
                                                                                                                                              "movb " CSYM(dsp3_byte) ", %al\n"
                                                                                                                                                                      "incw " CSYM(dsp3_address) "\n"
                                                                                                                                                                                                 "pushl %eax\n"
                                                                                                                                                                                                 "pushl %ecx\n"
                                                                                                                                                                                                 "pushl %edx\n"
                                                                                                                                                                                                 "call " CSYM(DSP3GetByte) "\n"
                                                                                                                                                                                                                           "popl %edx\n"
                                                                                                                                                                                                                           "popl %ecx\n"
                                                                                                                                                                                                                           "popl %eax\n"
                                                                                                                                                                                                                           "movb " CSYM(dsp3_byte) ", %ah\n"
                                                                                                                                                                                                                                                   "ret\n");

__asm__(
    ".globl " CSYM(DSP3Write16b) "\n" CSYM(DSP3Write16b) ":\n"
                                                         "testw $0x8000, %cx\n"
                                                         "jz " CSYM(regaccessbankw16) "\n"
                                                                                      "movw %cx, " CSYM(dsp3_address) "\n"
                                                                                                                      "movb %al, " CSYM(dsp3_byte) "\n"
                                                                                                                                                   "pushl %eax\n"
                                                                                                                                                   "pushl %ecx\n"
                                                                                                                                                   "pushl %edx\n"
                                                                                                                                                   "call " CSYM(DSP3SetByte) "\n"
                                                                                                                                                                             "popl %edx\n"
                                                                                                                                                                             "popl %ecx\n"
                                                                                                                                                                             "popl %eax\n"
                                                                                                                                                                             "movb %ah, " CSYM(dsp3_byte) "\n"
                                                                                                                                                                                                          "incw " CSYM(dsp3_address) "\n"
                                                                                                                                                                                                                                     "pushl %eax\n"
                                                                                                                                                                                                                                     "pushl %ecx\n"
                                                                                                                                                                                                                                     "pushl %edx\n"
                                                                                                                                                                                                                                     "call " CSYM(DSP3SetByte) "\n"
                                                                                                                                                                                                                                                               "popl %edx\n"
                                                                                                                                                                                                                                                               "popl %ecx\n"
                                                                                                                                                                                                                                                               "popl %eax\n"
                                                                                                                                                                                                                                                               "ret\n");
