#include <string.h>

#include "../link.h"
#include "c_sw_draw.h"


u1* SurfBufD;


void ClearWin16(void)
{
	u1* edi = SurfBufD;
	s4  ebx = 0;
	do memset(edi, 0, SurfaceX * 2); while (edi += pitch, ++ebx != SurfaceY);
}
