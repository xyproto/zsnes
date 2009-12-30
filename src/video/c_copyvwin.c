#include <string.h>

#include "../c_init.h"
#include "../cfg.h"
#include "../cpu/regs.h"
#include "../endmem.h"
#include "../gui/gui.h"
#include "../ui.h"
#include "../vcache.h"
#include "2xsaiw.h"
#include "c_copyvwin.h"
#include "copyvwin.h"
#include "makevid.h"
#include "newgfx16.h"

#ifdef __WIN32__
#include "../win/winlink.h"
#endif


void HighResProc(u2** psrc, u1** pdst, u1* ebx)
{
	u2* src = *psrc;
	u1* dst = *pdst;
	u4  ecx = 256;
	if (*ebx != 3 && *ebx != 7)
	{
		if (*ebx & 4 && scanlines == 0 && cfield & 1)
		{
			dst += NumBytesPerLine;
		}
		do
		{
			if (*ebx & 3)
			{
				if (MMXSupport != 1)
				{
					do
					{
						*(u4*)dst = src[75036 * 2] << 16 | *src;
						src += 1;
						dst += 4;
					}
					while (--ecx != 0);
					if (*ebx & 4 && scanlines == 0)
					{
						if (!(cfield & 1)) dst += NumBytesPerLine;
					}
					else
					{
						switch (scanlines)
						{
							case 1:
								break;

							case 3:
							{
								dst += AddEndBytes;
								src -= 256;
								u4 ecx = 256;
								do
								{
									u4 eax = src[75036 * 2] << 16 | *src;
									eax = (eax & HalfTrans[0]) >> 1;
									*(u4*)dst = eax;
									src += 1;
									dst += 4;
								}
								while (--ecx != 0);
								break;
							}

							case 2:
							{
								dst += AddEndBytes;
								src -= 256;
								u4 ecx = 256;
								do
								{
									u4 eax = src[75036 * 2] << 16 | *src;
									eax = (eax & HalfTrans[0]) >> 1;
									u4 edx = (eax & HalfTrans[0]) >> 1;
									eax += edx;
									*(u4*)dst = eax;
									src += 1;
									dst += 4;
								}
								while (--ecx != 0);
								break;
							}

							default:
							{
								dst += AddEndBytes;
								src -= 256;
								u4 ecx = 256;
								do
								{
									*(u4*)dst = src[75036 * 2] << 16 | *src;
									src += 1;
									dst += 4;
								}
								while (--ecx != 0);
								break;
							}
						}
					}
				}
				else
				{
					u1* eax = spritetablea + 512 * 256;
					u4  ecx = 64;
					do
					{
						asm volatile(
							"movq       (%1), %%mm0\n\t"
							"movq 300144(%1), %%mm1\n\t"
							"movq %%mm0, %%mm2\n\t"
							"punpcklwd %%mm1, %%mm0\n\t"
							"movq %%mm0,  (%2)\n\t"
							"punpckhwd %%mm1, %%mm2\n\t"
							"movq %%mm2, 8(%2)\n\t"
							"movq %%mm0,  (%0)\n\t"
							"movq %%mm2, 8(%0)\n\t"
							:: "r" (eax), "r" (src), "r" (dst) : "memory", "mm0", "mm1", "mm2"
						);
						src +=  4;
						dst += 16;
						eax += 16;
					}
					while (--ecx != 0);
					if (*ebx & 4 && scanlines == 0)
					{
						if (!(cfield & 1)) dst += NumBytesPerLine;
					}
					else
					{
						switch (scanlines)
						{
							case 1:
								break;

							case 3:
							{
								u1* eax = spritetablea + 512 * 256;
								u4  ecx = 32;
								dst += AddEndBytes;
								u8  mm4 = *(u8*)HalfTrans;
								do
								{
									asm volatile(
										"movq   (%0), %%mm0\n\t"
										"movq  8(%0), %%mm1\n\t"
										"movq 16(%0), %%mm2\n\t"
										"movq 24(%0), %%mm3\n\t"
										"pand %2, %%mm0\n\t"
										"pand %2, %%mm1\n\t"
										"pand %2, %%mm2\n\t"
										"pand %2, %%mm3\n\t"
										"psrlw $1, %%mm0\n\t"
										"psrlw $1, %%mm1\n\t"
										"psrlw $1, %%mm2\n\t"
										"psrlw $1, %%mm3\n\t"
										"movq %%mm0,   (%1)\n\t"
										"movq %%mm1,  8(%1)\n\t"
										"movq %%mm2, 16(%1)\n\t"
										"movq %%mm3, 24(%1)\n\t"
										:: "r" (eax), "r" (dst), "y" (mm4) : "memory", "mm0", "mm1", "mm2", "mm3"
									);
									eax += 32;
									dst += 32;
								}
								while (--ecx != 0);
								break;
							}

							case 2:
							{
								u1* eax = spritetablea + 512 * 256;
								u4  ecx = 64;
								dst += AddEndBytes;
								u8  mm4 = *(u8*)HalfTransC;
								do
								{
									asm volatile(
										"movq  (%0), %%mm0\n\t"
										"movq 8(%0), %%mm1\n\t"
										"pand %2, %%mm0\n\t"
										"pand %2, %%mm1\n\t"
										"psrlw $1, %%mm0\n\t"
										"psrlw $1, %%mm1\n\t"
										"movq %%mm0, %%mm2\n\t"
										"movq %%mm1, %%mm3\n\t"
										"pand %2, %%mm2\n\t"
										"pand %2, %%mm3\n\t"
										"psrlw $1, %%mm2\n\t"
										"psrlw $1, %%mm3\n\t"
										"paddd %%mm2, %%mm0\n\t"
										"paddd %%mm3, %%mm1\n\t"
										"movq %%mm0,  (%1)\n\t"
										"movq %%mm1, 8(%1)\n\t"
										:: "r" (eax), "r" (dst), "y" (mm4) : "memory", "mm0", "mm1", "mm2", "mm3"
									);
									eax += 16;
									dst += 16;
								}
								while (--ecx != 0);
								break;
							}

							default:
							{
								if (ebx[1] & 3 && (En2xSaI != 0 || antienab != 0))
								{
									dst += AddEndBytes;
									u1* eax = spritetablea + 512 * 256;
									u4  ecx = 64;
									src -= 256;
									u8  mm4 = *(u8*)HalfTrans;
									do
									{
										asm volatile(
											"movq    576(%1), %%mm0\n\t"
											"movq 300720(%1), %%mm1\n\t"
											"movq %%mm0, %%mm2\n\t"
											"punpcklwd %%mm1, %%mm0\n\t"
											"punpckhwd %%mm1, %%mm2\n\t"
											"movq  (%0), %%mm1\n\t"
											"movq 8(%0), %%mm3\n\t"
											"pand %3, %%mm0\n\t"
											"pand %3, %%mm1\n\t"
											"pand %3, %%mm2\n\t"
											"pand %3, %%mm3\n\t"
											"psrlw $1, %%mm0\n\t"
											"psrlw $1, %%mm1\n\t"
											"psrlw $1, %%mm2\n\t"
											"psrlw $1, %%mm3\n\t"
											"paddd %%mm1, %%mm0\n\t"
											"paddd %%mm3, %%mm2\n\t"
											"movq %%mm0,  (%2)\n\t"
											"movq %%mm2, 8(%2)\n\t"
											:: "r" (eax), "r" (src), "r" (dst), "y" (mm4) : "memory", "mm0", "mm1", "mm2", "mm3"
										);
										eax += 16;
										dst += 16;
										src +=  4;
									}
									while (--ecx != 0);
								}
								else
								{
									dst += AddEndBytes;
									u1* eax = spritetablea + 512 * 256;
									u4  ecx = 32;
									do
									{
										asm volatile(
											"movq   (%0), %%mm0\n\t"
											"movq %%mm0,   (%1)\n\t"
											"movq  8(%0), %%mm1\n\t"
											"movq %%mm1,  8(%1)\n\t"
											"movq 16(%0), %%mm2\n\t"
											"movq %%mm2, 16(%1)\n\t"
											"movq 24(%0), %%mm3\n\t"
											"movq %%mm3, 24(%1)\n\t"
											:: "r" (eax), "r" (dst) : "memory", "mm0", "mm1", "mm2", "mm3"
										);
										eax += 32;
										dst += 32;
									}
									while (--ecx != 0);
								}
								break;
							}
						}
					}
				}
				goto end;
			}
			else
			{
				do
				{
					*(u4*)dst = *src * 0x00010001;
					src += 1;
					dst += 4;
				}
				while (--ecx != 0);
			}
		}
		while (scanlines != 0);
		if (!(cfield & 1)) dst += NumBytesPerLine;
	}
	else if (MMXSupport != 1)
	{
		do
		{
			*(u4*)dst = *src * 0x00010001;
			src += 1;
			dst += 4;
		}
		while (--ecx != 0);
		dst += AddEndBytes;
		src -= 256;
		u4 ecx = 256;
		src += 75036 * 2;
		do
		{
			*(u4*)dst = *src * 0x00010001;
			src += 1;
			dst += 4;
		}
		while (--ecx != 0);
		src -= 75036 * 2;
	}
	else
	{
		{
			u4 ecx = 64;
			do
			{
				asm volatile(
					"movq (%0), %%mm0\n\t"
					"movq %%mm0, %%mm1\n\t"
					"punpcklwd %%mm1, %%mm0\n\t"
					"movq %%mm0,  (%1)\n\t"
					"punpckhwd %%mm1, %%mm1\n\t"
					"movq %%mm1, 8(%1)"
					:: "r" (src), "r" (dst) : "memory", "mm0", "mm1"
				);
				src +=  4;
				dst += 16;
			}
			while (--ecx != 0);
		}
		dst += AddEndBytes;
		src -= 256;
		src += 75036 * 2;
		{
			u4 ecx = 64;
			do
			{
				asm volatile(
					"movq (%0), %%mm0\n\t"
					"movq %%mm0, %%mm1\n\t"
					"punpcklwd %%mm1, %%mm0\n\t"
					"movq %%mm0,  (%1)\n\t"
					"punpckhwd %%mm1, %%mm1\n\t"
					"movq %%mm1, 8(%1)"
					:: "r" (src), "r" (dst) : "memory", "mm0", "mm1"
				);
				src +=  4;
				dst += 16;
			}
			while (--ecx != 0);
			src -= 75036 * 2;
		}
	}
end:
	*psrc = src;
	*pdst = dst;
}


