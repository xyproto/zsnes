/*
 * cpu/dsp_regs.h - DSP register ($00-$FF) write handlers, ported from the
 * WDSPReg00..FF routines in cpu/dsp.asm along with the ProcessGain,
 * ProcessGain2, SwitchSustain, VoiceAdsr, VoiceAdsr2, VoiceGain and keyoffm
 * macros they were built from.
 *
 * Textual include (cpu/c_dsp.c): the includer must first provide the
 * u1/u2/u4/u8/s1/s4 typedefs and declarations for the globals used below -
 * DSPMem, VolumeTableb, VolumeConvTable, MusicVol, GlobalVL, GlobalVR, EchoVL,
 * EchoVR, EchoFB, FIRTAPVal0, EchoRate, MaxEcho, NoiseSpeeds, NoiseInc,
 * dspPAdj, KeyOnStA, KeyOnStB, Voice0Noise..Voice7Noise, Voice0State,
 * Voice0Status, Voice0Time, Voice0EnvInc, Voice0IncNumber, GainDecBendDataPos,
 * GainDecBendDataTime, GainDecBendDataDat, AdsrBlocksLeft, AdsrSustLevLoc,
 * AdsrNextTimeDepth, DecayRate, SustainRate, Increase, Decrease,
 * DecreaseRateExp - and the VoiceStarter() function.
 *
 * The asm handlers took the register number in ebx and the value in al, and
 * preserved every register (each non-trivial one push/pops what it touches), so
 * the C entry point needs no register threading at all.
 */
#ifndef DSP_REGS_H
#define DSP_REGS_H

/* The envelope level lives in byte 2 of Voice0EnvInc; the low 16 bits are the
 * fractional part that Voice0IncNumber steps. */
static inline u1 dsp_envhi(u4 voice)
{
    return ((u1*)&Voice0EnvInc[voice])[2];
}

static inline void dsp_set_envhi(u4 voice, u1 level)
{
    ((u1*)&Voice0EnvInc[voice])[2] = level;
}

/* Master/echo volume: the raw byte folded through VolumeTableb, then scaled by
 * the user's volume setting via VolumeConvTable. */
static inline u1 dsp_vol(u1 al)
{
    return (u1)VolumeConvTable[((u4)MusicVol << 8) + VolumeTableb[al]];
}

/* Slope of an exponential (bent) envelope segment: the signed distance from the
 * current level to its image on curve `row` of VolumeConvTable, spread over
 * `time` steps. Computed in wrapping 32-bit unsigned arithmetic, as the asm did.
 */
static inline u4 dsp_bend(u4 voice, u1 row, u4 time)
{
    const u1 level = dsp_envhi(voice);
    const u1 target = (u1)VolumeConvTable[(u4)row * 256 + level];
    return -((((u4)level << 16) - ((u4)target << 16)) / time);
}

/* GAIN written while the envelope restarts from the top. */
static inline void dsp_process_gain(u4 voice)
{
    const u1 gain = DSPMem[0x07 + voice * 0x10];

    if (!(gain & 0x80)) { /* direct: the byte is the level itself */
        Voice0EnvInc[voice] = 0;
        dsp_set_envhi(voice, gain & 0x7F);
        Voice0Time[voice] = 0xFFFFFFFF;
        Voice0IncNumber[voice] = 0;
        Voice0State[voice] = 4;
    } else if (gain & 0x40) {
        const u4 time = Increase[gain & 0x1F];
        Voice0EnvInc[voice] = 0;
        Voice0Time[voice] = time;
        Voice0IncNumber[voice] = 127 * 65536 / time;
        if (gain & 0x20) { /* bent increase: the last quarter is the bend */
            Voice0Time[voice] = time - (time >> 2) - 1;
            Voice0State[voice] = 6;
        } else {
            Voice0State[voice] = 3;
        }
    } else if (gain & 0x20) {
        const u4 time = DecreaseRateExp[gain & 0x1F] >> 5;
        Voice0EnvInc[voice] = 0x007FFFFF;
        Voice0Time[voice] = time;
        GainDecBendDataTime[voice] = time;
        GainDecBendDataPos[voice] = 0;
        GainDecBendDataDat[voice] = 127;
        Voice0IncNumber[voice] = -((127 - 118) * 65536 / time);
        Voice0State[voice] = 7;
    } else {
        const u4 time = Decrease[gain & 0x1F];
        Voice0EnvInc[voice] = 0x007FFFFF;
        Voice0Time[voice] = time;
        Voice0IncNumber[voice] = -(127 * 65536 / time);
        Voice0State[voice] = 5;
    }
}

/* GAIN written mid-envelope: same shapes, but starting from the level reached
 * so far instead of from full scale. */
