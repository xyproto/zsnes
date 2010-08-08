#include <string.h>

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
