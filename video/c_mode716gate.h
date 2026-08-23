#ifndef C_MODE716GATE_H
#define C_MODE716GATE_H

#include "c_m716gate.h" /* m7regs */

/* The four thunks that were the last of video/mode716.asm. Each takes the
   caller's registers, runs the ported body on the seam block that body reads,
   and hands back what it left. See video/c_mode716gate.c. */
void drawmode7win16b(m7regs* r);
void drawmode7ngextbg16b(m7regs* r);
void drawmode7ngextbg216b(m7regs* r);
void processmode7hires16b(m7regs* r);

#endif
