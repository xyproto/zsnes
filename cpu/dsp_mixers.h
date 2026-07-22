/*
 * cpu/dsp_mixers.h - register-machine transliterations of the DSP voice mixers.
 *
 * Single source of truth, included by both the emulator (cpu/c_dspproc.c) and
 * the differential tests (test/difftest_*.c), so the code that ships is exactly
 * the code that's proven bit-identical to the assembly.
 *
 * This is a textual include with no dependencies of its own: the includer must
 * first provide the u1/u2/u4/s2/s4 integer typedefs and declarations for the
 * globals used below - Voice0Volume, Voice0EnvInc, BRRPlace0, VolumeConvTable,
 * UniqueSoundv, powhack, DSPMem, NoiseInc, NoisePointer, NoiseData, PModBuffer,
 * DSPBuffer.
 *
 * Each w_<name> has the mixer dispatch ABI (see `mixfn` in c_dspproc.c): it
 * reads the voice, the running DSP-buffer index (*pesi), the increment (*pebx)
 * and the decoded-sample buffer (edi), advances *pesi, and - for the
 * pitch-modulation variants - updates *pebx. The functions are `static inline`
 * so an includer that only uses some of them draws no unused-function warnings;
 * taking their address (for paramhack[]) still works.
 */
#ifndef DSP_MIXERS_H
#define DSP_MIXERS_H

/* A 32-bit x86 register: byte/word sub-registers alias (al=b[0], ah=b[1],
 * ax=w, eax=e), so partial writes are just field writes. */
typedef union {
    u4 e;
    u2 w;
    u1 b[4];
} x86reg;

/* ProcessPMod ebp: a side-effect that stores one byte into PModBuffer[esi].
 * Preserves the caller's ecx/edx (taken by value). */
static inline void mix_ProcessPMod(u4 ebp, u4 esi, s2* edi, x86reg edx, x86reg ecx)
{
    x86reg eax;
    ecx.b[0] = ((u1*)&Voice0EnvInc[ebp])[2]; /* mov cl,[Voice0EnvInc+ebp*4+2] */
    eax.w = *(u2*)((u1*)edi + edx.e * 2);      /* mov ax,[edi+edx*2] */
    {                                          /* imul cx -> dx:ax = ax * cx */
        s4 p = (s2)eax.w * (s2)ecx.w;
        eax.w = (u2)p;
        edx.w = (u2)(p >> 16);
    }
    eax.w >>= 7;                          /* shr ax,7 */
    edx.b[0] = (u1)(edx.b[0] + edx.b[0]); /* add dl,dl */
    eax.b[1] |= edx.b[0];                  /* or ah,dl */
    PModBuffer[esi] = eax.b[1];            /* mov [PModBuffer+esi],ah */
}

/* Volume lookup: index VolumeConvTable by (envinc<<8 | volbyte). */
static inline u2 mix_vconv(u4 voice, u1 volbyte)
{
    x86reg idx;
    idx.e = volbyte;                          /* movzx eax,volbyte */
    idx.b[1] = ((u1*)&Voice0EnvInc[voice])[2]; /* mov ah,[Voice0EnvInc+voice*4+2] */
    return *(u2*)((u1*)VolumeConvTable + idx.e * 2);
}

/* Volume scale of one sample: (sample * volconv) >> 7, truncated to 16 bits.
 * The asm's `imul cx; shr ax,7; add dl,dl; or ah,dl; movsx eax,ax` reconstructs
 * exactly this (proven by the diff-tests). */
static inline s4 mix_scale(s2 sample, u2 volconv)
{
    s4 p = (s2)sample * (s2)volconv;
    return (s2)(p >> 7);
}

/* Fetch this voice's next sample: noise, or the decoded sample (with a
 * ProcessPMod side-effect on the way when a following voice pitch-modulates). */
static inline s2 mix_sample(u4 voice, u4 esi, s2* edi, x86reg edx, x86reg ecx)
{
    if (UniqueSoundv != 0) {
        if (DSPMem[0x3D] & (u1)powhack) { /* test DSPMem[3D],al / jnz -> noise */
            NoisePointer += NoiseInc;
            return *(s2*)(NoiseData + (NoisePointer >> 18) * 2);
        }
        mix_ProcessPMod(voice, esi, edi, edx, ecx);
    }
    return *(s2*)((u1*)edi + edx.e * 2);
}

/* CalculatePMod ebp: the pitch-modulated increment for the next sample. */
static inline u4 mix_CalculatePMod(u4 voice, u4 esi)
{
    u1 m = (u1)(PModBuffer[esi] + 0x80);                       /* movzx + add al,80h */
    unsigned long long p = (unsigned long long)m * Voice0Freq[voice]; /* mul ebx */
    return (u4)(p >> 7);                                       /* shr eax,7 | shl edx,25 */
}

/* Common prologue: load the sample position and the first channel's volconv,
 * then fetch the sample. */
#define MIX_FETCH(voice, esi, edi, volbyte, sample, ecx)                     \
    u4 esi = *pesi;                                                          \
    x86reg edx;                                                             \
    edx.e = *(u4*)((u1*)BRRPlace0 + (voice) * 8 + 3);                        \
    x86reg ecx;                                                             \
    ecx.e = 0;                                                              \
    ecx.w = mix_vconv((voice), (volbyte));                                   \
    s2 sample = mix_sample((voice), esi, edi, edx, ecx)

