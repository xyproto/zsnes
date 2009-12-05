#include "../asm.h"
#include "../asm_call.h"
#include "../c_intrf.h"
#include "../cfg.h"
#include "../cpu/dspproc.h"
#include "../cpu/execute.h"
#include "../ui.h"
#include "dosintrf.h"
#include "sound.h"

extern unsigned short _djstat_flags;

u4 ZSNESBase;


// Find Selector - DOS only
static u2 findselec(u2 const segment)
{
	u2 selector;
	u4 failed;
	asm("int $0x31;  sbb %0, %0" : "=r" (failed), "=a" (selector) : "a" (2), "b" (segment) : "cc");
	if (failed)
	{
		char const* msg = "Cannot find selector!\n\r";
		asm volatile("call PrintStr" : "+d" (msg) :: "cc", "eax"); // XXX asm_call
		DosExit();
	}
	return selector;
}


void StartUp(void)
{
	_djstat_flags = 0xFFFF; // Optimize stat() calls by not calculating data useless for ZSNES

	// enable interrupts
	u1 res;
	asm volatile("int $0x31" : "=a" (res) : "a" (0x0901) : "cc");
	(void)res;

	asm("movw %ds, %0" : "=mr" (dssel));

	selcA000 = findselec(0xA000);
	selcB800 = findselec(0xB800);
	selc0040 = findselec(0x0040);

	// get previous video mode
	asm("pushl %%es;  movw %1, %%es;  movb %%es:0x49, %0;  popl %%es" : "=r" (previdmode) : "mr" (selc0040));

	// Get base address
	/* These variables are used for memory allocation so they can be ignored for
	 * non-DOS ports */
	u2 base_lo;
	u2 base_hi;
	u4 failed;
	asm("movw %%ds, %%bx;  int $0x31;  sbb %0, %0" : "=r" (failed), "=c" (base_hi), "=d" (base_lo) : "a" (0x0006) : "cc", "ebx");
	if (!failed) ZSNESBase = base_hi << 16 | base_lo;
}


void SystemInit(void)
{
	u2 es;
	asm volatile("movw %%es, %0" : "=mr" (es)); // XXX necessary?

	// Be sure to set SBHDMA to a value other than 0 if 16bit sound exists
	asm_call(getblaster); // get set blaster environment
	if (Force8b == 1) SBHDMA = 0;

	asm volatile("movw %0, %%es" : "mr" (es)); // XXX necessary?
}


char WaitForKey(void)
{
	char key;
	asm volatile("int $0x21" : "=a" (key) : "a" (0x0700) : "cc");
	return key;
}


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
