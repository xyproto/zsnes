#ifndef C_SA1REGS_H
#define C_SA1REGS_H

void RTCReset(void);
void RTCReset2(void);
void RTCinit(void);
void SA1Reset(void);
void SDD1Reset(void);
void initSA1regs(void);
void initSA1regsw(void);
void initSDD1regs(void);
void sa1dmabwram(void);
void sa1dmairam(void);

#endif
