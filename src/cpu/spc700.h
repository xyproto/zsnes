#ifndef SPC700_H
#define SPC700_H

#include "../types.h"

extern u1  spcA;     // The A register (general purpose)
extern u1  spcNZ;    // The processor NZ flag (little speed up hack :) )
extern u1  spcP;     // The processor status byte (Removed for each flags), NZ are not always processed...
extern u1  spcX;     // The X register (general purpose)
extern u1  spcY;     // The Y register (general purpose)
extern u1* spcPCRam; // Program Counter (with SPCRAM added)
extern u1* spcRamDP; // The direct page pointer
extern u4  spcS;     // The stack pointer (always from 100 to 1FF) (added Ram)

#endif