static u1* SelectTile(void)
{
	return GUIOn == 1 || newengen == 0 ? hirestiledat + 1 : SpecialLine + 1;
}


static void Process2xSaIwin(u2* src, u1* dst)
{
	InterPtr = SelectTile();
#ifdef __UNIXSDL__
	u1 dl = 224;
#else
	u1 dl = resolutn;
#endif
	lineleft = dl;
	src[256] = 0;

	u1* ebx = vidbufferofsb + 288 * 2;
	for (;;)
	{
		src[256 + 288] = 0;

		*(u4*)(dst + 512 * 2 - 6) = 0;
		*(u2*)(dst + 512 * 2 - 2) = 0;
#ifdef __WIN32__
		if (GUIDSMODE[cvidmode] == 0 && GUIWFVID[cvidmode] != 0)
		{
			*(u4*)(dst + 576 * 4 - 6) = 0;
			*(u2*)(dst + 576 * 4 - 2) = 0;
		}
		else
#endif
		{
			*(u4*)(dst + 512 * 4 - 6) = 0;
			*(u2*)(dst + 512 * 4 - 2) = 0;
		}

		u1* eax = InterPtr;
		if (*eax > 1)
		{
			u2* esi_ = src;
			u1* edi_ = dst;
			HighResProc(&esi_, &edi_, InterPtr);
			src = esi_;
			dst = edi_;
			memset(ebx, 0xFF, 576);
			src += 32;
			++InterPtr;
			dst += AddEndBytes;
			ebx += 576;
			if (--lineleft == 0) break;
		}
		else
		{
			LineFilter* f;
			switch (En2xSaI)
			{
				case 2:  f = _2xSaISuperEagleLine; break;
				case 3:  f = _2xSaISuper2xSaILine; break;
				default: f = _2xSaILine;           break;
			}
			f(src /* source pointer */, ebx, 576 /* source pitch */, 256 /* width */, dst /* destination offset */, NumBytesPerLine);
			src += 288;
			dst += NumBytesPerLine * 2;
			ebx += 576;
			++InterPtr;
			if (--lineleft == 0)
			{
				dst -= NumBytesPerLine;
				memset(dst, 0, 1024);
				break;
			}
		}
	}
	asm volatile("emms"); // XXX necessary?
}


