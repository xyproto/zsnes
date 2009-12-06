#include <conio.h>
#include <string.h>

#include "../asm_call.h"
#include "../c_intrf.h"
#include "../cfg.h"
#include "../cpu/dspproc.h"
#include "../cpu/execute.h"
#include "../intrf.h"
#include "../link.h"
#include "../ui.h"
#include "../vcache.h"
#include "../video/2xsaiw.h"
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


static void InitializeGfxStuff(void)
{
	static u1 const BitPosR  = 11;
	static u1 const BitPosG  =  5;
	static u1 const BitPosB  =  0;
	static u1 const BitSizeR =  5;
	static u1 const BitSizeG =  6;
	static u1 const BitSizeB =  5;

	{ // Process Red Stuff
		u1 al = BitPosR;
		u2 bx = 1U << al;
		if (BitSizeR == 6)
		{
			vesa2_usbit = bx;
			++al;
		}
		vesa2_clbit |= bx;
		vesa2_rpos   = al;
		--al;
		vesa2_rfull = al != 0xFF ? 0x1FU << al : 0x1FU >> 1;
		bx = 1U << (al + 5);
		vesa2_rtrcl  =  bx;
		vesa2_rtrcla = ~bx;
	}

	{ // Process Green Stuff
		u1 al = BitPosG;
		u2 bx = 1U << al;
		if (BitSizeG == 6)
		{
			vesa2_usbit = bx;
			++al;
		}
		vesa2_clbit |= bx;
		vesa2_gpos   = al;
		--al;
		vesa2_gfull = al != 0xFF ? 0x1FU << al : 0x1FU >> 1;
		bx = 1U << (al + 5);
		vesa2_gtrcl  =  bx;
		vesa2_gtrcla = ~bx;
	}

	{ // Process Blue Stuff
		u1 al = BitPosB;
		u2 bx = 1U << al;
		if (BitSizeB == 6)
		{
			vesa2_usbit = bx;
			++al;
		}
		vesa2_clbit |= bx;
		vesa2_bpos   = al;
		--al;
		vesa2_bfull = al != 0xFF ? 0x1FU << al : 0x1FU >> 1;
		bx = 1U << (al + 5);
		vesa2_btrcl  =  bx;
		vesa2_btrcla = ~bx;
	}

	vesa2_clbit ^= 0xFFFF;
	asm_call(genfulladdtab);
	Init_2xSaIMMX(converta != 1 ? 565 : 555);
}


void initvideo(void)
{
	static u4 firstvideo = 1;

	res640            =   1;
	res480            =   1;
	cbitmode          =   1;
	vesa2_x           = 512;
	vesa2_y           = 480;
	vesa2_bits        =  16;
	vesa2_rpos        =  11;
	vesa2_gpos        =   5;
	vesa2_bpos        =   0;
	vesa2red10        =   0;
	vesa2_rposng      =  11;
	vesa2_gposng      =   5;
	vesa2_bposng      =   0;
	vesa2_clbitng     = 0x0000F7DE;
	vesa2_clbitng2[0] = 0xF7DEF7DE;
	vesa2_clbitng2[1] = 0xF7DEF7DE;
	vesa2_clbitng3    = 0x00007BEF;

	initwinvideo();

	if (GUIWFVID[cvidmode] != 0)
		PrevFSMode = cvidmode;
	else
		PrevWinMode = cvidmode;

	if (firstvideo != 1)
		InitializeGfxStuff();
	firstvideo = 0;

	InitializeGfxStuff();
}


void UpdateDevices(void)
{
	WinUpdateDevices();
}
