#ifndef DSP2PROC_H
#define DSP2PROC_H

#include "../types.h"
#include <stdint.h>

extern u4 dsp2enforcerQueue[512][2];
extern u4 dsp2enforcerReaderCursor;
extern u4 dsp2enforcerWriterCursor;
extern u4 dsp2state;

uint8_t DSP2Read8b(uint32_t addr);
uint16_t DSP2Read16b(uint32_t addr);
void DSP2Write8b(uint32_t addr, uint8_t val);
void DSP2Write16b(uint32_t addr, uint16_t val);

#endif
