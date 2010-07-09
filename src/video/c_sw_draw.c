#include <string.h>

#include "../c_init.h"
#include "../link.h"
#include "c_sw_draw.h"

#ifdef __WIN32__
#	include "../cpu/regs.h"
#endif


u1* ScreenPtr;
u1* SurfBufD;


void ClearWin16(void)
{
	u1* edi = SurfBufD;
	s4  ebx = 0;
	do memset(edi, 0, SurfaceX * 2); while (edi += pitch, ++ebx != SurfaceY);
}


void ClearWin32(void)
{
	u1* edi = SurfBufD;
	s4  ebx = 0;
	do memset(edi, 0, SurfaceX * 4); while (edi += pitch, ++ebx != SurfaceY);
}


void DrawWin256x224x16(void)
{
	u1 const* esi = ScreenPtr;
	u1*       edi = SurfBufD;
	u4        eax = 0;
#ifdef __WIN32__
	u4 const  edx = resolutn;
#endif
	if (MMXSupport != 0)
	{
		do
		{
			u4 ecx = 32;
			do
			{
				asm(
					"movq  (%0), %%mm0\n\t"
					"movq 8(%0), %%mm1\n\t"
					"movq %%mm0,  (%1)\n\t"
					"movq %%mm1, 8(%1)"
					:: "r" (esi), "r" (edi) : "memory", "mm0", "mm1"
				);
				esi += 16;
				edi += 16;
			}
			while (--ecx != 0);
			edi += pitch - 512;
			esi += 64;
		}
#ifdef __WIN32__
		while (++eax != edx);
#else
		while (++eax != 223);
#endif
		asm volatile("emms");
	}
	else
	{
		do
		{
			memcpy(edi, esi, 512);
			edi += pitch;
			esi += 576;
		}
#ifdef __WIN32__
		while (++eax != edx);
#else
		while (++eax != 223);
#endif
	}
	memset(edi, 0, 512);
}
