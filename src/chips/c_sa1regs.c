#include "../ui.h"
#include "c_sa1regs.h"
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
