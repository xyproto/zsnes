#ifndef C_EXECUTE_H
#define C_EXECUTE_H

// this wonderful mess starts up the CPU and initialized the emulation state
void start65816(void);

void continueprog(void);

void endprog(void);

void interror(void);

// sets to either 60Hz or 50Hz depending on PAL/NTSC
void init60hz(void);

void init18_2hz(void);

#endif
