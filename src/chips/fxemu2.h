#ifndef FXEMU2_H
#define FXEMU2_H

#include "../types.h"

extern void MainLoop();

extern u4 NumberOfOpcodes; // Number of opcodes to execute
extern u4 SfxPBR;          // program bank register (8bit)
extern u4 SfxSCMR;         // screen mode register (8bit)

#endif
