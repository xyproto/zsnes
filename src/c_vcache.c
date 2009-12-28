#include "c_init.h"
#include "c_vcache.h"
#include "cfg.h"
#include "cpu/regs.h"
#include "gui/gui.h"
#include "macros.h"
#include "ui.h"
#include "vcache.h"


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


void ConvertToAFormat(void)
{
	if (GUIOn != 1 && newengen != 0) return;

	u1* buf = vidbuffer + 16 * 2 + 288 * 2;
	u4  h   = resolutn;
	if (MMXSupport == 1)
	{
		u4 w;
		u8 a;
		u8 b;
		asm volatile(
			"0:\n\t"
			"movl  $64, %2\n\t"
			"1:\n\t"
			"movq  (%0), %3\n\t"
			"movq  %3, %4\n\t"
			"pand  %5, %3\n\t"
			"pand  %6, %4\n\t"
			"psrlw $1, %3\n\t"
			"por   %4, %3\n\t"
			"movq  %3, (%0)\n\t"
			"addl  $8, %0\n\t"
			"decl  %2\n\t"
			"jnz   1b\n\t"
			"addl  $64, %0\n\t"
			"decl  %1\n\t"
			"jnz   0b\n\t"
			"emms"
			: "+r" (buf), "+r" (h), "=&r" (w), "=&y" (a), "=&y" (b)
			: "y" (0xFFC0FFC0FFC0FFC0ULL), "y" (0x001F001F001F001FULL)
			: "cc", "memory"
		);
		(void)w;
		(void)a;
		(void)b;
	}
	else
	{
		u4* b = (u4*)buf;
		do
		{
			u4 w = 128;
			do
			{
				u4 const val = *b;
				*b++ = (val & 0xFFC0FFC0) >> 1 | val & 0x001F001F;
			}
			while (--w != 0);
			b += 16;
		}
		while (--h != 0);
	}
}
