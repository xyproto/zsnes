#include "c_procvid.h"


u4 SwapMouseButtons(u4 const buttons)
{
	return (buttons & ~0x3) | (buttons << 1 & 0x2) | (buttons >> 1 & 0x1);
}
