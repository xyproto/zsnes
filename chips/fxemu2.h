#ifndef FXEMU2_H
#define FXEMU2_H

#include "../types.h"

extern void MainLoop();

extern u1* SCBRrel;
extern u4 NumberOfOpcodes; // Number of opcodes to execute
extern u4 SfxCLSR; // clock speed register (8bit)
extern u4 SfxCOLR; // Internal color register
extern u4 SfxPBR; // program bank register (8bit)
extern u4 SfxPOR; // Plot option register
extern u4 SfxSCBR; // screen bank register (8bit)
extern u4 SfxSCMR; // screen mode register (8bit)
extern u4 fxbit01pcal;
extern u4 fxbit23pcal;
extern u4 fxbit45pcal;
extern u4 fxbit67pcal;
extern u4 sfxclineloc;

#endif
