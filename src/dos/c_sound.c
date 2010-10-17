#include <string.h>

#include "../asm.h"
#include "../c_init.h"
#include "../c_intrf.h"
#include "../cfg.h"
#include "../cpu/dspproc.h"
#include "c_sound.h"
#include "sound.h"


u1 SBInt = 5 + 8;


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


void SB_dsp_reset(void)
{
	u2 const dx = SBPort;
	outb(dx + 0x06, 0x01);
	inb( dx + 0x06);
	inb( dx + 0x06);
	inb( dx + 0x06);
	inb( dx + 0x06);
	outb(dx + 0x06, 0x00);

	u2 si = 200;
	do
	{
		// Wait until bit 7 of SBDSPRdStat is set.
		u2 cx = 20000;
		while (!(inb(dx + 0x0E) & 0x80))
		{
			if (--cx == 0) goto cardfailed;
		}
		if (inb(dx + 0x0A) == 0xAA) return;
	}
	while (--si != 0);

cardfailed:
	u4 eax;
	asm volatile("int 0x10" : "=a" (eax) : "a" (0x0003));
	// Use extended DOS API to print a string.
	asm volatile("int 0x21" : "=a" (eax) : "a" (0x0900), "d" ("Sound card failed to initialize!\r\n$"));
	DosExit();
}


// Write value into DSP port
void SB_dsp_write(u1 const al)
{
	u2 const dx = SBPort + 0x0C;
	while (inb(dx) & 0x80) {}
	outb(dx, al);
}


void SB_quality_limiter(void)
{
	if (StereoSound != 1) return;
	if (SBHDMA      != 0) return;

	/* ViBRA16X support by Peter Santing
	 * Before REALLY switching back to 8-bit sucky mono mode, check that we're
	 * dealing with a ViBRA16X Creative Labs Card. */
	if (vibracard == 1) return;

	if (SoundQuality <= 2) return;
	if (SoundQuality == 4) return;
	SoundQuality = 2;
}


static void printnum(u4 val)
{
	char  buf[10];
	char* b = buf;
	do *b++ = '0' + val % 10; while ((val /= 10) != 0);
	do PrintChar(*--b); while (b != buf);
}


// Locates SET BLASTER environment
void getblaster(void)
{
	char const* esi = getenv("BLASTER");
	if (!esi)
	{
		if (soundon != 0)
		{
			soundon = 0;
			PrintStr(
				"ERROR : SET BLASTER environment NOT found!\r\n"
				"Unable to enable sound.\r\n"
				"\r\n"
				"Press any key to continue."
			);
			WaitForKey();
		}
		return;
	}

	u1 cursetting = 0;
	for (;;)
	{
		char dl = *esi++;
		if ('a' <= dl && dl <= 'z') dl -= 'a' - 'A';

		switch (dl)
		{
			case ' ':  cursetting = 0;             continue;
			case 'A':  cursetting = 1; SBPort = 0; continue;
			case 'I':  cursetting = 2; SBIrq  = 0; continue;
			case 'D':  cursetting = 3; SBDMA  = 0; continue;
			case 'H':  cursetting = 4; SBHDMA = 0; continue;
			case '\0': goto end;

			deafult:
				switch (cursetting)
				{
					case 1: // Process A.
						SBPort = SBPort * 16 + (dl - '0');
						break;

					case 2: // Process I.
						if (SBIrq == 1) SBIrq = 10;
						SBIrq += dl - '0';
						break;

					case 3: // Process D.
						SBDMA = dl - '0';
						break;

					case 4: // Process H.
						SBHDMA = dl - '0';
						break;
				}
				break;
		}
	}
end:

	if (SBIrq == 2) SBIrq = 9;

	u1 al = SBIrq + 8;
	if (SBIrq >= 8)
	{ // High IRQ.
		al       += 0x60;
		PICMaskP += 0x80;
	}
	SBInt = al;

	switch (SBHDMA)
	{
		/* This piece of code is added by Peter Santing.  It will enable ZSNES to use
		 * the full STEREO capability of the ViBRA16X line of Creative instead of
		 * playing 8-bit MONOURAL sound. */
#if 0 // XXX was commented out
		case 0:
#endif
		case 1:
		case 2:
		case 3:
			PrintStr(
				"Creative ViBRA16X PnP card detected (support coded by Peter Santing)\r\n"
				"High-DMA is below dma #4\r\n"
				"\r\n"
				"you have now full 16-bit stereo sound with the surround option!\r\n"
			);
			vibracard = 1;
			SBDMA     = SBHDMA;
			SBHDMA    = 0;
			break;

		case 4: SBHDMAPage = 0x8F; break;
		case 5: SBHDMAPage = 0x8B; break;
		case 6: SBHDMAPage = 0x89; break;
		case 7: SBHDMAPage = 0x8A; break;
	}

	switch (SBDMA)
	{
		case 0: SBDMAPage = 0x87; break;
		case 1: SBDMAPage = 0x83; break;
		case 2: SBDMAPage = 0x81; break;
		case 3: SBDMAPage = 0x82; break;
	}

	if (DisplayS == 1)
	{
		PrintStr("Sound Blaster Detection Values : \r\n\r\nPORT  : ");
		printhex(SBPort);
		PrintStr("\r\nIRQ   : ");
		printnum(SBIrq);
		PrintStr("\r\nDMA   : ");
		printnum(SBDMA);
		PrintStr("\r\nHDMA  : ");
		printnum(SBHDMA);
		PrintStr("\r\n\r\nPress any key to continue.");
		WaitForKey();
	}
}
