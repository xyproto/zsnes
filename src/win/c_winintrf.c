#include <string.h>

#include "../c_intrf.h"
#include "../cfg.h"
#include "../cpu/execute.h"
#include "../link.h"
#include "../ui.h"


void InitPreGame(void)
{
	pressed[1] = 2;
	Start60HZ();

	initwinvideo();

	if (V8Mode != GrayscaleMode) V8Mode ^= 1;

	memset(vidbufferofsb, 0, 288 * 128 * 4);

	clearwin();

	// set up interrupt handler
	// get old handler pmode mode address
	// Process stuff such as sound init, interrupt initialization
}
