#include <string.h>

#include "../asm_call.h"
#include "../c_vcache.h"
#include "../cpu/regs.h"
#include "../endmem.h"
#include "../initc.h"
#include "../macros.h"
#include "../ui.h"
#include "../vcache.h"
#include "c_makev16b.h"
#include "makev16b.h"
#include "makev16t.h"
#include "makevid.h"
#include "newgfx.h"
#include "procvid.h"


static void blanker16b(void)
{
	// calculate current video offset
	memset(vidbuffer + curypos * 576 + 32, 0, 512);
}


static void setpalallgamma(void)
{
	u4 i = 0;
	do
	{
		u2 const dx = cgram[i];
		prevpal[i] = dx;

		u2 r = (dx & 0x1F) + gammalevel16b;
		if (r > 31) r = 31;
		r = r * vidbright / 15 << vesa2_rpos;

		u2 g = (dx >> 5 & 0x1F) + gammalevel16b;
		if (g > 31) g = 31;
		g = g * vidbright / 15 << vesa2_gpos;

		u2 b = (dx >> 10 & 0x1F) + gammalevel16b;
		if (b > 31) b = 31;
		b = b * vidbright / 15 << vesa2_bpos;

		u2 c = r + g + b;
		if (c == 0 && vidbright != 0) c |= 0x0020;
		pal16b[i]    = pal16b[i]    & 0xFFFF0000 | c;
		pal16bcl[i]  = pal16bcl[i]  & 0xFFFF0000 | c            & vesa2_clbit;
		pal16bxcl[i] = pal16bxcl[i] & 0xFFFF0000 | (c ^ 0xFFFF) & vesa2_clbit;
	}
	while (++i != 256);
	prevbright = vidbright;
}


static void setpalette16bgamma(void)
{
	if (vidbright != prevbright)
	{
		setpalallgamma();
		return;
	}

	if (cgmod == 0) return;
	cgmod = 0;

	u4 i = 0;
	do
	{
		u2 const dx = cgram[i];
		if (prevpal[i] == dx) continue;
		prevpal[i] = dx;

		u2 r = (dx & 0x1F) + gammalevel16b;
		if (r > 31) r = 31;
		r = r * vidbright / 15 << vesa2_rpos;

		u2 g = (dx >> 5 & 0x1F) + gammalevel16b;
		if (g > 31) g = 31;
		g = g * vidbright / 15 << vesa2_gpos;

		u2 b = (dx >> 10 & 0x1F) + gammalevel16b;
		if (b > 31) b = 31;
		b = b * vidbright / 15 << vesa2_bpos;

		u2 c = r + g + b;
		if (c == 0 && vidbright != 0) c |= 0x0020;
		pal16b[i]    = pal16b[i]    & 0xFFFF0000 | c;
		pal16bcl[i]  = pal16bcl[i]  & 0xFFFF0000 | c            & vesa2_clbit;
		pal16bxcl[i] = pal16bxcl[i] & 0xFFFF0000 | (c ^ 0xFFFF) & vesa2_clbit;
	}
	while (++i != 256);
}


// Set palette 16bit
static void setpalall(void)
{
	if (V8Mode == 1) doveg();
	u4 i = 0;
	do
	{
		u2 const dx = cgram[i];
		prevpal[i] = dx;
		u2 const r = (dx       & 0x1F) * vidbright / 15 << vesa2_rpos;
		u2 const g = (dx >>  5 & 0x1F) * vidbright / 15 << vesa2_gpos;
		u2 const b = (dx >> 10 & 0x1F) * vidbright / 15 << vesa2_bpos;
		u2       c = r + g + b;
		if (c == 0 && vidbright != 0) c |= 0x0020;
		pal16b[i]    = pal16b[i]    & 0xFFFF0000 | c;
		pal16bcl[i]  = pal16bcl[i]  & 0xFFFF0000 | c            & vesa2_clbit;
		pal16bxcl[i] = pal16bxcl[i] & 0xFFFF0000 | (c ^ 0xFFFF) & vesa2_clbit;
	}
	while (++i != 256);
	prevbright = vidbright;
	if (V8Mode == 1) dovegrest();
}


