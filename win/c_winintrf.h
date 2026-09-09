#ifndef C_WININTRF_H
#define C_WININTRF_H

#include "../types.h"

// This function is called ~60 times/s at full speed
void SoundProcess(void);

extern u4 delayvalue;

#endif
