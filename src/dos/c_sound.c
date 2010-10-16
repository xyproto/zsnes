#include <string.h>

#include "c_sound.h"
#include "sound.h"


void SB_alloc_dma(void)
{
	// Allocate 16384 bytes of DOS memory.
	// To delocate this, use ax=0101h, dx=selector of block/int 31h
	u2 ax;
	u2 dx;
	u4 failed;
	asm volatile("int 0x31;  sbb %2, %2" : "=a" (ax), "=d" (dx), "=b" (failed) : "a" (0x0100), "b" (16384 / 16) : "cc");
	if (failed)
	{
		// Use extended DOS API to print a string.
		u4 eax;
		asm volatile("int 0x21" : "=a" (eax) : "a" (0x0900), "d" ("Unable to allocate conventional memory!\r\n$"));
		DosExit();
	}

	// Check which 8192 byte boundary doesn't cross a page
	memoryloc = ax << 4;
	sbselec   = dx;
	sbpmofs   = 0;
	if (memoryloc >> 16 != (memoryloc + 8192) >> 16)
	{ // Ext area.
		sbpmofs    = 8192;
		memoryloc += 8192;
	}

	// Clear DOS memory.
	memset(memoryloc, 0, 8192);
}