void MMXInterpolwin(u2* esi, u1* edi, u1 const dl)
{
	u1* ebx = SelectTile();
	u8  mm2 = *(u8*)HalfTransC;
	switch (scanlines)
	{
		case 1: // scanlines
		{
			lineleft = dl;
			// do scanlines
			u4 eax = *(u4*)(esi + 255); // XXX unaligned?
			u4 ecx = 64;
			*(u4*)(esi + 256) = eax;
			do
			{
				if (*ebx > 1)
				{
					u2* esi_ = esi;
					u1* edi_ = edi;
					HighResProc(&esi_, &edi_, ebx);
					esi = esi_;
					edi = edi_;
					mm2 = *(u8*)HalfTrans;
				}
				else
				{
					do
					{
						asm volatile(
							"movq  (%0), %%mm0\n\t"
							"movq %%mm0, %%mm4\n\t"
							"movq 2(%0), %%mm1\n\t"
							"pand %2, %%mm0\n\t"
							"pand %2, %%mm1\n\t"
							"psrlw $1, %%mm0\n\t"
							"psrlw $1, %%mm1\n\t"
							"paddd %%mm1, %%mm0\n\t"
							"movq %%mm4, %%mm5\n\t"
							/* mm4/mm5 contains original values, mm0 contains mixed values */
							"punpcklwd %%mm0, %%mm4\n\t"
							"punpckhwd %%mm0, %%mm5\n\t"
							"movq %%mm4,  (%1)\n\t"
							"movq %%mm5, 8(%1)"
							:: "r" (esi), "r" (edi), "y" (mm2) : "memory", "mm0", "mm1", "mm4", "mm5"
						);
						esi +=  4;
						edi += 16;
					}
					while (--ecx != 0);
				}
				esi += 32;
				edi += AddEndBytes;
				memset(edi, 0, 1024);
				edi += 1024;
				edi += AddEndBytes;
				++ebx;
				ecx = 64;
			}
			while (--lineleft != 0);
			break;
		}

		case 2: // quartscanlines
		{
			lineleft = dl;
			// do scanlines
			do
			{
				if (*ebx > 1)
				{
					u2* esi_ = esi;
					u1* edi_ = edi;
					HighResProc(&esi_, &edi_, ebx);
					esi = esi_;
					edi = edi_;
					mm2 = *(u8*)HalfTransC;
				}
				else
				{
					u4  eax = *(u4*)(esi + 255); // XXX unaligned?
					*(u4*)(esi + 256) = eax;
					u1* edx = spritetablea + 512 * 256;
					{ u4 ecx = 64;
						do
						{
							asm volatile(
								"movq  (%1), %%mm0\n\t"
								"movq %%mm0, %%mm3\n\t"
								"movq %%mm0, %%mm4\n\t"
								"movq 2(%1), %%mm1\n\t"
								"por %%mm1, %%mm3\n\t"
								"pand %3, %%mm0\n\t"
								"pand %3, %%mm1\n\t"
								"psrlw $1, %%mm0\n\t"
								"psrlw $1, %%mm1\n\t"
								"paddd %%mm1, %%mm0\n\t"
								"pand %4, %%mm3\n\t"
								"paddw %%mm3, %%mm0\n\t"
								"movq %%mm4, %%mm5\n\t"
								/* mm4/mm5 contains original values, mm0 contains mixed values */
								"punpcklwd %%mm0, %%mm4\n\t"
								"punpckhwd %%mm0, %%mm5\n\t"
								"movq %%mm4,  (%0)\n\t"
								"movq %%mm5, 8(%0)\n\t"
								"movq %%mm4,  (%2)\n\t"
								"movq %%mm5, 8(%2)"
								:: "r" (edx), "r" (esi), "r" (edi), "y" (mm2), "m" (*(u8*)HalfTransB) : "memory", "mm0", "mm1", "mm3", "mm4", "mm5"
							);
							esi +=  4;
							edi += 16;
							edx += 16;
						}
						while (--ecx != 0);
					}
					edi += AddEndBytes;
					edx -= 16 * 64;
					{ u4 ecx = 64;
						do
						{
							asm volatile(
								"movq  (%0), %%mm0\n\t"
								"movq 8(%0), %%mm1\n\t"
								"pand %2, %%mm0\n\t"
								"pand %2, %%mm1\n\t"
								"psrlw $1, %%mm0\n\t"
								"psrlw $1, %%mm1\n\t"
								"movq %%mm0, %%mm4\n\t"
								"movq %%mm1, %%mm5\n\t"
								"pand %2, %%mm4\n\t"
								"pand %2, %%mm5\n\t"
								"psrlw $1, %%mm4\n\t"
								"psrlw $1, %%mm5\n\t"
								"paddd %%mm4, %%mm0\n\t"
								"paddd %%mm5, %%mm1\n\t"
								"movq %%mm0,  (%1)\n\t"
								"movq %%mm1, 8(%1)"
								:: "r" (edx), "r" (edi), "y" (mm2) : "memory", "mm0", "mm1", "mm4", "mm5"
							);
							edi += 16;
							edx += 16;
						}
						while (--ecx != 0);
					}
				}
				esi += 32;
				edi += AddEndBytes;
				++ebx;
			}
			while (--lineleft != 0);
			break;
		}

		case 3: // halfscanlines
		{
			lineleft = dl;
			// do scanlines
			do
			{
				if (*ebx > 1)
				{
					u2* esi_ = esi;
					u1* edi_ = edi;
					HighResProc(&esi_, &edi_, ebx);
					esi = esi_;
					edi = edi_;
					mm2 = *(u8*)HalfTrans;
				}
				else
				{
					u4  eax = *(u4*)(esi + 255); // XXX unaligned?
					*(u4*)(esi + 256) = eax;
					u1* edx = spritetablea + 512 * 256;
					{ u4 ecx = 64;
						do
						{
							asm volatile(
								"movq  (%1), %%mm0\n\t"
								"movq %%mm0, %%mm4\n\t"
								"movq 2(%1), %%mm1\n\t"
								"pand %3, %%mm0\n\t"
								"pand %3, %%mm1\n\t"
								"psrlw $1, %%mm0\n\t"
								"psrlw $1, %%mm1\n\t"
								"paddd %%mm1, %%mm0\n\t"
								"movq %%mm4, %%mm5\n\t"
								/* mm4/mm5 contains original values, mm0 contains mixed values */
								"punpcklwd %%mm0, %%mm4\n\t"
								"punpckhwd %%mm0, %%mm5\n\t"
								"movq %%mm4,  (%0)\n\t"
								"movq %%mm5, 8(%0)\n\t"
								"movq %%mm4,  (%2)\n\t"
								"movq %%mm5, 8(%2)"
								:: "r" (edx), "r" (esi), "r" (edi), "y" (mm2) : "memory", "mm0", "mm1", "mm4", "mm5"
							);
							esi +=  4;
							edi += 16;
							edx += 16;
						}
						while (--ecx != 0);
					}
					edi += AddEndBytes;
					edx -= 16 * 64;
					{ u4 ecx = 64;
						do
						{
							asm volatile(
								"movq  (%0), %%mm0\n\t"
								"movq 8(%0), %%mm1\n\t"
								"pand %2, %%mm0\n\t"
								"pand %2, %%mm1\n\t"
								"psrlw $1, %%mm0\n\t"
								"psrlw $1, %%mm1\n\t"
								"movq %%mm0,  (%1)\n\t"
								"movq %%mm1, 8(%1)"
								:: "r" (edx), "r" (edi), "y" (mm2) : "memory", "mm0", "mm1"
							);
							edi += 16;
							edx += 16;
						}
						while (--ecx != 0);
					}
				}
				edi += AddEndBytes;
				esi += 32;
				++ebx;
			}
			while (--lineleft != 0);
			break;
		}

		default:
		{
			lineleft = dl;
			// do scanlines
			u4  eax = *(u4*)(esi + 255); // XXX unaligned?
			*(u4*)(esi + 256) = eax;
			u1* edx = spritetablea + 512 * 256;
			u4  ecx = 64;
			do
			{
				asm volatile(
					"movq  (%1), %%mm0\n\t"
					"movq %%mm0, %%mm3\n\t"
					"movq %%mm0, %%mm4\n\t"
					"movq 2(%1), %%mm1\n\t"
					"por %%mm1, %%mm3\n\t"
					"pand %3, %%mm0\n\t"
					"pand %3, %%mm1\n\t"
					"psrlw $1, %%mm0\n\t"
					"psrlw $1, %%mm1\n\t"
					"paddd %%mm1, %%mm0\n\t"
					"pand %4, %%mm3\n\t"
					"paddw %%mm3, %%mm0\n\t"
					"movq %%mm4, %%mm5\n\t"
					/* mm4/mm5 contains original values, mm0 contains mixed values */
					"punpcklwd %%mm0, %%mm4\n\t"
					"punpckhwd %%mm0, %%mm5\n\t"
					"movq %%mm4,  (%2)\n\t"
					"movq %%mm5, 8(%2)\n\t"
					"movq %%mm4,  (%0)\n\t"
					"movq %%mm5, 8(%0)"
					:: "r" (edx), "r" (esi), "r" (edi), "y" (mm2), "m" (*(u8*)HalfTransB) : "memory", "mm0", "mm1", "mm3", "mm4", "mm5"
				);
				esi +=  4;
				edi += 16;
				edx += 16;
			}
			while (--ecx != 0);
			esi += 32;
			edi += AddEndBytes;

			do
			{
				if (*ebx > 1)
				{
					u2* esi_ = esi;
					u1* edi_ = edi;
					HighResProc(&esi_, &edi_, ebx);
					esi = esi_;
					edi = edi_;
					mm2 = *(u8*)HalfTransC;
				}
				else
				{
					u4 eax = *(u4*)(esi + 255); // XXX unaligned?
					*(u4*)(esi + 256) = eax;
					{ u1* edx = spritetablea + 512 * 256;
						u4  ecx = 64;
						// Process next line
						do
						{
							asm volatile(
								"movq  (%1), %%mm0\n\t"
								"movq %%mm0, %%mm3\n\t"
								"movq %%mm0, %%mm4\n\t"
								"movq 2(%1), %%mm1\n\t"
								"por %%mm1, %%mm3\n\t"
								"pand %3, %%mm0\n\t"
								"pand %3, %%mm1\n\t"
								"psrlw $1, %%mm0\n\t"
								"psrlw $1, %%mm1\n\t"
								"paddd %%mm1, %%mm0\n\t"
								"pand %4, %%mm3\n\t" // HalfTransB
								"paddw %%mm3, %%mm0\n\t"
								"movq %%mm4, %%mm5\n\t"
								/* mm4/mm5 contains original values, mm0 contains mixed values */
								"movq  (%0), %%mm6\n\t"
								"movq 8(%0), %%mm7\n\t"
								"punpcklwd %%mm0, %%mm4\n\t"
								"punpckhwd %%mm0, %%mm5\n\t"
								"movq %%mm4,  (%0)\n\t"
								"movq %%mm5, 8(%0)\n\t"
								"movq %%mm6, %%mm0\n\t"
								"por %%mm4, %%mm0\n\t"
								"pand %3, %%mm4\n\t"
								"pand %3, %%mm6\n\t"
								"psrlw $1, %%mm4\n\t"
								"psrlw $1, %%mm6\n\t"
								"pand %4, %%mm0\n\t" // HalfTransB
								"paddd %%mm6, %%mm4\n\t"
								"paddw %%mm0, %%mm4\n\t"
								"movq %%mm5, %%mm0\n\t"
								"por %%mm7, %%mm0\n\t"
								"pand %3, %%mm5\n\t"
								"pand %3, %%mm7\n\t"
								"psrlw $1, %%mm5\n\t"
								"pand %4, %%mm0\n\t" // HalfTransB
								"psrlw $1, %%mm7\n\t"
								"paddd %%mm7, %%mm5\n\t"
								"paddw %%mm0, %%mm5\n\t"
								"movq %%mm4,  (%2)\n\t"
								"movq %%mm5, 8(%2)"
								:: "r" (edx), "r" (esi), "r" (edi), "y" (mm2), "m" (*(u8*)HalfTransB) : "memory", "mm0", "mm1", "mm3", "mm4", "mm5", "mm6", "mm7"
							);
							esi +=  4;
							edi += 16;
							edx += 16;
						}
						while (--ecx != 0);
					}
					edi += AddEndBytes;
					{ u1* edx = spritetablea + 512 * 256;
						u4  ecx = 64;
						do
						{
							// XXX memcpy()?
							asm volatile(
								"movq  (%0), %%mm0\n\t"
								"movq 8(%0), %%mm1\n\t"
								"movq %%mm0,  (%1)\n\t"
								"movq %%mm1, 8(%1)"
								:: "r" (edx), "r" (edi) : "memory", "mm0", "mm1"
							);
							edi += 16;
							edx += 16;
						}
						while (--ecx != 0);
					}
				}
				esi += 32;
				edi += AddEndBytes;
				++ebx;
			}
			while (--lineleft != 0);
			break;
		}
	}
	asm volatile("emms");
}