static inline void dsp_process_gain2(u4 voice)
{
    const u1 gain = DSPMem[0x07 + voice * 0x10];

    if (!(gain & 0x80)) {
        Voice0EnvInc[voice] = 0;
        dsp_set_envhi(voice, gain & 0x7F);
        Voice0Time[voice] = 0xFFFFFFFF;
        Voice0IncNumber[voice] = 0;
        Voice0State[voice] = 4;
    } else if (gain & 0x40) {
        const u4 time = Increase[gain & 0x1F];
        u1 headroom = (u1)(dsp_envhi(voice) + 1);
        if (headroom & 0x80) headroom = 127;
        headroom ^= 127;
        Voice0Time[voice] = time;
        Voice0IncNumber[voice] = ((u4)headroom << 16) / time;
        if (gain & 0x20) {
            Voice0Time[voice] = time - (time >> 2) - 1;
            Voice0State[voice] = 6;
        } else {
            Voice0State[voice] = 3;
        }
    } else if (gain & 0x20) {
        const u4 time = DecreaseRateExp[gain & 0x1F] >> 5;
        Voice0Time[voice] = time;
        GainDecBendDataTime[voice] = time;
        GainDecBendDataPos[voice] = 0;
        Voice0IncNumber[voice] = dsp_bend(voice, 118, time);
        GainDecBendDataDat[voice] = dsp_envhi(voice);
        Voice0State[voice] = 7;
    } else {
        const u4 time = Decrease[gain & 0x1F];
        Voice0Time[voice] = time;
        Voice0IncNumber[voice] = -(((u4)dsp_envhi(voice) << 16) / time);
        Voice0State[voice] = 5;
    }
}

/* ADSR reprogrammed while a note is held: rebuild the decay/sustain segment. */
static inline void dsp_switch_sustain(u4 voice)
{
    const u1 adsr1 = DSPMem[0x05 + voice * 0x10];
    const u1 adsr2 = DSPMem[0x06 + voice * 0x10];
    const u4 decay = DecayRate[(adsr1 >> 4) & 0x07];
    const u4 sustain = SustainRate[adsr2 & 0x1F];

    GainDecBendDataDat[voice] = dsp_envhi(voice);
    if (Voice0State[voice] == 8 || Voice0State[voice] < 2)
        GainDecBendDataDat[voice] = 0x7F;

    if (decay >= sustain) { /* decay outlasts sustain: one bend, no split */
        const u1 slope = (u1)((adsr2 >> 5) ^ 0x07);
        const u4 time = (sustain + (u4)((u8)(decay - sustain) * slope / 7)) >> 5;
        Voice0EnvInc[voice] = 0x007FFFFF;
        Voice0Time[voice] = time;
        GainDecBendDataTime[voice] = time;
        GainDecBendDataPos[voice] = 0;
        Voice0IncNumber[voice] = dsp_bend(voice, 118, time);
        Voice0State[voice] = 7;
        return;
    }

    /* Traverse `level` blocks in the decay time, then the remaining 64-level
     * blocks in what is left of the sustain time. */
    {
        const u1 level = AdsrSustLevLoc[adsr2 >> 5];
        AdsrBlocksLeft[voice] = level;
        Voice0Time[voice] = level != 0 ? decay / level : decay;
        GainDecBendDataTime[voice] = Voice0Time[voice];
        AdsrNextTimeDepth[voice] = (sustain - decay) / (u1)(64 - level);
        Voice0EnvInc[voice] = 0;
        dsp_set_envhi(voice, GainDecBendDataDat[voice]);
        GainDecBendDataPos[voice] = 0;
        Voice0IncNumber[voice] = dsp_bend(voice, 122, Voice0Time[voice]);
        Voice0State[voice] = 9;
    }
}

static inline void dsp_voice_adsr(u4 voice, u1 al) /* $x5 */
{
    u1* const adsr1 = &DSPMem[0x05 + voice * 0x10];

    if (Voice0State[voice] == 200 || *adsr1 == al) {
        *adsr1 = al;
    } else if (al & 0x80) { /* ADSR mode */
        *adsr1 = al;
        dsp_switch_sustain(voice);
    } else if (Voice0Status[voice] != 1 /* GAIN mode */
        || (DSPMem[0x06 + voice * 0x10] == 0xE0 && DSPMem[0x07 + voice * 0x10] == 0xA0)) {
        *adsr1 = al;
    } else if ((*adsr1 & 0x80) && (Voice0State[voice] == 8 || Voice0State[voice] < 2)) {
        *adsr1 = al;
        dsp_process_gain(voice);
    } else {
        *adsr1 = al;
        if (Voice0State[voice] == 210) {
            VoiceStarter(voice);
            Voice0EnvInc[voice] = 0x007FFFFF;
        }
        dsp_process_gain2(voice);
    }
}

