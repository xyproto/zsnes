#include "../gui/menu.h"
#include "c_dsp.h"
#include "c_dspproc.h"


void ProcessKeyOn(u1 const al)
{
	for (u4 i = 0; i != 8; ++i)
	{
		if (al & 1U << i) VoiceStart(i);
	}
	if (al != 0) keyonsn = 1;
}
