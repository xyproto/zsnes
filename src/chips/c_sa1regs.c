#include <string.h>

#include "../endmem.h"
#include "../init.h"
#include "../ui.h"
#include "c_sa1regs.h"
#include "sa1proc.h"
#include "sa1regs.h"


void RTCinit(void)
{
	RTCPtr = 0;
}


void RTCReset(void)
{
	REGPTR(0x2800) = RTC2800;
}


void RTCReset2(void)
{
	REGPTW(0x2801) = RTC2801w;
}


void SA1Reset(void)
{
	memset(SA1_BRF, 0, sizeof(SA1_BRF));
	SA1_in_cc1_dma = 0;
	SA1_CC2_line   = 0;
	SA1IRQData[1]  = 0;
	SA1Mode        = SA1Mode & 0xFFFFFF00;
	SA1Status      = 0;
	SA1Control     = 0x20;
	SA1DoIRQ       = 0;
	irqv           = irqv2;
	nmiv           = nmiv2;
	SA1RegPCS      = romdata - 0x8000;
	SA1RAMArea     = romdata + 4096 * 1024;
	u1* const eax  = romdata + 4096 * 1024 - 0x6000;
	CurBWPtr       = eax;
	SA1BWPtr       = eax;
	SNSBWPtr       = eax;
	SA1xa          = 0;
	SA1xx          = 0;
	SA1xy          = 0;
	SA1xd          = 0;
	SA1xdb         = 0;
	SA1xpb         = 0;
	SA1xs          = 0x01FF;
	SA1RegP        = 0;
	SA1RegE        = 0;
	SA1IRQExec     = 0;
	SA1IRQEnable   = 0;
	SA1Message     = 0;
	SA1Overflow    = 0;
}


void UpdateArithStuff(void)
{
	if (SA1ARC[1] != 1) return;

	SA1ARC[1] = 0;
	if (SA1ARC[0] & 0x02)
	{ // Cumultative sum
		// XXX Handled by caller
	}
	else if (SA1ARC[0] & 0x01)
	{ // Divison
		s2 const dividend = SA1AR1;
		u2 const divisor  = SA1AR2;
		if (divisor != 0)
		{
			s2 const quotient = dividend / divisor;
			u2 const reminder = dividend % divisor;
			SA1ARR1 = (u4)reminder << 16 | (u2)quotient;
		}
		else
		{ // Invalid
			SA1ARR1 = 0;
		}
	}
	else
	{ // Multiplication
		SA1ARR1 = (s4)(s2)SA1AR1 * (s2)SA1AR2;
	}
}


static void executesa1dma(void)
{
	sa1dmaptrs =
		SA1DMAInfo & 0x01 ? &SA1RAMArea[SA1DMASource & 0x0003FFFF] : // BWRAM
		SA1DMAInfo & 0x02 ? &IRAM[      SA1DMASource & 0x000007FF] : // IRAM
		((u1* const*)snesmmap)[SA1DMASource >> 16 & 0xFF] + (SA1DMASource & 0x0000FFFF);
	memcpy(sa1dmaptr, sa1dmaptrs, SA1DMACount);
}


void sa1dmairam(void)
{
	sa1dmaptr = &IRAM[SA1DMADest & 0x000007FF];
	executesa1dma();
}


void sa1dmabwram(void)
{
	sa1dmaptr = &SA1RAMArea[SA1DMADest & 0x03FFFF];
	executesa1dma();
}


void initSA1regs(void)
{
	REGPTR(0x2300) = sa12300r;
	REGPTR(0x2301) = sa12301r;
	REGPTR(0x2302) = sa12302r;
	REGPTR(0x2303) = sa12303r;
	REGPTR(0x2304) = sa12304r;
	REGPTR(0x2305) = sa12305r;
	REGPTR(0x2306) = sa12306r;
	REGPTR(0x2307) = sa12307r;
	REGPTR(0x2308) = sa12308r;
	REGPTR(0x2309) = sa12309r;
	REGPTR(0x230A) = sa1230Ar;
	REGPTR(0x230B) = sa1230Br;
	REGPTR(0x230C) = sa1230Cr;
	REGPTR(0x230D) = sa1230Dr;
	REGPTR(0x230E) = sa1230Er;

	// Set IRam, memory address 3000-37FF
	eop** i = &REGPTR(0x3000);
	do *i = IRamRead; while (++i != &REGPTR(0x3800));
}