void setpalette16b(void)
{
	if (gammalevel16b != 0)
	{
		setpalette16bgamma();
		return;
	}
	if (V8Mode == 1) doveg();
	if (vidbright != prevbright)
	{
		setpalall();
		return;
	}
	if (cgmod != 0)
	{
		cgmod = 0;
		u4 i = 0;
		do
		{
			u2 const dx = cgram[i];
			if (prevpal[i] == dx) continue;
			prevpal[i] = dx;

			u2 const r = (dx       & 0x1F) * vidbright / 15 << vesa2_rpos;
			u2 const g = (dx >>  5 & 0x1F) * vidbright / 15 << vesa2_gpos;
			u2 const b = (dx >> 10 & 0x1F) * vidbright / 15 << vesa2_bpos;
			u2       c = r + g + b;
			if (c == 0 && vidbright != 0) c |= 0x0020;
			pal16b[i]    = pal16b[i]    & 0xFFFF0000 | c;
			pal16bcl[i]  = pal16bcl[i]  & 0xFFFF0000 | c            & vesa2_clbit;
			pal16bxcl[i] = pal16bxcl[i] & 0xFFFF0000 | (c ^ 0xFFFF) & vesa2_clbit;
		}
		while (++i != 256);
	}
	if (V8Mode == 1) dovegrest();
}


// Clear Backarea, 16-bit mode
void clearback16b(void)
{
	u2 c;
	if (scaddtype & 0x20 && !(scaddtype & 0x80))
	{
		u2 const dx = cgram[0];
		c = 0;

		u2 r = (dx & 0x1F) + coladdr;
		if (r > 31) r = 31;
		c += r * vidbright << vesa2_rpos;

		u2 g = (dx >> 5 & 0x1F) + coladdg;
		if (g > 31) g = 31;
		c += g * vidbright << vesa2_gpos;

		u2 b = (dx >> 10 & 0x1F) + coladdb;
		if (b > 31) b = 31;
		c += b * vidbright << vesa2_bpos;
	}
	else
	{
		c = pal16b[0];
	}
	u4  eax = c * 0x00010001;
	u1* buf = curvidoffset;
	u4  n   = 128;
	do *(u4*)buf = eax; while (buf += 4, --n != 0);
}


static void sprdrawpra16b(u4 const eax, u1 const cl, u1 const ch, u4 const ebx, u2* const edi, u4 const p1)
{
	if (eax == 0) return;
	if (sprpriodata[ebx - p1 + 16] & cl) return;
	edi[ebx - p1] = pal16b[(eax + ch) & 0xFF];
	sprpriodata[ebx - p1 + 16] |= cl;
}


static void sprdrawprb16b(u4 const eax, u1 const cl, u1 const ch, u4 const ebx, u2* const edi, u4 const p1)
{
	if (eax == 0) return;
	edi[ebx - p1] = pal16b[(eax + ch) & 0xFF];
}


static void sprdrawa16b(u1 const cl, u1 const ch, u4 const ebx, u1* const esi, u2* const edi, void (* const f)(u4 eax, u1 cl, u1 ch, u4 ebx, u2* edi, u4 p1))
{
	f(esi[0], cl, ch, ebx, edi, 8);
	f(esi[1], cl, ch, ebx, edi, 7);
	f(esi[2], cl, ch, ebx, edi, 6);
	f(esi[3], cl, ch, ebx, edi, 5);
	f(esi[4], cl, ch, ebx, edi, 4);
	f(esi[5], cl, ch, ebx, edi, 3);
	f(esi[6], cl, ch, ebx, edi, 2);
	f(esi[7], cl, ch, ebx, edi, 1);
}


