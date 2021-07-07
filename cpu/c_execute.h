#ifndef C_EXECUTE_H
#define C_EXECUTE_H

#include "../types.h"

// this wonderful mess starts up the CPU and initialized the emulation state
void start65816(void);

void continueprog(void);

void continueprognokeys(void);

void reexecuteb(void);

void endprog(void);

void interror(void);

// sets to either 60Hz or 50Hz depending on PAL/NTSC
void init60hz(void);

void init18_2hz(void);

void Donextlinecache(void);

void execute(u4* pedx, u1** pebp, u1** pesi, eop*** pedi);

void StartSFXdebugb(void);

void UpdatePORSCMR(void);

void UpdateSCBRCOLR(void);

void UpdateCLSR(void);

void StartSFX(void);

#endif
