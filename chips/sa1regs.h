#ifndef SA1REGS_H
#define SA1REGS_H

#include "../types.h"

extern void sa12300r();
extern void sa12301r();
extern void sa12302r();
extern void sa12303r();
extern void sa12304r();
extern void sa12305r();
extern void sa12306r();
extern void sa12307r();
extern void sa12308r();
extern void sa12309r();
extern void sa1230Ar();
extern void sa1230Br();
extern void sa1230Cr();
extern void sa1230Dr();
extern void sa1230Er();

extern void sa12200w();
extern void sa12201w();
extern void sa12202w();
extern void sa12203w();
extern void sa12204w();
extern void sa12205w();
extern void sa12206w();
extern void sa12207w();
extern void sa12208w();
extern void sa12209w();
extern void sa1220Aw();
extern void sa1220Bw();
extern void sa1220Cw();
extern void sa1220Dw();
extern void sa1220Ew();
extern void sa1220Fw();
extern void sa12210w();
extern void sa12211w();
extern void sa12212w();
extern void sa12213w();
extern void sa12214w();
extern void sa12215w();
extern void sa12220w();
extern void sa12221w();
extern void sa12222w();
extern void sa12223w();
extern void sa12224w();
extern void sa12225w();
extern void sa12230w();
extern void sa12231w();
extern void sa12232w();
extern void sa12233w();
extern void sa12234w();
extern void sa12235w();
extern void sa12236w();
extern void sa12237w();
extern void sa12238w();
extern void sa12239w();
extern void sa1223Fw();
extern void sa12240w();
extern void sa12241w();
extern void sa12242w();
extern void sa12243w();
extern void sa12244w();
extern void sa12245w();
extern void sa12246w();
extern void sa12247w();
extern void sa12248w();
extern void sa12249w();
extern void sa1224Aw();
extern void sa1224Bw();
extern void sa1224Cw();
extern void sa1224Dw();
extern void sa1224Ew();
extern void sa1224Fw();
extern void sa12250w();
extern void sa12251w();
extern void sa12252w();
extern void sa12253w();
extern void sa12254w();
extern void sa12259w();
extern void sa1225Aw();
extern void sa1225Bw();

extern void sdd14804();
extern void sdd14805();
extern void sdd14806();
extern void sdd14807();

extern void sdd14801w();
extern void sdd14804w();
extern void sdd14805w();
extern void sdd14806w();
extern void sdd14807w();

extern void IRamRead();
extern void IRamWrite();
extern void IRamWrite2();
extern void RTC2800();
extern void RTC2801w();
extern void dbstop();

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
