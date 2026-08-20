#ifndef C_IRQ_H
#define C_IRQ_H

void IRQemulmode(zreg* pedx, u1** pesi);
void switchtovirq(zreg* pedx, u1** pesi);
void NMIemulmode(zreg* pedx, u1** pesi);
void switchtonmi(zreg* pedx, u1** pesi);

#endif
