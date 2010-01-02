#include "../macros.h"
#include "../types.h"
#include "../ui.h"
#include "c_mode716.h"


void Makemode7Table(void)
{
	for (u4 i = 0; i != lengthof(mode7tab); ++i)
	{
		mode7tab[i] = ((i & 0x07) << 4) + ((i >> 8 & 0x07) << 1) + 1;
	}
}
