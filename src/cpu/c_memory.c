#include "../endmem.h"
#include "../init.h"
#include "c_memory.h"


eop* DPageR16;
eop* DPageR8;
eop* DPageW16;
eop* DPageW8;


void UpdateDPage(void)
{
	u1 const d = xd >> 8;
	DPageR8  = Bank0datr8[d];
	DPageR16 = Bank0datr16[d];
	DPageW8  = Bank0datw8[d];
	DPageW16 = Bank0datw16[d];
}
