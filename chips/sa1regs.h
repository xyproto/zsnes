#ifndef SA1REGS_H
#define SA1REGS_H

#include "../types.h"

extern void sa12300r(void);
extern void sa12301r(void);
extern void sa12302r(void);
extern void sa12303r(void);
extern void sa12304r(void);
extern void sa12305r(void);
extern void sa12306r(void);
extern void sa12307r(void);
extern void sa12308r(void);
extern void sa12309r(void);
extern void sa1230Ar(void);
extern void sa1230Br(void);
extern void sa1230Cr(void);
extern void sa1230Dr(void);
extern void sa1230Er(void);

extern void sa12200w(void);
extern void sa12201w(void);
extern void sa12202w(void);
extern void sa12203w(void);
extern void sa12204w(void);
extern void sa12205w(void);
extern void sa12206w(void);
extern void sa12207w(void);
extern void sa12208w(void);
extern void sa12209w(void);
extern void sa1220Aw(void);
extern void sa1220Bw(void);
extern void sa1220Cw(void);
extern void sa1220Dw(void);
extern void sa1220Ew(void);
extern void sa1220Fw(void);
extern void sa12210w(void);
extern void sa12211w(void);
extern void sa12212w(void);
extern void sa12213w(void);
extern void sa12214w(void);
extern void sa12215w(void);
extern void sa12220w(void);
extern void sa12221w(void);
extern void sa12222w(void);
extern void sa12223w(void);
extern void sa12224w(void);
extern void sa12225w(void);
extern void sa12230w(void);
extern void sa12231w(void);
extern void sa12232w(void);
extern void sa12233w(void);
extern void sa12234w(void);
extern void sa12235w(void);
extern void sa12236w(void);
extern void sa12237w(void);
extern void sa12238w(void);
extern void sa12239w(void);
extern void sa1223Fw(void);
extern void sa12240w(void);
extern void sa12241w(void);
extern void sa12242w(void);
extern void sa12243w(void);
extern void sa12244w(void);
extern void sa12245w(void);
extern void sa12246w(void);
extern void sa12247w(void);
extern void sa12248w(void);
extern void sa12249w(void);
extern void sa1224Aw(void);
extern void sa1224Bw(void);
extern void sa1224Cw(void);
extern void sa1224Dw(void);
extern void sa1224Ew(void);
extern void sa1224Fw(void);
extern void sa12250w(void);
extern void sa12251w(void);
extern void sa12252w(void);
extern void sa12253w(void);
extern void sa12254w(void);
extern void sa12259w(void);
extern void sa1225Aw(void);
extern void sa1225Bw(void);

extern void sdd14804(void);
extern void sdd14805(void);
extern void sdd14806(void);
extern void sdd14807(void);

extern void sdd14801w(void);
extern void sdd14804w(void);
extern void sdd14805w(void);
extern void sdd14806w(void);
extern void sdd14807w(void);

extern void IRamRead(void);
extern void IRamWrite(void);
extern void IRamWrite2(void);
extern void RTC2800(void);
extern void RTC2801w(void);
extern void dbstop(void);

extern u1 IRAM[2049]; // 2 kbytes of iram
extern u1 SA1ARC[4];
extern u1 SA1Control; // Don't execute if b5 or 6 are set
extern u1 SA1DMAInfo;
extern u1 SA1IRQData[4];
extern u1 SA1IRQEnable;
extern u1 SA1RegP;
extern u1 SA1_BRF[16];
extern u1* CurBWPtr;
extern u1* SA1BWPtr;
extern u1* SA1RAMArea;
extern u1* SA1RegPCS;
extern u1* SNSBWPtr;
extern u1* sa1dmaptr;
extern u1* sa1dmaptrs;
extern u2 SA1AR1;
extern u2 SA1AR2;
extern u2 SA1DMACount;
extern u2 SA1Overflow;
extern u4 RTCPtr;
extern u4 SA1ARR1;
extern u4 SA1ARR2;
extern u4 SA1DMADest;
extern u4 SA1DMASource;
extern u4 SA1DoIRQ;
extern u4 SA1IRQExec;
extern u4 SA1Message;
extern u4 SA1Mode; // 0 = SNES CPU, 1 = SA1 CPU
extern u4 SA1RegE;
extern u4 SA1_CC2_line;
extern u4 SA1_in_cc1_dma;
extern u4 SA1xa;
extern u4 SA1xd;
extern u4 SA1xdb;
extern u4 SA1xpb;
extern u4 SA1xs;
extern u4 SA1xx;
extern u4 SA1xy;

#endif
