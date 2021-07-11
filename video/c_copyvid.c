#ifdef __MSDOS__
#include "c_copyvid.h"
#include "../chips/c4proc.h"
#include "../cpu/regs.h"
#include "../dos/c_dosintrf.h"
#include "../ui.h"
#include "procvid.h"

void hextestoutput(void)
{
#if 0
	u2 ax =
		bg1ptr[0] +
		(/*(*/bg3scroly/* & 0xF8)*/ >> 3 << 6) +
		((bg3scrolx & 0xF8) >> 3 << 1);
	u2 dx = bg3scrolx;
	if (dx & 0x8000) ax += (dx & 0xF000) >> 5;
	u4 edx = ax + 0x40;
#endif
    displayfpspal();

#if 0
	memset(vram, 0, 0x800);
#endif
#if 0 // XXX DecompAPtr does not exist
	outputhex(vidbuffer + 216 * 288 +  32,      DecompAPtr   >> 8 & 0xFF);
	outputhex(vidbuffer + 216 * 288 +  32 + 16, DecompAPtr        & 0xFF);
#endif
    outputhex(vidbuffer + 216 * 288 + 70, bg1objptr[0] >> 8 & 0xFF);
    outputhex(vidbuffer + 216 * 288 + 70 + 16, bg1objptr[0] & 0xFF);
    outputhex(vidbuffer + 216 * 288 + 108, C4Ram[4]);
    outputhex(vidbuffer + 216 * 288 + 108 + 16, C4Ram[9]);
    outputhex(vidbuffer + 216 * 288 + 146, C4Ram[10]);
    outputhex(vidbuffer + 216 * 288 + 146 + 16, C4Ram[11]);
}
#endif
