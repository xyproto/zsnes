#ifndef MSU1REGS_H
#define MSU1REGS_H

#include "../types.h"

//File
extern u1 MSU_StatusRead;
extern u1 MSU_MusicVolume;
extern int MSU_Track_Position;
extern char MSU_BasePath[];
int readMSU();

//Registers
extern void initMSU1regsRead();
extern void initMSU1regsWrite();

//Read Registers
extern void msuid1();
extern void msuid2();
extern void msuid3();
extern void msuid4();
extern void msuid5();
extern void msuid6();
extern void msudataread();
extern void msustatusread();

//Write Registers
extern void msudataseek0();
extern void msudataseek1();
extern void msudataseek2();
extern void msudataseek3();
extern void msu1track0();
extern void msu1track1();
extern void msu1volume();
extern void msu1statecontrol();

//Misc. Functions
extern void mixMSU1Audio(int* start, int* end, int rate);
extern void MSU1GetStatusBitsSpecial();
extern void MSU1HandleTrackChange();
extern void MSU1HandleStatusBits();

#endif