static inline void dsp_voice_adsr2(u4 voice, u1 al) /* $x6 */
{
    u1* const adsr2 = &DSPMem[0x06 + voice * 0x10];

    if (Voice0State[voice] != 200 && *adsr2 != al) {
        *adsr2 = al;
        if (DSPMem[0x05 + voice * 0x10] & 0x80) {
            dsp_switch_sustain(voice);
            return;
        }
    }
    *adsr2 = al;
}

static inline void dsp_voice_gain(u4 voice, u1 al) /* $x7 */
{
    u1* const gain = &DSPMem[0x07 + voice * 0x10];

    if (Voice0State[voice] != 200 && *gain != al) {
        *gain = al;
        if (Voice0Status[voice] == 1 && !(DSPMem[0x05 + voice * 0x10] & 0x80)) {
            if (Voice0State[voice] == 210) VoiceStarter(voice);
            dsp_process_gain2(voice);
            return;
        }
    }
    *gain = al;
}

/* KOF: run the envelope down to silence and flag the voice as ended. */
static inline void dsp_key_off(u4 voice)
{
    Voice0Time[voice] = 255;
    Voice0IncNumber[voice] = -(Voice0EnvInc[voice] >> 8);
    Voice0State[voice] = 200;
    DSPMem[0x08 + voice * 0x10] = 0;
    DSPMem[0x7C] |= 1U << voice;
}

/* Write `al` to DSP register `reg` ($00-$FF). */
static inline void dsp_write_reg(u4 reg, u1 al)
{
    switch (reg) {
    case 0x05: case 0x15: case 0x25: case 0x35: /* ADSR (1) */
    case 0x45: case 0x55: case 0x65: case 0x75:
        dsp_voice_adsr(reg >> 4, al);
        break;

    case 0x06: case 0x16: case 0x26: case 0x36: /* ADSR (2) */
    case 0x46: case 0x56: case 0x66: case 0x76:
        dsp_voice_adsr2(reg >> 4, al);
        break;

    case 0x07: case 0x17: case 0x27: case 0x37: /* GAIN */
    case 0x47: case 0x57: case 0x67: case 0x77:
        dsp_voice_gain(reg >> 4, al);
        break;

    case 0x0F: case 0x1F: case 0x2F: case 0x3F: /* echo FIR coefficients */
    case 0x4F: case 0x5F: case 0x6F: case 0x7F:
        DSPMem[reg] = al;
        FIRTAPVal0[reg >> 4] = (s1)al;
        break;

    case 0x0C: DSPMem[0x0C] = al; GlobalVL = dsp_vol(al); break;
    case 0x1C: DSPMem[0x1C] = al; GlobalVR = dsp_vol(al); break;
    case 0x2C: DSPMem[0x2C] = al; EchoVL = dsp_vol(al); break;
    case 0x3C: DSPMem[0x3C] = al; EchoVR = dsp_vol(al); break;

    case 0x0D: /* echo feedback */
        DSPMem[0x0D] = al;
        EchoFB = VolumeTableb[al];
        break;

    case 0x3D: /* noise enable */
        Voice0Noise = al >> 0 & 1;
        Voice1Noise = al >> 1 & 1;
        Voice2Noise = al >> 2 & 1;
        Voice3Noise = al >> 3 & 1;
        Voice4Noise = al >> 4 & 1;
        Voice5Noise = al >> 5 & 1;
        Voice6Noise = al >> 6 & 1;
        Voice7Noise = al >> 7 & 1;
        DSPMem[0x3D] = al;
        break;

    case 0x4C: /* KON - latched for the CPU thread, unless a KOF is pending */
        if (DSPMem[0x5C] != 0xFF) KeyOnStA |= al;
        DSPMem[0x4C] = al;
        DSPMem[0x7C] &= (u1)~al;
        break;

    case 0x5C: /* KOF */
        KeyOnStA &= (u1)~al;
        KeyOnStB &= (u1)~al;
        for (u4 voice = 0; voice != 8; voice++)
            if (al & 1U << voice) dsp_key_off(voice);
        DSPMem[0x5C] = al;
        break;

    case 0x6C: /* FLG - noise rate, and reset/mute silence every voice */
        DSPMem[0x6C] = al & 0x7F;
        if (al & 0xC0)
            for (u4 voice = 0; voice != 8; voice++)
                Voice0Status[voice] = 0;
        NoiseInc = (u4)((u8)NoiseSpeeds[al & 0x1F] * dspPAdj >> 17);
        break;

    case 0x7C: /* ENDX - any write clears it */
        DSPMem[0x7C] = 0;
        break;

    case 0x7D: /* echo delay */
        DSPMem[0x7D] = al;
        MaxEcho = EchoRate[al & 0x0F];
        break;

    default:
        DSPMem[reg] = al;
        break;
    }
}

#endif /* DSP_REGS_H */