static void sprdrawaf16b(u1 const cl, u1 const ch, u4 const ebx, u1* const esi, u2* const edi, void (* const f)(u4 eax, u1 cl, u1 ch, u4 ebx, u2* edi, u4 p1))
{
	f(esi[0], cl, ch, ebx, edi, 1);
	f(esi[1], cl, ch, ebx, edi, 2);
	f(esi[2], cl, ch, ebx, edi, 3);
	f(esi[3], cl, ch, ebx, edi, 4);
	f(esi[4], cl, ch, ebx, edi, 5);
	f(esi[5], cl, ch, ebx, edi, 6);
	f(esi[6], cl, ch, ebx, edi, 7);
	f(esi[7], cl, ch, ebx, edi, 8);
}


static void sprdrawpra2(u1 const dl, u4 const ebx, u4 const p1, u1 const p2)
{
	if (p2 == 0) return;
	sprpriodata[ebx - p1 + 16] |= dl;
}


static void sprdrawaf(u1 const dl, u4 const ebx, u1* const esi, void (* const f)(u1 dl, u4 ebx, u4 p1, u1 p2))
{
	f(dl, ebx, 1, esi[0]);
	f(dl, ebx, 2, esi[1]);
	f(dl, ebx, 3, esi[2]);
	f(dl, ebx, 4, esi[3]);
	f(dl, ebx, 5, esi[4]);
	f(dl, ebx, 6, esi[5]);
	f(dl, ebx, 7, esi[6]);
	f(dl, ebx, 8, esi[7]);
}


static void sprdrawa(u1 const dl, u4 const ebx, u1* const esi, void (* const f)(u1 dl, u4 ebx, u4 p1, u1 p2))
{
	f(dl, ebx, 8, esi[0]);
	f(dl, ebx, 7, esi[1]);
	f(dl, ebx, 6, esi[2]);
	f(dl, ebx, 5, esi[3]);
	f(dl, ebx, 4, esi[4]);
	f(dl, ebx, 3, esi[5]);
	f(dl, ebx, 2, esi[6]);
	f(dl, ebx, 1, esi[7]);
}


static void sprdrawpraw16b(u4 const eax, u1 const cl, u1 const ch, u4 const ebx, u2* const edi, u4 const p1)
{
	if (eax == 0) return;
	if (sprpriodata[ebx - p1 + 16] & cl) return;
	if (winspdata[ebx - p1 + 16] != 0) return;
	edi[ebx - p1] = pal16b[(eax + ch) & 0xFF];
	sprpriodata[ebx - p1 + 16] |= cl;
}


static void sprdrawprbw16b(u4 const eax, u1 const cl, u1 const ch, u4 const ebx, u2* const edi, u4 const p1)
{
	if (eax == 0) return;
	if (winspdata[ebx - p1 + 16] != 0) return;
	edi[ebx - p1] = pal16b[(eax + ch) & 0xFF];
}


static void drawspritesprio16bwinon(u1 cl, u4 const ebp)
{
	u1*       esi = currentobjptr; // XXX struct?
	u2* const edi = (u2*)curvidoffset;
	if (sprsingle == 1)
	{
		esi += (cl - 1) * 8;
		do
		{
			u2  const ebx  = *(u2*)esi;
			u1  const ch   = esi[6];
			u1* const esi_ = *(u1**)(esi + 2); // XXX unaligned?
			if (esi[7] & 0x20)
			{ // flip x
				sprdrawaf16b(cl, ch, ebx, esi_, edi, sprdrawprbw16b);
			}
			else
			{
				sprdrawa16b(cl, ch, ebx, esi_, edi, sprdrawprbw16b);
			}
		}
		while (esi -= 8, --cl != 0);
	}
	else
	{
		csprprlft = cl;
		do
		{
			u2  const ebx  = *(u2*)esi;
			u1  const ch   = esi[6];
			u4  const edx  = esi[7] & 0x03;
			u1* const esi_ = *(u1**)(esi + 2); // XXX unaligned?
			if (esi[7] & 0x20)
			{ // flip x
				if (edx == ebp)
				{
					sprdrawaf16b(csprbit, ch, ebx, esi_, edi, sprdrawpraw16b);
				}
				else
				{
					sprdrawaf(csprbit, ebx, esi_, sprdrawpra2);
				}
			}
			else
			{
				if (edx == ebp)
				{
					sprdrawa16b(csprbit, ch, ebx, esi_, edi, sprdrawpraw16b);
				}
				else
				{
					sprdrawa(csprbit, ebx, esi_, sprdrawpra2);
				}
			}
		}
		while (esi += 8, --csprprlft != 0);
		csprbit = ROL(csprbit, 1);
		if (csprbit == 1) memset(sprpriodata + 16, 0, 256);
	}
}


