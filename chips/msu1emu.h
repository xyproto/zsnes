#ifndef MSU1REGS_H
#define MSU1REGS_H

#include "../types.h"

// Status bits ($2000 read)
#define MSU_STATUS_REVISION 0x07 // bits 0-2
#define MSU_STATUS_ERROR 0x08
#define MSU_STATUS_PLAY 0x10
#define MSU_STATUS_LOOP 0x20
#define MSU_STATUS_AUDIO_BUSY 0x40
#define MSU_STATUS_DATA_BUSY 0x80

// Control bits ($2007 write)
#define MSU_CONTROL_PLAY 0x01
#define MSU_CONTROL_LOOP 0x02
#define MSU_CONTROL_RESUME 0x04

// MSU-1 Revision
#define MSU_REVISION 2

// File
extern u1 MSU_StatusRead;
extern u1 MSU_AudioVolume;
extern int MSU_Track_Position;
extern int MSU_Resume_Track_Position;
extern char MSU_BasePath[4096];
int readMSU(void);

// Registers
extern void initMSU1regsRead(void);
extern void initMSU1regsWrite(void);

// Read Registers
extern void msuid1(void);
extern void msuid2(void);
extern void msuid3(void);
extern void msuid4(void);
extern void msuid5(void);
extern void msuid6(void);
extern void msudataread(void);
extern void msustatusread(void);

// Write Registers
extern void msudataseek0(void);
extern void msudataseek1(void);
extern void msudataseek2(void);
extern void msudataseek3(void);
extern void msu1track0(void);
extern void msu1track1(void);
extern void msu1volume(void);
extern void msu1statecontrol(void);

// Misc. Functions
extern void mixMSU1Audio(int* start, int* end, int rate);
extern void MSU1HandleTrackChange(void);
extern void MSU1HandleControlBits(void);

#endif