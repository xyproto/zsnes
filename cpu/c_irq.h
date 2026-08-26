#ifndef C_IRQ_H
#define C_IRQ_H

/* pesi is the caller's esi slot in its register block, not a u1** - punning a
   zreg slot through an incompatible pointer type lets -O3 assume the two do
   not alias, and the reload after the call then reads a stale program
   counter. */
void IRQemulmode(zreg* pedx, zreg* pesi);
void switchtovirq(zreg* pedx, zreg* pesi);
void NMIemulmode(zreg* pedx, zreg* pesi);
void switchtonmi(zreg* pedx, zreg* pesi);

#endif
