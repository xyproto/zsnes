/*
 * test/ng2_harness.h - driving video/newg162.asm's tile and line drawers.
 *
 * Those twenty routines are what is left of the new-graphics engine, and no
 * commercial ROM here reaches nineteen of them (perf says so), so a whole
 * emulator A/B cannot verify them. This is the state and the calling sequence
 * needed to drive one directly.
 *
 * Three things are not obvious:
 *
 *  - They are reached by `jmp`, not `call`, with one word already pushed. Each
 *    ends `pop ebx / ret`, so a plain call has its return address eaten and
 *    control lands on the stack - which looks like a wild jump, not like
 *    missing state. NG2_CALL lays the stack out the way the dispatch in
 *    video/newgfx16.mac does.
 *  - vcache{2,4,8}bs hold the *base pointer* of the decoded-tile cache, so the
 *    generated stubs' zeros make a null pointer inside preparet2batile.
 *  - cachesingle{2,4,8}bng are already C (video/tilecache.c), so link the real
 *    thing rather than standing it in.
 */
#ifndef NG2_HARNESS_H
#define NG2_HARNESS_H

#include <string.h>

/* Entry state, from the dispatch in video/newgfx16.mac:
     eax = offset into vrama of the tile area   ebx = background index
     ecx = ebx + n*256, the window offset       dl  = 1 << n
     edi = output line pointer                  ebp = palette base
   ebp is easy to miss: the tile cache reads colours with [ebp+ebx*2], and
   newgfx16.mac loads it from cpalval[bg] just before the jmp. */
extern unsigned int NG2_EAX, NG2_EBX, NG2_ECX, NG2_EDX, NG2_EDI, NG2_ESI,
    NG2_EBP;

/* Two levels so a macro argument expands before stringification. */
#define NG2_CALL(sym) NG2_CALL_(sym)
#define NG2_CALL_(sym)                                                                                 \
    __asm__ volatile("pushl %%ebp\n\t"                                                                 \
                     "movl NG2_EDI, %%edi\n\t"                                                         \
                     "movl NG2_ESI, %%esi\n\t"                                                         \
                     "movl NG2_ECX, %%ecx\n\t"                                                         \
                     "movl NG2_EAX, %%eax\n\t"                                                         \
                     "movl NG2_EBX, %%ebx\n\t"                                                         \
                     "movl NG2_EDX, %%edx\n\t" /* Two exits with different stack contracts: a drawer   \
                                                  ends "pop ebx; ret", but a tail-jump to              \
                                                  domosaicng16b ends in a bare "ret". Pushing the same \
                                                  address twice satisfies both. */                     \
                     "pushl $1f\n\t"                                                                   \
                     "pushl $1f\n\t"                                                                   \
                     "jmp " #sym "\n\t"                                                                \
                     "1:\n\t"                                                                          \
                     "popl %%ebp\n\t" ::                                                               \
                         : "eax", "ebx", "ecx", "edx", "esi",                                          \
        "edi", "memory", "cc")

void ng2_init(void);

/* The routines mutate ngcwinptr/ngcwinmode/ofsmcptr2 and the tile counters, so
   two calls with "the same" inputs otherwise start from different states - the
   determinism check would be measuring the leftovers. Call before every run. */
void ng2_reset(void);

#endif /* NG2_HARNESS_H */
