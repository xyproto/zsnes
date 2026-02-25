#ifndef SA1REGS_H
#define SA1REGS_H

#include "../types.h"

extern mr8 sa12300r;
extern mr8 sa12301r;
extern mr8 sa12302r;
extern mr8 sa12303r;
extern mr8 sa12304r;
extern mr8 sa12305r;
extern mr8 sa12306r;
extern mr8 sa12307r;
extern mr8 sa12308r;
extern mr8 sa12309r;
extern mr8 sa1230Ar;
extern mr8 sa1230Br;
extern mr8 sa1230Cr;
extern mr8 sa1230Dr;
extern mr8 sa1230Er;

extern mw8 sa12200w;
extern mw8 sa12201w;
extern mw8 sa12202w;
extern mw8 sa12203w;
extern mw8 sa12204w;
extern mw8 sa12205w;
extern mw8 sa12206w;
extern mw8 sa12207w;
extern mw8 sa12208w;
extern mw8 sa12209w;
extern mw8 sa1220Aw;
extern mw8 sa1220Bw;
extern mw8 sa1220Cw;
extern mw8 sa1220Dw;
extern mw8 sa1220Ew;
extern mw8 sa1220Fw;
extern mw8 sa12210w;
extern mw8 sa12211w;
extern mw8 sa12212w;
extern mw8 sa12213w;
extern mw8 sa12214w;
extern mw8 sa12215w;
extern mw8 sa12220w;
extern mw8 sa12221w;
extern mw8 sa12222w;
extern mw8 sa12223w;
extern mw8 sa12224w;
extern mw8 sa12225w;
extern mw8 sa12230w;
extern mw8 sa12231w;
extern mw8 sa12232w;
extern mw8 sa12233w;
extern mw8 sa12234w;
extern mw8 sa12235w;
extern mw8 sa12236w;
extern mw8 sa12237w;
extern mw8 sa12238w;
extern mw8 sa12239w;
extern mw8 sa1223Fw;
extern mw8 sa12240w;
extern mw8 sa12241w;
extern mw8 sa12242w;
extern mw8 sa12243w;
extern mw8 sa12244w;
extern mw8 sa12245w;
extern mw8 sa12246w;
extern mw8 sa12247w;
extern mw8 sa12248w;
extern mw8 sa12249w;
extern mw8 sa1224Aw;
extern mw8 sa1224Bw;
extern mw8 sa1224Cw;
extern mw8 sa1224Dw;
extern mw8 sa1224Ew;
extern mw8 sa1224Fw;
extern mw8 sa12250w;
extern mw8 sa12251w;
extern mw8 sa12252w;
extern mw8 sa12253w;
extern mw8 sa12254w;
extern mw8 sa12259w;
extern mw8 sa1225Aw;
extern mw8 sa1225Bw;

extern mr8 sdd14804;
extern mr8 sdd14805;
extern mr8 sdd14806;
extern mr8 sdd14807;

extern mw8 sdd14801w;
extern mw8 sdd14804w;
extern mw8 sdd14805w;
extern mw8 sdd14806w;
extern mw8 sdd14807w;

extern mr8 IRamRead;
extern mw8 IRamWrite;
extern mw8 IRamWrite2;
extern mr8 RTC2800;
extern mw8 RTC2801w;
extern mw8 dbstop;

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
