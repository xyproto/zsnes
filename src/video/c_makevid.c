#include "../cpu/regs.h"
#include "../endmem.h"
#include "c_makevid.h"
#include "makevid.h"


void makedualwin(u1 const al, Layer const ebp)
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