static void drawsprites16bprio(u1 cl, u4 const ebp)
{
	if (sprclprio[ebp] == 0) return;
	if (cwinenabm & 0x10 && winonsp != 0)
	{
		drawspritesprio16bwinon(cl, ebp);
	}
	else
	{
		u1*       esi = currentobjptr; // XXX struct?
		u2* const edi = (u2*)curvidoffset;
		if (sprsingle == 1)
		{
			esi += (cl - 1) * 8;
			do
			{
				u2  const ebx  = *(u2*)esi;
				u1  const ch   = esi[6];
				u1* const esi_ = *(u1**)(esi + 2); // XXX unaligned?
				if (esi[7] & 0x20)
				{ // flip x
					sprdrawaf16b(cl, ch, ebx, esi_, edi, sprdrawprb16b);
				}
				else
				{
					sprdrawa16b(cl, ch, ebx, esi_, edi, sprdrawprb16b);
				}
			}
			while (esi -= 8, --cl != 0);
		}
		else
		{
			csprprlft = cl;
			do
			{
				u2  const ebx  = *(u2*)esi;
				u1  const ch   = esi[6];
				u4  const edx  = esi[7] & 0x03;
				u1* const esi_ = *(u1**)(esi + 2); // XXX unaligned?
				if (esi[7] & 0x20)
				{ // flip x
					if (edx == ebp)
					{
						sprdrawaf16b(csprbit, ch, ebx, esi_, edi, sprdrawpra16b);
					}
					else
					{
						sprdrawaf(csprbit, ebx, esi_, sprdrawpra2);
					}
				}
				else
				{
					if (edx == ebp)
					{
						sprdrawa16b(csprbit, ch, ebx, esi_, edi, sprdrawpra16b);
					}
					else
					{
						sprdrawa(csprbit, ebx, esi_, sprdrawpra2);
					}
				}
			}
			while (esi += 8, --csprprlft != 0);
			csprbit = ROL(csprbit, 1);
			if (csprbit == 1) memset(sprpriodata + 16, 0, 256);
		}
	}
}


