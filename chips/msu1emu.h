#ifndef MSU1REGS_H
#define MSU1REGS_H

#include "../types.h"

// File
extern u1 MSU_StatusRead;
extern u1 MSU_MusicVolume;
extern int MSU_Track_Position;
extern char MSU_BasePath[];
int readMSU();

// Registers
extern void initMSU1regsRead();
extern void initMSU1regsWrite();

// Read Registers
extern mr8 msuid1;
extern mr8 msuid2;
extern mr8 msuid3;
extern mr8 msuid4;
extern mr8 msuid5;
extern mr8 msuid6;
extern mr8 msudataread;
extern mr8 msustatusread;

// Write Registers
extern mw8 msudataseek0;
extern mw8 msudataseek1;
extern mw8 msudataseek2;
extern mw8 msudataseek3;
extern mw8 msu1track0;
extern mw8 msu1track1;
extern mw8 msu1volume;
extern mw8 msu1statecontrol;

// Misc. Functions
extern void mixMSU1Audio(int* start, int* end, int rate);
extern void MSU1GetStatusBitsSpecial();
extern void MSU1HandleTrackChange();
extern void MSU1HandleStatusBits();

#endif
