#include "../asm.h"
#include "../asm_call.h"
#include "../c_intrf.h"
#include "../cfg.h"
#include "../cpu/dspproc.h"
#include "../cpu/execute.h"
#include "../gui/c_gui.h"
#include "../gui/gui.h"
#include "../ui.h"
#include "dosintrf.h"
#include "sound.h"

extern unsigned short _djstat_flags;

u4 ZSNESBase;

static u1 previdmode; // previous video mode


// Find Selector - DOS only
static u2 findselec(u2 const segment)
{
	u2 selector;
	u4 failed;
	asm("int $0x31;  sbb %0, %0" : "=r" (failed), "=a" (selector) : "a" (2), "b" (segment) : "cc");
	if (failed)
	{
		PrintStr("Cannot find selector!\n\r");
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


void PrintChar(char const c)
{
	u4 res;
	asm volatile("int $0x21" : "=a" (res) : "a" (0x0200), "d" (c) : "cc");
	(void)res;
}


void PrintStr(char const* s)
{
	for (;;)
	{
		char const c = *s++;
		if (c == '\0') break;
		PrintChar(c);
	}
}


char WaitForKey(void)
{
	char key;
	asm volatile("int $0x21" : "=a" (key) : "a" (0x0700) : "cc");
	return key;
}


u1 Check_Key(void)
{
	u1 res;
	asm("int $0x21" : "=a" (res) : "a" (0x0B00) : "cc");
	return res;
}


char Get_Key(void)
{
	char c;
	asm volatile("int $0x21" : "=a" (c) : "a" (0x0700) : "cc");
	return c;
}


void delay(u4 n)
{
	u1 prev = inb(0x61) & 0x10;
	do
	{
		u1 cur;
		do cur = inb(0x61) & 0x10; while (prev == cur); // XXX busy waiting
		prev = cur;
	}
	while (--n != 0);
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


static void set_handler(u1 const irq, u2 const segment, IRQHandler* const handler)
{
	u4 failed;
	asm volatile("int $0x31;  sbb %0, %0" : "=a" (failed) : "a" (0x205), "b" (irq), "c" (segment), "d" (handler) : "cc");
	if (failed) asm_call(interror);
}


static u2 get_cs(void)
{
	u2 cs;
	asm("movw %%cs, %0" : "=mr" (cs));
	return cs;
}


void SetupPreGame(void)
{
	u2 const cs = get_cs();

	// set new handler
	if (soundon != 0 && DSPDisable != 1)
	{
		cli();
		u2 const PIC_port = PICMaskP;
		u1 const irq_bit  = 1 << (SBIrq & 0x07);
		outb(PIC_port, inb(PIC_port) |  irq_bit); // Turn off IRQ through controller
		set_handler(SBInt, cs, SBHandler);
		outb(PIC_port, inb(PIC_port) & ~irq_bit); // Turn on IRQ through controller
		asm_call(InitSB);
		sti();
	}
	cli();
	set_handler(0x09, cs, handler9h);
	set_handler(0x08, cs, handler8h);
	asm_call(init60hz); // Set timer to 60/50Hz
	sti();
}


void DeInitPostGame(void)
{
	// de-init interrupt handler
	cli();
	set_handler(0x09, oldhand9s, oldhand9o);
	set_handler(0x08, oldhand8s, oldhand8o);
	asm_call(init18_2hz); // Set timer to 18.2Hz
	sti();

	// DeINITSPC
	if (soundon != 0 && DSPDisable != 1)
	{
		asm_call(DeInitSPC);
		set_handler(SBInt, oldhandSBs, oldhandSBo);
	}
}


void GUIInit(void)
{
	get_handler(0x09, &GUIoldhand9s, &GUIoldhand9o);
	get_handler(0x08, &GUIoldhand8s, &GUIoldhand8o);
	u2 const cs = get_cs();
	set_handler(0x09,  cs,            GUIhandler9h);
	set_handler(0x08,  cs,            GUIhandler8h);
	GUIinit36_4hz();
}


void GUIDeInit(void)
{
	set_handler(0x09, GUIoldhand9s, GUIoldhand9o);
	set_handler(0x08, GUIoldhand8s, GUIoldhand8o);
	GUIinit18_2hz();
}


void initvideo(void)
{
	asm_call(dosinitvideo);
}


void deinitvideo(void)
{
	u4 const eax = 0x00U << 8 | previdmode;
	asm volatile("int $0x10" :: "a" (eax));
}


void DrawScreen(void)
{
	asm_call(DosDrawScreen);
}


void vidpastecopyscr(void)
{
	if (GUI16VID[cvidmode] == 1)
	{
		u1* const buf = vidbuffer;
		u4        n   = 224 * 288 - 288;
		u4        i   = 224 * 288 -   1;
		do ((u2*)buf)[i] = GUICPC[buf[i]]; while (--i, --n != 0);
	}
	asm_call(DosDrawScreenB);
}


void UpdateDevices(void)
{
	asm_call(DosUpdateDevices);
}


void JoyRead(void)
{
	asm_call(DOSJoyRead);
}


/*****************************
 * Mouse Stuff
 *****************************/


u4 Init_Mouse(void)
{
	u2 success;
	asm volatile("int $0x33" : "=a" (success) : "a" (0) : "cc", "ebx");
	if (!success) return 0;
	asm volatile("int $0x33" :: "a" (0x07), "c" (0), "d" (255));
	asm volatile("int $0x33" :: "a" (0x08), "c" (0), "d" (223));
	asm volatile("int $0x33" :: "a" (0x0F), "c" (8), "d" (8));
	asm volatile("int $0x33" :: "a" (0x04), "c" (0), "d" (0));
	return 1;
}


u4 Get_MouseData(void)
{
	// bx: bit 0 = left button, bit 1 = right button
	// cx = Mouse X Position, dx = Mouse Y Position
	u2 buttons;
	u2 x;
	u2 y;
	asm volatile("int $0x33" : "=b" (buttons), "=c" (x), "=d" (y) : "a" (0x03) : "cc");
	return y << 24 | x << 16 | buttons;
}
