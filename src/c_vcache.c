#include "asm_call.h"
#include "c_vcache.h"
#include "macros.h"
#include "ui.h"
#include "vcache.h"

#ifdef __MSDOS__
#	include "cfg.h"
#endif


void genfulladdtab(void)
{
	// Write to buffer
#ifdef __MSDOS__
	if (newengen == 1 && vesa2red10 != 0)
	{
		asm_call(genfulladdtabred);
	}
	else
#endif
	{
		for (u4 i = 0; i != lengthof(fulladdtab); ++i)
		{
			u2 v = i;
			if (i & vesa2_rtrcl) v = v & vesa2_rtrcla | vesa2_rfull;
			if (i & vesa2_gtrcl) v = v & vesa2_gtrcla | vesa2_gfull;
			if (i & vesa2_btrcl) v = v & vesa2_btrcla | vesa2_bfull;
			fulladdtab[i] = v << 1;
		}
	}
}
