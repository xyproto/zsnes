#include <string.h>

#include "../asm_call.h"
#include "../c_intrf.h"
#include "../cfg.h"
#include "../cpu/dspproc.h"
#include "../cpu/execute.h"
#include "../link.h"
#include "../ui.h"


void InitPreGame(void)
{
	pressed[1] = 2;
	Start60HZ();
	initwinvideo();

	if (V8Mode != GrayscaleMode) V8Mode ^= 1;

	asm_call(AdjustFrequency);

	memset(vidbufferofsb, 0, 288 * 128 * 4);

	clearwin();
}
