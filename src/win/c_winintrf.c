#include <conio.h>
#include <string.h>

#include "../c_intrf.h"
#include "../cfg.h"
#include "../cpu/dspproc.h"
#include "../cpu/execute.h"
#include "../link.h"
#include "../ui.h"
#include "winintrf.h"

#ifndef __RELEASE__
#	include "winlink.h"
#endif


void StartUp(void) {}


void SystemInit(void)
{
#ifndef __RELEASE__
	DisplayWIPDisclaimer();
#endif
	// Be sure to set SBHDMA to a value other than 0 if 16bit sound exists
	SBHDMA = 1;
}


char WaitForKey(void)
{
	return wfkey = getch();
}


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
