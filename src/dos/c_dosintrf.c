#include "../asm.h"
#include "../asm_call.h"
#include "../c_intrf.h"
#include "../cfg.h"
#include "../cpu/execute.h"
#include "../ui.h"
#include "sound.h"


static void get_handler(u1 const irq, u2* const segment, IRQHandler** const handler)
{
	u4 failed;
	asm("int $0x31;  sbb %0, %0" : "=a" (failed), "=c" (*segment), "=d" (*handler) : "a" (0x204), "b" (irq) : "cc");
	if (failed) asm_call(interror);
}


void InitPreGame(void)
{
	// set up interrupt handler
	// get old handler pmode mode address
	// Process stuff such as sound init, interrupt initialization
	cli();
	get_handler(0x09, &oldhand9s, &oldhand9o);
	get_handler(0x08, &oldhand8s, &oldhand8o);

	if (V8Mode != GrayscaleMode) V8Mode ^= 1;

	if (NoSoundReinit != 1 && soundon != 0 && DSPDisable != 1)
	{
		get_handler(SBInt, &oldhandSBs, &oldhandSBo);
	}
	sti();
}
