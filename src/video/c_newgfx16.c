#include "../cpu/regs.h"
#include "../ui.h"
#include "c_newgfx16.h"
#include "makev16b.h"
#include "newgfx16.h"
#include "procvid.h"


void setpalallng(void)
{
	palchanged = 1;
	cgmod      = 0;
	cpalptrng  = (cpalptrng + 1024) & (255 * 1024);
	u2* pal = (u2*)(vbufdptr + cpalptrng);
	u4  i   = 0;
	colleft16b = 0;
	do
	{
		u2 const dx = cgram[i];
		prevpal2[i] = dx;

		u2 c = 0;

		u4 r = (dx & 0x1F) + gammalevel16b;
		if (r > 31) r = 31;
		c += r * vidbright / 15 << ngrposng;

		u4 g = (dx >> 5 & 0x1F) + gammalevel16b;
		if (g > 31) g = 31;
		c += g * vidbright / 15 << nggposng;

		u4 b = (dx >> 10 & 0x1F) + gammalevel16b;
		if (b > 31) b = 31;
		c += b * vidbright / 15 << ngbposng;

		pal[  0] = c;                // standard
		pal[256] = c | UnusedBit[0]; // standard
	}
	while (++pal, ++i, ++colleft16b != 0);
	prevbright = vidbright;
	if (V8Mode == 1) dovegrest();
}