/* Non-pitch-mod tail: advance esi and step BRRPlace0 by the frequency. */
#define MIX_TAIL(voice, esi)                                                 \
    esi += 2;                                                               \
    *(u4*)((u1*)BRRPlace0 + (voice) * 8) += *pebx;                           \
    *pesi = esi

/* Pitch-mod tail: advance esi, recompute the increment, step BRRPlace0. */
#define MIX_TAIL_PM(voice, esi)                                              \
    esi += 2;                                                               \
    {                                                                       \
        u4 nb = mix_CalculatePMod((voice), esi);                            \
        *(u4*)((u1*)BRRPlace0 + (voice) * 8) += nb;                         \
        *pebx = nb;                                                         \
    }                                                                       \
    *pesi = esi

static inline void w_NonEchoMono(u4 voice, u4* const pesi, u4* const pebx, s2* edi)
{
    MIX_FETCH(voice, esi, edi, Voice0Volume[voice], s, ecx);
    *(s4*)((u1*)DSPBuffer + esi * 2) += mix_scale(s, ecx.w);
    MIX_TAIL(voice, esi);
}

static inline void w_NonEchoStereo(u4 voice, u4* const pesi, u4* const pebx, s2* edi)
{
    MIX_FETCH(voice, esi, edi, Voice0VolumeR[voice], s, ecx);
    *(s4*)((u1*)DSPBuffer + esi * 4) += mix_scale(s, ecx.w);
    *(s4*)((u1*)DSPBuffer + esi * 4 + 4) += mix_scale(s, mix_vconv(voice, Voice0VolumeL[voice]));
    MIX_TAIL(voice, esi);
}

static inline void w_EchoMono(u4 voice, u4* const pesi, u4* const pebx, s2* edi)
{
    MIX_FETCH(voice, esi, edi, Voice0Volume[voice], s, ecx);
    *(s4*)((u1*)DSPBuffer + esi * 2) += mix_scale(s, ecx.w);
    *(s4*)((u1*)EchoBuffer + esi * 2) += mix_scale(s, mix_vconv(voice, Voice0Volumee[voice]));
    MIX_TAIL(voice, esi);
}

static inline void w_EchoStereo(u4 voice, u4* const pesi, u4* const pebx, s2* edi)
{
    MIX_FETCH(voice, esi, edi, Voice0VolumeR[voice], s, ecx);
    *(s4*)((u1*)DSPBuffer + esi * 4) += mix_scale(s, ecx.w);
    *(s4*)((u1*)EchoBuffer + esi * 4) += mix_scale(s, mix_vconv(voice, Voice0VolumeRe[voice]));
    *(s4*)((u1*)DSPBuffer + esi * 4 + 4) += mix_scale(s, mix_vconv(voice, Voice0VolumeL[voice]));
    *(s4*)((u1*)EchoBuffer + esi * 4 + 4) += mix_scale(s, mix_vconv(voice, Voice0VolumeLe[voice]));
    esi += 2;
    /* This variant reused ebx to hold the sample, so it reloads Voice0Freq for
     * the increment (mov ebx,[Voice0Freq+ebp*4]); *pebx is left at that value. */
    *pebx = Voice0Freq[voice];
    *(u4*)((u1*)BRRPlace0 + voice * 8) += *pebx;
    *pesi = esi;
}

static inline void w_NonEchoMonoPM(u4 voice, u4* const pesi, u4* const pebx, s2* edi)
{
    MIX_FETCH(voice, esi, edi, Voice0Volume[voice], s, ecx);
    *(s4*)((u1*)DSPBuffer + esi * 2) += mix_scale(s, ecx.w);
    MIX_TAIL_PM(voice, esi);
}

static inline void w_NonEchoStereoPM(u4 voice, u4* const pesi, u4* const pebx, s2* edi)
{
    MIX_FETCH(voice, esi, edi, Voice0VolumeR[voice], s, ecx);
    *(s4*)((u1*)DSPBuffer + esi * 4) += mix_scale(s, ecx.w);
    *(s4*)((u1*)DSPBuffer + esi * 4 + 4) += mix_scale(s, mix_vconv(voice, Voice0VolumeL[voice]));
    MIX_TAIL_PM(voice, esi);
}

static inline void w_EchoMonoPM(u4 voice, u4* const pesi, u4* const pebx, s2* edi)
{
    MIX_FETCH(voice, esi, edi, Voice0Volume[voice], s, ecx);
    *(s4*)((u1*)DSPBuffer + esi * 2) += mix_scale(s, ecx.w);
    *(s4*)((u1*)EchoBuffer + esi * 2) += mix_scale(s, mix_vconv(voice, Voice0Volumee[voice]));
    MIX_TAIL_PM(voice, esi);
}

static inline void w_EchoStereoPM(u4 voice, u4* const pesi, u4* const pebx, s2* edi)
{
    MIX_FETCH(voice, esi, edi, Voice0VolumeR[voice], s, ecx);
    *(s4*)((u1*)DSPBuffer + esi * 4) += mix_scale(s, ecx.w);
    *(s4*)((u1*)EchoBuffer + esi * 4) += mix_scale(s, mix_vconv(voice, Voice0VolumeRe[voice]));
    *(s4*)((u1*)DSPBuffer + esi * 4 + 4) += mix_scale(s, mix_vconv(voice, Voice0VolumeL[voice]));
    *(s4*)((u1*)EchoBuffer + esi * 4 + 4) += mix_scale(s, mix_vconv(voice, Voice0VolumeLe[voice]));
    MIX_TAIL_PM(voice, esi);
}

#endif /* DSP_MIXERS_H */
