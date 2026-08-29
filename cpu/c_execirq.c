/* ProcessIRQStuff from cpu/execute.asm: does this scanline raise a V/H IRQ?
   Four dispatch sites each end by jumping to their own `.virq`, so this returns
   the decision instead of acting on it - non-zero means take .virq. Two dead
   branches in the original are left as comments. */
#include <stdint.h>

#include "../types.h"

enum { R_EDI, R_ESI, R_EBP, R_ESP, R_EBX, R_EDX, R_ECX, R_EAX };

extern u1 INTEnab, intrset, doirqnext, irqon;
extern u2 VIRQLoc, resolutn, curypos;

int c_process_irq(zreg* const r)
{
    /* dl bit 2 is the I flag: interrupts already disabled. */
    int const idis = (r[R_EDX] & 0x04u) != 0;

    if (!idis && doirqnext == 1)
        return 1;

    if (INTEnab & 0x20u) {
        u2 ax = VIRQLoc;
        if (ax == resolutn)
            ax--;
        if (ax == 0xFFFFu)
            ax = 0;
        r[R_EAX] = (r[R_EAX] & 0xFFFF0000u) | ax;
        if (curypos != ax)
            return 0;
        /* the INTEnab bit 4 test here reaches the same place either way */
    } else {
        if (!(INTEnab & 0x10u))
            return 0;
        if (intrset > 2) {
            intrset--;
            if (intrset > 2)
                return 0;
        }
        if (intrset == 1) {
            if (INTEnab & 0x80u) {
                intrset = 8;
                return 0;
            }
        } else if (idis) {
            return 0;
        }
    }

    /* startirq */
    if (intrset == 1)
        intrset = 2;
    irqon = 0x80;
    doirqnext = 1;
    return !idis;
}
