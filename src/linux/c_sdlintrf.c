#include <stdio.h>
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


void StartUp(void) {}


void SystemInit(void)
{
	// Be sure to set SBHDMA to a value other than 0 if 16bit sound exists
	SBHDMA = 1;
}


char WaitForKey(void)
{
	return getchar();
}


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
		asm_call(InitializeGfxStuff);
	firstvideo = 0;

	asm_call(InitializeGfxStuff);
}


void UpdateDevices(void) { /* Stub please fix */ }