static void drawsprites16bwinon(u1 cl)
{
	u1*       esi = currentobjptr; // XXX struct?
	u2* const edi = (u2*)curvidoffset;
	do
	{
		u2  const ebx  = *(u2*)esi;
		u1  const ch   = esi[6];
		u1* const esi_ = *(u1**)(esi + 2); // XXX unaligned?
		if (esi[7] & 0x20)
		{ // flip x
			if (esi_[7] & 0x0F && winspdata[ebx - 8 - 16] == 0) edi[ebx - 8] = pal16b[(esi_[7] + ch) & 0xFF];
			if (esi_[6] & 0x0F && winspdata[ebx - 7 - 16] == 0) edi[ebx - 7] = pal16b[(esi_[6] + ch) & 0xFF];
			if (esi_[5] & 0x0F && winspdata[ebx - 6 - 16] == 0) edi[ebx - 6] = pal16b[(esi_[5] + ch) & 0xFF];
			if (esi_[4] & 0x0F && winspdata[ebx - 5 - 16] == 0) edi[ebx - 5] = pal16b[(esi_[4] + ch) & 0xFF];
			if (esi_[3] & 0x0F && winspdata[ebx - 4 - 16] == 0) edi[ebx - 4] = pal16b[(esi_[3] + ch) & 0xFF];
			if (esi_[2] & 0x0F && winspdata[ebx - 3 - 16] == 0) edi[ebx - 3] = pal16b[(esi_[2] + ch) & 0xFF];
			if (esi_[1] & 0x0F && winspdata[ebx - 2 - 16] == 0) edi[ebx - 2] = pal16b[(esi_[1] + ch) & 0xFF];
			if (esi_[0] & 0x0F && winspdata[ebx - 1 - 16] == 0) edi[ebx - 1] = pal16b[(esi_[0] + ch) & 0xFF];
		}
		else
		{
			if (esi_[0] & 0x0F && winspdata[ebx - 8 + 16] == 0) edi[ebx - 8] = pal16b[(esi_[0] + ch) & 0xFF];
			if (esi_[1] & 0x0F && winspdata[ebx - 7 + 16] == 0) edi[ebx - 7] = pal16b[(esi_[1] + ch) & 0xFF];
			if (esi_[2] & 0x0F && winspdata[ebx - 6 + 16] == 0) edi[ebx - 6] = pal16b[(esi_[2] + ch) & 0xFF];
			if (esi_[3] & 0x0F && winspdata[ebx - 5 + 16] == 0) edi[ebx - 5] = pal16b[(esi_[3] + ch) & 0xFF];
			if (esi_[4] & 0x0F && winspdata[ebx - 4 + 16] == 0) edi[ebx - 4] = pal16b[(esi_[4] + ch) & 0xFF];
			if (esi_[5] & 0x0F && winspdata[ebx - 3 + 16] == 0) edi[ebx - 3] = pal16b[(esi_[5] + ch) & 0xFF];
			if (esi_[6] & 0x0F && winspdata[ebx - 2 + 16] == 0) edi[ebx - 2] = pal16b[(esi_[6] + ch) & 0xFF];
			if (esi_[7] & 0x0F && winspdata[ebx - 1 + 16] == 0) edi[ebx - 1] = pal16b[(esi_[7] + ch) & 0xFF];
		}
	}
	while (esi += 8, --cl != 0);
	currentobjptr = esi;
}


// Processes & Draws 4-bit sprites
void drawsprites16b(u1 cl, u4 const ebp)
{
	if (sprprifix == 1)
	{
		drawsprites16bprio(cl, ebp);
	}
	else if (cwinenabm & 0x10 && winonsp != 0)
	{
		drawsprites16bwinon(cl);
	}
	else
	{
		u1*       esi = currentobjptr; // XXX struct?
		u2* const dst = (u2*)curvidoffset;
		do
		{
			u2  const ebx = *(u2*)esi & 0x00007FFF;
			u1  const ch  = esi[6];
			u1* const src = *(u1**)(esi + 2); // XXX unaligned?
			if (esi[7] & 0x20)
			{ // flip x
				if (src[7] & 0x0F) dst[ebx - 8] = pal16b[(src[7] + ch) & 0xFF];
				if (src[6] & 0x0F) dst[ebx - 7] = pal16b[(src[6] + ch) & 0xFF];
				if (src[5] & 0x0F) dst[ebx - 6] = pal16b[(src[5] + ch) & 0xFF];
				if (src[4] & 0x0F) dst[ebx - 5] = pal16b[(src[4] + ch) & 0xFF];
				if (src[3] & 0x0F) dst[ebx - 4] = pal16b[(src[3] + ch) & 0xFF];
				if (src[2] & 0x0F) dst[ebx - 3] = pal16b[(src[2] + ch) & 0xFF];
				if (src[1] & 0x0F) dst[ebx - 2] = pal16b[(src[1] + ch) & 0xFF];
				if (src[0] & 0x0F) dst[ebx - 1] = pal16b[(src[0] + ch) & 0xFF];
			}
			else
			{
				if (src[0] & 0x0F) dst[ebx - 8] = pal16b[(src[0] + ch) & 0xFF];
				if (src[1] & 0x0F) dst[ebx - 7] = pal16b[(src[1] + ch) & 0xFF];
				if (src[2] & 0x0F) dst[ebx - 6] = pal16b[(src[2] + ch) & 0xFF];
				if (src[3] & 0x0F) dst[ebx - 5] = pal16b[(src[3] + ch) & 0xFF];
				if (src[4] & 0x0F) dst[ebx - 4] = pal16b[(src[4] + ch) & 0xFF];
				if (src[5] & 0x0F) dst[ebx - 3] = pal16b[(src[5] + ch) & 0xFF];
				if (src[6] & 0x0F) dst[ebx - 2] = pal16b[(src[6] + ch) & 0xFF];
				if (src[7] & 0x0F) dst[ebx - 1] = pal16b[(src[7] + ch) & 0xFF];
			}
		}
		while (esi += 8, --cl != 0);
		currentobjptr = esi;
	}
}


