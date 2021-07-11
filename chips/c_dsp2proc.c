#include "c_dsp2proc.h"
#include "dsp2proc.h"

void InitDSP2(void)
{
    dsp2state = 0;
    dsp2enforcerQueue[0][0] = 0;
    dsp2enforcerQueue[0][1] = 0x8000;
    dsp2enforcerReaderCursor = 0;
    dsp2enforcerWriterCursor = 1;
}
