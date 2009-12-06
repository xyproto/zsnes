#include "c_vcache.h"
#include "macros.h"
#include "ui.h"
#include "vcache.h"

#ifdef __MSDOS__
#	include "cfg.h"
#endif


#ifdef __MSDOS__
static void genfulladdtabred(void)
{
	// Write to buffer
	for (u4 i = 0; i != lengthof(fulladdtab); ++i)
	{
		u2 v = i;
		if (i & 0x4000) v = v & 0xBFFF | 0x3C00;
		if (i & 0x0200) v = v & 0xFDFF | 0x01E0;
		if (i & 0x0010) v = v & 0xFFEF | 0x000F;
		fulladdtab[i] = v << 1;
	}
}
#endif


void genfulladdtab(void)
{
	// Write to buffer
#ifdef __MSDOS__
	if (newengen == 1 && vesa2red10 != 0)
	{
		genfulladdtabred();
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