void copy640x480x16bwin(void)
{
	if (curblank == 0x40) return;

	u2 ds;
	asm volatile("movw %%ds, %0;  movw %0, %%es" : "=r" (ds)); // XXX necessary?
	(void)ds;

	u2* src = (u2*)vidbuffer + 16 + 288;
	u1* dst = WinVidMemStart;
#ifdef __UNIXSDL__
	if (GUIOn != 1 && resolutn == 239) src += 8 * 288;
#endif
#ifdef __UNIXSDL__
	u4 dl = 224;
#else
	u4 dl = resolutn;
#endif
	// Check if interpolation mode
	if (FilteredGUI != 0 || GUIOn2 != 1)
	{
		if (MMXSupport == 1 && En2xSaI != 0)
		{
			Process2xSaIwin(src, dst);
			return;
		}
		if (antienab == 1)
		{
			u4 edx = dl;
			asm volatile("call %P3" : "+d" (edx), "+S" (src), "+D" (dst) : "X" (interpolate640x480x16bwin) : "cc", "memory", "eax", "ecx", "ebx"); // asm_call
			return;
		}
	}
	switch (scanlines)
	{
		case 1: // scanlines
		{
			u1* ebx = SelectTile();
			do
			{
				{ u4 ecx = 256;
					if (*ebx < 1)
					{
						if (MMXSupport == 1)
						{
							u4 ecx = 64;
							do
							{
								asm volatile(
									"movq (%0), %%mm0\n\t"
									"movq %%mm0, %%mm1\n\t"
									"punpcklwd %%mm1, %%mm0\n\t"
									"punpckhwd %%mm1, %%mm1\n\t"
									"movq %%mm0,  (%1)\n\t"
									"movq %%mm1, 8(%1)"
									:: "r" (src), "r" (dst) : "memory", "mm0", "mm1"
								);
								src +=  4;
								dst += 16;
							}
							while (--ecx != 0);
						}
						else
						{
							do
							{
								u4 eax = *src++ * 0x00010001U;
								*(u4*)dst = eax;
								dst += 4;
							}
							while (--ecx != 0);
						}
					}
					else if (*ebx == 1)
					{
						*ebx = 0;
						if (res512switch & 1)
						{
							do
							{
								u2 ax = *src++;
								*(u2*)(dst + 2) = ax;
								dst += 4;
							}
							while (--ecx != 0);
						}
						else
						{
							do
							{
								u2 ax = *src++;
								*(u2*)dst = ax;
								dst += 4;
							}
							while (--ecx != 0);
						}
					}
					else
					{
						u2* src_ = src;
						u1* dst_ = dst;
						HighResProc(&src_, &dst_, ebx);
						src = src_;
						dst = dst_;
					}
				}
				src += 32;
				dst += AddEndBytes;
				{ u4 ecx = 256;
					do
					{
						*(u4*)dst = 0;
						dst += 4;
					}
					while (--ecx != 0);
				}
				dst += AddEndBytes;
				++ebx;
			}
			while (--dl != 0);
			res512switch ^= 1;
			break;
		}

		case 2: // quartscanlines
		{
			lineleft = dl;
			u1* ebx = SelectTile();
			do
			{
				if (*ebx <= 1)
				{
					if (MMXSupport == 1)
					{
						{ u1* eax = spritetablea + 512 * 256;
							u4  ecx = 64;
							do
							{
								asm volatile(
									"movq (%1), %%mm0\n\t"
									"movq %%mm0, %%mm1\n\t"
									"punpcklwd %%mm1, %%mm0\n\t"
									"punpckhwd %%mm1, %%mm1\n\t"
									"movq %%mm0,  (%2)\n\t"
									"movq %%mm1, 8(%2)\n\t"
									"movq %%mm0,  (%0)\n\t"
									"movq %%mm1, 8(%0)"
									:: "r" (eax), "r" (src), "r" (dst) : "memory", "mm0", "mm1"
								);
								src +=  4;
								dst += 16;
								eax += 16;
							}
							while (--ecx != 0);
						}
						{ u1*      eax   = spritetablea + 512 * 256;
							u4       ecx   = 64;
							dst += AddEndBytes;
							u8 const trans = *(u8*)HalfTrans;
							do
							{
								asm volatile(
									"movq  (%0), %%mm0\n\t"
									"movq 8(%0), %%mm1\n\t"
									"pand %2, %%mm0\n\t"
									"pand %2, %%mm1\n\t"
									"psrlw $1, %%mm0\n\t"
									"psrlw $1, %%mm1\n\t"
									"movq %%mm0, %%mm2\n\t"
									"movq %%mm1, %%mm3\n\t"
									"pand %2, %%mm2\n\t"
									"pand %2, %%mm3\n\t"
									"psrlw $1, %%mm2\n\t"
									"psrlw $1, %%mm3\n\t"
									"paddd %%mm2, %%mm0\n\t"
									"paddd %%mm3, %%mm1\n\t"
									"movq %%mm0,  (%1)\n\t"
									"movq %%mm1, 8(%1)"
									:: "r" (eax), "r" (dst), "y" (trans) : "memory", "mm0", "mm1", "mm2", "mm3"
								);
								eax += 16;
								dst += 16;
							}
							while (--ecx != 0);
						}
					}
					else
					{
						{ u4 ecx = 256;
							do
							{
								u4 eax = *src++ * 0x00010001U;
								*(u4*)dst = eax;
								dst += 4;
							}
							while (--ecx != 0);
						}
						{ u4 ecx = 256;
							src -= 256;
							dst += AddEndBytes;
							do
							{
								u4 eax = (*src++ * 0x00010001U & HalfTrans[0]) >> 1;
								u4 edx = (eax & HalfTrans[0]) >> 1;
								eax += edx;
								*(u4*)dst = eax;
								dst += 4;
							}
							while (--ecx != 0);
						}
					}
				}
				else
				{
					u2* src_ = src;
					u1* dst_ = dst;
					HighResProc(&src_, &dst_, ebx);
					src = src_;
					dst = dst_;
				}
				src += 32;
				dst += AddEndBytes;
				++ebx;
			}
			while (--lineleft != 0);
			break;
		}

		case 3: // halfscanlines
		{
			u1* ebx = SelectTile();
			do
			{
				if (*ebx <= 1)
				{
					if (MMXSupport == 1)
					{
						{ u1* eax = spritetablea + 512 * 256;
							u4  ecx = 64;
							do
							{
								asm volatile(
									"movq (%1), %%mm0\n\t"
									"movq %%mm0, %%mm1\n\t"
									"punpcklwd %%mm1, %%mm0\n\t"
									"punpckhwd %%mm1, %%mm1\n\t"
									"movq %%mm0,  (%2)\n\t"
									"movq %%mm1, 8(%2)\n\t"
									"movq %%mm0,  (%0)\n\t"
									"movq %%mm1, 8(%0)"
									:: "r" (eax), "r" (src), "r" (dst) : "memory", "mm0", "mm1"
								);
								src +=  4;
								dst += 16;
								eax += 16;
							}
							while (--ecx != 0);
						}
						{ u1*      eax   = spritetablea + 512 * 256;
							u4       ecx   = 32;
							dst += AddEndBytes;
							u8 const trans = *(u8*)HalfTrans;
							do
							{
								asm volatile(
									"movq   (%0), %%mm0\n\t"
									"movq  8(%0), %%mm1\n\t"
									"movq 16(%0), %%mm2\n\t"
									"movq 24(%0), %%mm3\n\t"
									"pand %2, %%mm0\n\t"
									"pand %2, %%mm1\n\t"
									"pand %2, %%mm2\n\t"
									"pand %2, %%mm3\n\t"
									"psrlw $1, %%mm0\n\t"
									"psrlw $1, %%mm1\n\t"
									"psrlw $1, %%mm2\n\t"
									"psrlw $1, %%mm3\n\t"
									"movq %%mm0,   (%1)\n\t"
									"movq %%mm1,  8(%1)\n\t"
									"movq %%mm2, 16(%1)\n\t"
									"movq %%mm3, 24(%1)"
									:: "r" (eax), "r" (dst), "y" (trans) : "memory", "mm0", "mm1", "mm2", "mm3"
								);
								eax += 32;
								dst += 32;
							}
							while (--ecx != 0);
						}
					}
					else
					{
						{ u4 ecx = 256;
							do
							{
								u4 eax = *src++ * 0x00010001U;
								*(u4*)dst = eax;
								dst += 4;
							}
							while (--ecx != 0);
						}
						{ u4 ecx = 256;
							src -= 256;
							dst += AddEndBytes;
							do
							{
								u4 eax = (*src++ * 0x00010001U & HalfTrans[0]) >> 1;
								*(u4*)dst = eax;
								dst += 4;
							}
							while (--ecx != 0);
						}
					}
				}
				else
				{
					u2* src_ = src;
					u1* dst_ = dst;
					HighResProc(&src_, &dst_, ebx);
					src = src_;
					dst = dst_;
				}
				src += 32;
				dst += AddEndBytes;
				++ebx;
			}
			while (--dl != 0);
			break;
		}

		default:
		{
			u1* ebx = hirestiledat + 1;
			if (newengen != 0) ebx = SpecialLine + 1;
			do
			{
				u4 ecx = 256;
				if (*ebx < 1)
				{
					if (MMXSupport == 1)
					{
						{ u1* eax = spritetablea + 512 * 256;
							u4  ecx = 64;
							do
							{
								asm volatile(
									"movq (%1), %%mm0\n\t"
									"movq %%mm0, %%mm1\n\t"
									"punpcklwd %%mm1, %%mm0\n\t"
									"movq %%mm0,  (%2)\n\t"
									"punpckhwd %%mm1, %%mm1\n\t"
									"movq %%mm1, 8(%2)\n\t"
									"movq %%mm0,  (%0)\n\t"
									"movq %%mm1, 8(%0)"
									:: "r" (eax), "r" (src), "r" (dst) : "memory", "mm0", "mm1"
								);
								src +=  4;
								dst += 16;
								eax += 16;
							}
							while (--ecx != 0);
						}
						{ u1* eax = spritetablea + 512 * 256;
							u4  ecx = 32;
							dst += AddEndBytes;
							do
							{
								asm volatile(
									"movq   (%0), %%mm0\n\t"
									"movq %%mm0,   (%1)\n\t"
									"movq  8(%0), %%mm1\n\t"
									"movq %%mm1,  8(%1)\n\t"
									"movq 16(%0), %%mm2\n\t"
									"movq %%mm2, 16(%1)\n\t"
									"movq 24(%0), %%mm3\n\t"
									"movq %%mm3, 24(%1)"
									:: "r" (eax), "r" (dst) : "memory", "mm0", "mm1", "mm2", "mm3"
								);
								eax += 32;
								dst += 32;
							}
							while (--ecx != 0);
						}
					}
					else
					{
						do
						{
							u4 eax = *src++ * 0x00010001U;
							*(u4*)dst = eax;
							dst += 4;
						}
						while (--ecx != 0);
						src -= 256;
						dst += AddEndBytes;
						u4 ecx = 256;
						do
						{
							u4 eax = *src++ * 0x00010001U;
							*(u4*)dst = eax;
							dst += 4;
						}
						while (--ecx != 0);
					}
				}
				else if (*ebx == 1)
				{
					*ebx = 0;
					if (res512switch & 1)
					{
						{ u4 ebx = NumBytesPerLine;
							do
							{
								u2 ax = *src++;
								*(u2*)(dst + 2)       = ax;
								*(u2*)(dst + 2 + ebx) = ax;
								dst += 4;
							}
							while (--ecx != 0);
						}
						dst += NumBytesPerLine;
					}
					else
					{
						{ u4 ebx = NumBytesPerLine;
							do
							{
								u2 ax = *src++;
								*(u2*)dst         = ax;
								*(u2*)(dst + ebx) = ax;
								dst += 4;
							}
							while (--ecx != 0);
						}
						dst += NumBytesPerLine;
					}
				}
				else
				{
					u2* src_ = src;
					u1* dst_ = dst;
					HighResProc(&src_, &dst_, ebx);
					src = src_;
					dst = dst_;
				}
				src += 32;
				dst += AddEndBytes;
				++ebx;
			}
			while (--dl != 0);
			res512switch ^= 1;
			break;
		}
	}

	if (MMXSupport == 1) asm volatile("emms");
}
