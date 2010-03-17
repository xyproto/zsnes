#include "../cpu/regs.h"
#include "../endmem.h"
#include "../initc.h"
#include "c_makevid.h"
#include "makevid.h"


static void makedualwin(u1 const al, Layer const ebp)
{
	u1 const cl  = winlogica >> (u1)(ebp * 2) & 0x03;
	u4 const ebx = winl1;
	winon = 1;

	if (cl == dualwinsp && al == pwinspenab && ebx == pwinsptype)
	{ // data matches previous sprite data
		cwinptr = winspdata + 16;
		winon   = winonstype;
		return;
	}
	else if (cl == dualwinbg && al == pwinbgenab && ebx == pwinbgtype)
	{ // data matches previous data
		cwinptr = winbgdata + 16;
		winon   = winonbtype;
		return;
	}
	else
	{
		dualwinbg   = cl;
		pwinbgenab  = al;
		pwinbgtype  = ebx;
		dwinptrproc = winbgdata + 16;
		cwinptr     = winbgdata + 16;
		winon       = 1;
		winonbtype  = 1;
		u1 al_ = al;
		u1 cl_ = cl;
		asm volatile("call %P2" : "+a" (al_), "+c" (cl_) : "X" (dualstartprocess) : "cc", "memory", "edx", "edi");
	}
}


void makewindow(u1 al, Layer const ebp)
{
	// upon entry, al = win enable bits
	if (disableeffects == 1) return;

	switch (al & 0x0A)
	{
		case 0x00: return;
		case 0x0A: makedualwin(al, ebp); return;
	}

	winon = 1;
	u4 const ebx = winl1;

	if (al == pwinspenab && ebx == pwinsptype)
	{ // data matches previous sprite data
		cwinptr = winspdata + 16;
		winon   = winonstype;
	}
	else if (al == pwinbgenab && ebx == pwinbgtype)
	{ // data matches previous data
		cwinptr = winbgdata + 16;
		winon   = winonbtype;
	}
	else
	{
		pwinbgenab = al;
		pwinbgtype = ebx;

		u1 dl;
		u1 dh;
		if (al & 0x02)
		{
			dl = winl1;
			dh = winl1 >> 8;
		}
		else
		{
			dl   = winl1 >> 16;
			dh   = winl1 >> 24;
			al >>= 2;
		}

		if (al & 0x01)
		{ // outside
			if (dl >= dh)
			{
				winon      = 0xFF;
				winonbtype = 0xFF;
				cwinptr    = winbgdata + 16;
				return;
			}
			if (dl <= 1 && dh >= 254) goto clipped;
			u1* const edi = winbgdata + 16;
			u1        eax = 0;
			// start drawing 1's from 0 to left
			do edi[eax] = 1; while (++eax != dl);
			do edi[eax] = 0; while (++eax != dh);
			edi[eax] = 0;
			// start drawing 1's from right to 255
			while (++eax != 0) edi[eax] = 1;
		}
		else
		{
			if (dl == 254 || dl >= dh) goto clipped;
			u1* const edi = winbgdata + 16;
			u1        eax = 0;
			// start drawing 1's from 0 to left
			while (eax != dl) edi[eax++] = 0;
			do edi[eax] = 1; while (++eax != dh);
			edi[eax] = 1;
			if (eax != 255)
			{ // start drawing 1's from right to 255
				do edi[eax] = 0; while (++eax != 0);
			}
		}
		winon      = 1;
		winonbtype = 1;
		cwinptr    = winbgdata + 16;
		return;

clipped:
		winon      = 0;
		winonbtype = 0;
		return;
	}
}
