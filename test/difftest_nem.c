/*
 * difftest_nem.c - worked example of a port-time differential test.
 *
 * Proves the register-machine transliteration of NonEchoMono (a hot DSP voice
 * mixer, cpu/dspproc.asm) is bit-identical to the original assembly. Run it
 * with `make difftest` (which extracts+assembles the asm and builds this).
 *
 * This is a transient test: once NonEchoMono's asm is deleted, remove this file
 * and its `difftest` recipe.
 */
#include "difftest.h"

typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef int16_t s2;
typedef int32_t s4;

/* --- shared globals (same names/types the asm references) --- */
u1 Voice0Volume[8];
u4 Voice0EnvInc[8];
u4 BRRPlace0[8][2];
u2 VolumeConvTable[65536];
u1 UniqueSoundv;
u4 powhack;
u1 DSPMem[256];
u4 NoiseInc;
u4 NoisePointer;
u1 NoiseData[131072];
u1 PModBuffer[8192];
u1 DSPBuffer[8192];
static u1 brrbuf[262144]; /* the voice's decoded-sample buffer (edi) */

/* --- the C port under test: the SAME source the emulator ships --- */
#include "../cpu/dsp_mixers.h"

/* --- call the standalone asm with its register ABI --- */
extern void asm_NonEchoMono(void);
static u4 call_asm(u4 ebp, u4 esi, u4 ebx, u1* edi)
{
    u4 eax = ebp; /* voice, moved into ebp below (asm uses ebp as a working reg) */
    __asm__ volatile(
        "push %%ebp; mov %%eax,%%ebp; call asm_NonEchoMono; pop %%ebp"
        : "+S"(esi), "+a"(eax)
        : "D"(edi), "b"(ebx)
        : "ecx", "edx", "cc", "memory");
    return esi;
}

#define BUFCMP 6000 /* touched region of the byte buffers, per iteration */

int main(void)
{
    u1 A_dsp[BUFCMP], C_dsp[BUFCMP], A_pm[BUFCMP], C_pm[BUFCMP];
    u4 A_brr[16], C_brr[16];

    /* fill the (large, static) lookup tables once */
    for (int i = 0; i < 65536; i++)
        VolumeConvTable[i] = (u2)(i * 131 + 7);
    dt_fill(brrbuf, sizeof brrbuf);
    dt_fill(NoiseData, sizeof NoiseData);

    DT_MAIN(2024, 300000)
    {
        u4 ebp = dt_mod(8);
        u4 esi = dt_mod(1000) * 2;
        u4 ebx = dt_u32();
        for (int i = 0; i < 8; i++) {
            Voice0Volume[i] = (u1)rand();
            Voice0EnvInc[i] = dt_u32();
            BRRPlace0[i][0] = dt_mod(256);
            BRRPlace0[i][1] = dt_mod(200);
        }
        UniqueSoundv = dt_mod(3) ? 0 : 1; /* mostly direct read, sometimes noise/pmod */
        powhack = 1u << ebp;
        DSPMem[0x3D] = (u1)rand();
        NoiseInc = dt_u32();
        u4 np0 = dt_u32();

        /* saved copies of everything the routine mutates */
        u1 dsp0[BUFCMP], pm0[BUFCMP];
        u4 brr0[16];
        dt_fill(dsp0, sizeof dsp0);
        dt_fill(pm0, sizeof pm0);
        memcpy(brr0, BRRPlace0, sizeof brr0);

        /* run the asm */
        memcpy(DSPBuffer, dsp0, sizeof dsp0);
        memcpy(PModBuffer, pm0, sizeof pm0);
        memcpy(BRRPlace0, brr0, sizeof brr0);
        NoisePointer = np0;
        u4 a_esi = call_asm(ebp, esi, ebx, brrbuf);
        memcpy(A_dsp, DSPBuffer, sizeof A_dsp);
        memcpy(A_pm, PModBuffer, sizeof A_pm);
        memcpy(A_brr, BRRPlace0, sizeof A_brr);
        u4 a_np = NoisePointer;

        /* run the C */
        memcpy(DSPBuffer, dsp0, sizeof dsp0);
        memcpy(PModBuffer, pm0, sizeof pm0);
        memcpy(BRRPlace0, brr0, sizeof brr0);
        NoisePointer = np0;
        u4 c_esi = esi, c_ebx = ebx;
        w_NonEchoMono(ebp, &c_esi, &c_ebx, (s2*)brrbuf);
        memcpy(C_dsp, DSPBuffer, sizeof C_dsp);
        memcpy(C_pm, PModBuffer, sizeof C_pm);
        memcpy(C_brr, BRRPlace0, sizeof C_brr);
        u4 c_np = NoisePointer;

        DT_EQ("esi", a_esi, c_esi);
        DT_EQ("NoisePointer", a_np, c_np);
        DT_MEM("DSPBuffer", A_dsp, C_dsp, sizeof A_dsp);
        DT_MEM("PModBuffer", A_pm, C_pm, sizeof A_pm);
        DT_MEM("BRRPlace0", A_brr, C_brr, sizeof A_brr);
    }
    DT_DONE("NonEchoMono");
}