void procspritesmain16b(u4 const ebp)
{
	if (scrndis  & 0x10)  return;
	if (!(scrnon & 0x10)) return;
	if (winonsp == 0xFF)  return;
	u1 const cl = cursprloc[curypos & 0x00FF];
	if (sprprifix == 0) cursprloc += 256;
	if (cl == 0) return;
	drawsprites16b(cl, ebp);
}


static void drawbackgrndmain16b(Layer const layer)
{
	if (colormodeofs[layer] == 0) return;
	if (!(scrnon     & curbgnum)) return;
	if (alreadydrawn & curbgnum)  return;
	if (scrndis      & curbgnum)  return;
	winon = 0;
	if (winenabm & curbgnum)
	{
		u1 al = winen[layer];
		u4 ecx;
		u4 edx;
		u4 ebx;
		u4 edi;
		asm volatile("push %%ebp;  mov %6, %%ebp;  call %P5;  pop %%ebp" : "+a" (al), "=c" (ecx), "=d" (edx), "=b" (ebx), "=D" (edi) : "X" (makewindow), "r" (layer) : "cc", "memory");
		if (winon == 0xFF) return;
	}
	curmosaicsz  = mosaicon & curbgnum ? mosaicsz + 1 : 1;
	bgcoloradder = bgmode == 0 ? layer * 0x20 : 0;
	u4 esi = bg1vbufloc[layer];
	u4 edi = bg1tdatloc[layer];
	u4 edx = bg1tdabloc[layer];
	u4 ebx = bg1cachloc[layer];
	u4 eax = bg1xposloc[layer];
	u4 ecx = bg1yaddval[layer];
	if (bgtilesz & curbgnum)
	{
		asm volatile("push %%ebp;  call %P6;  pop %%ebp" : "+a" (eax), "+c" (ecx), "+d" (edx), "+b" (ebx), "+S" (esi), "+D" (edi) : "X" (draw16x1616b) : "cc", "memory");
	}
	else
	{
		static u4 ebp; // XXX HACK: We are out of registers
		ebp = layer;
		asm volatile("push %%ebp;  mov %7, %%ebp;  call %P6;  pop %%ebp" : "+a" (eax), "+c" (ecx), "+d" (edx), "+b" (ebx), "+S" (esi), "+D" (edi) : "X" (draw8x816b), "m" (ebp) : "cc", "memory");
	}
	if (drawn == 33) alreadydrawn |= curbgnum;
}


