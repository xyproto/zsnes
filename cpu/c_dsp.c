#include "../gui/menu.h"
#include "../ui.h" /* MusicVol, VolumeConvTable */
#include "c_dsp.h"
#include "c_dspproc.h"
#include "dspproc.h"

/* The DSP register file. Kept 32-byte aligned as cpu/dsp.asm had it; the mixers
 * in cpu/dspproc.c still address it. */
_Alignas(32) u1 DSPMem[256];

/* Other state the register handlers touch that no header declares yet: scalars
 * still defined in cpu/dspproc.c and cpu/regs.inc. */
extern u1 VolumeTableb[256];
extern u1 KeyOnStA, KeyOnStB;
extern u1 Voice0Noise, Voice1Noise, Voice2Noise, Voice3Noise;
extern u1 Voice4Noise, Voice5Noise, Voice6Noise, Voice7Noise;
extern u4 NoiseSpeeds[32], NoiseInc;

#include "dsp_regs.h"

void DSPWriteReg(u4 const reg, u1 const val)
{
    dsp_write_reg(reg, val);
}

void ProcessKeyOn(u1 const al)
{
	for (u4 i = 0; i != 8; ++i)
	{
		if (al & 1U << i) VoiceStart(i);
	}
	if (al != 0) keyonsn = 1;
}
