#ifndef DSP2PROC_H
#define DSP2PROC_H

#include "../types.h"

// 512 enforcer entries of 8 bytes: cmd, param, 0, 0, expected addr (le16), 0, 0
extern u1 dsp2enforcerQueue[512 * 8];
extern u4 dsp2enforcerReaderCursor;
extern u4 dsp2enforcerWriterCursor;
extern u4 dsp2state;

#endif