static void priority216b(void)
{
	cwinenabm = winenabm;
	// do background 2
	curbgpr  = 0x00;
	curbgnum = 0x02;
	drawbackgrndmain16b(LAYER_BG2);
	procspritesmain16b(0);
	// do background 1
	curbgnum = 0x01;
	drawbackgrndmain16b(LAYER_BG1);
	procspritesmain16b(1);
	// do background 2
	curbgpr  = 0x20;
	curbgnum = 0x02;
	drawbackgrndmain16b(LAYER_BG2);
	procspritesmain16b(2);
	// do background 1
	curbgnum = 0x01;
	drawbackgrndmain16b(LAYER_BG1);
	procspritesmain16b(3);
}


void drawline16b(void)
{
	cwinenabm = winenabs;

	bg3high2 = bgmode == 1 ? bg3highst : 0;
	if (curblank != 0) return;
	if (maxbr < vidbright) maxbr = vidbright;
	if (forceblnk != 0)
	{
		blanker16b();
		return;
	}
	alreadydrawn = 0;
	colormodeofs = colormodedef[bgmode];

	if (scrnon == 0x0117 && scaddset == 0x02 && scaddtype == 0x82) scrnon = 0x0116;

	if (scaddset & 0x02 ||
			(scaddtype & 0x3F && (coladdr != 0 || coladdg != 0 || coladdb != 0 || colnull != 0)))
	{
		asm_call(drawline16t);
		return;
	}
	if (bgmode == 7)
	{
		asm_call(processmode716b);
		return;
	}
	// calculate current video offset
	curvidoffset = vidbuffer + curypos * 576 + 32;
	// do sprite windowing
	asm_call(makewindowsp);
	// set palette
	setpalette16b();
	// clear back area w/ back color
	clearback16b();
	// get current sprite table
	currentobjptr = spritetablea + (curypos & 0x00FF) * 512;
	// setup priorities
	if (sprprifix != 0)
	{
		cursprloc = sprlefttot;
		asm_call(preparesprpr);
	}
	else
	{
		cursprloc = sprleftpr;
	}
	// process backgrounds
	// do background 2
	curbgnum = 0x02;
	asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (procbackgrnd), "n" (0x01) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
	// do background 1
	curbgnum = 0x01;
	asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (procbackgrnd), "n" (0x00) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
	// do background 4
	curbgnum = 0x08;
	asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (procbackgrnd), "n" (0x03) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");
	// do background 3
	curbgnum = 0x04;
	asm volatile("push %%ebp;  mov %1, %%ebp;  call %P0;  pop %%ebp" :: "X" (procbackgrnd), "n" (0x02) : "cc", "memory", "eax", "ecx", "edx", "ebx", "esi", "edi");

	if (bgmode > 1)
	{
		priority216b();
		return;
	}
	cwinenabm = winenabm;
	curbgpr   = 0x00;
	// do background 4
	curbgnum = 0x08;
	drawbackgrndmain16b(LAYER_BG4);
	// do background 3
	curbgnum = 0x04;
	drawbackgrndmain16b(LAYER_BG3);
	procspritesmain16b(0);
	curbgpr = 0x20;
	// do background 4
	curbgnum = 0x08;
	drawbackgrndmain16b(LAYER_BG4);
	// do background 3
	if (bg3high2 != 1)
	{
		curbgnum = 0x04;
		drawbackgrndmain16b(LAYER_BG3);
	}
	procspritesmain16b(1);
	// do background 2
	curbgpr  = 0x00;
	curbgnum = 0x02;
	drawbackgrndmain16b(LAYER_BG2);
	// do background 1
	curbgnum = 0x01;
	drawbackgrndmain16b(LAYER_BG1);
	procspritesmain16b(2);
	// do background 2
	curbgpr  = 0x20;
	curbgnum = 0x02;
	drawbackgrndmain16b(LAYER_BG2);
	// do background 1
	curbgnum = 0x01;
	drawbackgrndmain16b(LAYER_BG1);
	procspritesmain16b(3);
	if (bg3high2 == 1)
	{ // do background 3
		curbgpr  = 0x20;
		curbgnum = 0x04;
		drawbackgrndmain16b(LAYER_BG3);
	}
}
