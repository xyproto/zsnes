#ifndef C_M716GATE_H
#define C_M716GATE_H

#include "../types.h"

/* The register set a mode 7 gate is entered with and hands to its renderer.
   See video/c_m716gate.c. */
typedef struct
{
    u4 ax, bx, cx, dx, si, di, bp;
} m7regs;

/* Each returns the tail id the caller has to dispatch on: 0 none, then
   drawmode716t, 716b, 716tb, 716extbg, 716textbg, 716extbg2, 716textbg2. */
u4 procmode716tsub(m7regs* r);
u4 procmode716tsubextbg(m7regs* r);
u4 procmode716tsubextbgb(m7regs* r);
u4 procmode716tsubextbg2(m7regs* r);
u4 procmode716tmain(m7regs* r);
u4 procmode716tmainextbg(m7regs* r);
u4 procmode716tmainextbgb(m7regs* r);
u4 procmode716tmainextbg2(m7regs* r);

/* 0 none, 1 drawsprites16t, anything else drawsprites16bt. */
u4 procspritesmain16t(m7regs* r);
u4 procspritessub16t(m7regs* r);
u4 procspritesmain16tfix(m7regs* r);
u4 procspritessub16tfix(m7regs* r);

/* The background gates come in two halves; the caller dispatches between them
   on the id, which is 1..6 for a renderer and 0 for "nothing more to do".
   drawbackgrnd_mark runs for any non-zero id, renderer or not. */
u4 drawbackgrndmain16t(m7regs* r);
u4 drawbackgrndsub16t(m7regs* r);
u4 drawbackgrndmain16tfix(m7regs* r);
u4 drawbackgrndsub16tfix(m7regs* r);
void drawbackgrnd_mark(m7regs* r);

/* The sprite renderers a gate's tail picks between. */
void drawsprites16t(m7regs* r);
void drawsprites16bt(m7regs* r);
void drawsprites16tprio(m7regs* r);

/* Tile renderers a background gate's id can select. Each returns non-zero if
   the mosaic tail is due, which the caller takes by running domosaic16b. */
u4 draw16x1616t(m7regs* r);
u4 draw16x1616bt(m7regs* r);

#endif
