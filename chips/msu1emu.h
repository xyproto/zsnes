#ifndef MSU1REGS_H
#define MSU1REGS_H

#include "../types.h"

// Bitfield Constants
// 0x2000 (MSU_STATUS)
#define MSU_STATUS_REVISION0 0x01
#define MSU_STATUS_REVISION1 0x02
#define MSU_STATUS_REVISION2 0x04
#define MSU_STATUS_ERROR 0x08
#define MSU_STATUS_PLAY 0x10
#define MSU_STATUS_LOOP 0x20
#define MSU_STATUS_AUDIO_BUSY 0x40
#define MSU_STATUS_DATA_BUSY 0x80
// 0x2007 (MSU_CONTROL)
#define MSU_CONTROL_PLAY 0x01
#define MSU_CONTROL_LOOP 0x02
#define MSU_CONTROL_RESUME 0x04
// Chip Revision
#define MSU_REVISION 2

// File
extern u1 MSU_StatusRead;
extern u1 MSU_AudioVolume;
extern int MSU_Track_Position;
extern int MSU_Resume_Track_Position;
extern char MSU_BasePath[4096];
int readMSU();

// Registers
extern void initMSU1regsRead();
extern void initMSU1regsWrite();

// Read Registers
extern void msuid1();
extern void msuid2();
extern void msuid3();
extern void msuid4();
extern void msuid5();
extern void msuid6();
extern void msudataread();
extern void msustatusread();

// Write Registers
extern void msudataseek0();
extern void msudataseek1();
extern void msudataseek2();
extern void msudataseek3();
extern void msu1track0();
extern void msu1track1();
extern void msu1volume();
extern void msu1statecontrol();

// Misc. Functions
extern void mixMSU1Audio(int* start, int* end, int rate);
extern void MSU1GetStatusBitsSpecial();
extern void MSU1HandleTrackChange();
extern void MSU1HandleStatusBits();

#endif