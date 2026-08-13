/* Definitions every makev16t difftest needs because they all link the whole
 * seam list, but which no emulator object in that list provides.
 *
 * video/c_mv16t8to.c calls c_cachesingle4bng, and the offset walk rebuilds its
 * map pointer as vram + a 16-bit offset every column. Only difftest_t8to.c
 * actually drives them; for the rest they exist so the link succeeds.
 */
#include <stdint.h>

typedef uint8_t u1;
typedef uint32_t u4;

u1* vram;

/* calldl16t's register block. video/c_mv16tline.c owns these in the real
   build, but that file calls the seam thunks by name and mkoracle renames
   every one of them, so no difftest can link it - the globals come from here
   instead. */
u4 DLR[7];
void (*DLFN)(void);

/* calldl16t itself, which lives in video/makev16t.asm in the real build. A
   difftest that links a ported file calling it needs a working one, not a
   stub - this is the same shim written where C can see it. */
__asm__(".pushsection .text\n"
        ".globl calldl16t\n"
        "calldl16t:\n"
        "  pushl %ebx\n  pushl %esi\n  pushl %edi\n  pushl %ebp\n"
        "  movl DLR+4, %ebx\n"
        "  movl DLR+8, %ecx\n"
        "  movl DLR+12, %edx\n"
        "  movl DLR+16, %esi\n"
        "  movl DLR+20, %edi\n"
        "  movl DLR+24, %ebp\n"
        "  movl DLR, %eax\n"
        "  call *DLFN\n"
        "  movl %eax, DLR\n"
        "  movl %ebx, DLR+4\n"
        "  movl %ecx, DLR+8\n"
        "  movl %edx, DLR+12\n"
        "  movl %esi, DLR+16\n"
        "  movl %edi, DLR+20\n"
        "  movl %ebp, DLR+24\n"
        "  popl %ebp\n  popl %edi\n  popl %esi\n  popl %ebx\n"
        "  ret\n"
        ".popsection\n");
u4 cs4_hits, cs4_last;

void c_cachesingle4bng(u4 ecx);
void c_cachesingle4bng(u4 ecx)
{
    cs4_last = ecx;
    cs4_hits++;
}

/* video/c_ngprocbg.c calls these by name. mkoracle renames every symbol the
   oracle defines, so the bare names do not exist in a difftest link - and
   unlike c_mv16tline.c this file has to be in the seam list, because the cur_
   object needs its entry points. No difftest reaches them, so a no-op is
   enough; a test that did would define its own recorder instead. */
void drawsprng16b(void);
void drawsprng16bhr(void);
void drawsprng16b(void) { }
void drawsprng16bhr(void) { }
