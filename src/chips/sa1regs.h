#ifndef SA1REGS_H
#define SA1REGS_H

#include "../types.h"

extern void RTC2800();
extern void RTC2801w();
extern void SDD1Reset();
extern void initSA1regs();
extern void initSA1regsw();
extern void initSDD1regs();

extern u1  SA1ARC[4];
extern u1  SA1Control;     // Don't execute if b5 or 6 are set
extern u1  SA1IRQData[4];
extern u1  SA1IRQEnable;
extern u1  SA1RegP;
extern u1  SA1_BRF[16];
extern u1* CurBWPtr;
extern u1* SA1BWPtr;
extern u1* SA1RAMArea;
extern u1* SA1RegPCS;
extern u1* SNSBWPtr;
extern u2  SA1AR1;
extern u2  SA1AR2;
extern u2  SA1Overflow;
extern u4  RTCPtr;
extern u4  SA1ARR1;
extern u4  SA1DoIRQ;
extern u4  SA1IRQExec;
extern u4  SA1Message;
extern u4  SA1Mode;        // 0 = SNES CPU, 1 = SA1 CPU
extern u4  SA1RegE;
extern u4  SA1_CC2_line;
extern u4  SA1_in_cc1_dma;
extern u4  SA1xa;
extern u4  SA1xd;
extern u4  SA1xdb;
extern u4  SA1xpb;
extern u4  SA1xs;
extern u4  SA1xx;
extern u4  SA1xy;

#endif
