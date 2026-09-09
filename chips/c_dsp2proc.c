#include <string.h>

#include "c_dsp2proc.h"
#include "dsp2proc.h"

void InitDSP2(void)
{
    dsp2state = 0;
    // Seed enforcer slot 0: command 0x00 expected at address 0x8000
    memset(dsp2enforcerQueue, 0, 8);
    dsp2enforcerQueue[5] = 0x80;
    dsp2enforcerReaderCursor = 0;
    dsp2enforcerWriterCursor = 1;
}
